# OpenXR startup with auxiliary DXVK devices

Tracked in [#36](https://github.com/bordplate/GTA4VR/issues/36).

## Failure and cause

The affected installation still loaded the VR pause tab, but OpenXR status
reported no session and `DXVK graphics objects were not created through the
selected OpenXR runtime (OpenXR 0, Vulkan 0)`.

The executable remained GTA IV Complete Edition **1.2.0.59**. The runtime was
SteamVR OpenXR **2.18.4**, with DXVK/Vulkan selected. The executable and graphics
DLL timestamps did not show replacement. A preceding launcher/game update was
reported, but its responsibility for introducing the helper is unproven.

The live Vulkan resolver hook remained installed. A gated startup trace showed
DXVK instance creation and two DXVK device creations reaching the OpenXR
enable2 adapter. Graphics startup recorded the ordinary 2560×1440 device and
an additional **120×61** helper device before the first OpenXR session bind.
Steam overlay modules were loaded, but the exact helper creator was not traced.

The adapter retained only the last-created device. The helper overwrote the
game device's ownership record, so the later comparison rejected a legitimate
OpenXR-created device. This was a bookkeeping failure, not an unsupported
game version or a failure of OpenXR Vulkan creation.

## Correction

Retain each successfully created instance/physical-device/device tuple and
match the actual game interop tuple against those records. A secondary device
or a later failed creation must not invalidate an earlier valid tuple.
Remove records on DXVK device/instance destruction to avoid stale handles.
Runtime-internal cleanup stays on the native route without acquiring the
adapter's ownership lock inside an enable2 call.

Optional startup tracing remains behind `DiagnosticLogging`; ordinary gameplay
does not enable it. Runtime selection and the native creation/fallback routes
remain unchanged.

## Evidence and coverage

- Regression checks cover two distinct successful devices, retention of the
  first, removal of the helper, instance destruction, exact tuple matching,
  mismatched-GPU fallback and disabled-adapter native creation.
- The x86 Release build passes; existing updater-library missing-PDB warnings
  remain unrelated.
- In the corrected live run, the interop device was `0AAC8078` and the last
  created device was `0C950078`; the retained tuple matched successfully.
  `xrCreateSession(DXVK Vulkan)` returned success and continuous stereo frames
  resumed at 1974×2192 per eye. Addresses are run-local evidence, not constants.
- A final restart with all four passive diagnostic controls off initialized
  focused stereo and both Vive controller profiles. Headset view and tracked
  hands were checked in that run.
- The observed correction is CE GTA IV / SteamVR / DXVK. Other runtimes and
  episodes were not retested for this change.

For development relaunches, wait for both `GTAIV.exe` and `PlayGTAIV.exe` to
exit and allow Steam to clear its running state. Game-process exit alone does
not guarantee Steam will accept an immediate new launch.
