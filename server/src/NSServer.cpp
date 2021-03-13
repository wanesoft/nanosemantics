//
// Created by Ivan Marochkin on 12.03.2021.
//

#include "NSServer.h"
// todo del me
#include <iostream>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


static constexpr int BUFSIZE = 1024;

// todo delete
std::atomic<int> gCount{0};

NSServer::NSServer(NSServerParams &params) : _params(params), _pool(params.numThreads, this) {
    _pollVec.reserve(params.maxConnections);
    _activeConnections = 0;
    for (int i = 0; i < _params.maxConnections; ++i) {
        pollfd tmp{ .fd = -1, .events = POLLIN, .revents = 0 };
        _pollVec.emplace_back(tmp);
    }
}

NSServer::~NSServer() {

}

int NSServer::start() {
    _general_sock = create_socket(_params.port);
    if (_general_sock < 0) {
        exit(EXIT_FAILURE);
    }

    _pollVec[0].fd = _general_sock;
    ++_activeConnections;

    while (_run) {
        auto pollcall = poll(_pollVec.data(), _activeConnections, -1);
        if (pollcall < 0) {
            std::cerr << "Poll crash\n";
            exit(EXIT_FAILURE);
        } else if (pollcall == 0) {
            std::cerr << "Timeout\n";
        } else {
            for (int i = 0; i < _activeConnections; ++i) {
                if (_pollVec[i].revents == 0) {
                    continue;
                }
                if (_pollVec[i].fd == _general_sock) {
                    auto curClientFd = accept(_general_sock, nullptr, nullptr);
                    if (_activeConnections - 1 == _params.maxConnections) {
                        std::clog << "Maximum connections\n";
                        close(curClientFd);
                    } else {
                        _pollVec[_activeConnections].fd = curClientFd;
                        auto *curClient = new Client;
                        curClient->_id = _nextClietnId++;
                        {
                            std::lock_guard<std::mutex> lk(_connectionsMtx);
                            _connections.emplace(curClientFd, curClient);
                        }
                        on_read(_activeConnections);
                        ++_activeConnections;
                    }
                } else {
                    if (_pollVec[i].revents | POLLIN) {
                        _pollVec[i].revents = 0;
                        on_read(i);
                    }
                }
            }
        }
        std::clog << "Cycle, curFdsCount: " << _activeConnections << ", and gCheck " << gCount <<'\n';
        for (int i = 0; i < 1024; ++i) {
            //std::clog << pollArr[i].fd << ' ';
        }
        std::clog << '\n';
    }

    return 0;
}

void NSServer::stop() {
    _pool.stop();
}

void NSServer::on_start_task(int fd) {
    std::lock_guard<std::mutex> lk(_connectionsMtx);
    _connections[fd]->_countTask++;
}

void NSServer::on_done_task(int fd) {
    Client *ptr;
    {
        std::lock_guard<std::mutex> lk(_connectionsMtx);
        ptr = _connections[fd];
    }

    ptr->_countTask--;
    if (ptr->_countTask == 0 && ptr->_eof) {
        close(fd);
        _connections.erase(fd);
        std::clog << "XXX gCount " << gCount << '\n';
    }
}

int NSServer::create_socket(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket\n";
        return -1;
    }

    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    memset(&addr, 0, addrLen);
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    int resBind = bind(sock, (sockaddr *)&addr, addrLen);
    if (resBind < 0) {
        std::cerr << "Failed to bind socket\n";
        return -1;
    }

    int resListen = listen(sock, SOMAXCONN);
    if (resListen < 0) {
        std::cerr << "Failed to listen socket\n";
        return -1;
    }

    return sock;
}

void NSServer::on_read(int index) {
    std::vector<uint8_t> buf;
    buf.reserve(BUFSIZE);
    static bool needCompress = false;
    int curClient = _pollVec[index].fd;

    auto resRecv = recv(curClient, buf.data(), BUFSIZE, MSG_DONTWAIT);
    if (resRecv == -1) {
        std::clog << "Connection X disconnect case: " << std::strerror(errno) << '\n';
//        _pollVec[index].fd = -1;
//        --_activeConnections;
//        close(curClient);
//        needCompress = true;
    } else if (resRecv == 0) {
        std::clog << "Connection X WR close case: " << std::strerror(errno) << '\n';
        _pollVec[index].fd = -1;
        --_activeConnections;
        {
            std::lock_guard<std::mutex> lk(_connectionsMtx);
            _connections[curClient]->_eof = true;
        }
        needCompress = true;
    } else {
        on_start_task(curClient);
        NSThreadPool::Task t;
        t.fd = curClient;
        t.f = [buf, resRecv, curClient]() {
            //std::clog << std::hash<std::thread::id>{}(std::this_thread::get_id()) << ' ';
            for (int i = 0; i < resRecv; ++i) {
                //std::clog << v[i];
            }
            using namespace std::chrono;
            // std::this_thread::sleep_for(10ms);
            // std::clog << '\n';
            gCount += resRecv;
            send(curClient, "ok\n", 3, MSG_DONTWAIT);
        };
        _pool.enqueue_task(std::move(t));
    }
    if (needCompress) {
        std::sort(_pollVec.begin(), _pollVec.end(), [](pollfd &l, pollfd &r) {
            return l.fd > r.fd;
        });
        needCompress = false;
    }
}
