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
			CSGO::g_pCVar->DbgPrint("  >>>  Exception #1: %s\n", e.what());
		}
	}

	void CDataManager::SendData()
	{
		// Collect the data to send
		std::vector<UpdateEntityPacket_t> players_data;
		
		for (int i = 1; i < 64; i++)
		{
			PlayerData* pData = &m_Data[i];
			if (pData->m_ShouldSend)
				players_data.push_back(pData->ToPacket(i));
		}

		std::string outbound_data_, outbound_header_;
		std::ostringstream archive_stream;
		boost::archive::text_oarchive archive(archive_stream);
		archive << (DWORD)NULL; // CRC32 of the server
		for (int i = 0; i < players_data.size(); i++)
			archive << players_data[i];
		outbound_data_ = archive_stream.str();

		// Packet header
		std::ostringstream header_stream;
		header_stream << std::setw(4) << std::hex << PacketType::Update;
		header_stream << std::setw(8) << std::hex << outbound_data_.size();
		outbound_header_ = header_stream.str();

		std::vector<boost::asio::const_buffer> buffers;
		buffers.push_back(boost::asio::buffer(outbound_header_));
		buffers.push_back(boost::asio::buffer(outbound_data_));
		//socket->send_to(buffers, server_endpoint);
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
			CSGO::g_pCVar->DbgPrint("  >>>  Exception #2: %s\n", e.what());
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
