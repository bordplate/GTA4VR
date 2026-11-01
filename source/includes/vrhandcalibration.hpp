#pragma once
#include "vrmath.hpp"
#include <string_view>
#include <optional>

namespace VRHandCalibration {
inline bool IsTouch(std::string_view profile) {
    return profile=="/interaction_profiles/oculus/touch_controller" ||
        profile=="/interaction_profiles/meta/touch_plus_controller" ||
        profile=="/interaction_profiles/facebook/touch_controller_pro";
}
inline VRMath::Matrix Default(bool touch,unsigned hand) {
    if(!touch)return VRMath::Identity();
    const float origin[3]={};
    const float left[4]={0.5f,0.5f,-0.5f,0.5f};
    const float right[4]={-0.5f,0.5f,0.5f,0.5f};
    return VRMath::Pose(origin,hand==0 ? left:right);
}
// Row-vector transforms: correction * controller basis = desired wrist.
inline VRMath::Matrix Fit(const VRMath::Matrix& frozen,const VRMath::Matrix& basis) {
    return VRMath::Multiply(frozen,VRMath::InverseRigid(basis));
}
struct Freeze {
    bool held=false,armed=false;
    VRMath::Matrix frozen=VRMath::Identity();
    void Cancel() {held=armed=false;}
    std::optional<VRMath::Matrix> Update(bool enabled,bool valid,bool grip,
        const VRMath::Matrix& basis,VRMath::Matrix& target) {
        if(!enabled || !valid) {Cancel();return {};}
        // Require a release after enabling, tracking loss or a profile switch.
        if(!grip && !held)armed=true;
        if(grip && armed && !held) {held=true;frozen=target;}
        if(!held)return {};
        const auto correction=Fit(frozen,basis);
        target=frozen;
        if(grip)return {};
        held=false;armed=true;
        return correction;
    }
};
}
