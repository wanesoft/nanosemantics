//
// Created by Ivan Marochkin on 12.03.2021.
//

#ifndef NSSERVER_NSSERVER_H
#define NSSERVER_NSSERVER_H

#include <NSThreadPool.h>


class NSServer {
public:
    NSServer() = delete;
    explicit NSServer(int numsThread);
    ~NSServer();

    int start(int port);
    void stop();

private:
    int create_socket(int port);

    NSThreadPool _pool;
    int _general_sock = -1;
    bool _run = true;
};


#endif //NSSERVER_NSSERVER_H
