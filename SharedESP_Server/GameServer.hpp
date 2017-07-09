#pragma once

#include "Main.hpp"

namespace Data
{
	class GameServer
	{
	public:
		GameServer(uint32_t _Hash);

	private:
		uint32_t m_ServerHash;
		std::mutex m_Mutex;
		PlayerData m_Data[65];

	public:
		std::chrono::time_point<std::chrono::steady_clock> last_write;

		void ResetData();
		PlayerData GetData(int i);
		void SetData(int i, PlayerData data);
		uint32_t GetHaskKey() { return m_ServerHash; };
	};
}
