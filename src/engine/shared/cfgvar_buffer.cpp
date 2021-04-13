/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
/* Modifications Copyright 2019 The InfclassR (https://github.com/yavl/teeworlds-infclassR/) Authors */

#include <base/system.h>
#include <engine/console.h>
#include "cfgvar_buffer.h"
#include "config.h"
#include "console.h"
#include "string.h"

int CCfgVarBuffer::m_CfgVarsNum;
CCfgVarBuffer::CfgVar *CCfgVarBuffer::m_pCfgVars;

bool CCfgVarBuffer::m_BackupRoundCfgVars;
int CCfgVarBuffer::m_ResetNextRoundCounter;
CCfgVarBuffer::CfgVarBackup *CCfgVarBuffer::m_pCfgVarRoundBackup;

CCfgVarBuffer::CfgVarBackup::CfgVarBackup()
{
	m_pCfgVarsTemp = new CfgVarTemp[m_CfgVarsNum];
	for (int i = 0; i < m_CfgVarsNum; i++)
	{
		m_pCfgVarsTemp[i].active = false;
		m_pCfgVarsTemp[i].m_pStrValue = NULL;
	}
}

CCfgVarBuffer::CfgVarBackup::~CfgVarBackup()
{
	for (int i = 0; i < m_CfgVarsNum; i++)
	{
		delete[] m_pCfgVarsTemp[i].m_pStrValue;
	}
	delete[] m_pCfgVarsTemp;
}

void CCfgVarBuffer::CfgVarBackup::Add(const char* pCfgVarScriptName, bool OverrideOld)
{
	int i = 0;
	for ( ; i < m_CfgVarsNum; i++)
		if (strcmp(m_pCfgVars[i].m_pScriptName, pCfgVarScriptName) == 0) break;
	if (i >= m_CfgVarsNum)
	{
		dbg_msg("CfgVarBuffer", "Error: Could not find config variable '%s'", pCfgVarScriptName);
		return;
	}
	if (!OverrideOld && m_pCfgVarsTemp[i].active) // var is already backed up
		return;
	if (OverrideOld && !m_pCfgVarsTemp[i].active) // nothing to override here
		return;
	m_pCfgVarsTemp[i].active = true;
	// copy data to backup buffer
	if (m_pCfgVars[i].m_Type == CFG_TYPE_INT)
	{
		m_pCfgVarsTemp[i].m_IntValue = *m_pCfgVars[i].m_pIntValue;
	}
	else if (m_pCfgVars[i].m_Type == CFG_TYPE_STR)
	{
		if (m_pCfgVarsTemp[i].m_pStrValue)
		{
			delete[] m_pCfgVarsTemp[i].m_pStrValue;
			m_pCfgVarsTemp[i].m_pStrValue = NULL;
		}
		if (!m_pCfgVars[i].m_pStrValue)
			return;
		int StrSize = strlen(m_pCfgVars[i].m_pStrValue) + 1;
		m_pCfgVarsTemp[i].m_pStrValue = new char[StrSize];
		str_copy(m_pCfgVarsTemp[i].m_pStrValue, m_pCfgVars[i].m_pStrValue, StrSize);
	}
	else 
		dbg_msg("CfgVarBuffer", "Error: config variable '%s' has an unknown type", pCfgVarScriptName);
}

void CCfgVarBuffer::CfgVarBackup::ConsolePrint(CConsole *pConsole, const char *pCfgName)
{
	char aBuff[256];
	if (!pCfgName || pCfgName[0] == 0)
		str_format(aBuff, 256, "Printing all cfg vars changed for this round :");
	else
		str_format(aBuff, 256, "Printing all cfg vars changed for this round containing '%s' :", pCfgName);
	pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Console", "- - - - - - - - - - - - - - - - - - - - - - - -");
	pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Console", aBuff);

	for (int i = 0; i < m_CfgVarsNum; i++)
	{
		if (!m_pCfgVarsTemp[i].active) continue;
		if (pCfgName && pCfgName[0] != 0 && !strstr(m_pCfgVars[i].m_pScriptName, pCfgName)) continue; // check if pCfgName is a substring of m_pScriptName
		char lineBuff[512];
		if (m_pCfgVars[i].m_Type == CFG_TYPE_INT)
			str_format(lineBuff, 512, "%s %i -> %i", m_pCfgVars[i].m_pScriptName, m_pCfgVarsTemp[i].m_IntValue, *m_pCfgVars[i].m_pIntValue);
		else
		{
			if (strstr(m_pCfgVars[i].m_pScriptName, "password")) // dont print passwords
				str_format(lineBuff, 512, "%s *****************", m_pCfgVars[i].m_pScriptName);
			else if (m_pCfgVarsTemp[i].m_pStrValue && m_pCfgVars[i].m_pStrValue)
				str_format(lineBuff, 512, "%s %s -> %s", m_pCfgVars[i].m_pScriptName, m_pCfgVarsTemp[i].m_pStrValue, m_pCfgVars[i].m_pStrValue);
			else if (m_pCfgVarsTemp[i].m_pStrValue && !m_pCfgVars[i].m_pStrValue)
				str_format(lineBuff, 512, "%s %s -> NULL", m_pCfgVars[i].m_pScriptName, m_pCfgVarsTemp[i].m_pStrValue);
			else if (!m_pCfgVarsTemp[i].m_pStrValue && m_pCfgVars[i].m_pStrValue)
				str_format(lineBuff, 512, "%s NULL -> %s", m_pCfgVars[i].m_pScriptName, m_pCfgVars[i].m_pStrValue);
			else if (!m_pCfgVarsTemp[i].m_pStrValue && !m_pCfgVars[i].m_pStrValue)
				str_format(lineBuff, 512, "%s NULL -> NULL", m_pCfgVars[i].m_pScriptName);
		}
		pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Console", lineBuff);
	}
}

void CCfgVarBuffer::CfgVarBackup::Apply()
{
	for (int i = 0; i < m_CfgVarsNum; i++)
	{
		if (!m_pCfgVarsTemp[i].active) continue;
		if (m_pCfgVars[i].m_Type == CFG_TYPE_INT)
			*m_pCfgVars[i].m_pIntValue = m_pCfgVarsTemp[i].m_IntValue;
		else 
		{
			if (!m_pCfgVarsTemp[i].m_pStrValue)
				dbg_msg("CfgVarBuffer", "Error: cannot apply cfg backup for config variable '%s' -> NULL pointer string", m_pCfgVars[i].m_pScriptName);
			else
				str_copy(m_pCfgVars[i].m_pStrValue, m_pCfgVarsTemp[i].m_pStrValue, strlen(m_pCfgVarsTemp[i].m_pStrValue)+1);
		}
	}
}

void CCfgVarBuffer::Init()
{
	m_BackupRoundCfgVars = false;
	m_ResetNextRoundCounter = 0;
	delete m_pCfgVarRoundBackup;
	m_pCfgVarRoundBackup = NULL;

	// Count how many config variables there are
	m_CfgVarsNum = 0;
	#define MACRO_CONFIG_INT(Name,ScriptName,Def,Min,Max,Flags,Desc) \
	{ \
		m_CfgVarsNum++; \
	}

	#define MACRO_CONFIG_STR(Name,ScriptName,Len,Def,Flags,Desc) \
	{ \
		m_CfgVarsNum++; \
	}

	#include "config_variables.h"
	#include "game/variables.h"

	#undef MACRO_CONFIG_INT
	#undef MACRO_CONFIG_STR

	m_pCfgVars = new CfgVar[m_CfgVarsNum];
	int tCount = 0;

	// read all config variables and save their information to m_pCfgVars
	#define MACRO_CFGVAR_SAVE_NAME(ScriptName) \
	{ \
		int i = strlen(#ScriptName) + 1; \
		m_pCfgVars[tCount].m_pScriptName = new char[i]; \
		m_pCfgVars[tCount].m_ScriptNameLength = i; \
		str_copy(m_pCfgVars[tCount].m_pScriptName, #ScriptName, i); \
		m_pCfgVars[tCount].m_pScriptName[i-1] = 0; \
	}

	#define MACRO_CONFIG_INT(Name,ScriptName,Def,Min,Max,Flags,Desc) \
	{ \
		m_pCfgVars[tCount].m_Type = CFG_TYPE_INT; \
		m_pCfgVars[tCount].m_pIntValue = &g_Config.m_##Name; \
		MACRO_CFGVAR_SAVE_NAME(ScriptName); \
		tCount++; \
	} 

	#define MACRO_CONFIG_STR(Name,ScriptName,Len,Def,Flags,Desc) \
	{ \
		m_pCfgVars[tCount].m_Type = CFG_TYPE_STR; \
		m_pCfgVars[tCount].m_pStrValue = g_Config.m_##Name; \
		MACRO_CFGVAR_SAVE_NAME(ScriptName); \
		tCount++; \
	}

	#include "config_variables.h"
	#include "game/variables.h"

	#undef MACRO_CONFIG_INT
	#undef MACRO_CONFIG_STR
	#undef MACRO_CFGVAR_SAVE_NAME
}

void CCfgVarBuffer::RegisterConsoleCommands(CConsole *pConsole)
{
	pConsole->Register("set_cfg_for_nextround_begin", "", CFGFLAG_SERVER, ConSetCfgForNextRound_Begin, pConsole, 
		"cfg var changes between this begin command and the end command will be reset at the end of next round");
	pConsole->Register("set_cfg_for_nextround_end", "", CFGFLAG_SERVER, ConSetCfgForNextRound_End, pConsole, 
		"cfg var changes between the begin command and this end command will be reset at the end of next round");
	pConsole->Register("print_round_cfg", "?s", CFGFLAG_SERVER, ConPrintRoundCfg, pConsole, 
		"show round specifc config vars, format: config_var original_value -> temp_value");
}

bool CCfgVarBuffer::IsConfigVar(const char* pStr)
{
	if (!pStr) return false;
	for (int i = 0; i < m_CfgVarsNum; i++)
	{
		if (str_comp_nocase(m_pCfgVars[i].m_pScriptName, pStr) == 0) return true;
	}
	return false;
}

void CCfgVarBuffer::ConPrintCfg(CConsole* pConsole, const char *pCfgName)
{
	char aBuff[256];
	if (!pCfgName || pCfgName[0] == 0)
		str_format(aBuff, 256, "Printing all config variables :");
	else
		str_format(aBuff, 256, "Printing all config variables containing '%s' :", pCfgName);
	pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Console", "- - - - - - - - - - - - - - - - - - - - - - - -");
	pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Console", aBuff);

	// search for config vars that contain pCfgName and print them and their values - if pCfgName is NULL print all vars
	for (int i = 0; i < m_CfgVarsNum; i++)
	{
		if (pCfgName && pCfgName[0] != 0)
			if (!strstr(m_pCfgVars[i].m_pScriptName, pCfgName)) continue;

		char lineBuff[512];
		if (m_pCfgVars[i].m_Type == CFG_TYPE_INT)
			str_format(lineBuff, 512, "%s %i", m_pCfgVars[i].m_pScriptName, *m_pCfgVars[i].m_pIntValue);
		else
		{
			if (strstr(m_pCfgVars[i].m_pScriptName, "password"))  // dont print passwords
				str_format(lineBuff, 512, "%s *****************", m_pCfgVars[i].m_pScriptName);
			else
			{
				if (m_pCfgVars[i].m_pStrValue)		
					str_format(lineBuff, 512, "%s %s", m_pCfgVars[i].m_pScriptName, m_pCfgVars[i].m_pStrValue);
				else
					str_format(lineBuff, 512, "%s NULL", m_pCfgVars[i].m_pScriptName);
			}
		}
		pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Console", lineBuff);
	}
}

bool CCfgVarBuffer::ConSetCfgForNextRound_Begin(IConsole::IResult *pResult, void *pUserData)
{
	//((CConsole*)pUserData)->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Console", "Reset upcomming config variables at the end of next round");
	m_BackupRoundCfgVars = true;
	m_ResetNextRoundCounter = 2;
	if (m_pCfgVarRoundBackup)
	{
		m_pCfgVarRoundBackup->Apply();
		delete m_pCfgVarRoundBackup;
	}
	m_pCfgVarRoundBackup = new CfgVarBackup();
	return true;
}

bool CCfgVarBuffer::ConSetCfgForNextRound_End(IConsole::IResult *pResult, void *pUserData)
{
	m_BackupRoundCfgVars = false;
	return true;
}

bool CCfgVarBuffer::ConPrintRoundCfg(IConsole::IResult *pResult, void *pUserData)
{
	if (!m_pCfgVarRoundBackup)
	{
		((CConsole*)pUserData)->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Console", "Round cfg vars are disabled, you can activate them with resetcfg_nextround_start");
		return true;
	}
	m_pCfgVarRoundBackup->ConsolePrint((CConsole*)pUserData, pResult->GetString(0));
	return true;
}

void CCfgVarBuffer::BeforeSetCfg(const char* pCfgVarScriptName)
{
	if (!m_BackupRoundCfgVars) return;
	if (!m_pCfgVarRoundBackup) return;
	if (!IsConfigVar(pCfgVarScriptName)) return;
	m_pCfgVarRoundBackup->Add(pCfgVarScriptName);
}

void CCfgVarBuffer::AfterSetCfg(const char* pCfgVarScriptName)
{
	if (m_BackupRoundCfgVars) return;
	if (!m_pCfgVarRoundBackup) return;
	if (!IsConfigVar(pCfgVarScriptName)) return;
	m_pCfgVarRoundBackup->Add(pCfgVarScriptName, true);
}

void CCfgVarBuffer::OnRoundStart()
{
	if (m_ResetNextRoundCounter <= 0) return;
	m_ResetNextRoundCounter--;
	if (m_ResetNextRoundCounter > 0) return;

	m_pCfgVarRoundBackup->Apply();

	// reset
	delete m_pCfgVarRoundBackup;
	m_pCfgVarRoundBackup = NULL;
	m_BackupRoundCfgVars = false;
}

/*
// puts all config vars and their values inside a string
// returns false if it runs out of memory
bool CCfgVarBuffer::GetCfgStr(char *pStr, int StrSize)
{
	for (int i = 0; i < m_CfgVarsNum; i++)
	{
		char lineBuff[512];
		if (m_pCfgVars[i].m_Type == CFG_TYPE_INT)
			str_format(lineBuff, 512, "%s %i \n", m_pCfgVars[i].m_pScriptName, *m_pCfgVars[i].m_pIntValue);
		else
			str_format(lineBuff, 512, "%s %s \n", m_pCfgVars[i].m_pScriptName, m_pCfgVars[i].m_pStrValue);
		int m = 0;
		for ( ; m < 512; m++)
			if (lineBuff[m] == 0) break;
		if (StrSize-m <= 0)
		{
			*pStr = 0;
			dbg_msg("GetCfgStr", "Error: out of memory");
			return false;
		}
		for (int u = 0; u < m; u++)
			pStr[u] = lineBuff[u];
		StrSize -= m;
		pStr += m;
	}
	*pStr = 0;
	return true;
}
*/

