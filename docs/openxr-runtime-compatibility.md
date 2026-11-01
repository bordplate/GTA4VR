# OpenXR runtime compatibility

Tracked under [#27](https://github.com/bordplate/GTA4VR/issues/27), roadmap stage 7.

## Runtime selection

GTA4VR loads the bundled x86 OpenXR loader. The loader selects the configured
runtime, including a process-level `XR_RUNTIME_JSON` override when present.
The mod does not enumerate runtime processes, launch SteamVR explicitly, change
the active runtime registration or maintain a list of runtime vendors.
Select the runtime using its own setup tools. GTA IV is a 32-bit application:
a working 64-bit OpenXR installation alone does not establish x86 availability.

The runtime must provide a connected headset and the selected graphics binding.
The DXVK path requires `XR_KHR_vulkan_enable2`; the system D3D9Ex bridge requires
`XR_KHR_D3D11_enable`. Unsupported initialization retains flat-screen gameplay.
The Quest still runs the Windows game through PC streaming; this is not a native
standalone Quest build. VDXR is a separate path from Virtual Desktop + SteamVR.

## Runtime-owned Vulkan creation

Before DXVK creates graphics objects, the mod creates an OpenXR instance/system
and queries `xrGetVulkanGraphicsRequirements2KHR`. A small process-local adapter
intercepts the Vulkan loader's creation-function lookup. Only callers from DXVK
(scoped to the D3D9 provider module using its public factory exports) route Vulkan instance
and device creation through `xrCreateVulkanInstanceKHR` and
`xrCreateVulkanDeviceKHR`. Original Vulkan create infos, feature chains,
extensions, allocation callbacks and queue requests are passed to the runtime.
The runtime adds its own requirements; there are no runtime-name branches.

The runtime's graphics-device query must agree with DXVK's chosen GPU. A mismatch
or failed runtime creation retains native graphics creation for flat-screen play
and prevents VR from binding an incompatible device. Session creation checks the
actual interop instance/device against the objects created through OpenXR.
The adapter uses only the verified DXVK Vulkan-loader path; an already-created
device or incompatible replacement is rejected rather than assumed compatible.

Calls from other Vulkan clients keep native creation. OpenXR receives an
unintercepted resolver to prevent recursive creation callbacks. Returned callback
and trampoline storage has process lifetime; it is disabled before runtime
destruction. No OpenXR work or graphics cleanup runs from DllMain.
Ordinary D3D9 resets preserve the OpenXR instance while recreating its session;
the native DXVK device and creation requirements remain valid. Events from a
retired session are ignored. Existing DXVK
queue locking and GPU-completion checks for submission are unchanged.

The previous `VulkanOpenVRBootstrap` setting is obsolete and ignored. No OpenVR
DLL is discovered or preloaded. Runtime resources are created through the same
selected OpenXR instance used for headset submission.

## Controller profiles

Tracking, poses and semantic actions use OpenXR APIs. Suggested defaults cover
HTC Vive Wands, Oculus Touch, Valve Index and Microsoft motion controllers.
Unsupported profiles are skipped without preventing other profiles from binding.
The runtime chooses the current profile; it is not inferred from runtime name.
Providing defaults is not a claim of tested pose/ergonomic compatibility.

Touch defaults use the left stick to move/steer, left stick click to sprint,
right A to jump/confirm/handbrake, right B to enter/exit/back, triggers to fire
or accelerate/brake, and squeezes for grip. Left Menu retains tap pause/hold
phone. Right-stick deflection turns the view. On foot X selects the previous
weapon and Y the next; while driving X cycles weapons (including unarmed), Y
performs context interaction and right-stick click honks directly. Phone and
script-menu navigation use right-stick deflection without clicking; A confirms
and B backs out. Context-dependent action-set
priorities retain menu/phone ownership. SteamVR users retain their existing
custom action bindings; other runtimes need not provide a binding editor.

Touch/Quest grip-to-rig rotation defaults use the Quest 3 calibration supplied
by a hardware tester: left quaternion `(0.5, 0.5, -0.5, 0.5)`, right
`(-0.5, 0.5, 0.5, 0.5)` in x/y/z/w order. These are selected per hand from
the active Touch, Touch Plus or Touch Pro interaction profile, including after
controller changes. Vive and unknown profiles retain the identity correction.
Explicit `HandGripRotationLeft` / `HandGripRotationRight` INI values override
the automatic defaults. No positional offset is added by the Quest defaults.

### Calibrating controller placement in the headset

Enable **VR > Calibrate controller placement**, then resume on foot with empty
hands. Release both grips first. Hold either grip to freeze that hand in space;
move and rotate the physical controller to match the frozen hand, then release
grip to apply and save. Repeat for the other hand, then turn calibration mode off.
The menu description reports the last save or a write error. Mode itself is
session-only and starts off on every launch.

Gameplay controls are suspended during on-foot calibration; pause remains
available. Vehicle, phone and menu interactions retain their normal controls.
Tracking loss, a controller-profile change, opening the menu, or disabling mode
cancels an unfinished hold rather than saving it. Position and rotation are
stored together per hand under `[VRHands <OpenXR interaction-profile path>]` in
the ASI INI. Saved profile calibration overrides the supplied default and the
legacy global quaternion setting; an explicit bridge calibration overrides both
for the current session. Removing the corresponding `Left` or `Right` entry
restores the configured/default placement. Profiles are selected automatically
when controllers change; no runtime-specific detection is involved.

Controller turning and its shared snap-angle/continuous-speed menu control are
described in [controller turning](controller-turning.md).

## Reporting failures

`tools/game-command.ps1 status` includes runtime identity, process bitness,
runtime version, controller readiness, accepted suggested-profile count, current controller
profile paths and the last OpenXR error. The count describes accepted
suggestions, not the number of connected controllers. The command bridge remains
a development option and is off in distributed settings.

For a startup log, enable only `[DEVELOPMENT] DiagnosticLogging=1`, reproduce
once and collect `plugins/GTAIV.VR.openxr.log`; turn it back off afterward.
Passive counters, observation hooks and automatic captures are not needed.
Include the mod build, runtime/version, connection method, graphics backend and
whether the failure affects initialization, controllers or rendered images.

## Validation limits

The x86 Release build and mocked creation/fallback tests pass. Tests cover
preservation of creation parameters, native resolver forwarding, GPU mismatch,
runtime failure and disabled-runtime fallback, plus controller defaults.
Local CE 1.2.0.59 / SteamVR OpenXR 2.18.4 / DXVK 3.1.1 startup created the OpenXR Vulkan session without
an OpenVR module preload. Explicit captures show the room rendered in both eyes,
and the runtime reports the Vive profile for both controllers. All four suggested
controller profiles are accepted by this runtime; this does not test their
physical controllers. Headset checks pass the Vive view, tracked hands and
usual Wand controls with the new initialization path. A separate
`DisableOpenXR=1` launch reached the command bridge with no OpenXR instance or
session initialized; the original installed INI was restored byte-for-byte.
This is a flat-screen startup smoke check, not a full gameplay regression.

Alternate-runtime headset rendering, controller poses, reset/loss lifecycle and
performance remain pending physical testing. GTA IV, TLAD and TBoGT coverage
is separate. No alternate-runtime performance claim is made.

## References

- [OpenXR Vulkan enable2](https://registry.khronos.org/OpenXR/specs/1.1/man/html/XR_KHR_vulkan_enable2.html)
- [DXVK 3.1.1 instance creation](https://github.com/doitsujin/dxvk/blob/v3.1.1/src/dxvk/dxvk_instance.cpp)
- [DXVK 3.1.1 D3D9 exports](https://github.com/doitsujin/dxvk/blob/v3.1.1/src/d3d9/d3d9_main.cpp)
- [VDXR runtime and controller capabilities](https://github.com/mbucchia/VirtualDesktop-OpenXR/wiki/Developers)
