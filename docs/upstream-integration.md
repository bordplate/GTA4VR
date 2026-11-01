# FusionFix upstream integration

Tracking: https://github.com/bordplate/GTA4VR/issues/28

## Baseline

The integration branch merges upstream `87db4f6` (11 commits after the previous
upstream base) into GTA4VR `16a8224`. Existing uncommitted renderer experiments
are excluded. Both histories are retained by the merge.

## Ownership and compatibility decisions

- Upstream owns the graphics API routing through `graphicsapi.ixx` and its
  graphics API client. VR reads the effective API rather than independently
  reading the legacy `d3d9.cfg`. Graphics routing is installed before the VR
  module prepares runtime-selected Vulkan creation, and both precede the
  game's Direct3D factory/device calls.
- On first launch without an ASI Settings CFG, legacy general FusionFix INI
  settings and `d3d9.cfg` are imported into the new CFG. Existing CFG files win.
  VR options, diagnostics and controller calibration retain the ASI INI;
  weapon/vehicle profiles retain their own files. The old backend file is a
  migration input, not a live backend switch.
- Upstream's menu API retains the VR tab, pointer, map-waypoint input and
  custom slider labels/values/descriptions. VR spacers use upstream empty rows;
  custom VR sliders suppress duplicate native numeric text.
- The upstream temporal/upscaling implementation has one camera/history pair.
  Its temporal modes are disabled while an OpenXR session runs. Existing
  non-temporal AA remains available. Per-eye temporal history and HDR in VR
  are not claimed supported by this merge.
- The fork retains its ZIP, offline/web installers and its own GitHub release
  updater. The upstream Hub updater is inactive; packaging needs no private
  Hub credentials. New upstream settings validation and precompiled-shader
  build/verification steps are retained.
- The existing lowercase `external/vulkan-headers` submodule path is shared by
  both features, avoiding duplicate paths differing only by case on Windows.

## Validation

x86 Release solution build passes, including the ASI, settings validator,
pass-through d3d9.dll and retained installer. Known updater-library missing-PDB
warnings remain. Focused OpenXR creation/fallback/controller-calibration,
settings-model and eye-resolution checks pass.

The full release pipeline produces the ZIP and both installers. Package
validation passes 344 entries, required x86 binaries, distribution settings
and license notices. Local packaging reuses the existing tested x86 OpenXR
loader and dependency binaries; CI builds its own loader.

Initial runtime check: GTA IV CE 1.2.0.59, SteamVR OpenXR 2.18.4, DXVK 3.1.1,
Vive/Wands. Runtime-selected Vulkan creation succeeds, both explicit eye
captures render apartment geometry, and the VR menu registers. The headset
was not focused during this capture; it does not establish controller or
comfort acceptance. The VR opt-out startup check reaches game callbacks with no OpenXR instance,
session or eye images on the retained DXVK backend. The installed INI is restored
byte-for-byte afterward. Further headset, flat-screen gameplay and episode
checks are tracked in the issue. No performance improvement is claimed.
