#include "Server.hpp"

using boost::asio::ip::udp;

namespace Server
{
	SharedESP_Server::SharedESP_Server()
	{
		try
		{
			socket = new udp::socket(ioservice, udp::endpoint(udp::v4(), 21370));
			service_thread = std::thread([this] { this->service_loop(); });
		}
		catch (const std::exception& e) {
			std::cerr << "Server: Initialization exception: " << e.what() << std::endl;
		}
	}

	SharedESP_Server::~SharedESP_Server()
	{
		ioservice.stop();
		service_thread.join();
	}

	void SharedESP_Server::start_receive()
	{
		socket->async_receive_from(boost::asio::buffer(recv_buffer), remote_endpoint,
			[this](boost::system::error_code ec, std::size_t bytes_recvd) { this->handle_receive(ec, bytes_recvd); });
	}

	void SharedESP_Server::handle_receive(const boost::system::error_code & error, std::size_t bytes_transferred)
	{
		if (!error)
		{
			char* buffer = recv_buffer.data();

			std::istringstream iss(std::string(recv_buffer.data(), recv_buffer.data() + bytes_transferred));
			boost::archive::text_iarchive iarchive(iss);

			PacketHeader_t PHeader;
			iarchive >> PHeader;

			//std::cout << "Server: Recv: " << std::hex << PH.m_Type << " packet..." << std::endl;
			//std::cout << "   >>>  Hash: " << std::hex << PH.m_ServerHash << std::endl;
			//std::cout << "   >>>  Size: " << PH.m_SizeParam << std::endl;

			if (PHeader.m_Type == PacketType::Update)
			{
				std::cout << "Recv UPDATE request: (" << PHeader.m_SizeParam << ")" << std::endl;
				for (int i = 0; i < PHeader.m_SizeParam; i++)
				{
					UpdateEntityPacket_t UpdatePacket;
					iarchive >> UpdatePacket;
					std::cout << "   [" << UpdatePacket.m_Index << "] Sim: " << UpdatePacket.m_SimulationTime << "; Vec.x: " << UpdatePacket.m_Position[0] << std::endl;
				}
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

		while (!ioservice.stopped())
		{
			try
			{
				ioservice.run();
			}
			catch (const std::exception& e) {
				std::cerr << "Server: Network exception: " << e.what() << std::endl;
			}
			catch (...) {
				std::cerr << "Server: Network exception: Unknown" << std::endl;
			}
		}

		std::cout << "Server: Service loop has been stopped!" << std::endl;
	}
}
