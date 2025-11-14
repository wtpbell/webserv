#include "EpollManager.hpp"
#include "Server.hpp"

#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <atomic>
#include <csignal>
#include <netinet/in.h>

std::atomic<bool> g_shutdown(false);

void	signalHandler(int sig)
{
	if (sig == SIGINT || sig == SIGTERM)
		g_shutdown.store(true);
}

void setupSignals()
{
	struct sigaction sa;
	sa.sa_handler = signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, nullptr);
	sigaction(SIGTERM, &sa, nullptr);
	sigaction(SIGTSTP, &sa, nullptr);
}

int main()
{
	setupSignals();

	try
	{
		EpollManager ep;
		Server server(ep, 8080);

		std::cout << "Server running on :8080\n";
		ep.eventLoop();

		std::cout << "\nShutting down gracefully...\n";

	} catch (const std::exception& e)
	{
		std::cerr << "Fatal error: " << e.what() << "\n";
	}
}
