#include "../../source/includes/vrbindingconfig.hpp"
#include <cassert>
#include <iostream>

int main() {
    using namespace VRBindingConfig;
    const std::vector<std::string> fallback{"/user/hand/right/input/a/click"};
    std::istringstream input("\xEF\xBB\xBF[On_Foot]\r\nconfirm = /user/hand/left/input/x/click, /user/hand/right/input/a/click ; comment\n"
        "jump=none\nfire=\nturn=not-a-path\nunused=none\nconfirm=none\n[broken\nconfirm=none\n");
    auto file=File::Parse(input);
    const auto mapped=file.Resolve("on_foot","confirm",fallback);
    assert(mapped.size()==2 && mapped[0]=="/user/hand/left/input/x/click");
    assert(file.Resolve("on_foot","jump",fallback).empty());
    assert(file.Resolve("on_foot","fire",fallback).empty());
    assert(file.Resolve("on_foot","turn",fallback)==fallback);
    assert(file.Resolve("on_foot","missing",fallback)==fallback);
    file.CheckUnused();
    assert(file.errors.size()==5); // duplicate, bad section, orphan, bad path, unknown key
    assert(!Paths("/user/hand/right/input/a/click,"));
    assert(!Paths("/user/hand/right/input/a/click",0));
    assert(!Paths("/user/hand/left/output/haptic"));
    assert(!Paths("/user/hand/left/input//click"));
    assert(Paths("NONE")->empty());
    assert(Paths("/user/hand/right/input/a/click,/user/hand/right/input/a/click")->size()==1);
    const auto missing=File::Load("build/nonexistent-controller-bindings.ini");
    assert(missing.values.empty() && missing.errors.empty());
    std::cout<<"Binding parsing, overrides, unbinding, hand ownership and fallback checks passed\n";
}
