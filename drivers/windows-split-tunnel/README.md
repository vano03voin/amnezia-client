# Windows application VPN inclusion mode

This directory contains the Mullvad split tunnel driver sources pinned to
`mullvad/win-split-tunnel@198ecde6e8cf547a01810b9ae87cefba562f7f99`.
That is the source revision referenced by the binary repository commit used
by AmneziaVPN 5.0.3.0. Original license files are retained here.

The fork adds `IOCTL_ST_SET_INCLUSION_CONFIGURATION` (function 12). It uses
the same validated image-path buffer as the exclusion command (function 6),
but the list specifies executables that stay inside the VPN. All other
executables bypass it. Function 13 returns a UINT32 capability bitmap; bit 0
advertises inclusion support. An unmodified driver rejects both commands.

The policy is stored in the registered-image context and published under
the existing driver state lock, together with the list. Both process
discovery and process-arrival callbacks use the same policy. Existing WFP
redirection, process discovery, unknown-process protection and transactional
filter updates remain in use. Inclusion mode matches executable paths
without inheriting the bypass policy of an unlisted parent. Helper
executables need their own entries. Original exclusion inheritance remains.
An empty inclusion list bypasses all applications; clearing the configuration
resets the mode and disables splitting.

## Build and packaging

Run from a VS 2022 x64 developer command prompt with NuGet available:

```bat
nuget restore packages.config -PackagesDirectory packages
cd src
msbuild mullvad-split-tunnel.sln /m /p:Configuration=Release /p:Platform=x64 /p:SignMode=Off
```

The matching SDK and WDK are pinned to 10.0.26100.1. CI builds the driver
without signing and uploads it separately. **This is a development artifact,
not an installable production driver.** The upstream Mullvad signature does
not apply to this modified binary. A production distribution requires the
modified driver to pass Microsoft's signing process and runtime validation.
No security settings are changed by these build scripts.

After obtaining a signed package, point the client CMake configuration at
the directory containing `.sys`, `.inf`, and `.cat`:

```text
-DAMNEZIA_SPLIT_TUNNEL_DRIVER_DIR=C:/path/to/signed-driver-package
```

Without this override, client builds package the original signed driver.
Exclusion mode works as before, but inclusion connections are rejected.
The ordinary CI client artifact uses that original driver and is intended
to validate compilation, not provide a working inclusion installation.

## Limits and runtime acceptance tests

Use an isolated Windows test machine equipped to load development drivers.
Do not replace the production installation to perform initial testing.

Disable site splitting for these tests. Select a browser executable for
VPN inclusion and keep a second browser unlisted. Verify their public IPv4
and IPv6 addresses independently, with TCP and UDP/QUIC connections. Repeat
with both browsers already running before connection and started afterward.
Verify that launching the selected browser from Explorer keeps it inside
the VPN. Test helper executables explicitly added and omitted. Repeat with
AmneziaWG, WireGuard, OpenVPN and Xray, KillSwitch on and off, reconnect,
sleep/wake, network changes, an empty list and switching back to exclusion.
Verify cleanup on disconnect and on a deliberately rejected driver command.
These runtime tests are required before claiming that the feature is ready
for regular use; the portable policy tests do not replace them.

Windows system DNS resolution is shared between applications. This fork
preserves native DNS in inclusion mode; DNS requests made by the Windows DNS
Client service may go directly. Browser Secure DNS/DoH runs in the browser
and follows its application policy. This is not per-application DNS isolation.
Private VPN-only DNS should not be configured as the native Windows resolver.
Independent site rules still affect routing to their destinations.
