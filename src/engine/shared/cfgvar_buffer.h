/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
/* Modifications Copyright 2019 The InfclassR (https://github.com/yavl/teeworlds-infclassR/) Authors */

#ifndef ENGINE_SHARED_CFGVAR_BUFFER_H
#define ENGINE_SHARED_CFGVAR_BUFFER_H

#include <base/system.h>
#include <engine/console.h>
#include "console.h"

class CCfgVarBuffer
{

	enum
	{
		CFG_TYPE_INT = 0,
		CFG_TYPE_STR = 1
	};

	struct CfgVar
	{
		int m_Type; // CFG_TYPE_INT or CFG_TYPE_STR
		char *m_pScriptName; // for example "sv_rcon_password"
		int m_ScriptNameLength;
		int *m_pIntValue; // contains pointer to get/set and int config var
		char *m_pStrValue; // contains pointer to get/set a string config var
	};

	struct CfgVarTemp
	{
		bool active;
		int m_IntValue; 
		char *m_pStrValue; 
	};

	// use delete on objects from this class if you dont need them anymore
	class CfgVarBackup
	{
	public:
		CfgVarBackup();
		~CfgVarBackup();
		void Add(const char* pCfgVarScriptName, bool OverrideOld = false);
		void ConsolePrint(CConsole *pConsole, const char *pCfgName = NULL);
		void Apply();

	private:
		CfgVarTemp *m_pCfgVarsTemp;
	};

public:
	static void Init();
	static void RegisterConsoleCommands(CConsole *pConsole);
	static bool IsConfigVar(const char* pStr);

	static void ConPrintCfg(CConsole* pConsole, const char *pCfgName);
	static bool ConSetCfgForNextRound_Begin(IConsole::IResult *pResult, void *pUserData);
	static bool ConSetCfgForNextRound_End(IConsole::IResult *pResult, void *pUserData);
	static bool ConPrintRoundCfg(IConsole::IResult *pResult, void *pUserData);

	static void BeforeSetCfg(const char* pCfgVarScriptName);
	static void AfterSetCfg(const char* pCfgVarScriptName);
	static void OnRoundStart();

private:
	static CfgVar *m_pCfgVars; // array that contains the name and pointers of all config vars once
	static int m_CfgVarsNum; // how many different config vars there are - how many elements inside m_CfgVars

	static bool m_BackupRoundCfgVars;
	static int m_ResetNextRoundCounter;
	static CfgVarBackup *m_pCfgVarRoundBackup; // saves config vars in the same order as m_pCfgVars to restore them at the end of a round

};


#endif
