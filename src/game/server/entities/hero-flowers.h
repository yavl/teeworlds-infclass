/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <game/server/entity.h>

class CHeroFlowers : public CEntity
{
private:
	int m_OwnerID;
	int m_ID;

	vec2 m_OwnerPos;

	float m_swirlX;
	float m_swirlY;

public:
	CHeroFlowers(CGameWorld *pGameWorld, int ClientID, vec2 pos);
	~CHeroFlowers();

	virtual void Tick();
	virtual void Snap(int SnappingClient);	
};
