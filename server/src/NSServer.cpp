//
// Created by Ivan Marochkin on 12.03.2021.
//

#include "NSServer.h"
#include <iostream>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


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
                        close(curClientFd);
                    } else {
                        _pollVec[_activeConnections].fd = curClientFd;
                        auto *curClient = new Client;
                        curClient->prevPacket = new std::vector<uint8_t>;
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
        auto &cur = _connections[fd];
        delete cur->prevPacket;
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

void NSServer::on_read(int index) {
    auto buf = new std::vector<uint8_t>;
    buf->resize(_params.bufSize + 1);
    static bool needCompress = false;
    int curClient = _pollVec[index].fd;

    errno = 0;
    auto resRecv = recv(curClient, buf->data(), _params.bufSize, MSG_DONTWAIT);
    if (resRecv == -1) {
        assert(errno == EAGAIN);
        return;
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
        uint64_t curMaxReadPos;
        {
            std::lock_guard<std::mutex> lk(_connectionsMtx);
            auto &it = _connections[curClient];
            if (!it->prevPacket->empty()) {
                // increase readable size
                resRecv += it->prevPacket->size();
                // increase current max read position
                it->_readPos += resRecv;
                // but decrease it cuz we have old word
                it->_readPos -= it->prevPacket->size();
                it->prevPacket->reserve(it->prevPacket->size() + _params.bufSize);
                std::copy(buf->begin(), buf->end(), std::back_inserter(*it->prevPacket));
                // todo use pointers for cancel deep copying
                std::swap(buf, it->prevPacket);
            } else {
                it->_readPos += resRecv;
            }
            curMaxReadPos = it->_readPos;
            *it->prevPacket = get_last_word(*buf, resRecv);
        }
        auto &ref = _params.wordsForSearch;
        (*buf)[resRecv] = '\0';
        _pool.enqueue_task([this, buf, resRecv, curClient, curMaxReadPos, ref]() {

            // описание алгоритма:
            // в настоящий момент сложность O(N + M * k)
            // где N - это размер входящего потока
            // M - длина одного искомого слова
            // и k - их количество
            // так как M и k известны на стадии запуска - они являются константами
            // и мы можем ими пренебречь. итого сложность: O(N)

            for (int i = 0; i < resRecv; ++i) {
                if (isspace((*buf)[i]) || i == 0) {
                    if (i != 0 || isspace((*buf)[i])) {
                        ++i;
                    }
                    // todo find min word's size for optimize `i` position
                    for (auto &cur : ref) {
                        auto res = strncmp((char *)&(*buf)[i], cur.data(), cur.size());
                        if (res == 0 && ((*buf)[i + cur.size()] == ' '
                                      || (*buf)[i + cur.size()] == '\0'
                                      || (*buf)[i + cur.size()] == '\n')) { // here is just for safety
                            std::string ret = cur;
                            ret += ',';
                            ret += std::to_string(curMaxReadPos - resRecv + i);
                            ret += ';';
                            // todo fix for nonblocking
                            send(curClient, ret.data(), ret.size(), 0);
                        }
                    }
                }
            }
            this->on_done_task(curClient);
            delete buf;
        });
    }
    if (needCompress) {
        std::sort(_pollVec.begin(), _pollVec.end(), [](pollfd &l, pollfd &r) {
            return l.fd > r.fd;
        });
        needCompress = false;
    }
}

std::vector<uint8_t> NSServer::get_last_word(std::vector<uint8_t> &vector, int resRecv) {
    if (vector.empty()) {
        return {};
    }

    auto counter = 0;
    auto it = (vector.begin() + resRecv - 1);
    for ( ; it != vector.begin(); --it) {
        if (*it == ' ') {
            break;
        }
        ++counter;
    }

    if (counter == 0) {
        return {};
    }

    std::vector<uint8_t> ret(0, counter);
    it = (vector.begin() + resRecv - 1);
    for ( ; it != vector.begin(); --it) {
        if (*it == ' ') {
            break;
        }
        ret.emplace_back(*it);
    }

    std::reverse(ret.begin(), ret.end());
    return ret;
}
