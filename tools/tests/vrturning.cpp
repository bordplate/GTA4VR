#include "../../source/includes/vrturning.hpp"
#include <cassert>
#include <limits>
using namespace VRTurning;
bool Near(float a,float b) {return std::abs(a-b)<.0001f;}
int main() {
    State snap;
    assert(snap.Sample(1,0,0,true,100,45,90)==0);
    assert(snap.Sample(1,1,0,true,110,45,90)==45);
    assert(snap.Sample(1,1,0,true,110,45,90)==0);
    assert(snap.Sample(1,1,0,true,120,45,90)==0);
    assert(snap.Sample(1,-1,0,true,130,45,90)==0); // Opposite direction alone doesn't rearm.
    assert(snap.Sample(1,0,0,true,140,45,90)==0);
    assert(snap.Sample(1,-1,0,true,150,45,90)==-45);
    assert(snap.Sample(1,0,1,true,160,45,90)==0);
    assert(snap.Sample(1,.8f,1,true,170,45,90)==0); // Vertical intent doesn't turn.
    assert(snap.Sample(1,1,0,false,180,45,90)==0); // Menu/phone/tracking loss.
    assert(snap.Sample(1,1,0,true,190,45,90)==0); // Held axis cannot turn on resumption.
    assert(snap.Sample(1,0,0,true,200,45,90)==0);
    assert(snap.Sample(1,1,0,true,210,30,90)==30); // Live angle update.
    for(unsigned hz:{30,60,90,120}) {
        State continuous;float total=0;
        continuous.Sample(2,0,0,true,100,45,90);
        for(unsigned i=1;i<=hz;++i)total+=continuous.Sample(2,1,0,true,100+(1000*i)/hz,45,90);
        assert(Near(total,90));
    }
    State continuous;
    continuous.Sample(2,0,0,true,100,45,90);
    assert(continuous.Sample(2,.2f,0,true,110,45,90)==0);
    assert(Near(continuous.Sample(2,-1,0,true,120,45,90),-.9f));
    assert(continuous.Sample(2,1,0,true,1000,45,90)==0); // No catch-up spin after a stall.
    assert(continuous.Sample(0,1,0,true,1010,45,90)==0);
    assert(continuous.Sample(2,1,0,true,1020,45,90)==0);
    assert(continuous.Sample(2,std::numeric_limits<float>::quiet_NaN(),0,true,1030,45,90)==0);
    const float origin[3]{},rotation[4]{0,.38268343f,0,.92387953f};
    const float head[3]{2.3f,1.7f,-1.2f};
    auto reference=VRMath::Pose(origin,rotation);reference[3]={-1,0,3,1};
    const auto before=VRMath::Transform({head[0],head[1],head[2],1},reference);
    const auto turned=AroundHead(reference,head,45);
    const auto after=VRMath::Transform({head[0],head[1],head[2],1},turned);
    for(unsigned i=0;i<4;++i)assert(Near(before[i],after[i]));
    const auto right=VRMath::Transform({0,0,-1,0},AroundHead(VRMath::Identity(),origin,90));
    assert(Near(right[0],1) && Near(right[1],0) && Near(right[2],0));
    const auto roundtrip=AroundHead(turned,head,-45);
    for(unsigned r=0;r<4;++r)for(unsigned c=0;c<4;++c)assert(Near(roundtrip[r][c],reference[r][c]));
}
