/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EpollManager.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/14 15:42:09 by bewong        #+#    #+#                 */
/*   Updated: 2025/11/17 12:53:27 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLLMANAGER_HPP
#define EPOLLMANAGER_HPP

#include <atomic>
#include <functional>
#include <string>
#include <unordered_map>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <csignal>
#include <iostream>

extern std::atomic<bool> g_shutdown;

class EpollManager
{
  public:
    using EventCallback = std::function<void(uint32_t events)>;

    EpollManager();
    ~EpollManager();

    void addFd(int fd, uint32_t events, EventCallback cb);
    void modifyFd(int fd, uint32_t events, EventCallback cb = nullptr);
    void removeFd(int fd);
    void eventLoop(void);

  private:
    int epFd_;
    std::unordered_map<int, EventCallback> callbacks_;
};
#endif  // EPOLLMANAGER_HPP_
