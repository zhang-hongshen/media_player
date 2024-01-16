//
// Created by 张鸿燊 on 8/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_THREAD_H
#define SIMPLEST_MEDIA_PLAYER_THREAD_H


#include <thread>

class Thread {
public:
    Thread();
    virtual ~Thread();
    int Stop();
protected:
    virtual int Run() = 0;

public:
    constexpr static int EXIT = 1;
protected:
    int abort_ = 0;
    std::thread *thread = nullptr;
};


#endif //SIMPLEST_MEDIA_PLAYER_THREAD_H
