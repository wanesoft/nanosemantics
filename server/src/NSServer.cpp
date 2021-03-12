//
// Created by Ivan Marochkin on 12.03.2021.
//

#include "NSServer.h"
#include <iostream>
NSServer::NSServer(int numsThread) : _pool(numsThread) {

    for (int i = 0; i < 100; ++i) {
        _pool.enqueue_task([]() {
            std::clog << std::hash<std::thread::id>{}(std::this_thread::get_id()) <<  " asddasda\n";
        });
    }
}

NSServer::~NSServer() {

}
