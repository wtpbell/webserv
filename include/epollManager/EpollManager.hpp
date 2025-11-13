#ifndef	EPOLLMANAGER_HPP
#define EPOLLMANAGER_HPP

#include <string>
#include <functional>
#include <unordered_map>

class EpollManager
{
	public:
		using EventCallback = std::function<void(uint32_t events)>;

		EpollManager();
		~EpollManager();

		void addFd(int fd, uint32_t events, EventCallback cb);
		void removeFd(int fd);
		void eventLoop();

	private:
		int										epollFd_;
		std::unordered_map<int, EventCallback>	callbacks_;

};

#endif //EPOLLMANAGER_HPP
