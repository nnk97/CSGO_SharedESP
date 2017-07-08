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
		auto cur_time = std::chrono::system_clock::now();
		return;

		for (int i = 0; i < m_Servers.size(); i++)
		{
			auto& pServer = m_Servers[i];
			
			auto diff = cur_time - pServer->last_write;
			auto ms_diff = std::chrono::duration_cast<std::chrono::milliseconds>(diff);

			if (ms_diff > std::chrono::milliseconds(5000))
			{
				std::cout << "Killing server: " << std::hex << pServer->GetHaskKey() << std::endl;
				m_Servers.erase(pServer->GetHaskKey());
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
