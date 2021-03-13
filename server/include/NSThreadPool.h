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

class NSServer;

class NSThreadPool {
public:
    struct Task {
        std::function<void()> f;
        int fd = -1;
    };

    NSThreadPool() = delete;
    explicit NSThreadPool(int numsThread, NSServer *parent);
    ~NSThreadPool();

    void enqueue_task(Task &&task);
    void stop();

private:

    struct {
        std::mutex mtx;
        std::queue<Task> work_queue;
        std::atomic_bool aborting{false};
    } _state;
    std::vector<std::thread> _workers;
    std::condition_variable _cv;
    NSServer *_parent;

    void worker_loop();
};


#endif //NSSERVER_NSTHREAD_H
