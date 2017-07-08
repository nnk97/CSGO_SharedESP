#pragma once

#include "Main.hpp"

namespace Data
{
	struct PlayerData
	{
		float m_Simulation;
		float m_Position[3];
		bool m_Crouch;

		PlayerData()
		{
			m_Simulation = 0.f;
			for (int i = 0; i < 3; i++)
				m_Position[i] = 0.f;
			m_Crouch = false;
		}

		PlayerData(float SimTime, float Pos[3], bool Crouch)
		{
			m_Simulation = SimTime;
			m_Crouch = Crouch;
			std::memcpy(m_Position, Pos, sizeof(float) * 3);
		}
	};
}
