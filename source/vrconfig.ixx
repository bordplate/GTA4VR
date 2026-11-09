module;
#include <common.hxx>
#include "includes/vrsettingsmodel.hpp"
#include "includes/vrturning.hpp"
#include <atomic>
export module vrconfig;

export namespace VRConfig
{
    using Number=VRSettingsModel::Number;
    using Flag=VRSettingsModel::Flag;
    using Values=VRSettingsModel::Values;
    void Initialize();
    unsigned Revision();
    Values Read();
    bool Set(Number id,float value);
    bool Set(Flag id,bool value);
    bool SetTurningMode(int mode);
    bool SetAimingAid(int mode);
    bool SetMagazinePickupLocation(int location);
    bool SetFirstPersonBody(int mode);
}
namespace
{
    SRWLOCK configLock=SRWLOCK_INIT;
    VRConfig::Values configured;
    std::atomic<unsigned> revision{0};
}
void VRConfig::Initialize()
{
    CIniReader ini("");
    AcquireSRWLockExclusive(&configLock);
    configured.magazinePickupLocation=ini.ReadInteger("VR","MagazinePickupLocation",2);
    if(!VRSettingsModel::ValidMagazinePickup(configured.magazinePickupLocation))configured.magazinePickupLocation=2;
    configured.aimingAid=ini.ReadInteger("VR","AimingAid",0);
    if(!VRSettingsModel::ValidAimingAid(configured.aimingAid))configured.aimingAid=0;
    configured.turningMode=ini.ReadInteger("VR","TurningMode",1);
    if(!VRTurning::ValidMode(configured.turningMode))configured.turningMode=1;
    configured.firstPersonBody=ini.ReadInteger("VR","ShowFirstPersonBody",1);
    if(!VRSettingsModel::ValidFirstPersonBody(configured.firstPersonBody))configured.firstPersonBody=1;
    for(unsigned i=0;i<configured.values.size();++i)
    {
        const auto& spec=VRSettingsModel::numbers[i];
        const float value=ini.ReadFloat("VR",spec.key,spec.fallback);
        configured.values[i]=VRSettingsModel::Valid(Number(i),value) ? value:spec.fallback;
    }
    for(unsigned i=0;i<configured.enabled.size();++i)
        configured.enabled[i]=i==unsigned(Flag::HandCalibration) ? false:ini.ReadInteger("VR",VRSettingsModel::flags[i],configured.enabled[i])!=0;
    revision.fetch_add(1,std::memory_order_release);
    ReleaseSRWLockExclusive(&configLock);
}
unsigned VRConfig::Revision() {return revision.load(std::memory_order_acquire);}
VRConfig::Values VRConfig::Read()
{
    AcquireSRWLockShared(&configLock);auto result=configured;ReleaseSRWLockShared(&configLock);return result;
}
bool VRConfig::Set(Number id,float value)
{
    if(!VRSettingsModel::Valid(id,value))return false;
    AcquireSRWLockExclusive(&configLock);
    try
    {
        const auto key=VRSettingsModel::numbers[unsigned(id)].key;
        CIniReader ini("");ini.WriteString("VR",key,std::format("{:.9g}",value),true);
        if(CIniReader("").ReadFloat("VR",key,NAN)!=value)
        {ReleaseSRWLockExclusive(&configLock);return false;}
        configured.values[unsigned(id)]=value;revision.fetch_add(1,std::memory_order_release);
    }
    catch(...) {ReleaseSRWLockExclusive(&configLock);return false;}
    ReleaseSRWLockExclusive(&configLock);return true;
}
bool VRConfig::Set(Flag id,bool value)
{
    if(unsigned(id)>=VRSettingsModel::flags.size())return false;
    AcquireSRWLockExclusive(&configLock);
    try
    {
        const auto key=VRSettingsModel::flags[unsigned(id)];
        CIniReader ini("");
        if(id!=Flag::HandCalibration)ini.WriteInteger("VR",key,value ? 1:0,true);
        if(id!=Flag::HandCalibration && CIniReader("").ReadInteger("VR",key,-1)!=(value ? 1:0))
        {ReleaseSRWLockExclusive(&configLock);return false;}
        configured.enabled[unsigned(id)]=value;revision.fetch_add(1,std::memory_order_release);
    }
    catch(...) {ReleaseSRWLockExclusive(&configLock);return false;}
    ReleaseSRWLockExclusive(&configLock);return true;
}

bool VRConfig::SetTurningMode(int mode)
{
    if(!VRTurning::ValidMode(mode))return false;
    AcquireSRWLockExclusive(&configLock);
    try {
        CIniReader ini("");ini.WriteInteger("VR","TurningMode",mode,true);
        if(CIniReader("").ReadInteger("VR","TurningMode",-1)!=mode)
        {ReleaseSRWLockExclusive(&configLock);return false;}
        configured.turningMode=mode;revision.fetch_add(1,std::memory_order_release);
    } catch(...) {ReleaseSRWLockExclusive(&configLock);return false;}
    ReleaseSRWLockExclusive(&configLock);return true;
}

bool VRConfig::SetAimingAid(int mode)
{
    if(!VRSettingsModel::ValidAimingAid(mode))return false;
    AcquireSRWLockExclusive(&configLock);
    try {
        CIniReader ini("");ini.WriteInteger("VR","AimingAid",mode,true);
        if(CIniReader("").ReadInteger("VR","AimingAid",-1)!=mode)
        {ReleaseSRWLockExclusive(&configLock);return false;}
        configured.aimingAid=mode;revision.fetch_add(1,std::memory_order_release);
    } catch(...) {ReleaseSRWLockExclusive(&configLock);return false;}
    ReleaseSRWLockExclusive(&configLock);return true;
}

bool VRConfig::SetMagazinePickupLocation(int location)
{
    if(!VRSettingsModel::ValidMagazinePickup(location))return false;
    AcquireSRWLockExclusive(&configLock);
    try {
        CIniReader ini("");ini.WriteInteger("VR","MagazinePickupLocation",location,true);
        if(CIniReader("").ReadInteger("VR","MagazinePickupLocation",-1)!=location)
        {ReleaseSRWLockExclusive(&configLock);return false;}
        configured.magazinePickupLocation=location;revision.fetch_add(1,std::memory_order_release);
    } catch(...) {ReleaseSRWLockExclusive(&configLock);return false;}
    ReleaseSRWLockExclusive(&configLock);return true;
}

bool VRConfig::SetFirstPersonBody(int mode)
{
    if(!VRSettingsModel::ValidFirstPersonBody(mode))return false;
    AcquireSRWLockExclusive(&configLock);
    try {
        CIniReader ini("");ini.WriteInteger("VR","ShowFirstPersonBody",mode,true);
        if(CIniReader("").ReadInteger("VR","ShowFirstPersonBody",-1)!=mode)
        {ReleaseSRWLockExclusive(&configLock);return false;}
        configured.firstPersonBody=mode;revision.fetch_add(1,std::memory_order_release);
    } catch(...) {ReleaseSRWLockExclusive(&configLock);return false;}
    ReleaseSRWLockExclusive(&configLock);return true;
}
