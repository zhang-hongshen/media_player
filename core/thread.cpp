//
// Created by 张鸿燊 on 8/1/2024.
//
#include "thread.h"

Thread::~Thread(){
    Thread::Stop();
}

int Thread::Stop() {
    abort_ = 1;
    if(thread) {
        if(thread->joinable()) {
            thread->join();
        }
        delete thread;
        thread = nullptr;
    }
    return 0;
}
void Thread::Abort() {
    abort_ = 1;
}

int Thread::Aborted() {
    return abort_;
}

