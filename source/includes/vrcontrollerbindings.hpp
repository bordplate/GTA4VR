#pragma once
#include <string_view>
#include <cmath>

namespace VRControllerBindings {
enum class Profile {Vive,Touch,Index,Motion};
inline unsigned Hand(Profile profile,std::string_view name,unsigned original) {
    if(profile==Profile::Touch && (name=="next_weapon" || name=="previous_weapon"))return 0;
    return original;
}
// A bound click retains click-to-navigate semantics. Without one, the axis
// navigates directly; never synthesize a centre click that could confirm.
inline bool NavigationClick(bool pressed,bool active,float x,float y) {
    return pressed || (!active && std::hypot(x,y)>=.5f);
}
inline const char* Path(Profile profile) {
    switch(profile) {
    case Profile::Touch:return "/interaction_profiles/oculus/touch_controller";
    case Profile::Index:return "/interaction_profiles/valve/index_controller";
    case Profile::Motion:return "/interaction_profiles/microsoft/motion_controller";
    default:return "/interaction_profiles/htc/vive_controller";
    }
}
inline std::string_view Component(Profile profile,std::string_view name,unsigned hand,
    std::string_view original,std::string_view group) {
    if(profile==Profile::Vive)return original;
    if(profile==Profile::Motion)return original=="/input/trigger/click" ? "/input/trigger/value":original;
    if(profile==Profile::Index) {
        if(original=="/input/trackpad")return "/input/thumbstick";
        if(original=="/input/trackpad/touch")return "/input/thumbstick/touch";
        if(original=="/input/trackpad/click")return "/input/thumbstick/click";
    }
    if(profile==Profile::Touch) {
        if(name=="cycle_weapon" || name=="weapon_direction" || name=="phone_click" || name=="navigate_click")return {};
        if(name=="previous_weapon")return group=="driving" ? std::string_view{}:"/input/x/click";
        if(name=="next_weapon")return group=="driving" ? "/input/x/click":"/input/y/click";
        if(name=="horn")return "/input/thumbstick/click";
        if(name=="move_enable" || name=="steer_enable" || name=="turn_enable")return {}; // Stick deflection works without touch sensing.
        if(name=="confirm" || name=="phone_accept")return "/input/a/click";
        if(name=="jump" || name=="handbrake")return "/input/a/click";
        if(name=="context_interact" && group=="driving")return "/input/y/click";
        if(original=="/input/trackpad")return "/input/thumbstick";
        if(original=="/input/trackpad/touch")return "/input/thumbstick/touch";
        if(original=="/input/trackpad/click")return "/input/thumbstick/click";
        if(original=="/input/trigger/click")return "/input/trigger/value";
    }
    if(original=="/input/squeeze/click")return "/input/squeeze/value";
    if(original=="/input/menu/click") {
        if(profile==Profile::Touch && !hand)return original;
        return "/input/b/click";
    }
    return original;
}
}
