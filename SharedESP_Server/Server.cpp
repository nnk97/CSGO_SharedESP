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
