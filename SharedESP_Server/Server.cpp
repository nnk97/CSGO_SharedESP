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

	void SharedESP_Server::HandlePacketUpdate(PacketHeader_t& _Header, boost::archive::text_iarchive& data)
	{
		//std::cout << "Recv UPDATE request (Count: " << _Header.m_SizeParam << ")" << std::endl;
		for (int i = 0; i < _Header.m_SizeParam; i++)
		{
			UpdateEntityPacket_t UpdatePacket;
			try
			{
				data >> UpdatePacket;

				//std::cout << "   [" << UpdatePacket.m_Index << "] Simulation: " << UpdatePacket.m_SimulationTime << std::endl;

				Data::PlayerData PD;
				PD.m_Crouch = UpdatePacket.m_Crouching;
				PD.m_Simulation = UpdatePacket.m_SimulationTime;
				std::memcpy(PD.m_Position, UpdatePacket.m_Position, sizeof(float) * 3);

				Data::Manager->PushData(_Header.m_ServerHash, UpdatePacket.m_Index, PD);
			}
			catch (...)
			{
				std::cerr << "Server: Error inside " << __func__ << "!" << std::endl;
			}
		}
	}

	void SharedESP_Server::HandlePacketQuery(PacketHeader_t& _Header, boost::archive::text_iarchive& data)
	{
		std::vector<std::pair<int, Data::PlayerData>> m_ValidTargets;

		for (int i = 0; i < _Header.m_SizeParam; i++)
		{
			QueryEntityPacket_t QueryPacket;
			try
			{
				data >> QueryPacket;

				Data::PlayerData PD;
				Data::Manager->PopData(_Header.m_ServerHash, QueryPacket.m_Index);

				if (PD.m_Simulation > QueryPacket.m_SimulationTime)
					m_ValidTargets.push_back(std::make_pair(QueryPacket.m_Index, PD));
			}
			catch (...)
			{
				std::cerr << "Server: Error inside " << __func__ << "!" << std::endl;
			}
		}

		// Create & send response for the client
		try
		{
			std::string outbound_data_;
			std::ostringstream archive_stream;
			boost::archive::text_oarchive archive(archive_stream);

			PacketHeader_t PH;
			PH.m_Type = PacketType::Update;
			PH.m_ServerHash = _Header.m_ServerHash;
			PH.m_SizeParam = m_ValidTargets.size();
			archive << PH;

			for (int i = 0; i < PH.m_SizeParam; i++)
				archive << m_ValidTargets[i].second.ToPacket(m_ValidTargets[i].first);

			outbound_data_ = archive_stream.str();
			m_socket->send_to(boost::asio::buffer(outbound_data_), m_remote_endpoint);
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
				std::istringstream iss(std::string(m_recv_buffer.data(), m_recv_buffer.data() + bytes_transferred));
				boost::archive::text_iarchive iarchive(iss);

				PacketHeader_t PHeader;
				iarchive >> PHeader;

				if (!PHeader.m_SizeParam || !PHeader.m_ServerHash)
				{
					start_receive();
					return;
				}

				switch (PHeader.m_Type)
				{
				case PacketType::Update:
					HandlePacketUpdate(PHeader, iarchive);
					break;

				case PacketType::Query:
					HandlePacketQuery(PHeader, iarchive);
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
