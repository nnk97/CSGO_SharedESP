#pragma once

#include "Main.hpp"
#include "Vector.h"
#include "PacketDefinitions.hpp"

using boost::asio::ip::udp;

class Vector;

namespace SyncData
{
	class CDataManager
	{
	public:
		CDataManager();

		bool m_bExit = false;
		std::thread m_Thread;

	private:
		std::mutex m_Mutex;


		// asio
		boost::array<char, 2048> m_recv_buffer;
		boost::asio::io_service m_io_service;
		udp::endpoint m_server_endpoint;
		udp::socket* m_socket;

		void ResetData();
		void InitConnection();
		void MainLoop();
		void SendData();
		void QueryData();

	public:
		struct PlayerData
		{
		public:
			bool m_ShouldQuery;
			bool m_ShouldSend;
			float m_SimulationTime;
			Vector m_Position;
			bool m_Crouching;
			float m_RecvTime;
			
			UpdateEntityPacket_t ToPacket(int i)
			{
				UpdateEntityPacket_t temp;
				temp.m_Index = i;
				temp.m_SimulationTime = m_SimulationTime;
				for (int i = 0; i < 3; i++)
					temp.m_Position[i] = m_Position[i];
				temp.m_Crouching = m_Crouching;
				return temp;
			}

			QueryEntityPacket_t ToQuery(int i)
			{
				QueryEntityPacket_t temp;
				temp.m_Index = i;
				temp.m_SimulationTime = m_SimulationTime;
				return temp;
			}
		};
		PlayerData m_Data[65];

		void MarkAsInvalidEntity(int i);
		void SetSendingStatus(int i, bool bShouldQuery);
		void SetQueryStatus(int i, bool bShouldQuery);
		void GetLastRecord(int i, PlayerData& pData);
		void SetLastRecord(int i, bool bCrouching, float flSimTime, Vector vecPosition);
		//void SetLastRecvTime(int i, float time);
	};
	
	extern std::unique_ptr<CDataManager> g_DataManager;
	extern const char* g_ServerIP;
}
