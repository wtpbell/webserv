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

class EpollManager
{
	public:
		using EventCallBack = std::function<void(uint32_t)>;

		EpollManager(void)
		{
			epFd_ = epoll_create1(0);
			if (epFd_ < 0)
				throw std::runtime_error("Failed to create epoll");
		}

		~EpollManager(void)
		{
			close(epFd_);
		}

		void	addFd(int fd, uint32_t events, EventCallBack cd)
		{
			struct epoll_event	ev;

			ev.events = events;
			ev.data.fd = fd;
			if (epoll_ctl(epFd_, EPOLL_CTL_ADD, fd, &ev) < 0)
				throw std::runtime_error("epoll_ctl ADD failed");
			callbacks_[fd] = cd;
		}

		void	modifyFd(int fd, uint32_t events)
		{
			struct epoll_event	ev;

			ev.events = events;
			ev.data.fd = fd;
			if (epoll_ctl(epFd_, EPOLL_CTL_MOD, fd, &ev) < 0)
				throw std::runtime_error("epoll_ctl ADD failed");
		}

		void	removeFd(int fd)
		{
			epoll_ctl(epFd_, EPOLL_CTL_DEL, fd, nullptr);
			callbacks_.erase(fd);
		}

		void	eventLoop(void)
		{
			constexpr int		MAX_EVENTS = 64;
			struct epoll_event	events[MAX_EVENTS];
			sigset_t			waitMask;

			sigemptyset(&waitMask);
			while (!g_shutdown.load())
			{
				;
				int n = epoll_pwait(epFd_, events, MAX_EVENTS, -1, &waitMask);
				if (n < 0)
					continue ; // EINTR from signal loop again, check shutdown
				for (int i = 0; i < n; i++)
				{
					int fd = events[i].data.fd;
					auto it = callbacks_.find(fd);
					if (it != callbacks_.end())
						it->second(events[i].events);
				}
			}
		}

	private:
		int										epFd_;
		std::unordered_map<int, EventCallBack>	callbacks_;
};


class ServerSocket
{
	public:
		int fd;
		ServerSocket(int port)
		{
			fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

			int opt = 1;
			setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

			sockaddr_in addr{};
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = INADDR_ANY;
			addr.sin_port = htons(port);

			bind(fd, (sockaddr*)&addr, sizeof(addr));
			listen(fd, 128);
		}

};

class Server
{
	public:
		Server(EpollManager& ep, int port): epoll(ep), listener(port)
		{
			epoll.addFd(listener.fd, EPOLLIN, [&](uint32_t /*ev*/) {handleAccept();});
		}
	private:
		EpollManager&	epoll;
		ServerSocket	listener;

		void	handleAccept(void)
		{
			while (true)
			{
				int client = accept4(listener.fd, nullptr, nullptr, SOCK_NONBLOCK);
				if (client < 0)
					break;
				std::cout << "Client connected: " << client << "\n";
				epoll.addFd(client, EPOLLIN, [this, client](uint32_t ev){handleClient(client, ev);});
			}
		}
		void handleClient(int client, uint32_t events)
		{
			if (events & EPOLLIN)
			{
				char buf[1024];
				ssize_t n = read(client, buf, sizeof(buf));

				if (n <= 0)
				{
					epoll.removeFd(client);
					close(client);
					return;
				}

				// Ignore request content for now
				std::string body = "Hello from C++ server!\n";
				std::string response =
					"HTTP/1.1 200 OK\r\n"
					"Content-Length: " + std::to_string(body.size()) + "\r\n"
					"Content-Type: text/plain\r\n"
					"Connection: close\r\n"
					"\r\n" +
					body;

				write(client, response.c_str(), response.size());
				epoll.removeFd(client);
				close(client);
			}
		}

};


int main()
{
	setupSignals();

	try {
		EpollManager ep;
		Server server(ep, 8080);

		std::cout << "Server running on :8080\n";
		ep.eventLoop();

		std::cout << "\nShutting down gracefully...\n";

	} catch (const std::exception& e) {
		std::cerr << "Fatal error: " << e.what() << "\n";
	}
}
