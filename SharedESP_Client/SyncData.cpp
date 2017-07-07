#include "SyncData.hpp"

namespace SyncData
{
	std::unique_ptr<CDataManager> g_DataManager;
	const char* g_ServerIP = "127.0.0.1";

	void UpdateThread(CDataManager* pDataManager)
	{

	}

	CDataManager::CDataManager()
	{
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)UpdateThread, (LPVOID)this, 0, 0);
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
		for (int j = 0; j < 3; i++)
			pData->m_Position[j] = flPosition[j];
	}

	void CDataManager::GetLastRecord(int i, PlayerData& pData)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		
		std::memcpy(&pData, &m_Data[i], sizeof(PlayerData));
	}
}
