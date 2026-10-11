#pragma once

// Shared by the kernel and portable regression tests. A listed executable
// stays in the tunnel in inclusion mode, regardless of its parent's policy.
namespace inclusionpolicy
{
constexpr bool SplitByConfig(bool onlyVpnApps, bool listed)
{
    return onlyVpnApps ? !listed : listed;
}

constexpr bool InheritBypass(bool onlyVpnApps)
{
    return !onlyVpnApps;
}
}
