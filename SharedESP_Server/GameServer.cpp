#include "GameServer.hpp"

namespace Data
{
	GameServer::GameServer(uint32_t _Hash)
	{
		m_ServerHash = _Hash;
		last_write = std::chrono::system_clock::now();
		ResetData();
	}

	void GameServer::ResetData()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		last_write = std::chrono::system_clock::now();

	}

	PlayerData GameServer::GetData(int i)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		return m_Data[i];
	}

	void GameServer::SetData(int i, PlayerData data)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		last_write = std::chrono::system_clock::now();
		m_Data[i] = data;
	}
}
