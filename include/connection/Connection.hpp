#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

// Track state of a client connection
// Hold socket FD, buffer, parser state
// Handle reading/writing events
enum class ConnectionState
{
	WAIT_REQUEST,
	READING,
	PROCESSING,
	WRITING,
	COMPLETE
};

class Connection
{
	public:
		Connection(int cFd, int epFd);

	private:
		int				cFd_;
		int				epFd_;
		ConnectionState	stat_;
		void			handleWrite();
		void			handleRead();
};



#endif //CONNECTION_HPP_
