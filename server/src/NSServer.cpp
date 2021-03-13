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


// todo delete
std::atomic<uint64_t> gCount{0};

//static void logStrFormat(const char *fmt, ...) {
//    static std::mutex gPrintMtx;
//    std::lock_guard<std::mutex> lk(gPrintMtx);
//    int r;
//    std::string s;
//    va_list va;
//
//    va_start(va, fmt);
//    r = vsnprintf(NULL, 0, fmt, va);
//    va_end(va);
//    if (r < 0) {
//        assert(0);
//    }
//    s.resize(r + 1);
//
//    va_start(va, fmt);
//    r = vsnprintf(&s[0], s.capacity(), fmt, va);
//    va_end(va);
//    if (r < 0) {
//        assert(0);
//    }
//    s.resize(r);
//
//    std::clog << s << '\n';
//}

NSServer::NSServer(NSServerParams &params) : _params(params), _pool(params.numThreads) {
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
                        std::clog << "New connection: " << curClientFd << "\n";
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
        // logStrFormat("Cycle, curFdsCount: %d and gCheck %d", _activeConnections, gCount.load());
        for (int i = 0; i < 1024; ++i) {
            //std::clog << pollArr[i].fd << ' ';
        }
        //std::clog << '\n';
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
    std::lock_guard<std::mutex> lk(_connectionsMtx);
    auto it = _connections.find(fd);
    if (it == _connections.end()) {
        return;
    }

    //std::clog << __func__ << " " << fd << " " << it->second->_eof << " " << it->second->_countTask << '\n';

    it->second->_countTask--;
    if (it->second->_countTask <= 0 && it->second->_eof) {
        close(fd);
        _connections.erase(fd);
        std::clog << "XXX gCount " << gCount << '\n';
    }
}

int NSServer::create_socket(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    memset(&addr, 0, addrLen);
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        std::cerr << "Failed to set socket as reusable\n";
    }

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
#include <array>
void NSServer::on_read(int index) {
    std::array<uint8_t, 4096> buf;
    // buf.resize(_params.bufSize);
    static bool needCompress = false;
    int curClient = _pollVec[index].fd;

    errno = 0;
    auto resRecv = recv(curClient, buf.data(), 4096, MSG_DONTWAIT);
    if (resRecv == -1) {
        std::clog << "Connection X disconnect case for: " << curClient << " " << std::strerror(errno) << '\n';
        assert(errno == EAGAIN);
    } else if (resRecv == 0) {
        if (errno == EAGAIN) {
            return;
        }
        std::clog << "Connection X WR close case: for: " << curClient << " "  << std::strerror(errno) << '\n';
        {
            std::lock_guard<std::mutex> lk(_connectionsMtx);
            _connections[curClient]->_countTask++;
            _connections[curClient]->_eof = true;
        }
        _pollVec[index].fd = -1;
        --_activeConnections;
        needCompress = true;
        on_done_task(curClient);
    } else {
        if (resRecv != 4096) {
            std::clog << "get " << resRecv << " from " << curClient << '\n';
        }
        gCount += resRecv;
        on_start_task(curClient);
        _pool.enqueue_task([this, buf, resRecv, curClient]() {
            //std::clog << std::hash<std::thread::id>{}(std::this_thread::get_id()) << ' ';
            for (int i = 0; i < resRecv; ++i) {
                buf[i];
            }
            using namespace std::chrono;
            // std::this_thread::sleep_for(10ms);
            // std::clog << '\n';
            send(curClient, "ok\n", 3, MSG_DONTWAIT);
            this->on_done_task(curClient);
        });
    }
    if (needCompress) {
        std::sort(_pollVec.begin(), _pollVec.end(), [](pollfd &l, pollfd &r) {
            return l.fd > r.fd;
        });
        needCompress = false;
    }
}
