/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_MOD_H
#define GAME_SERVER_GAMEMODES_MOD_H
#include <game/server/gamecontroller.h>
#include <game/server/gameworld.h>
#include <game/server/classes.h>
#include <game/server/entities/hero-flag.h>

// you can subclass GAMECONTROLLER_CTF, GAMECONTROLLER_TDM etc if you want
// todo a modification with their base as well.
class CGameControllerMOD : public IGameController
{
public:
	CGameControllerMOD(class CGameContext *pGameServer);
	virtual ~CGameControllerMOD();
	virtual void Tick();
	virtual void Snap(int SnappingClient);
	// add more virtual functions here if you wish
	
	virtual bool OnEntity(const char* pName, vec2 Pivot, vec2 P0, vec2 P1, vec2 P2, vec2 P3, int PosEnv);
	virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	virtual void OnCharacterSpawn(class CCharacter *pChr);
	virtual void OnPlayerInfoChange(class CPlayer *pP);
	virtual void OnClientDrop(int ClientID, int Type);
	bool PreSpawn(CPlayer* pPlayer, vec2 *pPos);
	bool PickupAllowed(int Index);
	int ChooseHumanClass(CPlayer* pPlayer);
	int ChooseInfectedClass(CPlayer* pPlayer);
	bool IsEnabledClass(int PlayerClass);
	bool IsChoosableClass(int PlayerClass);
	bool CanVote();
	bool IsInfectionStarted();
	int GetFirstInfNb();
	
private:
	void ResetFinalExplosion();
	bool IsSpawnable(vec2 Pos, int TeleZoneIndex);
	void SetFirstInfectedNumber();
	void DoInitialInfection();
	void DoWinCheck();
	void DoFinalExplosion();
	void EndRound();
	
private:	
	int m_MapWidth;
	int m_MapHeight;
	int* m_GrowingMap;
	bool m_ExplosionStarted;
	bool m_ExplosionActive;
	bool m_InfectionInstant;
	bool m_InitialInfTriggered;
	int m_NbFirstInfected;
};
#endif
