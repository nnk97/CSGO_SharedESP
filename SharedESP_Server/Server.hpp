#pragma once

#include "Main.hpp"
#include "PacketDefinitions.hpp"

using boost::asio::ip::udp;

namespace Server
{
	class SharedESP_Server
	{
	private:
		SharedESP_Server(SharedESP_Server&);

		udp::socket* socket;
		udp::endpoint server_endpoint;
		udp::endpoint remote_endpoint;

		std::array<char, 2048> recv_buffer;
		std::thread service_thread;

		void start_receive();
		void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);
		void handle_send(std::string message, const boost::system::error_code& error, std::size_t bytes_transferred) {};
		void service_loop();

	public:
		boost::asio::io_service ioservice;

		SharedESP_Server();
		~SharedESP_Server();
	};
}