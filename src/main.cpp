/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 17:24:28 by jboon         #+#    #+#                 */
/*   Updated: 2025/11/17 16:20:26 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <atomic>
#include <csignal>
#include <iostream>
#include <thread>

#include "EpollManager.hpp"
std::atomic<bool> g_shutdown(false);

void signalHandler(int sig)
{
  if (sig == SIGINT || sig == SIGTERM) g_shutdown.store(true);
}


/*
SIGPIPE: This signal is generated when a process tries to write to a pipe, a FIFO, or a
socket for which there is no corresponding reader process. This normally
occurs because the reading process has closed its file descriptor for the
IPC channel. See Section 44.2 for further details. 
*/

void setupSignals()
{
  struct sigaction sa{};
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, nullptr) < 0
		|| sigaction(SIGTERM, &sa, nullptr) < 0)
    {
        throw std::runtime_error("SIGINT and SIGTERM caught");
        std::exit(1);
    }

    // Keep ignoring SIGPIPE (prevent crashes on closed sockets)
    struct sigaction ign{};
    ign.sa_handler = SIG_IGN;
    sigemptyset(&ign.sa_mask);
    ign.sa_flags = 0;
	//https://stackoverflow.com/questions/21687695/getting-sigpipe-with-non-blocking-sockets-is-this-normal
    if (sigaction(SIGPIPE, &ign, nullptr) < 0)
    {
        throw std::runtime_error("SIGPIPE caught");
        std::exit(1);
    }
}

int main()
{
  std::cout << "Testing EpollManager exception handling...\n";

  EpollManager epoll;

  int pipefd[2];
  if (pipe(pipefd) < 0)
  {
    perror("pipe");
    return 1;
  }

  int read_fd = pipefd[0];
  int write_fd = pipefd[1];

  // 1) ADD
  epoll.addFd(read_fd, EPOLLIN, [&](uint32_t events) { std::cout << "Event caught: " << events << "\n"; });
  std::cout << "ADD OK\n";

  // 2) ADD again -> should throw
  try
  {
    epoll.addFd(read_fd, EPOLLIN, nullptr);
    std::cout << "ERROR: ADD did NOT throw\n";
  }
  catch (std::exception& e)
  {
    std::cout << "ADD throw OK: " << e.what() << "\n";
  }

  // 3) MODIFY
  epoll.modifyFd(read_fd, EPOLLOUT);
  std::cout << "MODIFY OK\n";

  // 4) REMOVE 
  epoll.removeFd(read_fd);
  std::cout << "DEL OK\n";

  // 5) MODIFY after removal -> should throw
  try
  {
    epoll.modifyFd(read_fd, EPOLLIN);
    std::cout << "ERROR: MODIFY did NOT throw\n";
  }
  catch (std::exception& e)
  {
    std::cout << "MODIFY throw OK: " << e.what() << "\n";
  }

  // 6) REMOVE again -> should throw
  try
  {
    epoll.removeFd(read_fd);
    std::cout << "ERROR: DEL did NOT throw\n";
  }
  catch (std::exception& e)
  {
    std::cout << "DEL throw OK: " << e.what() << "\n";
  }

  close(read_fd);
  close(write_fd);

  std::cout << "All tests finished.\n";
  

  std::cout << "Entering sleep loop. Press Ctrl-C to test signal handling.\n";
    while (!g_shutdown.load())
    {
        sleep(1);
    }

    std::cout << "Shutdown signal received. Exiting.\n";
    return 0;
}
