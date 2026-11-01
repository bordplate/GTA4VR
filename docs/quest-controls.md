# Quest controller guide

These are the default Touch/Quest bindings, including when using VDXR.
They use OpenXR and do not require SteamVR. Existing custom SteamVR bindings
may need updating or resetting to defaults. Quest/VDXR hardware testing is
limited; please report binding, tracking or interaction problems.

## Movement and interaction

| Action | Control |
| --- | --- |
| Move | Left stick. Push toward the edge to jog |
| Sprint | Hold left stick click while moving |
| Turn | Right stick left/right. Select Off, Snap or Continuous and adjust angle/speed in the VR menu |
| Jump / pull up from a ledge | A |
| Drop from a ledge | Grip while hanging |
| Climb ladders | Left stick forward/back; centre to stop |
| Crouch | Physically lower your head |
| Enter / exit vehicle | B |
| Shops, vendors and other context interactions | Left trigger on foot; Y while driving |
| Confirm purchases / prompts | A |
| Pause / skip eligible cutscenes | Tap left Menu |
| Open phone, including while driving | Hold left Menu |

## Weapons, combat and physics

| Action | Control / interaction |
| --- | --- |
| Previous / next weapon on foot | X / Y |
| Aim | Physically point the weapon |
| Fire on foot | Right trigger |
| Support weapon | Hold left grip near its support point |
| Punch | Hold grip + trigger to form a fist, then punch |
| Knife / bat / pool cue | Physically swing the equipped weapon |
| Throw grenade / Molotov | Hold right trigger, make a throwing motion, then release trigger |
| Grab object / ragdoll | Hold either grip near it; keep holding to retain it |
| Two-handed grab | Grip the same object with both hands |
| Drop / throw grabbed object | Release grip; release during a throwing motion to throw |
| Open vehicle doors, bonnet or trunk | Grip near the part and pull. Locked parts stay locked |
| Read ammo display | Pins show loaded rounds; digits show reserve ammunition. Diegetic counters can be disabled in the VR menu |

## Physical reloading

Physical reloading is currently **on foot only**. Vehicles use native
ammunition/reload handling.

| Action / weapon | Control / interaction |
| --- | --- |
| Eject magazine | Press right grip while holding a magazine-fed gun |
| Take fresh ammunition | Hold left grip over your left shoulder |
| Insert magazine | Align it with the magazine well, then push it in |
| Chamber a round | Grip the slide / charging handle with the left hand, pull and release |
| Load shotgun shells | Take shells from your left shoulder and insert them into the loading gate |
| Pump shotgun | Hold left grip on the fore-end, pull back and push forward after **every shot** |
| Combat shotgun | Cycles automatically after shots; operate the bolt after an empty reload |
| RPG | Take a rocket from your left shoulder and guide it into the front of the launcher |

Remaining rounds in an ejected magazine are lost. A chambered round can remain
ready during a magazine change. Racking an already-loaded chamber ejects that
round.

## Vehicles and radio

| Action | Control / interaction |
| --- | --- |
| Accelerate / brake or reverse | Right / left trigger |
| Steer | Left stick |
| Physical steering | Hold either grip near the wheel or bike handle and move naturally |
| Return to stick steering | Release steering grips |
| Handbrake | A |
| Horn | Right stick click |
| Cycle drive-by weapons / holster | X; cycling includes unarmed |
| Drive-by aim / fire | Weapon is in the **left hand**; point it and use left trigger |
| Brake / reverse while armed | Left stick click |
| Vehicle context interaction | Y |
| Helicopter ascend / descend | Right / left trigger |
| Helicopter pitch / roll | Left stick, or hold right grip near the cyclic and move it |
| Helicopter yaw while gripping cyclic | Left stick left/right |
| Change radio station | Grip the radio panel, drag horizontally until the desired station is centred, then release |
| Reposition radar / radio | Enable **Grip to move vehicle panels** under **VR → Vehicle camera and radar**, then grip, move/rotate and release |

Panel positions save automatically, separately for each vehicle model. Disable
panel editing again to use grip-and-drag radio tuning.

## Phone, menus and setup

| Action | Control / interaction |
| --- | --- |
| Navigate phone / interaction menus | Right stick deflection; no click required |
| Confirm / accept | A |
| Back / leave interaction | B |
| Pause-menu pointer | Point and press trigger to select; hold to drag |
| Set / remove map waypoint | Point at the pause map and click right stick |
| Calibrate controller placement | Enable **Calibrate controller placement** in the VR menu, resume on foot and unarmed, then hold either grip to freeze that hand. Move/rotate the controller to match and release to save |
| Finish calibration | Repeat for the other hand, then disable calibration mode |

For turning behavior and settings, see [controller turning](controller-turning.md).
For runtime selection and requirements, see [OpenXR runtime compatibility](openxr-runtime-compatibility.md).
