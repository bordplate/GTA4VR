#pragma once
#include <optional>

namespace LegacyWindowMode {
// Existing unified settings win. Legacy booleans map to upstream's named modes:
// fullscreen=0, ordinary window=1, borderless window=2.
inline std::optional<int> Import(int current,int windowed,int borderless) {
    if(current>=0 && current<=2)return {};
    if(windowed==0)return 0;
    if(windowed==1)return borderless==0 ? 1:2;
    return {};
}
}
