//
// Created by Ivan Marochkin on 12.03.2021.
//

#ifndef NSSERVER_H
#define NSSERVER_H

#include <NSThreadPool.h>
#include <unordered_map>
#include <memory>
#include <poll.h>
#include <vector>
#include <functional>


struct NSServerParams {
    std::vector<std::string> wordsForSearch;
    int numThreads;
    int port;
    int maxConnections;
    int bufSize;
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
        std::vector<uint8_t> *prevPacket = nullptr;
        int64_t _countTask = 0;
        uint64_t _readPos = 0;
        bool _eof = false;
    };
    using ClientPtr = std::unique_ptr<Client>;

    int create_socket(int port);
    void on_read(int index);
    std::vector<uint8_t> get_last_word(std::vector<uint8_t> &vector, int resRecv);

    NSServerParams &_params;
    NSThreadPool _pool;
    std::vector<pollfd> _pollVec;
    std::unordered_map<int, ClientPtr> _connections;
    std::mutex _connectionsMtx;
    int _general_sock = -1;
    int _activeConnections;
    bool _run = true;
};


#endif //NSSERVER_H
