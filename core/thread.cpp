//
// Created by 张鸿燊 on 8/1/2024.
//
#include <spdlog/spdlog.h>

#include "thread.h"

Thread::Thread(){
    spdlog::info("Thread::Thread() \n");
}

Thread::~Thread(){
    spdlog::info("Thread::~Thread() \n");
    Thread::Stop();
}

int Thread::Stop() {
    while(Thread::EXIT != abort_);
    if(thread) {
        if(thread->joinable()) {
            thread->join();
        }
        delete thread;
        thread = nullptr;
    }
    return 0;
}

