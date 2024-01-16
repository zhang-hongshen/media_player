//
// Created by 张鸿燊 on 13/1/2024.
//

extern "C" {
#include <libavutil/common.h>
}

#include "mp_state.h"


MPState::MPState() {
    sync = std::make_unique<AVSync>(AVSync::AUDIO);
}

MPState::~MPState() {

}

void MPState::SetClock(double pts) {
    sync->SetClock(pts);
}

double MPState::GetClock() {
    return sync->GetClock();
}

void MPState::TogglePaused() {
    paused = !paused;
}

void MPState::ToggleMuted() {
    muted = !muted;
}

void MPState::Seek(double pts) {
    if(need_seek) {
        return;
    }
    seek_pts = av_clipd_c(pts, 0, std::numeric_limits<double>::max());
    need_seek = true;
}

void MPState::NextPlayBackSpeed() {
    playback_speed_index = av_clip_c(playback_speed_index + 1, 0, 4);
}

void MPState::PrePlayBackSpeed() {
    playback_speed_index = av_clip_c(playback_speed_index - 1, 0, 4);
}

double MPState::GetPlayBackSpeed() {
    return playback_speeds[playback_speed_index];
}
