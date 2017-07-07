#include "SyncData.hpp"

namespace SyncData
{
	std::unique_ptr<CDataManager> g_DataManager;
	const char* g_ServerIP = "127.0.0.1";

	CDataManager::CDataManager()
	{
		ResetData();
		m_Thread = std::thread([this] { UpdateThread(); });
	}

	void CDataManager::UpdateThread()
	{
		while (true)
		{

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	void CDataManager::ResetData()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		for (int i = 0; i <= 64; i++)
		{
			PlayerData* pData = &m_Data[i];
			pData->m_SimulationTime = 0.f;
			for (int j = 0; j < 3; j++)
				pData->m_Position[j] = 0.f;
		}
	}

	void CDataManager::SetLastRecord(int i, float flSimTime, float flPosition[3])
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
	
		PlayerData* pData = &m_Data[i];
		
		// We don't want older data than current one...
		if (pData->m_SimulationTime >= flSimTime)
			return;

		// Save the data
		pData->m_SimulationTime = flSimTime;
		for (int j = 0; j < 3; j++)
			pData->m_Position[j] = flPosition[j];
	}

	void CDataManager::GetLastRecord(int i, PlayerData& pData)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		
		std::memcpy(&pData, &m_Data[i], sizeof(PlayerData));
	}
}
