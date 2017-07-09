#include "GameServer.hpp"

namespace Data
{
	GameServer::GameServer(uint32_t _Hash)
	{
		m_ServerHash = _Hash;
		last_usage = std::chrono::steady_clock::now();
	}

	PlayerData GameServer::GetData(int i)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		last_usage = std::chrono::steady_clock::now();
		return m_Data[i];
	}

	void GameServer::SetData(int i, PlayerData data)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		last_usage = std::chrono::steady_clock::now();
		m_Data[i] = data;
	}
}
