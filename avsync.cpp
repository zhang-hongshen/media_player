//
// Created by 张鸿燊 on 10/1/2024.
//

#include "avsync.h"

#include <iostream>
#include <chrono>
#include <limits>

extern "C" {
#include "libavutil/time.h"
}

AVSync::AVSync(SyncType type): type_(type) {
    setClock(std::numeric_limits<double>::quiet_NaN());
}

AVSync::~AVSync() {

}

void AVSync::setClock(double pts, double time) {
    last_pts_ = pts;
    last_pts_drift = last_pts_ - time;
}

double AVSync::getClock() const {
    double now = static_cast<double>(av_gettime()) / 1000000.0;
    return last_pts_drift + now;
}

void AVSync::setClock(double pts) {
    double time = static_cast<double>(av_gettime()) / 1000000.0;
    setClock(pts, time);
}