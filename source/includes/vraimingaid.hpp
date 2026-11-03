#pragma once
#include "vrammocounter.hpp"

namespace VRAimingAid {
enum class Mode {None,Crosshair,Laser};
struct Snapshot {
    VRMath::Vector muzzle{},target{};
    int mode=0;
    bool valid=false;
    bool ped=false;
    unsigned health=0,armour=0;
};
// Native CE 5C87B0 quantizes health/armour into eight HUD segments.
inline unsigned Segments(float percent) {
    if(!std::isfinite(percent) || percent<=0)return 0;
    for(unsigned i=1;i<8;++i)if(percent<=14.f*i+(i==7 ? 1.f:0.f))return i;
    return 8;
}
struct ReticleVertex {float x,y,z;unsigned color;float u,v;};
inline bool Finite(const VRMath::Vector& v) {
    return std::isfinite(v[0]) && std::isfinite(v[1]) && std::isfinite(v[2]);
}
inline std::vector<VRAmmoCounter::Vertex> Geometry(const Snapshot& aim,const VRMath::Matrix& eye) {
    using VRMath::Vector;
    std::vector<VRAmmoCounter::Vertex> vertices;
    if(!aim.valid || aim.mode<1 || aim.mode>2 || !Finite(aim.muzzle) || !Finite(aim.target))return vertices;
    const auto towardEye=VRMath::Difference3(eye[3],aim.target);
    const float distance=VRMath::Length3(towardEye);
    const auto ray=VRMath::Difference3(aim.target,aim.muzzle);
    const float length=VRMath::Length3(ray);
    if(!std::isfinite(distance+length) || distance<.02f || length<.005f)return vertices;
    auto quad=[&](const Vector& center,const Vector& right,const Vector& up,unsigned color) {
        VRAmmoCounter::Vertex corners[4]{};
        for(unsigned i=0;i<4;++i) {
            const float x=(i&1) ? 1.f:-1.f,y=(i&2) ? 1.f:-1.f;
            corners[i]={center[0]+right[0]*x+up[0]*y,center[1]+right[1]*x+up[1]*y,
                center[2]+right[2]*x+up[2]*y,color};
        }
        for(unsigned index:{0u,1u,2u,2u,1u,3u})vertices.push_back(corners[index]);
    };
    auto scaled=[](Vector value,float scale) {for(unsigned a=0;a<3;++a)value[a]*=scale;value[3]=0;return value;};
    if(aim.mode==int(Mode::Laser)) {
        const auto middle=VRMath::Vector{(aim.muzzle[0]+aim.target[0])*.5f,
            (aim.muzzle[1]+aim.target[1])*.5f,(aim.muzzle[2]+aim.target[2])*.5f,1};
        auto side=VRMath::Cross3(ray,VRMath::Difference3(eye[3],middle));
        float sideLength=VRMath::Length3(side);
        if(sideLength<1.e-5f) {side=VRMath::Cross3(ray,eye[1]);sideLength=VRMath::Length3(side);}
        if(sideLength<1.e-5f) {side=VRMath::Cross3(ray,eye[0]);sideLength=VRMath::Length3(side);}
        if(sideLength>1.e-5f)quad(middle,scaled(ray,.5f),scaled(side,.0015f/sideLength),0xFFFF3030);
    }
    // Lift the marker toward the eye to avoid coplanar depth fighting at a hit.
    // Both eyes retain the same world target; this offset is along their sight ray.
    auto center=aim.target;
    for(unsigned a=0;a<3;++a)center[a]+=towardEye[a]*(std::min(.02f,distance*.001f)/distance);
    const auto right=eye[0],up=eye[1];
    if(aim.mode==int(Mode::Crosshair)) {
        // Native centre dot; the textured circle/health ring draw separately.
        quad(center,scaled(right,distance*.0006f),scaled(up,distance*.0006f),0xFFE1E1E1);
    } else {
        quad(center,scaled(right,distance*.0025f),scaled(up,distance*.0025f),0xFF101010);
        quad(center,scaled(right,distance*.0015f),scaled(up,distance*.0015f),0xFFFF3030);
    }
    return vertices;
}
inline std::vector<ReticleVertex> ReticleGeometry(const Snapshot& aim,const VRMath::Matrix& eye,bool healthRing=false,bool armourRing=false,unsigned nativeColor=0) {
    std::vector<ReticleVertex> out;
    if(!aim.valid || aim.mode!=int(Mode::Crosshair) || !Finite(aim.target) || !Finite(aim.muzzle))return out;
    const auto towardEye=VRMath::Difference3(eye[3],aim.target);
    const float distance=VRMath::Length3(towardEye);
    if(!std::isfinite(distance) || distance<.02f || VRMath::Length3(VRMath::Difference3(aim.target,aim.muzzle))<.005f)return out;
    auto center=aim.target;
    for(unsigned a=0;a<3;++a)center[a]+=towardEye[a]*(std::min(.02f,distance*.001f)/distance);
    const bool ring=healthRing||armourRing;
    const unsigned segments=ring ? (aim.ped ? (armourRing ? aim.armour:aim.health):0):8;
    const float radius=distance*.012f*(armourRing ? .7f:(ring ? .97f:1.f));
    const unsigned color=nativeColor ? nativeColor:(armourRing ? 0xFFCEC997:(ring ? 0xFFA6C99F:0xFFE1E1E1));
    auto vertex=[&](float x,float y,float u,float v) {
        return ReticleVertex{center[0]+radius*(eye[0][0]*x-eye[1][0]*y),
            center[1]+radius*(eye[0][1]*x-eye[1][1]*y),center[2]+radius*(eye[0][2]*x-eye[1][2]*y),color,u,v};
    };
    // Native hud_crosshair is mirrored into four quarters. hud_target uses
    // rotated quarters, with a triangle for each remaining half-quarter.
    for(unsigned q=0;q<4;++q) {
        if(ring && segments<=2*q)break;
        ReticleVertex v[4];
        // 8D4040 consumes a triangle strip for the circle; 8FDB00 emits
        // a centre-first triangle fan for health. Both textures place the
        // quarter-circle centre at their bottom-right corner.
        const float circleXY[4][2]={{0,-1},{0,0},{1,0},{1,-1}};
        const float circleUV[4][2]={{.98f,.02f},{.98f,.98f},{.02f,.98f},{.02f,.02f}};
        const float ringXY[4][2]={{0,0},{0,-1},{1,-1},{1,0}};
        const float ringUV[4][2]={{.98f,.98f},{.02f,.98f},{.02f,.02f},{.98f,.02f}};
        const auto& xy=ring ? ringXY:circleXY;
        const auto& uv=ring ? ringUV:circleUV;
        for(unsigned i=0;i<4;++i) {
            float x=xy[i][0],y=xy[i][1];
            if(ring) {for(unsigned r=0;r<(3-q);++r){const float old=x;x=-y;y=old;}}
            else {if(q==1 || q==2)y=-y;if(q==2 || q==3)x=-x;}
            v[i]=vertex(x,y,uv[i][0],uv[i][1]);
        }
        if(!ring || segments>=2*q+2)for(unsigned i:{0u,1u,2u})out.push_back(v[i]);
        for(unsigned i:{0u,2u,3u})out.push_back(v[i]);
    }
    return out;
}
}
