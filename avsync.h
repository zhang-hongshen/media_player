//
// Created by 张鸿燊 on 10/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_AVSYNC_H
#define SIMPLEST_MEDIA_PLAYER_AVSYNC_H


class AVSync {
public:
    enum SyncType{
        AUDIO,
        EXTERNAL
    };
public:
    AVSync(SyncType type = EXTERNAL);
    ~AVSync();
    void setClock(double pts, double time);
    void setClock(double pts);
    double getClock() const;

private:
    SyncType type_ = AUDIO;

    double last_pts_ = 0;
    double last_pts_drift = 0;
};


#endif //SIMPLEST_MEDIA_PLAYER_AVSYNC_H
