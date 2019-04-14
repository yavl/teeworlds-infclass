/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_SAFE_ZONE_CIRCLE_H
#define GAME_SERVER_ENTITIES_SAFE_ZONE_CIRCLE_H

#include <game/server/entity.h>
#include <base/tl/array.h>

class CSafeZoneCircle : public CEntity
{
	//CPredictionCircle as nested class
	class CPredictionCircle
	{
		//has implicit access on enclosing class variables
		//are called by outer class functions
		
		public:
			void Snap(int SnappingClient);
			//void Reset();
			
		protected:
			CPredictionCircle(vec2 Pos, CSafeZoneCircle *enclosing);
			~CPredictionCircle();
			
		private:
			
			enum
			{
				NUM_SIDE = 256,
				NUM_HINT = 256,
				NUM_IDS = NUM_SIDE + NUM_HINT,
			};
			
			//Inner radius
			float m_PredictionCurRadius;
			int m_IDs[NUM_IDS];
			CSafeZoneCircle *m_SafeZoneCircle; //instance of outer class
	}
	
	
	
	public:
		CSafeZoneCircle(CGameWorld *pGameWorld, vec2 Pos);
		~CSafeZoneCircle();

		virtual void Snap(int SnappingClient);
		virtual void Tick();
		virtual void Reset();
		void Shrink(); 	// shrink radius
		float GetCurrentRadius(return m_SafeZoneCurRadius);
		bool IsMinimal();

	private:
		
		enum
		{
			NUM_SIDE = 128,
			NUM_HINT = 128,
			NUM_IDS = NUM_SIDE + NUM_HINT,
		};
		
		int m_IDs[NUM_IDS];
		float m_SafeZoneCurRadius;
		float m_SafeZoneMinRadius;
		float m_SafeZoneMaxRadius;
		CPredictionCircle* m_PredictionCircle;
};

#endif
