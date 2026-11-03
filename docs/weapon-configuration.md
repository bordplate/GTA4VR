# Weapon configuration

Open **VR → Weapon configuration** in the pause menu. Changes save to `[VR]`
in `plugins/GTAIV.EFLC.FusionFix.ini` and apply immediately.

| Option | Behavior |
| --- | --- |
| Diegetic ammo counters | Show the ammunition displays attached to supported weapons |
| Aiming aid | None, native circular crosshair with NPC health, or laser sight; see [aiming aids](vr-aiming-aids.md) |
| Magazine pickup location | Left shoulder (`0`), left hip (`1`), or both (`2`, default) |
| Hip pickup left | Horizontal offset left of the live headset heading; default 0.25 m |
| Hip pickup below head | Vertical distance below the live headset; default 0.52 m |
| Hip pickup forward | Offset along the headset's horizontal heading; default 0 m; negative moves behind |
| Hip pickup radius | Left-hand activation distance; default 0.22 m |

Use left grip at the selected location to take a detachable magazine. The hip
source follows the current headset position and horizontal heading. The source
rearms after releasing grip or leaving its radius. Existing manual insertion,
racking and ammunition ownership are retained. Shotgun shells and RPG rounds
always use the existing shoulder pickup.

Direct INI keys are `MagazinePickupLocation`, `MagazineHolsterLeft`,
`MagazineHolsterDown`, `MagazineHolsterForward` and `MagazineHolsterRadius`.
INI edits require a restart; menu changes do not. Numeric INI values must be
finite, and radius must be positive. Menu slider ranges do not restrict valid
INI tuning values.

Individual weapon grip, magazine-insertion, charging-handle and display tuning
remain in `plugins/VRWeapons/*.ini`, with `reload-weapon-config` available through
the development bridge.

Integrated with [PR #39](https://github.com/bordplate/GTA4VR/pull/39).
The same integration preserves native sprint/stamina behavior, adds native NPC
punch contact audio and suppresses accidental taxi/context input during a
left-hand fist. These gameplay paths were tested in-game by the PR author.
Local checks cover x86 Release, settings validation, ammunition ownership,
fist hysteresis and input pulses. Local VR startup, stereo output and full menu
registration pass on CE 1.2.0.59 with Vive/Wands and SteamVR/OpenXR 2.18.4.
Live magazine source selection, hip sliders and hip magazine acquisition also
pass on Vive/Wands. Episode-specific coverage remains unverified.
