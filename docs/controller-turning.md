# Controller turning

Tracking: https://github.com/bordplate/GTA4VR/issues/29

## Controls and settings

On foot and Driving expose **Turn view**, a two-dimensional right stick/pad
position action. Its horizontal component turns without a click; vertical
intent is ignored. Vive, Touch, Index and motion-controller suggestions use the
existing profile-specific trackpad/thumbstick mapping. **Enable turning (touch)**
uses pad contact on Wands; Touch sticks need no touch gate. In a custom SteamVR
binding add Turn view to right position and, for Wands, Enable turning to touch.
Saved custom bindings may leave new actions unbound until edited.

Touch/Quest defaults reserve right-stick deflection for turning. X/Y select
previous/next weapons on foot; X cycles driving weapons including unarmed and Y
retains driving context interaction. Right-stick click honks directly. Phone
and interaction menus navigate by stick deflection, with A confirm and B back.
These defaults use the Touch profile for both VDXR and SteamVR. Existing SteamVR
custom bindings are not overwritten; reset to defaults or update them manually.
When no navigation-click action is bound, a deflected navigation axis activates
directions directly and returning to centre never confirms. A bound click
retains the existing click-to-navigate behavior, including the Wand defaults.

The VR menu has **Turning: Off / Snap / Continuous** and one shared slider:
**Snap angle** in degrees or **Turn speed** in degrees/second. Each mode retains
its own saved value. Defaults are Snap, 45 degrees and 90 degrees/second. The
INI keys are TurningMode (0/1/2), SnapTurnAngle and ContinuousTurnSpeed under VR.
Menu ranges are controls for editing, not limits on finite INI tuning values.

Snap turns once beyond the activation threshold and rearms toward centre or
when the pad is no longer touched. Continuous turning scales with deflection
and elapsed time. Changing mode, losing tracking or interrupting gameplay
requires a neutral sample before turning resumes; long stalls do not cause a
catch-up rotation. Right-pad click actions suppress turning while pressed.
As position and click share the pad, touching an edge before clicking can still
turn first; unbind Turn view or choose Off if that conflicts with a custom layout.

## Camera ownership

Camera preparation consumes each XR input sample once and rotates the common
tracking reference about the current eye midpoint. It does not rotate around
the calibrated floor origin, move the player or write vehicle steering. Head,
hands, locomotion, muzzle aim and spatial audio use the resulting shared frame.
Seated turning changes view orientation relative to the vehicle; entering or
leaving a vehicle recenters through the existing anchor transition.

Turning is gated during pause/phone/script menus, scripted cameras,
controller-placement calibration, native body animations, ragdoll and vehicle
exit, or when poses/input are not focused/fresh. Native flat-screen input is
unchanged. The explicit input-status bridge reports turnActive and the sampled
turn axis; no passive logging or observation hooks are required.

## Verification

Deterministic checks cover held-snap suppression, neutral rearming, opposing
and vertical inputs, lost-focus/mode/stall suppression, 30/60/90/120 Hz continuous
integration, correct rightward sign, and head-pivot invariance with an offset
tracking origin. Affected OpenXR, settings and eye-resolution checks pass.
Touch binding checks cover the left-hand X/Y paths, direct horn, removal of
click-direction defaults, and axis navigation without accidental centre confirm.
The x86 Release build passes. Basic turning and shared-slider operation were
checked on CE 1.2.0.59 / SteamVR OpenXR 2.18.4 / DXVK 3.1.1 / Vive Wands.
Quest/VDXR hardware testing of the revised defaults remains pending.
