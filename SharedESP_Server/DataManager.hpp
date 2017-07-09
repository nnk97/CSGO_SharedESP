#pragma once

#include "Main.hpp"

#include <map>

#include "PlayerData.hpp"
#include "GameServer.hpp"

class GameServer;

namespace Data
{
	class DataManager
	{
	private:
		const std::chrono::milliseconds death_time_limit = std::chrono::milliseconds(25000);

		std::map<uint32_t, std::shared_ptr<GameServer>> m_Servers;
		std::shared_ptr<GameServer> FindOrCreateServer(uint32_t ServerHash);

	public:
		// Functions to access internal containers
		void PushData(uint32_t ServerHash, int i, PlayerData data);
		const PlayerData PopData(uint32_t ServerHash, int i);
		
		void LookForDeadServers();
	};
	extern std::unique_ptr<DataManager> Manager;

	extern void Initialize();
}
