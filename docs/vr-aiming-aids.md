# VR aiming aids

Tracked in [#34](https://github.com/bordplate/GTA4VR/issues/34).

Choose **VR → Aiming aid** in the pause menu:

| Mode | Behavior |
| --- | --- |
| None | Default; no additional weapon aiming graphic |
| Crosshair | Native circular reticle, with NPC health/armour segments |
| Laser sight | Red beam from the muzzle, with a red target marker |

The setting applies immediately and saves as `[VR] AimingAid=0/1/2` in
`plugins/GTAIV.EFLC.FusionFix.ini`. No restart or new controller binding is
needed when using the VR menu.

The aid follows the actual tracked gun, including two-hand aiming, recoil and
the left-hand drive-by attachment. It uses the same native muzzle lookup and
forward axis as VR firing. A native world query stops at the first surface or
actor; with no hit, the marker is at the weapon's range. The player's own body,
gun and owned arm proxies are excluded. The query runs once when building the
main-eye gun commands, not on the graphics command execution thread.

Both eyes render the same world target with their respective camera matrices.
The crosshair, centre dot and NPC health/armour rings render without scene-depth
occlusion. The laser retains native logarithmic scene-depth testing.
The circle uses the game's existing `hud_crosshair` and `hud_target` artwork
and HUD palette; no game assets are bundled. NPC health and armour use the
native eight-segment thresholds. The health arc uses 97% of the outer circle's
radius to improve visibility while remaining inside its boundary. The mirrored
circle quarters and rotated health triangle fans use their separate native vertex/UV layouts.
All aiming graphics reuse the weapon-display renderer's state restoration.

Menus, phone, controller calibration, stale/unfocused tracking and non-firearm
states suppress it. Grenade/Molotov trajectories and melee weapons do not get
an aiming aid. Weapon spread, shotgun pellet patterns, damage and projectile
flight remain native: the graphic indicates the central muzzle ray, not a
guaranteed impact point or ballistic prediction.

## Verification

Focused checks cover the None default, enum validity, finite geometry,
zero-length/invalid rays, native arc mapping and all eight health levels.
The x86 Release build passes. The circular crosshair, NPC health display and
enlarged health arc are verified on Vive/Wands with GTA IV CE 1.2.0.59 and
SteamVR/OpenXR 2.18.4. Laser headset checks, armour-specific visual checks,
other runtimes/controllers and episode coverage remain pending.

## Native reticle reference

GTA IV CE 1.2.0.59, Ghidra addresses (image base 0x00400000):
`5C87B0` updates the native reticle; `8FD3F0` draws four mirrored circle
quarters; `8FDB00` draws health/armour in eight half-quarter segments.
Live widget descriptors confirm the two existing textures. Health uses the
ped virtual getter at vtable +FC minus 100, normalized against max(current,
ped+A84 minus 100); armour is ped+B84. The native threshold percentages are
0/14/28/42/56/70/84/99/100. These findings are saved in Ghidra.

VR reads health with the gun-query hit on the command producer, then carries
plain segment counts to both eyes; rendering never retains a target ped
pointer. The native aiming tasks and the vanilla HUD widget state are not
modified. Relationship-specific reticle color changes are not yet reproduced.

The native-circle lookup is anchored to the specific reticle update branch at
CE `5C8E91`, with unique-match validation over readable executable regions.
The development bridge `status` exposes the current aiming mode, valid target,
NPC/health segment state and resource lookup result on request; it does not
enable passive logging or captures.

Health fan verification uses its actual native quarter-arc texture in a local
ignored preview for all eight segment counts. The fan starts at the reticle
centre; the texture's bottom-right corner is the arc centre for both artwork
variants. Committed math checks verify that texture-space radius matches
world-space radius at triangle vertices and interiors, catching inward-facing
cross-shaped arrangements. No native texture is added to the repository.
