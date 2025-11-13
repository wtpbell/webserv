#ifndef	EPOLLMANAGER_HPP
#define EPOLLMANAGER_HPP

// #include <string>
// #include <functional>
// #include <unordered_map>

#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <vector>
#include <stdexcept>

class EpollManager
{
	// public:
	// 	using EventCallback = std::function<void(uint32_t events)>;

	// 	EpollManager();
	// 	~EpollManager();

	// 	void addFd(int fd, uint32_t events, EventCallback cb);
	// 	void removeFd(int fd);
	// 	void eventLoop();

	// private:
	// 	int										epollFd_;
	// 	std::unordered_map<int, EventCallback>	callbacks_;

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

			while (true)
			{
				
			}
		}

	private:
		int										epFd_;
		std::unordered_map<int, EventCallBack>	callbacks_;
};

#endif //EPOLLMANAGER_HPP
