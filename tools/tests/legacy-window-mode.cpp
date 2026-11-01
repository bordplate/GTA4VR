#include "../../source/includes/legacywindowmode.hpp"
#include <cassert>
int main() {
    using LegacyWindowMode::Import;
    assert(Import(-1,1,1)==2); // Existing VR borderless cfg must survive the schema upgrade.
    assert(Import(-1,1,0)==1);
    assert(Import(-1,0,1)==0);
    assert(Import(-1,0,0)==0);
    assert(Import(-1,1,-1)==2); // Legacy borderless default was enabled.
    assert(!Import(0,1,1)); // An explicit new choice must not be overwritten by stale keys.
    assert(!Import(1,0,0));
    assert(!Import(2,0,0));
    assert(!Import(-1,-1,-1)); // Absent legacy settings leave upstream defaults alone.
}
