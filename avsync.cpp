//
// Created by 张鸿燊 on 10/1/2024.
//

#include "avsync.h"

#include <limits>

extern "C" {
#include <libavutil/time.h>
}

AVSync::AVSync(SyncType type): type_(type) {
    SetClock(std::numeric_limits<double>::quiet_NaN());
}

AVSync::~AVSync() {

}

void AVSync::SetClock(double pts, double time) {
    last_pts_ = pts;
    last_pts_drift = last_pts_ - time;
}

double AVSync::GetClock() const {
    double now = static_cast<double>(av_gettime()) / 1000000.0;
    return last_pts_drift + now;
}

void AVSync::SetClock(double pts) {
    double time = static_cast<double>(av_gettime()) / 1000000.0;
    SetClock(pts, time);
}