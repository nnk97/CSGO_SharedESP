#include "SyncData.hpp"

using boost::asio::ip::udp;

namespace SyncData
{
	std::unique_ptr<CDataManager> g_DataManager;
	const char* g_ServerIP = "127.0.0.1";

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
			udp::resolver resolver(io_service);
			udp::resolver::query query(udp::v4(), g_ServerIP, "21370");
			server_endpoint = *resolver.resolve(query);

			socket = new udp::socket(io_service);
			socket->open(udp::v4());

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

	int32_t GetServerCRC32()
	{
		const char* str = CSGO::g_pServerIP->GetPtr();
		if (!str)
			return 0;

		boost::crc_32_type result;
		result.process_bytes(str, strlen(str));
		return result.checksum();
	}

	void CDataManager::SendData()
	{
		try
		{
			std::string outbound_data_, outbound_header_;
			std::ostringstream archive_stream;
			boost::archive::text_oarchive archive(archive_stream);

			uint32_t iObjectCount = 0;
			for (int i = 1; i < 64; i++)
			{
				auto pData = &m_Data[i];
				if (pData->m_ShouldSend)
					iObjectCount++;
			}

			PacketHeader_t PH;
			PH.m_Type = PacketType::Update;
			PH.m_ServerHash = GetServerCRC32();
			PH.m_SizeParam = iObjectCount;
			archive << PH;

			for (int i = 1; i < 64; i++)
			{
				auto pData = &m_Data[i];
				if (pData->m_ShouldSend)
				{
					archive << pData->ToPacket(i);
					SetSendingStatus(i, false);
				}
			}

			outbound_data_ = archive_stream.str();
			socket->send_to(boost::asio::buffer(outbound_data_), server_endpoint);
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			CSGO::g_pCVar->DbgPrint("  >>>  Exception (%s): %s\n", __func__, e.what());
		}
	}

	void CDataManager::QueryData()
	{

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
					continue;

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
	}

	void CDataManager::GetLastRecord(int i, PlayerData& pData)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		
		std::memcpy(&pData, &m_Data[i], sizeof(PlayerData));
	}
}
