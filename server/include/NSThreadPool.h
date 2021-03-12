//
// Created by Ivan Marochkin on 12.03.2021.
//

#ifndef NSSERVER_NSTHREAD_H
#define NSSERVER_NSTHREAD_H

#include <thread>
#include <future>
#include <vector>
#include <queue>
#include <mutex>


class NSThreadPool {
public:
    using UniqueFunction = std::packaged_task<void()>;

    NSThreadPool() = delete;
    explicit NSThreadPool(int numsThread);
    ~NSThreadPool();

    void enqueue_task(UniqueFunction task);

private:
    struct {
        std::mutex mtx;
        std::queue<UniqueFunction> work_queue;
        bool aborting = false;
    } _state;
    std::vector<std::thread> _workers;
    std::condition_variable _cv;

    void worker_loop();
};


#endif //NSSERVER_NSTHREAD_H
