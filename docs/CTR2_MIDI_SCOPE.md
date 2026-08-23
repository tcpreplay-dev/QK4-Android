# CTR2-MIDI integration scope

Status: planned; resume from this document before implementation.

Reference: CTR2-MIDI Operation Manual v2.01.01a:
https://ctr2.lynovation.com/wp-content/uploads/2026/03/CTR2-MIDI_Operation_Manual_v20101a.pdf

## Design boundary

QK4 Mobile will receive and map the MIDI messages emitted by CTR2-MIDI. It
will not duplicate settings that are configured and executed entirely by the
controller.

CTR2-managed settings that do not need QK4 controls:

- Beep Mode (Off, Normal, or Long-Press)
- Speed Tuning selection (Off, Normal, or Fast)
- Knob Map 1/2 selection and control-type configuration
- Bluetooth radio enable/disable
- Extended Button mode enable/disable
- Button calibration, firmware updates, configuration import/export, and
  factory reset
- Flex WiFi, HID, and RemoteTx-specific operation

QK4 must correctly interpret the messages resulting from these settings. In
particular, it must preserve WheelA magnitude so CTR2 proportional/speed tuning
works instead of reducing every encoder report to one tuning step.

## Discovery and persistence

- Add a built-in CTR2-MIDI profile.
- Discover CTR2-MIDI over both Bluetooth LE MIDI and USB MIDI.
- Recognize normal CTR2 BLE names such as `CTR2_####` and accommodate the USB
  ESP32-S3/XIAO identity exposed by Android.
- Remember the selected transport and physical-device identity.
- Remember the user's CTR2 control mappings per physical device.
- Provide CTR2 defaults, Restore CTR2 Defaults, and MIDI Learn for modified or
  unusual configurations.

## Device-specific keying modes

Keying capabilities are profile-specific, not universal across MIDI devices:

- TinyMidi: paddles only. Do not expose straight-key mode.
- HaliKey MIDI: selectable Paddles or Straight Key / External Keyer. In
  straight-key mode, let the user select the left or right physical input and
  ignore the unused input.
- CTR2-MIDI: selectable Paddles or Straight Key + PTT, following the CTR2
  paddle-jack modes and allowing TIP/RING assignment to be swapped.
- Custom profile: do not expose straight-key behavior by default; it must be
  explicitly configured as a supported input capability.

Paddle/iambic input continues through QK4 Mobile's existing local iambic keyer
and K4 `KZ` paddle stream. Straight-key or external-keyer input bypasses the
local iambic element generator and preserves the incoming key-down/key-up
timing using the K4 `KZ` raw key elements. Verify the precise indefinite
key-down/release sequence against current upstream QK4 and the K4 Programmer's
Reference before implementation.

The K4's CW VOX (hit-the-key), QSK, and DLY settings continue to control
transmit and receive behavior. Do not force CW VOX or any other operator
setting merely because a MIDI device connects. A mapped PTT input remains an
explicit, separate PTT source.

CTR2 paddle assignments documented by firmware mode:

| CTR2 configuration | Extended BTN off | Extended BTN on | QK4 use |
|---|---:|---:|---|
| Normal paddle mode | Notes 20/21 | Notes 96/97 | Left/right iambic paddles |
| Extended paddle mode | Notes 30/31 | Notes 98/99 | Straight key and PTT |

## Buttons

Support both CTR2 button layouts:

- Normal BTN mode: six short-press actions (notes 1-6) and six long-press
  actions (notes 11-16). CTR2 sends these on button release.
- Extended BTN mode: all 48 documented button actions, selected by the current
  knob mode.
- Add a QK4 setup selection: `CTR2 button layout: Normal (12) | Extended (48)`.
- Warn that the app selection must match the CTR2's own Extended BTN setting;
  QK4 cannot reliably infer that state from otherwise valid incoming notes.
- Allow every action to map to an applicable K4 command or an existing local
  QK4 function. Do not replace local implementations such as GEN with a radio
  command.

## Knob controls

Support all four CTR2 knob modes and both actions in each mode:

- Turn: CC 100, 102, 104, and 106
- Push and turn: CC 101, 103, 105, and 107

Suggested QK4 default mapping:

| CTR2 action | QK4 default |
|---|---|
| Home turn | Active VFO tuning |
| Home push-turn | RIT/XIT adjustment |
| Mode 1 turn | Main AF gain |
| Mode 1 push-turn | Sub AF gain |
| Mode 2 turn | Filter bandwidth |
| Mode 2 push-turn | Filter shift |
| Mode 3 turn | RF power |
| Mode 3 push-turn | CW speed in CW; mode-appropriate alternate elsewhere |

All eight assignments remain user-configurable.

Support every documented knob-output format:

- Button: directional NoteOn pairs, notes 40-55
- SliderA: absolute CC values 0-127
- SliderB: absolute CC values 0-127
- WheelA: relative values centered on 64, including accelerated magnitude
- WheelB: direction values 1 and 126
- WheelB-r: reversed WheelB direction

For absolute SliderA/SliderB mappings, prevent a newly selected knob mode from
jumping the associated radio setting. Establish synchronization on the first
value or require meaningful movement before applying a new absolute value.

## Setup UX and feedback

- Base CTR2 setup on the current working CW Keyer MIDI discovery/profile
  screen rather than creating a disconnected setup path.
- Keep controls touch-sized and vertically scrollable without horizontal pan.
- Ensure scrolling does not capture slider or learn-control gestures.
- Show a visible saved confirmation when mappings change.
- Keep all live console controls, meters, panadapter, and PTT immediately
  recoverable after closing setup.

## Validation checklist

- BLE and USB discovery, connection, reconnection, and per-device persistence
- Normal and Extended button layouts, including short/long release behavior
- All eight knob actions with Button, SliderA, SliderB, WheelA, WheelB, and
  WheelB-r formats
- Slow, Normal, and Fast WheelA proportional tuning without lost magnitude
- Iambic paddle timing over USB and BLE
- CTR2 straight key and PTT with TIP/RING swapped both ways
- HaliKey paddle and straight-key/external-keyer modes
- TinyMidi remains paddle-only
- K4 CW VOX on/off, QSK, and DLY behavior without connection-side mutation
- TEST TX first for keying/PTT safety, followed by controlled on-air validation
