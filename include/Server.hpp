#ifndef	SERVER_HPP_
#define SERVER_HPP_

#include <vector>

// Create listening sockets
// Bind to ports
// Register sockets with epoll
// Accept new client connections

class Server
{
	public:
		Server(const ServerConfig& config, int epFd);
		void	listening(void);
		void	acceptClient(void);
	private:
		int					epFd_;
		std::vector<int>	listeningSockets_;
		ServerConfig		config_;
		
};

#endif //SERVER_HPP_
