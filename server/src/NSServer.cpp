//
// Created by Ivan Marochkin on 12.03.2021.
//

#include "NSServer.h"
#include <iostream>
#include <array>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>


NSServer::NSServer(int numsThread) : _pool(numsThread) {

}

NSServer::~NSServer() {

}
std::atomic<int> gCount{0};
int NSServer::start(int port) {
    _general_sock = create_socket(port);
    if (_general_sock < 0) {
        exit(EXIT_FAILURE);
    }

    std::array<pollfd, 1024> pollArr;
    std::for_each(pollArr.begin(), pollArr.end(), [](pollfd &cur) {
       cur.fd = -1;
       cur.events = POLLIN;
    });

    pollArr[0].fd = _general_sock;
    int curFdsCount = 1;
    bool needCompress = false;

    while (_run) {
        auto pollcall = poll(pollArr.data(), curFdsCount, -1);
        if (pollcall < 0) {
            std::cerr << "Poll crash\n";
            exit(EXIT_FAILURE);
        } else if (pollcall == 0) {
            std::cerr << "Timeout\n";
        } else {
            for (int i = 0; i < curFdsCount; ++i) {
                if (pollArr[i].revents == 0) {
                    continue;
                }
                if (pollArr[i].fd == _general_sock) {
                    auto curClient = accept(_general_sock, nullptr, nullptr);
                    if (curFdsCount - 1 == 1024) {
                        std::clog << "Maximum connections\n";
                        close(curClient);
                    } else {
                        pollArr[curFdsCount].fd = curClient;
                        ++curFdsCount;
                        char buf[1024] = {0};
                        auto resRecv = recv(curClient, buf, 1024, MSG_DONTWAIT);
                        if (resRecv == -1) {
                            std::clog << "Connection disconnect case: " << std::strerror(errno) << '\n';
                        } else if (resRecv == 0) {
                            std::clog << "Connection WR close case\n";
                        } else {
                            std::vector<uint8_t> v(buf, buf + resRecv);
                            _pool.enqueue_task([v, curClient](){
                                std::clog << '\n';
                                for (unsigned long i = 0; i < v.size(); ++i) {
                                    std::clog << v[i];
                                }
                                std::clog << '\n';
                                gCount += v.size();
                                send(curClient, "ok\n", 3, MSG_DONTWAIT);
                            });
                        }
                    }
                } else {
                    if (pollArr[i].revents | POLLIN) {
                        char buf[1024] = {0};
                        auto curClient = pollArr[i].fd;
                        pollArr[i].revents = 0;
                        auto resRecv = recv(curClient, buf, 1024, MSG_DONTWAIT);
                        if (resRecv == -1) {
                            std::clog << "Connection X disconnect case: " << std::strerror(errno) << '\n';
                        } else if (resRecv == 0) {
                            needCompress = true;
                            _pool.enqueue_task([curClient](){
                                close(curClient);
                            });
                            pollArr[i].fd = -1;
                            --curFdsCount;
                            std::clog << "Connection X WR close case\n";
                        } else {
                            std::vector<uint8_t> v(buf, buf + resRecv);
                            _pool.enqueue_task([v, curClient]() {
                                //std::clog << std::hash<std::thread::id>{}(std::this_thread::get_id()) << ' ';
                                for (unsigned long i = 0; i < v.size(); ++i) {
                                    //std::clog << v[i];
                                }
                                using namespace std::chrono;
                                std::this_thread::sleep_for(10ms);
                                //std::clog << '\n';
                                gCount += v.size();
                                send(curClient, "ok\n", 3, MSG_DONTWAIT);
                            });
                        }
                    }
                }
            }
        }
        if (needCompress) {
            std::sort(pollArr.begin(), pollArr.end(), [](pollfd &l, pollfd &r) {
                return l.fd > r.fd;
            });
            needCompress = false;
        }
        std::clog << "Cycle, curFdsCount: " << curFdsCount << ", and gCheck " << gCount <<'\n';
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
