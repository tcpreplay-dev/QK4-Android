# CTR2-MIDI on-screen controls and feedback

QK4 Mobile integrates CTR2-MIDI with the application's operating interface.
Mapped controls do more than send commands to the K4: where QK4 has a suitable
control, the corresponding adjustment appears on screen and follows the CTR2
knob. Other actions update the main operating display or show a brief
confirmation message.

QK4 provides 20 selectable knob adjustments. Fifteen open an on-screen control;
the remaining five use the main operating display and transient feedback.

When a knob mode is mapped directly to an adjustment, turning the knob opens
the appropriate control surface. When a knob mode is mapped to **Selected
adjustment**, pressing an **Adjust:** button opens and selects the surface,
displays `CTR2 KNOB: ...`, and assigns the knob to it.

## Adjustments that open QK4 controls

| CTR2 adjustment | QK4 Mobile UI invoked |
|---|---|
| Attenuator | Attenuator adjustment overlay with the current level and enabled state |
| Noise blanker | NB Level overlay with the current level, enabled state, and filter setting |
| Noise reduction | NR Adjust overlay showing the LMS/SSNR selection, state, and level |
| Manual notch | Manual Notch overlay with its enabled state and pitch |
| Main volume | K4 Controls drawer positioned at Main Volume |
| Sub volume | K4 Controls drawer positioned at Sub Volume |
| RIT/XIT frequency | K4 Controls drawer with the RIT/XIT row visible; turning the knob enables RIT when necessary |
| Filter bandwidth | K4 Controls drawer positioned at Filter Bandwidth |
| Filter shift | K4 Controls drawer positioned at Filter Shift |
| Main RF gain | K4 Controls drawer positioned at Main RF Gain |
| Sub RF gain | K4 Controls drawer positioned at Sub RF Gain |
| Main squelch | K4 Controls drawer positioned at Main Squelch |
| Sub squelch | K4 Controls drawer positioned at Sub Squelch |
| RF power | K4 Controls drawer positioned at RF Power |
| CW speed | K4 Controls drawer positioned at CW Speed while operating in CW or CW-R |

The visible sliders, values, and radio indicators update as the CTR2 knob is
turned. Volume, filter, power, and CW-speed adjustments also show brief numeric
confirmation.

## Adjustments shown through the operating display

These adjustments do not open another menu because the affected information is
already visible. Selecting one through an **Adjust:** button still displays the
`CTR2 KNOB: ...` confirmation.

| CTR2 adjustment | Visible feedback |
|---|---|
| Active VFO frequency | The active VFO frequency and panadapter cursor update |
| Other VFO frequency | The other VFO frequency display updates |
| Panadapter zoom | The panadapter changes and a message shows the resulting span |
| Panadapter reference | The spectrum reference changes and a message shows the dBm value |
| Waterfall brightness | The waterfall changes and a message shows the WTR CLRS value |

## Immediate button actions

These predefined button actions perform their function immediately rather than
assigning the knob.

| Button action | QK4 Mobile response |
|---|---|
| Band Up / Band Down | Frequency, band, and panadapter update |
| Mode Next / Mode Previous | The VFO mode display updates |
| Main Mute | Volume controls update and `MAIN AUDIO MUTED` or `MAIN AUDIO RESTORED` appears |
| Attenuator Toggle | The ATT state and level indication update |
| Noise Blanker Toggle | The NB indication updates |
| NR Toggle | The NR/SSNR indication updates |
| Manual Notch Toggle | The normal QK4 notch state follows the K4 response |
| RIT Toggle | The RIT/XIT label and offset display update |
| Split Toggle | SPLIT and transmit-VFO indications update |
| TX/RX Toggle | The transmit UI changes and `TRANSMIT ON` or `RECEIVE` appears |
| Pan Zoom In / Pan Zoom Out | The panadapter changes and a message shows the new span |
| Rate | The VFO tuning-rate indication updates |
| KHZ | The K4 1 kHz rate is selected for VFO A, or VFO B while B-SET is active |
| TUNE | The K4 TUNE function is invoked and normal radio-state indications follow |
| Custom Command | The entered K4 command is sent and QK4 displays any resulting state it understands |
| Disabled | No action |

Custom commands do not automatically open a menu because QK4 deliberately does
not interpret or rewrite arbitrary K4 command sequences.

For shared A/B controls, the current **B-SET** state determines which receiver
is adjusted. Actions explicitly named Main or Sub always operate their named
receiver.

[Return to the QK4 Android README](../README.md)
