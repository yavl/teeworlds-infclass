/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>
#include "safe-zone-circle.h"

CSafeZoneCircle::CSafeZoneCircle(CGameWorld *pGameWorld, vec2 Pos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_SAFE_ZONE_CIRCLE)
{
	// create instance of nested class
	m_PredictionCircle = new CSafeZoneCircle::CPredictionCircle(vec2 Pos, this);
	
	m_Pos = Pos; //passed by MapInfo via GameServer()->m_pController
	GameWorld()->InsertEntity(this);
	
	//config variables
	m_SafeZoneMaxRadius = g_Config.m_InfSafeZoneMinRadius; // 500 via map config variable
	m_SafeZoneMinRadius = g_Config.m_InfSafeZoneReductionRate; // 6.0f; // how fast the hole growths when it is created
	m_SafeZoneCurRadius = g_Config.m_InfSafeZoneMaxRadius; // on creation use maximal radius
	
	for(int i=0; i<NUM_IDS; i++)
	{
		m_IDs[i] = Server()->SnapNewID();
	}
}

CSafeZoneCircle::~CSafeZoneCircle()
{
	// call destructor from nested class
	if(m_PredictionCircle)
		m_PredictionCircle->~CPredictionCircle();
	
	for(int i=0; i<NUM_IDS; i++)
	{
		Server()->SnapFreeID(m_IDs[i]);
	}
}

void CSafeZoneCircle::Reset()
{
	// Call nested class
	//m_PredictionCircle->Reset()
	
	GameServer()->m_World.DestroyEntity(this);
}

bool CSafeZoneCircle::IsMinimal()
{
	return (m_SafeZoneCurRadius == m_InfSafeZoneMinRadius);
}

void CSafeZoneCircle::Tick()
{	
	if(m_MarkedForDestroy) return;
	
	if (m_SafeZoneCurRadius != g_Config.m_InfSafeZoneMinRadius)
	{
		Shrink();
	}
	else
	{
		// Now do something in the GameServer()
		GameServer()->m_SafeZoneIsMinimal = true; 
		Reset();
	}
	
	// Find other players
	for(CCharacter *p = (CCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); p; p = (CCharacter *)p->TypeNext())
	{
		if(p->IsZombie()) continue;
		
		// Calculate distance
		float Distance = distance(p->m_Pos, m_Pos);
		
		
		if(Distance > p->m_ProximityRadius + m_SafeZoneCurRadius)
		{
			//Deal every 100f 1 additional dmg point or do minimal damage
			DamageToDeal = min(m_InfSafeZoneMinDmg,(Distance-m_SafeZoneCurRadius)/100.0f) );
			p->TakeDamage(NULL, DamageToDeal, NULL, WEAPON_HAMMER, TAKEDAMAGEMODE_INFECTION);
		}
	}
}

void CSafeZoneCircle::Shrink()
{
	//shrink safe zone circle
	m_SafeZoneCurRadius -= m_InfSafeZoneReductionRate;
	if (m_SafeZoneCurRadius < g_Config.m_InfSafeZoneMinRadius)
		m_SafeZoneCurRadius = g_Config.m_InfSafeZoneMinRadius;
	

	//shrink prediciton circle
	if (m_PredictionCircle)
	{
		m_PredictionCircle->m_PredictionCurRadius -= m_InfSafeZoneReductionRate;
		if (m_SafeZoneCurRadius == g_Config.m_InfSafeZoneMinRadius)
			m_PredictionCircle->~CPredictionCircle();
	}
	
	
}

void CSafeZoneCircle::Snap(int SnappingClient)
{
	// Call nested class
	if (m_PredictionCircle)
	{
		m_PredictionCircle->Snap(SnappingClient);
	}
	
	if(NetworkClipped(SnappingClient))
		return;

	if(g_Config.m_InfInvisibleCircle4Zombies)
	{
		CPlayer* pClient = GameServer()->m_apPlayers[SnappingClient];
		if(pClient->IsInfected()) // invisible for zombies
			return;
	}

	float AngleStart = AngleStart*2.0f;
	float AngleStep = 2.0f * pi / CSafeZoneCircle::NUM_SIDE;
	float R = m_SafeZoneCurRadius;
	for(int i=0; i<CSafeZoneCircle::NUM_SIDE; i++)
	{
		vec2 PosStart = m_Pos + vec2(R * cos(AngleStart + AngleStep*i), R * sin(AngleStart + AngleStep*i));
			
		CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_IDs[i], sizeof(CNetObj_Pickup)));
		if(!pP)
			return;

		pP->m_X = (int)PosStart.x;
		pP->m_Y = (int)PosStart.y;
		pP->m_Type = POWERUP_HEALTH;
		pP->m_Subtype = 0;
	}
}


// begin nested class declaration


// constructor
CSafeZoneCircle::CPredictionCircle::CPredictionCircle(vec2 Pos,CSafeZoneCircle *enclosing)
{
	for(int i=0; i < CSafeZoneCircle::CPredictionCircle::NUM_IDS; i++)
	{
		CSafeZoneCircle::CPredictionCircle::m_IDs[i] = Server()->SnapNewID();
	}

	m_SafeZoneCircle = enclosing;
}

// destructor
CSafeZoneCircle::CPredictionCircle::~CPredictionCircle()
{
	for(int i=0; i<CSafeZoneCircle::CPredictionCircle::NUM_IDS; i++)
	{
		Server()->SnapFreeID(CSafeZoneCircle::CPredictionCircle::m_IDs[i]);
	}
}

// Is no entity

// void CSafeZoneCircle::CPredictionCircle::Reset()
// {
// 	GameServer()->m_World.DestroyEntity(this);
// }
// 

void CSafeZoneCircle::CPredictionCircle::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	if(g_Config.m_InfInvisibleCircle4Zombies)
	{
		CPlayer* pClient = GameServer()->m_apPlayers[SnappingClient];
		if(pClient->IsInfected()) // invisible for zombies
			return;
	}

	float AngleStart = 2.0f * pi * 1;
	float AngleStep = 2.0f * pi / CSafeZoneCircle::CPredictionCircle::NUM_SIDE;
	float R = m_PredictionCurRadius;

	for(int i=0; i<CSafeZoneCircle::CPredictionCircle::NUM_SIDE; i++)
	{
		vec2 PosStart = m_Pos + vec2(R * cos(AngleStart + AngleStep*i), R * sin(AngleStart + AngleStep*i));
		
		CNetObj_Projectile *pObj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_IDs[CSafeZoneCircle::CPredictionCircle::NUM_SIDE+i], sizeof(CNetObj_Projectile)));
		if(pObj)
		{
			pObj->m_X = (int)PosStart.x;
			pObj->m_Y = (int)PosStart.y;
			pObj->m_VelX = 0;
			pObj->m_VelY = 0;
			pObj->m_StartTick = Server()->Tick();
			pObj->m_Type = WEAPON_HAMMER;
		}
	}
}
