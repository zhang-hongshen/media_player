//
// Created by 张鸿燊 on 8/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_QUEUE_H
#define SIMPLEST_MEDIA_PLAYER_QUEUE_H

#include <mutex>
#include <queue>
#include <condition_variable>
#include <limits>

template<typename T>
class Queue {
public:
    Queue() {};
    ~Queue() {};

    // only allow one thread to push
    int push(const T& val) {
        std::lock_guard<std::mutex> lock(lock_);

        q_.push(val);
        cond_.notify_one();

        return 0;
    }

    int push(T&& val) {
        std::lock_guard<std::mutex> lock(lock_);

        q_.push(val);
        cond_.notify_one();

        return 0;
    }

    int pop(T &val, int timeout = 0) {
        std::unique_lock<std::mutex> lock(lock_);
        if(q_.empty()) {
            bool ret = cond_.wait_for(lock, std::chrono::milliseconds(timeout), [&]{
                return !q_.empty();
            });
            if(!ret) {
                return -1;
            }
        }

        val = q_.front();
        q_.pop();

        return 0;
    }

    int front(T &val) {
        std::lock_guard<std::mutex> lock(lock_);
        if(q_.empty()) {
            return -1;
        }
        val = q_.front();
        return 0;
    }

    size_t size() {
        return q_.size();
    }

public:
    constexpr static size_t CAP_NOT_SET = std::numeric_limits<size_t>::max();
private:
    std::mutex lock_;
    std::condition_variable cond_;
    std::queue<T> q_;
    size_t cap_ = CAP_NOT_SET;
};


#endif //SIMPLEST_MEDIA_PLAYER_QUEUE_H
