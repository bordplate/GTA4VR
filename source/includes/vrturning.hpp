#pragma once
#include "vrmath.hpp"
#include <cstdint>

namespace VRTurning {
enum class Mode {Off,Snap,Continuous};
inline bool ValidMode(int mode) {return mode>=0 && mode<=2;}
struct State {
    uint64_t at=0;
    int previousMode=-1;
    bool armed=false;
    float Sample(int mode,float x,float y,bool allowed,uint64_t time,float angle,float speed) {
        if(!allowed || !ValidMode(mode) || mode==0 || !std::isfinite(x+y) ||
            !std::isfinite(angle) || !std::isfinite(speed)) {at=0;armed=false;previousMode=mode;return 0;}
        if(time<=at)return 0; // Camera preparation can run more than once per XR sample.
        const auto elapsed=at ? time-at:0;
        const bool reset=!at || elapsed>250 || previousMode!=mode;
        at=time;previousMode=mode;
        if(reset)armed=false;
        if(std::abs(x)<=.25f)armed=true;
        if(!armed || std::abs(x)<=std::abs(y))return 0;
        if(mode==int(Mode::Snap)) {
            if(std::abs(x)<.7f)return 0;
            armed=false;
            return float(std::remainder((x>0 ? 1.0:-1.0)*angle,360.0));
        }
        if(reset || std::abs(x)<=.25f)return 0;
        const float axis=std::copysign((std::min(std::abs(x),1.f)-.25f)/.75f,x);
        return float(std::remainder(double(axis)*speed*double(std::min<uint64_t>(elapsed,100))/1000.0,360.0));
    }
};
// Rotate the tracking origin around the current head, not around the floor origin.
// Positive input turns right; OpenXR's positive Y rotation turns left.
inline VRMath::Matrix AroundHead(const VRMath::Matrix& reference,const float (&head)[3],float degrees) {
    const auto pivot=VRMath::Transform({head[0],head[1],head[2],1},reference);
    const double half=-std::remainder(double(degrees),360.0)*3.14159265358979323846/360.0;
    const float origin[3]{},q[4]{0,float(std::sin(half)),0,float(std::cos(half))};
    auto rotation=VRMath::Pose(origin,q);
    const auto rotated=VRMath::Transform(pivot,rotation);
    for(unsigned i=0;i<3;++i)rotation[3][i]=pivot[i]-rotated[i];
    return VRMath::Multiply(reference,rotation);
}
}
