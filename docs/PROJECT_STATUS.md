# Project status

Last updated: 2026-09-06

## Released build

**QK4 Mobile v1.0.4.1** uses Android version code 31.

- Straight-key and external-keyer local sidetone now maintains a small 12 ms
  write-ahead window, refilled every 3 ms, instead of relying on exactly one
  10 ms audio write per 10 ms Android timer interval. This is intended to
  eliminate audible underrun rasp while retaining a 3 ms attack/fall and a
  bounded key-up tail. Android compilation passes; physical audio validation
  remains required.
- CTR2 Extended Button Mode now keeps physical-button notes 1-48 separate from
  knob Button output. Normal knob direction pairs remain 40-55; Extended mode
  uses 60/61 through 74/75 for CC100-107 within Lynovation's clarified 60-95
  range. Extended paddle/straight-key notes remain 96-99. Enabling Extended
  Button Mode carries the 12 shared assignments into Home only; the 36 newly
  exposed Mode 1-3 assignments start disabled instead of cloning Home actions.
- Exported CTR2 mappings identify each button's physical label, MIDI note,
  press type, knob mode, and action/macro, and identify each knob's mode,
  gesture, CC, output, action, and mode-dependent Button direction notes.

**QK4 Mobile v1.0.4** adds a dedicated CTR2-MIDI setup and preserves the
existing CW Keyer workflow as a separate, simultaneous USB/Bluetooth MIDI
device role. Operators can customize all CTR2 knob modes and independent
short/long button actions, use on-screen QK4 adjustments and feedback, assign
predefined radio actions or exact K4 commands, and save or load portable CTR2
mapping files. F1-F8 labels and commands can likewise be saved and loaded in
user-editable files. Straight-key and external-keyer input with local sidetone
is available for TinyMIDI, HaliKey MIDI, learnable Custom MIDI, and CTR2-MIDI.
Radio-control mappings, custom commands, TinyMIDI/CTR2 straight-key operation,
and CTR2 USB/BLE disconnect and reconnect behavior passed physical testing on
the Samsung Galaxy S26 Ultra with an Elecraft K4. All nine current native test
targets pass. Android version code is 30, and the WorldWideDX-signed ARM64 APK
was signature-verified, installed, and cold-launched.

**QK4 Mobile v1.0.3** balances weak-signal SSTV recovery with stronger false-
start rejection. A damaged-header recovery remains provisional until three
consecutive, tightly timed line-sync pulses confirm it; unconfirmed candidates
return silently to AUTO RX. Ten private Main-RX false-trigger captures replay
with no mode events or completed images, while deterministic coverage includes
all 22 supported modes at 5 dB input SNR and a range of header impairments. The
SSTV transmit screen keeps its original portrait layout and moves FSK ID, CW ID,
and CW speed below the left-aligned MY CALL field in landscape. Low-latency CW
sidetone now follows speaker, Bluetooth, and USB-C output changes in both
directions; those routes passed physical testing with TinyMIDI on the Samsung
Galaxy S26 Ultra. Android hearing-aid sidetone routing is implemented where the
device is exposed as `TYPE_HEARING_AID`, but still awaits hardware validation.
Android version code is 29. The WorldWideDX-signed ARM64 APK was installed in
place, verified byte-for-byte against the source package, and cold-launched.

**QK4 Mobile v1.0.2** adds complete SSTV templates that can optionally retain
their source image, crop, zoom, position, text, and markup while preserving
layout-only templates for use over the current TX image. Built-in CQ, REPORT,
and 73 templates can be customized directly and restored to their factory
versions. SSTV mode, font, template, and retention selectors now render inside
the existing SSTV window, avoiding the Android EGL-surface crash caused by
rapid native popup creation and teardown. The release also guards and rebuilds
the local CW sidetone audio device when Android invalidates its output handle,
correcting the TinyMIDI test dit/dah crash. Android version code is 28. The
release-signed ARM64 package was signature-verified and installed in place on
the Samsung Galaxy S26 Ultra; complete image-template recall and TinyMIDI
dit/dah testing passed on the device.

**QK4 Mobile v1.0.1** adds K4-compatible reverse alternate taps for DATA,
AFSK, FSK, and PSK, including reverse-aware VFO labels and panadapter/mini-pan
orientation. It also refines both SSTV image editors with selectable ordered
objects, shape fill/outline controls, 45-degree object rotation, true
undo/redo, dynamic callsign variables, responsive portrait/landscape layouts,
and a shared 12-112 px text-size range while retaining existing saved-template
compatibility. Android version code is 27. All 16 focused SSTV composer tests
pass, and the release-signed ARM64 package was installed in place and tested
successfully on the Samsung Galaxy S26 Ultra.

**QK4 Mobile v1.0** is the first complete QK4 Mobile release with integrated
CW device support and integrated SSTV. The new SSTV workspace provides
automatic receive, transmit, exact-mode image composition, reusable templates,
post-image FSK/CW identification, callsign-aware reply, and retained RX history
across 22 modes. SSTV reception was tested through extended live monitoring
and interactive debugging while remotely connected to an Elecraft K4 and
receiving varied real-world signals. Transmit audio was captured over the air
by a remote WebSDR and successfully processed through a local SSTV decoder.
This release also adds the searchable DX Prefix List, changes the Android
application ID to `com.w9wdx.qk4phone`, keeps the display awake while the app
is active, and simplifies the mobile CTRL drawer by removing MON and BAL.
The release-signed ARM64 APK was signature-verified, confirmed as version code
26 / version 1.0, and installed on the Samsung Galaxy S26 Ultra.

**QK4 Mobile v0.8.3** corrects transverter frequency rendering so leading
digits are retained at VHF, UHF, and higher displayed frequencies. Direct
frequency entry for VFO A and VFO B now accepts ordinary radio-style MHz
shorthand: `7.2`, `7.215`, and `144.2` imply the omitted trailing zeros, while
fully grouped values and raw-Hz input remain supported. The release-signed
candidate was installed in place on the Samsung Galaxy S26 Ultra; automated
parser and display-format tests passed. Final K4/XVTR operating validation is
still pending.

**QK4 Mobile v0.8.2** corrects raspy Android receive audio that was most
noticeable on steady CW and digital signals. RX resampling now remains
continuous across K4 packet boundaries, partial non-blocking Android playback
writes are preserved, and later packets remain queued until earlier audio has
been accepted. The release-signed build was installed in place and the
improvement was confirmed while receiving CW on the Samsung Galaxy S26 Ultra.

**QK4 Mobile v0.8.1** adds combined USB and Bluetooth LE MIDI
discovery for the CW Keyer setup. Opening the screen no longer starts a BLE
scan; SCAN enumerates attached USB MIDI devices immediately while discovering
BLE MIDI devices, labels both transports in one selector, and remembers the
selected transport and USB identity for reconnecting.

**QK4 Mobile v0.8.0** is the current major-release source state. It fixes
the EQ preset-name editor so it appears above the graphic-EQ popup, enlarges
the preset recall/save controls for touch use, aligns the dB and Hz labels,
and adds deliberate long-press clearing for populated presets. The shared EQ
popup applies these changes to Main RX, Sub RX, and TX.

The preceding v0.7.6.5
corrects the Main RX, Sub RX, and TX graphic-EQ **FLAT** controls: the first
tap sets flat response and the second restores the exact prior eight-band
curve. Main and Sub RX share their RX EQ restore curve, matching the K4's
shared RX EQ behavior, while TX EQ restores independently. These controls
update K4 radio EQ settings. For mobile use, start with K4 RX EQ flat and use
phone/headset tone controls for personal listening preference.

The preceding v0.7.7.0 gives the formerly unused GEN BAND control a mobile-only
shortwave-listening bank: the 14 broadcast-band labels tune the active VFO to
AM defaults and retain a persistent, local last-used frequency per GEN band.
GEN preserves the K4's normal direct-frequency and nearest amateur-band-stack
behavior; it does not alter regular BN amateur-band selection or stacking.

The preceding v0.7.6.4 fixes the four right-side CTRL long-press adjustment
editors (ATTN, NB LEVEL, NR ADJ, and NTCH MANUAL): CTRL now dismisses before
the requested popup opens, and each editor provides a visible **↩** close
control. This is a touch-layout fix only; K4 radio/audio/protocol behavior is
unchanged.

The preceding v0.7.6.3 adds an RX-only Android hearing-aid output preference
for endpoints reported as `TYPE_HEARING_AID`. The change uses the existing
native Android media playback track and device-change rebuild path; it does
not change K4 audio streaming, TX, PTT, Bluetooth headset behavior, USB-C
behavior, or microphone selection. Field validation with Starkey Livio 2400
hearing aids is pending.

The preceding v0.7.6.2 point release temporarily forces the proven compact
landscape phone layout on every Android display size, including tablets, so
unvalidated alternate tablet geometry is not selected. The original detection
logic remains commented in `src/ui/k4styles.cpp` for restoration after physical
tablet testing.

The preceding release-signed ARM64 build, v0.7.6.1, added USB-C headset RX/TX
hot-swap after the radio session begins while preserving the Android TLS runtime
and Bluetooth headset routing across TX/RX transitions. Where Android supports
independent routes, Bluetooth RX can remain active while a USB-C headset
microphone provides TX audio. The product package is `com.ai5qk.qk4phone`,
with Android API 26 minimum and API 34 target.

Version 0.7.4 adds an **experimental** in-window TX input shield. During a
phone-initiated transmit state, it blocks all other console touch input while
leaving the red TX ON control available to return to RX. It is UI-only and
does not alter K4 PTT, CAT, microphone, or audio-stream behavior.

Version 0.7.5 adds Android spectrum-renderer compatibility for devices that
could render waterfall data while omitting the normal spectrum trace. It also
adds touch-first B SET cancellation, receiver-specific filter cycling from the
displayed filter shapes, and more forgiving A/B MODE touch targets. The normal
spectrum renderer remains the only rendering path changed; waterfall, K4
protocol, audio, and PTT behavior are unchanged.

The only physical UI acceptance device so far is a Samsung Galaxy S26 Ultra in
landscape. Until tablet testing is available, all screen sizes deliberately use
the compact layout; broader device validation is still required.

## Verified Android behavior

- K4 profile management, TCP/TLS connection, and disconnect/error handling.
- K4 RX audio streaming plus microphone TX audio and deliberate phone PTT
  operation, physically retested on the development K4 after the v0.7.3 fix.
- USB-C headset receive and transmit hot-swap, physically tested after radio
  connection. Android reports the active USB headset microphone input during
  transmit; Bluetooth RX remains available when Android maintains a split route.
- Experimental TX input shield, physically tested during a successful contact
  on the Samsung Galaxy S26 Ultra; broader device and field testing remains
  required before treating it as fully validated.
- VFO A/B operation, transmission-VFO selection, tuning digit selection,
  direct frequency entry, and panadapter tuning at the selected VFO step.
- Spectrum, waterfall, mini-pan, 50/50 initial spectrum/waterfall split, and
  user-adjustable waterfall height.
- Phone-oriented Control, TX, DISP, FN, Main RX, and Sub RX touch menus;
  touch-safe scrolling and long-press alternate actions.
- TX secondary editors dismiss with their parent menu after confirmation; the
  right CTRL-bank REV control is guarded against accidental activation while
  vertically scrolling.
- AF controls for main/sub receiver; relevant slider controls and mode-aware
  filter shift/bandwidth ranges.
- RIT/XIT activation and long-press jog control.
- CW text decode screen and F1–F8 macro editor/execution.
- Local non-decaying red Peak Hold trace, reset on toggle/geometry changes.
- Local WTR CLRS 5–30 brightness adjustment; it intentionally does not send a
  CAT command because it maps the application's local waterfall LUT.

## Known boundaries / next validation

### Included in v1.0

- FN > DX LIST now opens a local **DX PREFIX LIST** reference screen rather
  than the former informational placeholder. The supplied prefix/country data
  is sorted with numbered prefixes first followed by A-Z, supports direct
  touch-drag scrolling, and searches both prefixes and country/area text with
  previous/next navigation through multiple matches. It is read-only local UI
  and sends no CAT command or radio setting.
- The Android application ID is now `com.w9wdx.qk4phone`. A guarded one-time
  development-device migration copies the former `com.ai5qk.qk4phone`
  package's private QK4 settings and SSTV data, verifies every file by SHA-256,
  and removes the old package only after successful verification. Because
  Android isolates package data, this `run-as` migration applies to debuggable
  builds; a public release migration would require an old-package export
  bridge signed with the release key. On the Samsung Galaxy S26 Ultra, the
  three existing settings/draft files were hash-verified in the new package,
  the old debug package was removed, and the renamed activity cold-launched
  successfully.
- Android now requests keep-screen-on while the QK4 activity is in the
  foreground and releases the request when the activity is paused. This does
  not alter the user's system-wide screen-timeout setting. The behavior was
  observed on the Samsung Galaxy S26 Ultra during live testing.
- The in-window SSTV foundation now supports automatic Main-RX VIS detection,
  progressive receive decode with automatic slant correction, and image RX/TX
  in all 22 registered modes: Robot 36; Martin M1/M2/M3/M4; Scottie
  S1/S2/DX/S3/S4; PD-50/90/120/160/180/240/290; Wraase SC2-120/180; and
  Pasokon P3/P5/P7. Robot 36 now uses canonical even-Cr/odd-Cb paired chroma.
  The codec is native C++; mode timing and the Robot correction were
  cross-checked against Open-SSTV by Kevin, W0AEZ. TX uses an exclusive,
  generation-gated program-audio path with microphone exclusion, socket
  backpressure failure, immediate STOP/RX cleanup, final-packet progress, and
  automatic return to RX. Android Photo Picker/camera cancellation and activity
  state are handled without replacing the previous prepared image. All native
  tests and the ARM64 APK build pass. Live receive testing and WebSDR-captured
  transmit decoding validate the end-to-end K4 path, although every one of the
  22 modes has not been independently exercised over the air. Open-SSTV's
  image-linked QSO log and ADIF export are planned in
  detail as a deferred, dedicated future branch in `docs/SSTV_SCOPE.md`; no log
  implementation belongs in the current SSTV codec/RX/TX work.
- SSTV TX keying uses the previously proven K4 `TX;` path followed by a fixed
  500 ms key-up guard before program audio; it does not require a `TQ1` echo to
  begin or cancel merely because that echo is absent. Each send resets the
  program-audio codec and sequence, carries 400 ms of silent audio before the
  complete VIS header, and carries 300 ms of silent audio plus an
  SL-frame-aware drain before `RX;`. Immediate STOP still closes the audio gate
  and unkeys without waiting. A USB-connected device test completed the full
  application TX lifecycle and automatic return to RX without a socket stall,
  packet rejection, or early RX-state cancellation. Subsequent transmissions
  were recorded over the air by a remote WebSDR and successfully processed by
  a local SSTV decoder, validating the leader, VIS header, image data, and
  ending sequence. The TX screen reports ALC at or above 5 so the operator can
  reduce K4 DATA/LINE input gain; automatic gain changes are intentionally not
  made without radio measurements. The `MY CALL` and `TO CALL` fields share the
  same compact nine-character width.
- K4 MON and BAL controls are intentionally omitted from the mobile CTRL
  drawer. Delayed transmit-monitor return audio is not a useful representation
  of the transmitted signal and would be objectionable to monitor while
  speaking, so the former MON button, overlay, `SW128`, and `ML` adjustment path
  have been removed. NORM now sits with BW/SHFT and invokes the K4
  nominal-filter action. A AF and B AF remain independent volume controls, and
  M.RF/S.SQL cannot be replaced by a BAL overlay.
- SSTV TX now includes a touch composition canvas. It keeps the selected
  background intact while adding editable multiline text, selected fonts,
  color, pen strokes, line/arrow/rectangle/ellipse markup, source rotation,
  movement, object deletion, undo/redo, and reset. The preview confirmation
  freezes the native-size rendered frame that is passed to the existing SSTV
  encoder. The mode selector now precedes the editing controls and immediately
  defines the editor's exact native pixel canvas; source zoom/position and all
  overlays render against those transmitted pixels. Preview/send perform no
  late image resize, and the live canvas identifies its mode and dimensions.
  CQ, REPORT, and 73 starter layouts, user templates, the selected TX
  mode, and a debounced recovery draft persist locally. User templates now
  open in a dedicated in-window editor that loads the
  selected layout over the current mode frame, provides the complete text,
  font, size, color, drawing, shape, undo, and reset controls, and saves back
  to that same named template before loading it onto the TX canvas for review.
  The built-in CQ, REPORT, and 73 starters open in the same editor and SAVE
  TEMPLATE directly persists an editable default override, including an
  optional background image. RESET TEMPLATES restores the factory starters;
  the main save icon remains the separate save-as-new-template action.
  Complete-image templates now normalize legacy/default zero font stretch to
  normal width on restore, preserving their visible text and markup; the image
  gallery now presents the live TX composition as its first selectable tile,
  enables actions from actual selection state, and keeps saved thumbnails
  separate from the current composition. Its single responsive list replaces
  the former empty-state panel that could overlap the title after rotation.
  The TO CALL placeholder is explicitly left-aligned in the compact TX form.
  The operator callsign
  is an editable, validated, persistent SSTV setting used dynamically by the
  built-in templates and TX status; no product callsign is hard-coded. On
  Android, SSTV now owns an opaque full-window canvas and hides the console
  widget tree until BACK TO RADIO is selected. The TX editor keeps the image
  large while its compact control pane scrolls vertically. A newly imported
  Gallery or Camera source now clears the prior source's markup and undo/redo
  history. The markup color control now opens a compact touch palette with four
  shades each of the seven rainbow color families, plus black, white, charcoal,
  several greys, and silver. Choosing
  a color immediately recolors selected text and remains undoable. The font
  selector now offers 16 curated RF-readable family, weight, width, and italic
  variants; font and size changes apply immediately to selected text and are
  undoable. Zoom, text-size, and power sliders use the main console's larger
  touch geometry, a shorter horizontal activation distance, tap-to-position,
  and vertical-swipe direction locking. The permanent SSTV header now shows
  live RX and TX frequency/mode on both pages; RX follows Main/VFO A and TX
  follows the K4 split-selected transmit VFO. Independent persistent FSK ID
  and CW ID options append the operator callsign after the image while
  retaining the current K4 mode and the same immediate STOP/automatic-RX
  lifecycle. Fresh installs enable both; an existing SSTV installation
  preserves its CW choice and initially leaves the new FSK ID off. FSK uses
  MMSSTV/QSSTV-compatible 1900/2100 Hz six-bit checksum framing; CW remains
  shaped 700 Hz Morse at the remembered 5-40 WPM setting and its speed control
  appears only while CW is selected. The CW-speed field suppresses Android's
  clipped native stepper artifacts and uses separate visible one-WPM
  minus/plus touch buttons. When both IDs are enabled, progress visibly
  advances through image, FSK ID, then CW ID. The TX control pane now uses the main
  phone drawer's delayed-press/scroll-cancellation behavior plus direction-
  locked sliders. Both rotate glyphs use the conventional arrowhead position,
  and the compact power row gives more width to the slider with smaller step
  buttons.
- Completed SSTV RX images are atomically retained as app-private lossless PNG
  files with UTC/mode/frequency/slant and callsign metadata. After image
  completion, a bounded tail detector recognizes checksum-valid FSK ID first
  and callsign-shaped CW second without disarming AUTO RX; a new VIS header
  preempts the tail. RX CALL remains editable, records source/confidence and
  raw FSK/CW candidates, and manual correction is preserved. REPLY preloads
  the received mode and TO CALL in the TX editor without keying the K4; REPORT
  and 73 templates resolve that station while retaining the normal preview and
  deliberate TX confirmation. The receive screen also provides
  selectable retention, starring, individual deletion, protected history
  clearing, and explicit read-only Android sharing. Starred and exported images
  are not auto-deleted. The storage, draft, template, codec, and composition
  tests pass, and the complete ARM64 APK builds. The receive, callsign, reply,
  template, and transmit workflows were exercised on the physical Android/K4
  setup during extended live monitoring.
- The SSTV receive screen now distinguishes a live decoded K4 PCM stream from a
  stopped/missing stream and shows its input level, while keeping the image and
  history controls within the compact phone canvas. On the Galaxy S26 Ultra,
  live Main-RX PCM and the Android `FULL_SENSOR` orientation request were
  observed on-device. VIS acquisition now debounces noisy tone transitions,
  follows up to 300 Hz of leader-derived AFC through header/image decoding,
  recognizes short breaks hidden by discriminator transitions, and times out a
  partial/false leader without contaminating the next header. Offset/dropout
  and false-header recovery regressions pass. The receive DSP now also applies
  a streaming 1.0-2.5 kHz linear-phase prefilter, impulse-resistant adaptive
  line-sync detection, sample-rate-aware robust pixel estimation, and slow
  per-line AFC interpolation from accepted 1200 Hz sync pulses. Deterministic
  tests cover stronger out-of-band interferers plus clicks and a 0 to +120 Hz
  image-body drift; all 22 clean mode round trips remain covered. Portrait and
  landscape reflow and live over-the-air reception were exercised on the
  Samsung Galaxy S26 Ultra; not every supported mode received separate live
  over-the-air validation.

The FM PL/FM-T, DTMF CMD-memory, DATA/AFSK IIP, mode-aware Main/Sub RX,
Sub-RX targeting, APF availability, and informational-dialog CLOSE items from
the 2026-08-12 device findings were completed in v0.8.0 and are no longer
pending work.

- Validate landscape usability, system insets, font scaling, touch scrolling,
  and audio behavior on smaller Android phones, a Pixel/non-Samsung phone, and
  a foldable or tablet. Tablet devices temporarily use the compact phone layout;
  do not call them supported until tested.
- Android sideloading can still show a Play Protect/unknown-source notice even
  for the release-signed APK. Google Play distribution requires a signed AAB
  and Play App Signing.
- Hearing-aid RX routing is unverified on physical hardware. It applies only
  where Android exposes a `TYPE_HEARING_AID` output, and TX remains on the
  phone microphone unless Android supplies a separately supported input route.
- DR+ indication remains deferred until its source/state semantics are proven.
- Peak Hold is intentionally local: K4 stream data does not provide a rendered
  radio peak trace. WTR CLRS is local too; do not conflate it with `#WFC`.

Version 0.7.5.1 restores the original A/B VFO mode-control geometry. The
v0.7.5 extra touch padding pushed the SUB/DIV badges toward the VFO B frequency
and meter display on some layouts; this point release removes only that padding.

## References

- [Elecraft K4 manuals](https://elecraft.com/pages/k4-high-performance-direct-sampling-sdr-manuals)
- [Elecraft K4 Programmer's Reference](https://ftp.elecraft.com/K4/Manuals%20Downloads/K4ProgrammersReferencerev.D12.html)
- [Upstream QK4](https://github.com/mikeg-dal/QK4)
