#include "EpollManager.hpp"
#include <sys/epoll.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>

EpollManager::EpollManager(void)
{
	epFd_ = epoll_create1(0);
	if (epFd_ < 0)
		throw std::runtime_error("Failed to create epoll");
}

EpollManager::~EpollManager(void)
{
	close(epFd_);
}

void	EpollManager::addFd(int fd, uint32_t events, EventCallback cb)
{
	struct epoll_event	ev;

	ev.data.fd = fd;
	ev.events = events;
	if (epoll_ctl(epFd_, EPOLLIN, fd, &ev) < 0)
		throw std::runtime_error("epoll_ctl ADD failed");
	callbacks_[fd] = cb;
}

void	EpollManager::modifyFd(int fd, uint32_t events)
{
	struct epoll_event	ev;
	
	ev.data.fd = fd;
	ev.events = events;

}

