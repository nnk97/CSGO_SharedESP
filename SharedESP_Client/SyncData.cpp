#include "SyncData.hpp"

using boost::asio::ip::udp;

namespace SyncData
{
	std::unique_ptr<CDataManager> g_DataManager;
	const char* g_ServerIP = "127.0.0.1"; // <- IP address where you host the server

	CDataManager::CDataManager()
	{
		ResetData();
		m_Thread = std::thread([this] { InitConnection(); });
	}

	void CDataManager::InitConnection()
	{
		try
		{
			// Try to resolve & connect to the server
			udp::resolver resolver(m_io_service);
			udp::resolver::query query(udp::v4(), g_ServerIP, "21370");	// <- Set desired port here
			m_server_endpoint = *resolver.resolve(query);

			m_socket = new udp::socket(m_io_service);
			m_socket->open(udp::v4());

			if (!socket)
				std::runtime_error("Failed to open socket!");

			MainLoop();
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			CSGO::g_pCVar->DbgPrint("  >>>  Exception (%s): %s\n", __func__, e.what());
		}
	}

	uint32_t GetServerCRC32()
	{
		const char* str = CSGO::g_pServerIP->GetPtr();
		if (!str)
			return 0;

		std::string hash_str = std::string(str);
		hash_str.append(CSGO::g_pEngine->GetLevelNameShort());

		boost::crc_32_type result;
		result.process_bytes(hash_str.c_str(), hash_str.length());
		return (uint32_t)result.checksum();
	}

	void CDataManager::SendData()
	{
		try
		{

			uint32_t iObjectCount = 0;
			for (int i = 1; i < 64; i++)
			{
				PlayerData pd;
				GetLastRecord(i, pd);
				if (pd.m_ShouldSend)
					iObjectCount++;
			}

			uint32_t ServerHash = GetServerCRC32();
			if (!iObjectCount || !ServerHash)
				return;

			std::size_t DesiredPacketSize = sizeof(PacketHeader_t) + sizeof(UpdateEntityPacket_t) * iObjectCount;
			std::size_t BufferPos = 0;
			char* SendBuffer = new char[DesiredPacketSize];

			PacketHeader_t PH;
			PH.m_Type = PacketType::Update;
			PH.m_ServerHash = ServerHash;
			PH.m_SizeParam = iObjectCount;

			// Copy header to buffer
			std::memcpy(SendBuffer, &PH, sizeof(PacketHeader_t));
			BufferPos += sizeof(PacketHeader_t);

			for (int i = 1; i < 64; i++)
			{
				PlayerData pd;
				GetLastRecord(i, pd);
				if (pd.m_ShouldSend)
				{
					std::memcpy((SendBuffer + BufferPos), &pd.ToPacket(i), sizeof(UpdateEntityPacket_t));
					BufferPos += sizeof(UpdateEntityPacket_t);
					SetSendingStatus(i, false);
				}
			}

			m_socket->send_to(boost::asio::buffer(SendBuffer, DesiredPacketSize), m_server_endpoint);
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			CSGO::g_pCVar->DbgPrint("  >>>  Exception (%s): %s\n", __func__, e.what());
		}
	}

	void CDataManager::QueryData()
	{
		try
		{
			uint32_t iObjectCount = 0;
			for (int i = 1; i < 64; i++)
			{
				PlayerData pd;
				GetLastRecord(i, pd);
				if (pd.m_ShouldQuery)
					iObjectCount++;
			}

			auto ServerHash = GetServerCRC32();
			if (!iObjectCount || !ServerHash)
				return;

			std::size_t DesiredPacketSize = sizeof(PacketHeader_t) + sizeof(QueryEntityPacket_t) * iObjectCount;
			std::size_t BufferPos = 0;
			char* SendBuffer = new char[DesiredPacketSize];

			PacketHeader_t PH;
			PH.m_Type = PacketType::Query;
			PH.m_ServerHash = ServerHash;
			PH.m_SizeParam = iObjectCount;

			// Copy header to buffer
			std::memcpy(SendBuffer, &PH, sizeof(PacketHeader_t));
			BufferPos += sizeof(PacketHeader_t);

			for (int i = 1; i < 64; i++)
			{
				PlayerData pd;
				GetLastRecord(i, pd);
				if (pd.m_ShouldQuery)
				{
					std::memcpy((SendBuffer + BufferPos), &pd.ToQuery(i), sizeof(QueryEntityPacket_t));
					BufferPos += sizeof(QueryEntityPacket_t);
				}
			}

			m_socket->send_to(boost::asio::buffer(SendBuffer, DesiredPacketSize), m_server_endpoint);

			// Read the response
			size_t recv_len = m_RecvManager.RecvWithTimeout();
			if (!recv_len)
				return;

			if (recv_len < sizeof(PacketHeader_t))
				throw std::exception("Not enough data recv.");

			const char* Buffer = m_recv_buffer.data();
			PacketHeader_t* _Header = (PacketHeader_t*)(Buffer);

			if (_Header->m_Type != PacketType::Update)
				throw std::exception("Wrong packet type in response.");

			if (!_Header->m_SizeParam)
				return;

			DesiredPacketSize = sizeof(PacketHeader_t) + sizeof(QueryEntityPacket_t) * _Header->m_SizeParam;

			if (_Header->m_ServerHash != ServerHash)
				throw std::exception("Wrong server in response. (Server-sided bug?)");

			for (int i = 0; i < PH.m_SizeParam; i++)
			{
				UpdateEntityPacket_t* UpdatePacket = (UpdateEntityPacket_t*)(Buffer + sizeof(PacketHeader_t) + sizeof(UpdateEntityPacket_t) * i);

				if (UpdatePacket->m_Index < 0 || UpdatePacket->m_Index > 64)
					throw std::exception("Wrong user index in response. (Server-sided bug?)");

				PlayerData pd;
				GetLastRecord(UpdatePacket->m_Index, pd);
					
				// We already have more recent info, ignore
				if (pd.m_SimulationTime >= UpdatePacket->m_SimulationTime)
					continue;

				// Write new data to memory
				SetLastRecord(UpdatePacket->m_Index, UpdatePacket->m_Crouching, UpdatePacket->m_SimulationTime, Vector(UpdatePacket->m_Position[0], UpdatePacket->m_Position[1], UpdatePacket->m_Position[2]));
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			CSGO::g_pCVar->DbgPrint("  >>>  Exception (%s): %s\n", __func__, e.what());
		}
	}

	void CDataManager::MainLoop()
	{
		try
		{
			while (true)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				if (m_bExit)
					break;

				if (!CSGO::g_pEngine->IsInGame() || !CSGO::g_pEngine->IsConnected())
				{
					ResetData();
					continue;
				}

				SendData();
				QueryData();
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			CSGO::g_pCVar->DbgPrint("  >>>  Exception (%s): %s\n", __func__, e.what());
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
			pData->m_RecvTime = CSGO::g_pGlobals->curtime;
		}
	}

	void CDataManager::MarkAsInvalidEntity(int i)
	{
		SetQueryStatus(i, false);
		SetSendingStatus(i, false);
	}

	void CDataManager::SetQueryStatus(int i, bool bShouldQuery)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		PlayerData* pData = &m_Data[i];
		pData->m_ShouldQuery = bShouldQuery;
	}

	void CDataManager::SetSendingStatus(int i, bool bShouldSend)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		PlayerData* pData = &m_Data[i];
		pData->m_ShouldSend = bShouldSend;
	}

	void CDataManager::SetLastRecord(int i, bool bCrouching, float flSimTime, Vector vecPosition)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
	
		PlayerData* pData = &m_Data[i];
		pData->m_SimulationTime = flSimTime;
		pData->m_Position = vecPosition;
		pData->m_Crouching = bCrouching;
		pData->m_RecvTime = CSGO::g_pGlobals->curtime;
	}

	void CDataManager::GetLastRecord(int i, PlayerData& pData)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		
		std::memcpy(&pData, &m_Data[i], sizeof(PlayerData));
	}

	void CDataManager::RecvManager::SetRecvState(bool state)
	{
		std::lock_guard<std::mutex> lock(m_RecvMutex);
		m_RecvComplete = state;
	}

	bool CDataManager::RecvManager::GetRecvState()
	{
		std::lock_guard<std::mutex> lock(m_RecvMutex);
		return m_RecvComplete;
	}

	void CDataManager::RecvManager::RecvThread()
	{
		m_RecvLenght = g_DataManager->m_socket->receive_from(boost::asio::buffer(g_DataManager->m_recv_buffer), g_DataManager->m_server_endpoint);
		SetRecvState(true);
	}

	size_t CDataManager::RecvManager::RecvWithTimeout()
	{
		SetRecvState(false);
		m_RecvLenght = 0;

		std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
		std::thread recv_thread = std::thread([this] { RecvThread(); });

		while (!GetRecvState())
		{
			auto ms_diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
			if (ms_diff > std::chrono::milliseconds(1000))
			{
				CSGO::g_pCVar->DbgPrint("Timeouted on %s!\n", __func__);
				recv_thread.detach();
				SuspendThread(recv_thread.native_handle());
				TerminateThread(recv_thread.native_handle(), 0);
				return 0;
			}
		}

		if (recv_thread.joinable())
			recv_thread.join();

		return m_RecvLenght;
	}
}
