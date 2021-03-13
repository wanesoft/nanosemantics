//
// Created by Ivan Marochkin on 12.03.2021.
//

#include "NSServer.h"
#include <iostream>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

const char *gNeedle;
uint64_t gCheck = 0;

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
    gNeedle = _params.wordForSearch.c_str();
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
                        close(curClientFd);
                    } else {
                        _pollVec[_activeConnections].fd = curClientFd;
                        auto *curClient = new Client;
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

    it->second->_countTask--;
    if (it->second->_countTask <= 0 && it->second->_eof) {
        close(fd);
        // std::clog << gcher << '\n';
        _connections.erase(fd);
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
    // todo get size from params
    std::array<uint8_t, 4096> buf;
    // buf.resize(_params.bufSize);
    static bool needCompress = false;
    int curClient = _pollVec[index].fd;

    errno = 0;
    auto resRecv = recv(curClient, buf.data(), 4096, MSG_DONTWAIT);
    if (resRecv == -1) {
        assert(errno == EAGAIN);
    } else if (resRecv == 0) {
        if (errno == EAGAIN) {
            return;
        }
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
        on_start_task(curClient);
        uint64_t curReadPos;
        {
            std::lock_guard<std::mutex> lk(_connectionsMtx);
            auto &it = _connections[curClient];
            curReadPos = it->_readPos;
            it->_readPos += resRecv;
        }
        _pool.enqueue_task([this, buf, resRecv, curClient, curReadPos]() {
            // todo part of word in another packets???
            for (int i = 0; i < resRecv; ++i) {
                if (isspace(buf[i])) {
                    ++i;
                    auto res = strncmp((char *)&buf[i], gNeedle, strlen(gNeedle));
                    if (res == 0 && buf[i + strlen(gNeedle)] == ' ') {
                        gCheck++;
                        auto ret = std::to_string(curReadPos + i);
                        ret += ';';
                        // todo fix for nonblocking
                        send(curClient, ret.data(), ret.size(), 0);
                    }
                }
            }
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
