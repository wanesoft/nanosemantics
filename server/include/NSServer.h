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

private:
    NSThreadPool _pool;
};


#endif //NSSERVER_NSSERVER_H
