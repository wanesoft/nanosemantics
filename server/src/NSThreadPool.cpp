//
// Created by Ivan Marochkin on 12.03.2021.
//

#include "NSThreadPool.h"


NSThreadPool::NSThreadPool(int numsThread) {
    _workers.reserve(numsThread);
    for (int i=0; i < numsThread; ++i) {
         _workers.emplace_back([this]() {
             worker_loop();
         });
    }
}

NSThreadPool::~NSThreadPool() {
    while (true) {
        {
            std::lock_guard<std::mutex> lk(_state.mtx);
            if (_state.work_queue.empty() || _state.aborting) {
                break;
            } else {
                using namespace std::chrono;
                _state.mtx.unlock();
                std::this_thread::sleep_for(100ms);
            }
        }
    }

    _state.aborting = true;
    _cv.notify_all();
    for (std::thread &t : _workers) {
        t.join();
    }
}

void NSThreadPool::stop() {
    _state.aborting = true;
    _cv.notify_all();
}

void NSThreadPool::enqueue_task(Task &&task) {
    {
        std::lock_guard<std::mutex> lk(_state.mtx);
        _state.work_queue.push(std::move(task));
    }
    _cv.notify_one();
}

void NSThreadPool::worker_loop() {
    while (true) {
        std::unique_lock<std::mutex> lk(_state.mtx);
        while (_state.work_queue.empty() && !_state.aborting) {
            _cv.wait(lk);
        }
        if (_state.aborting) {
            break;
        }
        Task task = std::move(_state.work_queue.front());
        _state.work_queue.pop();
        lk.unlock();
        task();
    }
}
