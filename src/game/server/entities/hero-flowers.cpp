/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include "hero-flowers.h"

CHeroFlowers::CHeroFlowers(CGameWorld *pGameWorld, int ClientID, vec2 pos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_HERO_FLOWERS)
{
	m_OwnerID = ClientID;
	m_Pos = pos;
	m_OwnerPos = pos;
	m_swirlX = 0;
	m_swirlY = 0;
	m_ID = Server()->SnapNewID();
	GameWorld()->InsertEntity(this);
}

CHeroFlowers::~CHeroFlowers()
{
	Server()->SnapFreeID(m_ID);
	GameWorld()->DestroyEntity(this);
}

void CHeroFlowers::Tick()
{
	if(m_MarkedForDestroy) return;

	m_swirlX+= 0.05;
	m_swirlY+= 0.05;
	
	for(CCharacter *p = (CCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); p; p = (CCharacter *)p->TypeNext())
	{
		if(p->GetPlayer()->GetCID() == m_OwnerID)
		{
			m_OwnerPos = p->m_Pos;
			
			// delayed follower
			vec2 dir = p->m_Pos - m_Pos;
			float dist = 0.1f * length(dir);
			m_Pos += normalize(dir) * clamp(dist, 1.0f, 80.0f);

			// swirl around player in circles
			if(dist < 8.0)
				m_Pos += vec2(cos(m_swirlX), 0.5 * sin(m_swirlY) - 1) * 4.0f;
		}

		if(p->GetPlayer()->GetClass() == PLAYERCLASS_MEDIC)
		{
			float dist = distance(p->m_Pos, m_OwnerPos);

			// The hero may give his/her medic of choice a bouquet of flowers
			// Through this display of romance the medic may revive another zombie
			if(dist < 50.0)
			{
				p->GiveWeapon(WEAPON_RIFLE, 1);
				p->SetEmote(EMOTE_HAPPY, Server()->Tick() + Server()->TickSpeed());
				GameServer()->SendEmoticon(p->GetPlayer()->GetCID(), EMOTICON_HEARTS);
				GameServer()->SendScoreSound(p->GetPlayer()->GetCID());
				GameServer()->SendChatTarget_Localization(p->GetPlayer()->GetCID(), CHATCATEGORY_DEFAULT, _("You have received a medkit! You can revive another zombie."));

				for(CCharacter *pp = (CCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); pp; pp = (CCharacter *)pp->TypeNext())
				{
					if(pp->GetPlayer()->GetCID() == m_OwnerID)
					{
						pp->SetEmote(EMOTE_HAPPY, Server()->Tick() + Server()->TickSpeed());
						GameServer()->SendEmoticon(m_OwnerID, EMOTICON_HEARTS);
					}
				}
				
				Destroy();
			}
		}
	}
}

void CHeroFlowers::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Pickup *pObj = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_ID, sizeof(CNetObj_Pickup)));
	if(!pObj)
		return;

	vec2 PosStart = m_Pos;

	pObj->m_X = (int)PosStart.x;
	pObj->m_Y = (int)PosStart.y;
	pObj->m_Type = POWERUP_HEALTH;
	pObj->m_Subtype = 0;

}

