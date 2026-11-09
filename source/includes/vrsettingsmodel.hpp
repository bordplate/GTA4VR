#pragma once
#include <array>
#include <cmath>
#include <optional>

namespace VRSettingsModel
{
    enum class Number : unsigned
    {
        PlayerHeight, VehicleEyeHeight, EyeRenderScale, HudWidth, HudDistance,
        WristSize, WristBack, WristLift, DashboardSize, DashboardForward,
        DashboardDown, DashboardRight, DashboardTilt, BodyForward, TurnAngle, TurnSpeed, MagazineHolsterLeft, MagazineHolsterDown,
        MagazineHolsterForward, MagazineHolsterRadius, Count
    };
    enum class Flag : unsigned { DisableOpenXR, PhysicalCrouch, AmmoCounters, VehiclePanelEditing, HandCalibration, Count };
    struct NumberSpec
    {
        const char* key;
        const wchar_t* label;
        const wchar_t* unit;
        float fallback,minimum,maximum,step; // Menu range only, not INI validation.
        bool positive;
    };
    inline constexpr std::array<NumberSpec,unsigned(Number::Count)> numbers{{
        {"PlayerHeight",L"Standing eye height",L"m",1.75f,.5f,2.5f,.01f,true},
        {"VehicleEyeHeightOffset",L"Seated eye offset",L"m",.075f,-.5f,.5f,.005f,false},
        {"EyeRenderScale",L"Eye resolution scale",L"x",1.f,.25f,1.5f,.01f,true},
        {"ViewHudWidth",L"Text HUD size",L"x",.5f,.1f,1.f,.01f,true},
        {"ViewHudDistance",L"Text HUD distance",L"m",2.f,.5f,5.f,.05f,true},
        {"WristRadarSize",L"Watch diameter",L"m",.10f,.04f,.4f,.005f,true},
        {"WristRadarBackOffset",L"Watch toward elbow",L"m",.075f,-.25f,.5f,.005f,false},
        {"WristRadarLift",L"Watch above wrist",L"m",.04f,-.1f,.3f,.005f,false},
        {"DashboardRadarSize",L"Dashboard diameter",L"m",.16f,.04f,.5f,.005f,true},
        {"DashboardRadarForward",L"Dashboard forward",L"m",.55f,0.f,2.f,.01f,false},
        {"DashboardRadarDown",L"Dashboard down",L"m",.32f,-1.f,1.f,.01f,false},
        {"DashboardRadarRight",L"Dashboard right",L"m",.45f,-1.f,1.f,.01f,false},
        {"DashboardRadarTilt",L"Dashboard tilt",L"deg",12.f,-90.f,90.f,1.f,false},
        {"FirstPersonForwardOffset",L"Body behind eyes",L"m",.08f,-.5f,.5f,.005f,false},
        {"SnapTurnAngle",L"Snap angle",L"deg",45.f,5.f,180.f,5.f,false},
        {"ContinuousTurnSpeed",L"Turn speed",L"deg/s",90.f,10.f,360.f,10.f,false},
        {"MagazineHolsterLeft",L"Hip pickup left",L"m",.25f,-.5f,.75f,.01f,false},
        {"MagazineHolsterDown",L"Hip pickup below head",L"m",.52f,0.f,1.5f,.01f,false},
        {"MagazineHolsterForward",L"Hip pickup forward",L"m",0.f,-.75f,.75f,.01f,false},
        {"MagazineHolsterRadius",L"Hip pickup radius",L"m",.22f,.01f,.6f,.01f,true}
    }};
    inline constexpr std::array<const char*,unsigned(Flag::Count)> flags{
        "DisableOpenXR","ExperimentalPhysicalCrouch","DiegeticAmmoCounters","EditVehiclePanels","HandCalibrationMode"};
    inline bool Valid(Number id,float value)
    {
        const auto index=unsigned(id);
        return index<numbers.size() && std::isfinite(value) && (!numbers[index].positive || value>0);
    }
    inline bool ValidMagazinePickup(int location) {return location>=0 && location<=2;}
    inline bool ValidAimingAid(int mode) {return mode>=0 && mode<=2;}
    inline bool ValidFirstPersonBody(int mode) {return mode>=0 && mode<=2;}
    struct Values
    {
        int magazinePickupLocation=2; // 0 shoulder, 1 left hip, 2 both.
        int aimingAid=0; // 0 none, 1 crosshair, 2 laser sight.
        int turningMode=1; // 0 off, 1 snap, 2 continuous.
        int firstPersonBody=1; // 0 off, 1 full body, 2 hands only.
        std::array<float,unsigned(Number::Count)> values{};
        std::array<bool,unsigned(Flag::Count)> enabled{false,true,true,false,false};
        Values() {for(unsigned i=0;i<values.size();++i)values[i]=numbers[i].fallback;}
        float Get(Number id) const {return values[unsigned(id)];}
        bool Get(Flag id) const {return enabled[unsigned(id)];}
    };
    inline std::optional<float> FloorHeight(float left,float right,bool floor,bool tracked,bool fresh)
    {
        if(!floor || !tracked || !fresh || !std::isfinite(left) || !std::isfinite(right))return {};
        const float height=left*.5f+right*.5f;
        return std::isfinite(height) && height>0 ? std::optional<float>{height}:std::nullopt;
    }
}
