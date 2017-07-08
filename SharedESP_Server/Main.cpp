#include "Main.hpp"

int main(int argc, char* argv[])
{
	auto Server = new Server::SharedESP_Server();
	while (!Server->ioservice.stopped())
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	return 0;
}
