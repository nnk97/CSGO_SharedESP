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

		boost::asio::io_service m_io_service;
		boost::asio::io_service::work m_work;

		std::thread m_service_thread;

		std::unique_ptr<udp::socket> m_socket;

		std::array<char, 2048> m_recv_buffer;
		udp::endpoint m_remote_endpoint;

		void start_receive();
		void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);
		void handle_send(std::string message, const boost::system::error_code& error, std::size_t bytes_transferred) {};
		void service_loop();

		void HandlePacketUpdate(PacketHeader_t& _Header, boost::archive::text_iarchive& data);
		void HandlePacketQuery(PacketHeader_t& _Header, boost::archive::text_iarchive& data);

	public:

		SharedESP_Server();
		~SharedESP_Server();

		bool IsRunning();
	};
}