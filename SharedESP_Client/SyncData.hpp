#pragma once

#include "Main.hpp"

namespace SyncData
{
	struct PlayerData 
	{
		float m_SimulationTime;
		float m_Position[3];
	};

	class CDataManager
	{
	public:
		CDataManager();

	private:
		std::mutex m_Mutex;
		PlayerData m_Data[65];
		std::thread m_Thread;
		void ResetData();
		void UpdateThread();
		bool m_bConnected = false;

	public:
		bool IsConnected() { return m_bConnected; }
		void GetLastRecord(int i, PlayerData& pData);

	private:
		void SetLastRecord(int i, float flSimTime, float flPosition[3]);
	};
	
	extern std::unique_ptr<CDataManager> g_DataManager;
	extern const char* g_ServerIP;
}
