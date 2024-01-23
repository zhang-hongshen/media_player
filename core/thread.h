//
// Created by 张鸿燊 on 8/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_THREAD_H
#define SIMPLEST_MEDIA_PLAYER_THREAD_H


#include <thread>

class Thread {
public:
    template< class Function, class... Args >
    explicit Thread( Function&& f, Args&&... args );
    ~Thread() noexcept;
    int Stop();
    int Aborted();
    void Abort();
private:
    int abort_ = 0;
    std::thread *thread = nullptr;
};

template< class Function, class... Args >
Thread::Thread(Function && f, Args&&... args) {
    thread = new std::thread(f, std::forward<Args>(args)...);
}

#endif //SIMPLEST_MEDIA_PLAYER_THREAD_H
