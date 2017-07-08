#include "Main.hpp"

int main(int argc, char* argv[])
{
	Data::Initialize();
	auto Server = new Server::SharedESP_Server();

	while (Server->IsRunning())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		Data::Manager->LookForDeadServers();
	}

	return 0;
}
