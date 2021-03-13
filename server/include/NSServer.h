//
// Created by Ivan Marochkin on 12.03.2021.
//

#ifndef NSSERVER_NSSERVER_H
#define NSSERVER_NSSERVER_H

#include <NSThreadPool.h>
#include <unordered_map>
#include <memory>
#include <poll.h>


struct NSServerParams {
    int numThreads;
    int port;
    int maxConnections;
};

class NSServer {
public:
    NSServer() = delete;
    explicit NSServer(NSServerParams &params);
    ~NSServer();

    int start();
    void stop();
    void on_start_task(int fd);
    void on_done_task(int id);

private:
    struct Client {
        uint64_t _countTask = 0;
        uint64_t _id = 0;
        bool _eof = false;
    };
    using ClientPtr = std::unique_ptr<Client>;

    int create_socket(int port);
    void on_read(int index);

    NSServerParams &_params;
    NSThreadPool _pool;
    std::vector<pollfd> _pollVec;
    std::unordered_map<int, Client *> _connections;
    std::mutex _connectionsMtx;
    uint64_t _nextClietnId = 1000000ULL;
    int _general_sock = -1;
    int _activeConnections;
    bool _run = true;
};


#endif //NSSERVER_NSSERVER_H
