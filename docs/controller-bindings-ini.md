# Controller bindings in INI files

Tracked in [#38](https://github.com/bordplate/GTA4VR/issues/38).

Use these files to change the mod's suggested OpenXR bindings, including on
VDXR. The shipped values match the existing defaults; installing them does
not change controls. They are next to the ASI in **`plugins/VRBindings/`**:

| File | OpenXR controller profile |
| --- | --- |
| `touch.ini` | Quest / Oculus Touch |
| `vive.ini` | HTC Vive Wands |
| `index.ini` | Valve Index controllers |
| `motion.ini` | Microsoft / Windows Mixed Reality motion controllers |

Choose the controller profile, not the runtime. Quest users normally edit
`touch.ini` whether using VDXR or SteamVR. The active profile is selected by
OpenXR. This does not add new device profiles or claim hardware testing for
all four families.

## Editing

1. Quit GTA IV and back up the file you want to change.
2. Edit the action in its section, using a full OpenXR input path.
3. Restart GTA IV. Bindings are suggested before the OpenXR action sets attach;
   `reload-weapon-config` does not reload controller bindings.

Existing SteamVR custom bindings can override these suggestions. Update or
reset that custom binding if you want SteamVR to use the INI defaults.
The INI files do not overwrite runtime-owned binding configurations.

Each entry has a comment describing the action and its input type. These
sections are separate contexts:

| Section | Controls |
| --- | --- |
| `[on_foot]` | Movement, turning, weapons, grabbing and contextual interaction |
| `[driving]` | Vehicle controls, turning, drive-by weapons, phone and radio |
| `[menus]` | Pause pointer, waypoints and scripted interaction menus |
| `[system]` | Global pause/skip and hold-to-open phone |
| `[phone]` | Phone navigation, accept and back |
| `[hands]` | Per-hand tracking poses, grip state and finger animation |

For example, move Quest on-foot confirmation from A to X:

```ini
[on_foot]
confirm = /user/hand/left/input/x/click
```

Edit the existing entry rather than adding a duplicate. Other actions already
on X still receive it when active; rebinding one action does not move or unbind
others automatically. Phone and Menus retain their higher-priority ownership.

Give a Quest radio action a button:

```ini
[driving]
radio_next = /user/hand/left/input/x/click
```

X already cycles drive-by weapons in that context. To dedicate it to the radio,
also set `next_weapon = none` or move that action to another supported input.

Bind more than one input to an action with commas:

```ini
[menus]
confirm = /user/hand/right/input/a/click, /user/hand/left/input/x/click
```

## Paths and input types

Common Touch/Quest input paths:

| Input | Path |
| --- | --- |
| X / Y | `/user/hand/left/input/x/click`, `/user/hand/left/input/y/click` |
| A / B | `/user/hand/right/input/a/click`, `/user/hand/right/input/b/click` |
| Left Menu | `/user/hand/left/input/menu/click` |
| Left / right stick | `/user/hand/left/input/thumbstick`, `/user/hand/right/input/thumbstick` |
| Stick click | Add `/click` to the corresponding thumbstick path |
| Trigger | `/user/hand/left/input/trigger/value` or the right-hand equivalent |
| Grip | `/user/hand/left/input/squeeze/value` or the right-hand equivalent |

Use components supported by that controller and compatible with the action's
type. A **vector** action such as movement needs the whole stick/pad path,
not its `/x`, `/y` or `/click` component. A **scalar** action uses an analog
value; **boolean** actions use buttons or runtime-supported conversions from
analog inputs. A **pose** action needs a pose component.

Entries ending in `.left` or `.right` belong to a two-hand action with OpenXR
subaction paths. Keep their paths on that same hand. Other action entries may
be bound to either hand. Changing a binding does not change the mod's weapon
handedness or add left-handed weapon support.

Physical reload and steering gestures use the tracked hand's grip state from
`[hands] close_hand.left/right`. Prop grabbing uses the separate
`grab_left/right` actions. If changing a grip input, adjust the relevant actions
together; the finger/hand actions are exposed separately for the same reason.

## Unbinding, defaults and errors

- `none` or an empty value explicitly unbinds an action.
- A missing file, section or key uses the existing built-in default. Delete a
  key to restore its default; an empty key value does not restore it.
- Section and key names are case-insensitive; OpenXR paths are lowercase.
- `;`, `#` and `//` introduce comments. Do not quote paths.
- Duplicate keys retain the first entry and report an error.
- Invalid path syntax or a wrong-hand subaction path falls back for that entry.
- If OpenXR rejects the configured bindings for a profile (for example, an
  unsupported component or incompatible input type), that whole profile falls
  back to its built-in defaults. Other profiles remain independent.

The development command bridge's **`status` → `openxr.bindingWarnings`** reports
syntax errors, unknown keys and runtime-rejected overrides. Diagnostic logging
also records these warnings when explicitly enabled. No logging or command
bridge is required for ordinary binding configuration.

## Maintaining the defaults

`tools/controller-binding-defaults.cpp` exports the existing action catalog and
profile mappings with `--write`; without arguments it verifies every shipped
entry against the built-in defaults. Run from the repository root. The current
catalog exposes 64 actions and 284 per-profile/per-hand entries. Focused parser
checks are in `tools/tests/vrbindingconfig.cpp`.
