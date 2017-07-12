#include "Server.hpp"

using boost::asio::ip::udp;

namespace Server
{
	SharedESP_Server::SharedESP_Server() : m_work(m_io_service)
	{
		try
		{
			m_socket = std::make_unique<udp::socket>(m_io_service, udp::endpoint(udp::v4(), 21370));
			m_service_thread = std::thread([this] { this->service_loop(); });
		}
		catch (const std::exception& e)
		{
			std::cerr << "Server: Initialization exception: " << e.what() << std::endl;
		}
		catch (...)
		{
			std::cerr << "Server: Initialization exception: unknown" << std::endl;
		}
	}

	SharedESP_Server::~SharedESP_Server()
	{
		m_io_service.stop();
		m_service_thread.join();
	}

	void SharedESP_Server::start_receive()
	{
		m_socket->async_receive_from(boost::asio::buffer(m_recv_buffer), m_remote_endpoint,
			[this](boost::system::error_code ec, std::size_t bytes_recvd) { this->handle_receive(ec, bytes_recvd); });
	}

	void SharedESP_Server::HandlePacketUpdate(PacketHeader_t* _Header, size_t recv_lenght)
	{
		size_t DesiredPacketSize = sizeof(PacketHeader_t) + sizeof(UpdateEntityPacket_t) * _Header->m_SizeParam;
		const char* Buffer = m_recv_buffer.data();

		if (recv_lenght != DesiredPacketSize)
		{
			std::cerr << "Recv wrong amount of data (R: " << recv_lenght << "; D: " << DesiredPacketSize << ") in " << __func__ << std::endl;
			return;
		}

		for (int i = 0; i < _Header->m_SizeParam; i++)
		{
			UpdateEntityPacket_t* UpdatePacket = (UpdateEntityPacket_t*)(Buffer + sizeof(PacketHeader_t) + sizeof(UpdateEntityPacket_t) * i);

			Data::PlayerData PD;
			PD.m_Crouch = UpdatePacket->m_Crouching;
			PD.m_Simulation = UpdatePacket->m_SimulationTime;
			std::memcpy(PD.m_Position, UpdatePacket->m_Position, sizeof(float) * 3);

			Data::Manager->PushData(_Header->m_ServerHash, UpdatePacket->m_Index, PD);
		}
	}

	void SharedESP_Server::HandlePacketQuery(PacketHeader_t* _Header, size_t recv_lenght)
	{
		size_t DesiredPacketSize = sizeof(PacketHeader_t) + sizeof(QueryEntityPacket_t) * _Header->m_SizeParam;
		const char* Buffer = m_recv_buffer.data();

		if (recv_lenght != DesiredPacketSize)
		{
			std::cerr << "Recv wrong amount of data (R: " << recv_lenght << "; D: " << DesiredPacketSize << ") in " << __func__ << std::endl;
			return;
		}

		std::vector<std::pair<int, Data::PlayerData>> ValidTargets;

		for (int i = 0; i < _Header->m_SizeParam; i++)
		{
			QueryEntityPacket_t* QueryPacket = (QueryEntityPacket_t*)(Buffer + sizeof(PacketHeader_t) + sizeof(QueryEntityPacket_t) * i);
			Data::PlayerData PD = Data::Manager->PopData(_Header->m_ServerHash, QueryPacket->m_Index);

			if (PD.m_Simulation > QueryPacket->m_SimulationTime)
				ValidTargets.push_back(std::make_pair(QueryPacket->m_Index, PD));
		}

		// Create & send response for the client
		try
		{
			std::size_t DesiredPacketSize = sizeof(PacketHeader_t) + sizeof(UpdateEntityPacket_t) * ValidTargets.size();
			std::size_t BufferPos = 0;
			char* SendBuffer = new char[DesiredPacketSize];

			PacketHeader_t PH;
			PH.m_Type = PacketType::Update;
			PH.m_ServerHash = _Header->m_ServerHash;
			PH.m_SizeParam = ValidTargets.size();

			// Copy header to buffer
			std::memcpy(SendBuffer, &PH, sizeof(PacketHeader_t));
			BufferPos += sizeof(PacketHeader_t);

			for (int i = 0; i < PH.m_SizeParam; i++)
			{
				std::memcpy((SendBuffer + BufferPos), &(ValidTargets[i].second.ToPacket(ValidTargets[i].first)), sizeof(UpdateEntityPacket_t));
				BufferPos += sizeof(UpdateEntityPacket_t);
			}

			m_socket->send_to(boost::asio::buffer(SendBuffer, DesiredPacketSize), m_remote_endpoint);
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void SharedESP_Server::handle_receive(const boost::system::error_code & error, std::size_t bytes_transferred)
	{
		if (!error)
		{
			try
			{
				if (bytes_transferred < sizeof(PacketHeader_t))
				{
					start_receive();
					return;
				}

				PacketHeader_t* Header = (PacketHeader_t*)(m_recv_buffer.data());
				if (!Header->m_SizeParam || !Header->m_ServerHash)
				{
					start_receive();
					return;
				}

				switch (Header->m_Type)
				{
				case PacketType::Update:
					HandlePacketUpdate(Header, bytes_transferred);
					break;

				case PacketType::Query:
					HandlePacketQuery(Header, bytes_transferred);
					break;
				}
			}
			catch (const std::exception& e)
			{
				std::cerr << "Server: Parsing exception: " << e.what() << std::endl;
			}
			catch (...)
			{
				std::cerr << "Server: Parsing exception: Unknown" << std::endl;
			}
		}
		else
			std::cerr << "Server: Error inside " << __func__ << "!" << std::endl;

		start_receive();
	}

	void SharedESP_Server::service_loop()
	{
		start_receive();

		std::cout << "Server: Service loop has been started!" << std::endl;

		while (!m_io_service.stopped())
		{
			try
			{
				m_io_service.run();
			}
			catch (const std::exception& e)
			{
				std::cerr << "Server: Network exception: " << e.what() << std::endl;
			}
			catch (...) 
			{
				std::cerr << "Server: Network exception: Unknown" << std::endl;
			}
		}

		std::cout << "Server: Service loop has been stopped!" << std::endl;
	}

	bool SharedESP_Server::IsRunning()
	{
		return !m_io_service.stopped();
	}
}
