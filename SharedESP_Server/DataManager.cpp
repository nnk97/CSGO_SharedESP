#include "DataManager.hpp"

namespace Data
{
	std::unique_ptr<DataManager> Manager = nullptr;

	void Initialize()
	{
		Manager = std::make_unique<Data::DataManager>();
	}

	void DataManager::LookForDeadServers()
	{
		if (m_Servers.size() == 0)
			return;

		std::map<uint32_t, std::shared_ptr<GameServer>>::iterator itterator;
		for (itterator = m_Servers.begin(); itterator != m_Servers.end(); itterator++)
		{
 			auto pServer = itterator->second;

			auto ms_diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - pServer->last_usage);
			if (ms_diff > death_time_limit)
			{
				std::cout << "Killing inactive server: " << std::hex << pServer->GetHaskKey() << std::endl;
				m_Servers.erase(pServer->GetHaskKey());
				break;
			}
		}
	}

	std::shared_ptr<GameServer> DataManager::FindOrCreateServer(uint32_t ServerHash)
	{
		auto itterator = m_Servers.find(ServerHash);
		if (itterator == m_Servers.end())
		{
			// Create new server instance
			std::shared_ptr<GameServer> Server = std::make_shared<GameServer>(ServerHash);
			m_Servers.insert(std::make_pair(ServerHash, Server));
			std::cout << "Creating server: " << std::hex << ServerHash << std::endl;
			return Server;
		}
		else
			return itterator->second;
	}

	void DataManager::PushData(uint32_t ServerHash, int i, PlayerData data)
	{
		std::shared_ptr<GameServer> pServer = FindOrCreateServer(ServerHash);
		pServer->SetData(i, data);
	}

	const PlayerData DataManager::PopData(uint32_t ServerHash, int i)
	{
		std::shared_ptr<GameServer> pServer = FindOrCreateServer(ServerHash);
		return pServer->GetData(i);
	}
}
