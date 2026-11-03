#include "../../source/includes/vraimingaid.hpp"
#include "../../source/includes/vrsettingsmodel.hpp"
#include <cassert>
#include <limits>
int main() {
    using namespace VRAimingAid;
    auto eye=VRMath::Identity();
    Snapshot aim{{0,0,-.5f,1},{0,0,-10,1},0,true};
    assert(VRSettingsModel::Values{}.aimingAid==0);
    assert(VRSettingsModel::ValidAimingAid(0) && VRSettingsModel::ValidAimingAid(2));
    assert(!VRSettingsModel::ValidAimingAid(-1) && !VRSettingsModel::ValidAimingAid(3));
    assert(Geometry(aim,eye).empty());
    aim.mode=1;
    const auto crosshair=Geometry(aim,eye);
    assert(crosshair.size()==6);
    assert(ReticleGeometry(aim,eye).size()==24);
    assert(ReticleGeometry(aim,eye,true).empty());
    aim.ped=true;aim.health=3;aim.armour=8;
    assert(ReticleGeometry(aim,eye,true).size()==9);
    assert(ReticleGeometry(aim,eye,false,true).size()==24);
    assert(Segments(0)==0 && Segments(14)==1 && Segments(15)==2);
    assert(Segments(98)==7 && Segments(99)==7 && Segments(100)==8);
    assert(Segments(std::numeric_limits<float>::quiet_NaN())==0);
    // Native thresholds must remove exactly one half-quarter at each step.
    for(unsigned segments=0;segments<=8;++segments) {
        aim.health=segments;
        const auto ring=ReticleGeometry(aim,eye,true);
        assert(ring.size()==segments*3);
        for(const auto& v:ring)assert(std::isfinite(v.x+v.y+v.z+v.u+v.v));
    }
    // Each native quarter must meet at the same world centre using the
    // artwork's own centre corner: bottom-right for both native textures.
    for(bool ring:{false,true}) {
        aim.health=8;
        const auto mesh=ReticleGeometry(aim,eye,ring);
        unsigned centers=0;
        for(const auto& v:mesh)if(std::abs(v.x)<1.e-6f && std::abs(v.y)<1.e-6f) {
            assert(std::abs(v.u-.98f)<1.e-6f);
            assert(std::abs(v.v-.98f)<1.e-6f);
            ++centers;
        }
        assert(centers>=4);
        // A quarter-arc's distance from its artwork centre must equal its
        // distance from the reticle centre, including between vertices.
        const float radius=.12f*(ring ? .97f:1.f);
        for(size_t i=0;i<mesh.size();i+=3) {
            for(unsigned sample=0;sample<4;++sample) {
                float x=0,y=0,u=0,v=0;
                for(unsigned j=0;j<3;++j) {
                    const float weight=sample==3 ? 1.f/3.f:(sample==j ? 1.f:0.f);
                    x+=mesh[i+j].x*weight;y+=mesh[i+j].y*weight;
                    u+=mesh[i+j].u*weight;v+=mesh[i+j].v*weight;
                }
                const float textureRadius2=((.98f-u)*(.98f-u)+(.98f-v)*(.98f-v))/(.96f*.96f);
                assert(std::abs((x*x+y*y)/(radius*radius)-textureRadius2)<1.e-5f);
            }
        }
    }
    aim.ped=false;assert(ReticleGeometry(aim,eye,true).empty());
    // Both modes handle a view looking exactly down the firing ray.
    aim.mode=2;
    const auto laser=Geometry(aim,eye);
    assert(laser.size()==18);
    for(const auto& vertex:laser)assert(std::isfinite(vertex.x+vertex.y+vertex.z));
    assert(laser[0].z>=aim.target[2] && laser[0].z<=aim.muzzle[2]);
    auto otherEye=eye;otherEye[3][0]=.064f;
    const auto other=Geometry(aim,otherEye);
    assert(other.size()==laser.size());
    for(const auto& vertex:other)assert(std::isfinite(vertex.x+vertex.y+vertex.z));
    aim.valid=false;assert(Geometry(aim,eye).empty());
    aim.valid=true;aim.target=aim.muzzle;assert(Geometry(aim,eye).empty());
    aim.target[0]=std::numeric_limits<float>::quiet_NaN();assert(Geometry(aim,eye).empty());
}
