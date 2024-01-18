//
// Created by 张鸿燊 on 18/1/2024.
//

#ifndef MEDIA_PLAYER_QUEUE_H
#define MEDIA_PLAYER_QUEUE_H

#include <mutex>
#include <queue>
#include <condition_variable>
#include <limits>

template<typename T>
class Queue {
public:
    Queue(size_t cap = CAP_NOT_SET): cap_(cap)  {};
    ~Queue() {};

    // only allow one thread to push
    int push(const T& val) {
        std::unique_lock<std::mutex> lock(lock_);
        if(cap_ != CAP_NOT_SET && q_.size() >= cap_) {
            w_cond_.wait(lock, [&]{
                return q_.size() < cap_;
            });
        }
        q_.push(val);
        r_cond_.notify_one();
        return 0;
    }

    int push(T&& val) {
        std::unique_lock<std::mutex> lock(lock_);
        if(cap_ != CAP_NOT_SET && q_.size() >= cap_) {
            w_cond_.wait(lock, [&]{
                return q_.size() < cap_;
            });
        }
        q_.push(val);
        r_cond_.notify_one();
        return 0;
    }

    int pop(T &val, int timeout = 0) {
        std::unique_lock<std::mutex> lock(lock_);
        if(q_.empty()) {
            bool ret = r_cond_.wait_for(lock, std::chrono::milliseconds(timeout), [&]{
                return !q_.empty();
            });
            if(!ret) {
                return -1;
            }
        }

        val = q_.front();
        q_.pop();
        w_cond_.notify_one();
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
    std::condition_variable r_cond_;
    std::condition_variable w_cond_;
    std::queue<T> q_;
    size_t cap_ = CAP_NOT_SET;
};

#endif //MEDIA_PLAYER_QUEUE_H
