#include "../src/inclusionpolicy.h"
#include <cassert>

struct Process { bool listed; int parent; };
bool bypass(const Process* processes, int index, bool onlyVpnApps)
{
    if (inclusionpolicy::SplitByConfig(onlyVpnApps, processes[index].listed)) return true;
    return inclusionpolicy::InheritBypass(onlyVpnApps) && processes[index].parent >= 0
        && bypass(processes, processes[index].parent, onlyVpnApps);
}

int main()
{
    // Explorer -> selected browser -> unlisted helper, plus an unrelated app.
    const Process tree[] = {{false, -1}, {true, 0}, {false, 1}, {false, 0}};
    assert(bypass(tree, 0, true));
    assert(!bypass(tree, 1, true));
    assert(bypass(tree, 2, true));
    assert(bypass(tree, 3, true));
    // Original exclusion mode preserves child-process inheritance.
    assert(!bypass(tree, 0, false));
    assert(bypass(tree, 1, false));
    assert(bypass(tree, 2, false));
    assert(!bypass(tree, 3, false));
    // Selected children never inherit bypass from an unlisted parent.
    const Process selectedChild[] = {{false, -1}, {true, 0}, {true, 1}};
    assert(!bypass(selectedChild, 1, true));
    assert(!bypass(selectedChild, 2, true));
    // Empty inclusion list bypasses everything, empty exclusion list bypasses nothing.
    const Process empty[] = {{false, -1}, {false, 0}};
    assert(bypass(empty, 0, true) && bypass(empty, 1, true));
    assert(!bypass(empty, 0, false) && !bypass(empty, 1, false));
    // Switching modes updates policy for an already running process tree.
    assert(bypass(tree, 1, false) != bypass(tree, 1, true));
}
