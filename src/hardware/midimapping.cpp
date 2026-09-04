#include "midimapping.h"

#include <QJsonArray>
#include <algorithm>

namespace MidiMapping {
namespace {

// Revision 1 maps were written before the built-in K4-Control profile was
// corrected from WheelA to SliderA for CC101-107.  This is separate from the
// file-format version: it identifies which factory defaults a saved map was
// based on without invalidating user mapping files.
constexpr int Ctr2DefaultsRevision = 2;

QString outputName(KnobOutput output) {
    switch (output) {
    case KnobOutput::WheelA: return QStringLiteral("wheelA");
    case KnobOutput::WheelB: return QStringLiteral("wheelB");
    case KnobOutput::WheelBReverse: return QStringLiteral("wheelB-r");
    case KnobOutput::SliderA: return QStringLiteral("sliderA");
    case KnobOutput::SliderB: return QStringLiteral("sliderB");
    case KnobOutput::Button: return QStringLiteral("button");
    }
    return QStringLiteral("wheelA");
}

bool parseOutput(const QString &name, KnobOutput *output) {
    static const QMap<QString, KnobOutput> values = {
        {QStringLiteral("wheelA"), KnobOutput::WheelA},
        {QStringLiteral("wheelB"), KnobOutput::WheelB},
        {QStringLiteral("wheelB-r"), KnobOutput::WheelBReverse},
        {QStringLiteral("sliderA"), KnobOutput::SliderA},
        {QStringLiteral("sliderB"), KnobOutput::SliderB},
        {QStringLiteral("button"), KnobOutput::Button},
    };
    const auto it = values.constFind(name);
    if (it == values.cend())
        return false;
    *output = it.value();
    return true;
}

ButtonBinding action(const char *id) {
    ButtonBinding binding;
    binding.action = QString::fromLatin1(id);
    return binding;
}

KnobBinding knob(const char *id, KnobOutput output) {
    KnobBinding binding;
    binding.action = QString::fromLatin1(id);
    binding.output = output;
    return binding;
}

} // namespace

bool DeviceMapping::operator==(const DeviceMapping &other) const {
    return name == other.name && profile == other.profile && keyingMode == other.keyingMode &&
           straightKeyInput == other.straightKeyInput && extendedButtons == other.extendedButtons &&
           tipRingSwapped == other.tipRingSwapped && cwInputEnabled == other.cwInputEnabled &&
           customDitStatus == other.customDitStatus && customDitData1 == other.customDitData1 &&
           customDahStatus == other.customDahStatus && customDahData1 == other.customDahData1 &&
           knobs == other.knobs && buttons == other.buttons && macros == other.macros;
}

DeviceMapping ctr2Default() {
    DeviceMapping mapping;
    mapping.name = QStringLiteral("K4-Control Default");
    mapping.profile = Profile::Ctr2;

    // CTR2 Map 1 defines only CC100 as speed-sensitive WheelA. The other seven
    // knob modes are absolute SliderA controls. Pickup converts each changed
    // position report into one signed step and ignores the first sample and
    // duplicates, preventing jumps when modes are selected or reports skip.
    mapping.knobs.insert(100, knob("active_vfo_frequency", KnobOutput::WheelA));
    mapping.knobs.insert(101, knob("main_volume", KnobOutput::SliderA));
    mapping.knobs.insert(102, knob("other_vfo_frequency", KnobOutput::SliderA));
    mapping.knobs.insert(103, knob("filter_bandwidth", KnobOutput::SliderA));
    mapping.knobs.insert(104, knob("rit_xit_frequency", KnobOutput::SliderA));
    mapping.knobs.insert(105, knob("nr_level", KnobOutput::SliderA));
    mapping.knobs.insert(106, knob("rf_power", KnobOutput::SliderA));
    mapping.knobs.insert(107, knob("cw_speed", KnobOutput::SliderA));

    mapping.buttons.insert(1, action("mode_next"));
    mapping.buttons.insert(11, action("mode_previous"));
    mapping.buttons.insert(2, action("band_up"));
    mapping.buttons.insert(12, action("band_down"));
    mapping.buttons.insert(3, action("main_mute"));
    mapping.buttons.insert(13, action("nr_toggle"));
    mapping.buttons.insert(4, action("rit_toggle"));
    mapping.buttons.insert(14, action("split_toggle"));
    mapping.buttons.insert(5, action("pan_zoom_in"));
    mapping.buttons.insert(15, action("pan_zoom_out"));
    mapping.buttons.insert(6, action("tune_step"));
    mapping.buttons.insert(16, action("tune"));
    return mapping;
}

DeviceMapping ctr2ExtendedDefault() {
    DeviceMapping mapping = ctr2Default();
    mapping.name = QStringLiteral("K4-Control Extended Default");
    mapping.extendedButtons = true;
    mapping.buttons.clear();

    const int shortNotes[4][6] = {
        {1, 2, 3, 4, 5, 6}, {7, 8, 9, 10, 11, 12},
        {13, 14, 15, 16, 17, 18}, {19, 20, 21, 22, 23, 24}};
    const int longNotes[4][6] = {
        {25, 26, 27, 28, 29, 30}, {31, 32, 33, 34, 35, 36},
        {37, 38, 39, 40, 41, 42}, {43, 44, 45, 46, 47, 48}};
    const char *shortActions[6] = {"mode_next", "band_up", "main_mute", "rit_toggle",
                                   "pan_zoom_in", "tune_step"};
    const char *longActions[6] = {"mode_previous", "band_down", "nr_toggle", "split_toggle",
                                  "pan_zoom_out", "tune"};
    for (int mode = 0; mode < 4; ++mode) {
        for (int button = 0; button < 6; ++button) {
            mapping.buttons.insert(shortNotes[mode][button], action(shortActions[button]));
            mapping.buttons.insert(longNotes[mode][button], action(longActions[button]));
        }
    }
    return mapping;
}

static void upgradeKnownCtr2Default(DeviceMapping *mapping) {
    if (!mapping || mapping->profile != Profile::Ctr2)
        return;
    const DeviceMapping corrected = mapping->extendedButtons ? ctr2ExtendedDefault()
                                                             : ctr2Default();
    if (mapping->name != corrected.name)
        return;

    // Upgrade only unchanged factory knob bindings.  Buttons, macros, keying
    // choices, and customized knob actions remain exactly as the operator
    // saved them.  The old whole-map comparison incorrectly skipped this
    // correction as soon as the operator changed even one button assignment.
    for (int cc = 101; cc <= 107; ++cc) {
        auto saved = mapping->knobs.find(cc);
        const auto factory = corrected.knobs.constFind(cc);
        if (saved != mapping->knobs.end() && factory != corrected.knobs.cend()
            && saved->action == factory->action && saved->output == KnobOutput::WheelA) {
            saved->output = KnobOutput::SliderA;
        }
    }
}

DeviceMapping tinyMidiDefault() {
    DeviceMapping mapping;
    mapping.name = QStringLiteral("TinyMIDI");
    mapping.profile = Profile::TinyMidi;
    return mapping;
}

DeviceMapping haliKeyDefault() {
    DeviceMapping mapping;
    mapping.name = QStringLiteral("HaliKey MIDI");
    mapping.profile = Profile::HaliKey;
    return mapping;
}

QStringList supportedKnobActions() {
    return {QStringLiteral("disabled"),
            QStringLiteral("selected_adjustment"),
            QStringLiteral("active_vfo_frequency"),
            QStringLiteral("other_vfo_frequency"),
            QStringLiteral("main_volume"),
            QStringLiteral("sub_volume"),
            QStringLiteral("rit_xit_frequency"),
            QStringLiteral("filter_bandwidth"),
            QStringLiteral("filter_shift"),
            QStringLiteral("attenuator_level"),
            QStringLiteral("noise_blanker_level"),
            QStringLiteral("nr_level"),
            QStringLiteral("manual_notch_pitch"),
            QStringLiteral("main_squelch"),
            QStringLiteral("sub_squelch"),
            QStringLiteral("main_rf_gain"),
            QStringLiteral("sub_rf_gain"),
            QStringLiteral("rf_power"),
            QStringLiteral("cw_speed"),
            QStringLiteral("pan_zoom"),
            QStringLiteral("pan_reference_level"),
            QStringLiteral("waterfall_brightness")};
}

QStringList supportedButtonActions() {
    QStringList predefined = {
            QStringLiteral("disabled"),
            QStringLiteral("mode_next"),     QStringLiteral("mode_previous"),
            QStringLiteral("band_up"),       QStringLiteral("band_down"),
            QStringLiteral("main_mute"),     QStringLiteral("nr_toggle"),
            QStringLiteral("attenuator_toggle"),
            QStringLiteral("noise_blanker_toggle"),
            QStringLiteral("manual_notch_toggle"),
            QStringLiteral("khz"),
            QStringLiteral("rit_toggle"),    QStringLiteral("split_toggle"),
            QStringLiteral("tx_rx_toggle"),
            QStringLiteral("pan_zoom_in"),   QStringLiteral("pan_zoom_out"),
            QStringLiteral("tune_step"),     QStringLiteral("tune")};
    QStringList adjustments = {
            QStringLiteral("adjust_active_vfo_frequency"),
            QStringLiteral("adjust_other_vfo_frequency"),
            QStringLiteral("adjust_main_volume"),
            QStringLiteral("adjust_sub_volume"),
            QStringLiteral("adjust_rit_xit_frequency"),
            QStringLiteral("adjust_filter_bandwidth"),
            QStringLiteral("adjust_filter_shift"),
            QStringLiteral("adjust_attenuator_level"),
            QStringLiteral("adjust_noise_blanker_level"),
            QStringLiteral("adjust_nr_level"),
            QStringLiteral("adjust_manual_notch_pitch"),
            QStringLiteral("adjust_main_squelch"),
            QStringLiteral("adjust_sub_squelch"),
            QStringLiteral("adjust_main_rf_gain"),
            QStringLiteral("adjust_sub_rf_gain"),
            QStringLiteral("adjust_rf_power"),
            QStringLiteral("adjust_cw_speed"),
            QStringLiteral("adjust_pan_zoom"),
            QStringLiteral("adjust_pan_reference_level"),
            QStringLiteral("adjust_waterfall_brightness")};

    const auto byLabel = [](const QString &left, const QString &right) {
        return QString::compare(buttonActionLabel(left), buttonActionLabel(right),
                                Qt::CaseInsensitive) < 0;
    };
    std::sort(predefined.begin(), predefined.end(), byLabel);
    std::sort(adjustments.begin(), adjustments.end(), byLabel);

    QStringList ordered{QStringLiteral("macro")};
    ordered.append(predefined);
    ordered.append(adjustments);
    return ordered;
}

bool isSupportedKnobAction(const QString &actionId) {
    return supportedKnobActions().contains(actionId);
}

bool isSupportedButtonAction(const QString &actionId) {
    return supportedButtonActions().contains(actionId);
}

QString knobActionForButtonAction(const QString &buttonAction) {
    static const QMap<QString, QString> actions = {
        {QStringLiteral("adjust_active_vfo_frequency"), QStringLiteral("active_vfo_frequency")},
        {QStringLiteral("adjust_other_vfo_frequency"), QStringLiteral("other_vfo_frequency")},
        {QStringLiteral("adjust_main_volume"), QStringLiteral("main_volume")},
        {QStringLiteral("adjust_sub_volume"), QStringLiteral("sub_volume")},
        {QStringLiteral("adjust_rit_xit_frequency"), QStringLiteral("rit_xit_frequency")},
        {QStringLiteral("adjust_filter_bandwidth"), QStringLiteral("filter_bandwidth")},
        {QStringLiteral("adjust_filter_shift"), QStringLiteral("filter_shift")},
        {QStringLiteral("adjust_attenuator_level"), QStringLiteral("attenuator_level")},
        {QStringLiteral("adjust_noise_blanker_level"), QStringLiteral("noise_blanker_level")},
        {QStringLiteral("adjust_nr_level"), QStringLiteral("nr_level")},
        {QStringLiteral("adjust_manual_notch_pitch"), QStringLiteral("manual_notch_pitch")},
        {QStringLiteral("adjust_main_squelch"), QStringLiteral("main_squelch")},
        {QStringLiteral("adjust_sub_squelch"), QStringLiteral("sub_squelch")},
        {QStringLiteral("adjust_main_rf_gain"), QStringLiteral("main_rf_gain")},
        {QStringLiteral("adjust_sub_rf_gain"), QStringLiteral("sub_rf_gain")},
        {QStringLiteral("adjust_rf_power"), QStringLiteral("rf_power")},
        {QStringLiteral("adjust_cw_speed"), QStringLiteral("cw_speed")},
        {QStringLiteral("adjust_pan_zoom"), QStringLiteral("pan_zoom")},
        {QStringLiteral("adjust_pan_reference_level"), QStringLiteral("pan_reference_level")},
        {QStringLiteral("adjust_waterfall_brightness"), QStringLiteral("waterfall_brightness")},
    };
    return actions.value(buttonAction);
}

bool isValidK4Command(const QString &command, QString *error) {
    const auto fail = [error](const QString &message) {
        if (error)
            *error = message;
        return false;
    };
    if (command.trimmed().isEmpty())
        return fail(QStringLiteral("K4 command is empty"));
    if (command != command.trimmed())
        return fail(QStringLiteral("K4 command cannot start or end with whitespace"));
    if (command.size() > 4096)
        return fail(QStringLiteral("K4 command is too long"));
    if (!command.endsWith(QLatin1Char(';')))
        return fail(QStringLiteral("K4 command must end with a semicolon"));
    for (const QChar character : command) {
        const ushort value = character.unicode();
        if (value < 0x20 || value > 0x7e)
            return fail(QStringLiteral("K4 command must contain printable ASCII only"));
    }
    if (error)
        error->clear();
    return true;
}

QString knobActionLabel(const QString &actionId) {
    static const QMap<QString, QString> labels = {
        {"disabled", "Disabled"}, {"selected_adjustment", "Selected adjustment (button)"},
        {"active_vfo_frequency", "Active VFO frequency"},
        {"other_vfo_frequency", "Other VFO frequency"}, {"main_volume", "Main volume"},
        {"sub_volume", "Sub volume"}, {"rit_xit_frequency", "RIT/XIT frequency"},
        {"filter_bandwidth", "Filter bandwidth"}, {"filter_shift", "Filter shift"},
        {"attenuator_level", "Attenuator level"}, {"noise_blanker_level", "Noise blanker level"},
        {"nr_level", "Noise reduction level"}, {"manual_notch_pitch", "Manual notch pitch"},
        {"main_squelch", "Main squelch"}, {"sub_squelch", "Sub squelch"},
        {"main_rf_gain", "Main RF gain"}, {"sub_rf_gain", "Sub RF gain"},
        {"rf_power", "RF power"},
        {"cw_speed", "CW speed"}, {"pan_zoom", "Panadapter zoom"},
        {"pan_reference_level", "Panadapter reference"},
        {"waterfall_brightness", "Waterfall brightness"}};
    return labels.value(actionId, actionId);
}

QString buttonActionLabel(const QString &actionId) {
    static const QMap<QString, QString> labels = {
        {"disabled", "Disabled"}, {"macro", "Custom Command"}, {"mode_next", "Mode next"},
        {"mode_previous", "Mode previous"}, {"band_up", "Band up"},
        {"band_down", "Band down"}, {"main_mute", "Main mute"},
        {"nr_toggle", "NR toggle"}, {"attenuator_toggle", "Attenuator toggle"},
        {"noise_blanker_toggle", "Noise blanker toggle"},
        {"manual_notch_toggle", "Manual notch toggle"}, {"rit_toggle", "RIT toggle"},
        {"split_toggle", "Split toggle"}, {"tx_rx_toggle", "TX/RX toggle"},
        {"pan_zoom_in", "Pan zoom in"},
        {"pan_zoom_out", "Pan zoom out"}, {"tune_step", "Rate"}, {"khz", "KHZ"},
        {"tune", "TUNE"},
        {"adjust_active_vfo_frequency", "Adjust: Active VFO frequency"},
        {"adjust_other_vfo_frequency", "Adjust: Other VFO frequency"},
        {"adjust_main_volume", "Adjust: Main volume"},
        {"adjust_sub_volume", "Adjust: Sub volume"},
        {"adjust_rit_xit_frequency", "Adjust: RIT/XIT frequency"},
        {"adjust_filter_bandwidth", "Adjust: Filter bandwidth"},
        {"adjust_filter_shift", "Adjust: Filter shift"},
        {"adjust_attenuator_level", "Adjust: Attenuator"},
        {"adjust_noise_blanker_level", "Adjust: Noise blanker"},
        {"adjust_nr_level", "Adjust: Noise reduction"},
        {"adjust_manual_notch_pitch", "Adjust: Manual notch"},
        {"adjust_main_squelch", "Adjust: Main squelch"},
        {"adjust_sub_squelch", "Adjust: Sub squelch"},
        {"adjust_main_rf_gain", "Adjust: Main RF gain"},
        {"adjust_sub_rf_gain", "Adjust: Sub RF gain"},
        {"adjust_rf_power", "Adjust: RF power"},
        {"adjust_cw_speed", "Adjust: CW speed"},
        {"adjust_pan_zoom", "Adjust: Panadapter zoom"},
        {"adjust_pan_reference_level", "Adjust: Panadapter reference"},
        {"adjust_waterfall_brightness", "Adjust: Waterfall brightness"}};
    return labels.value(actionId, actionId);
}

QString knobOutputLabel(KnobOutput output) {
    switch (output) {
    case KnobOutput::WheelA: return QStringLiteral("Wheel A (relative)");
    case KnobOutput::WheelB: return QStringLiteral("Wheel B (relative)");
    case KnobOutput::WheelBReverse: return QStringLiteral("Wheel B reversed");
    case KnobOutput::SliderA: return QStringLiteral("Slider A (pickup)");
    case KnobOutput::SliderB: return QStringLiteral("Slider B (pickup)");
    case KnobOutput::Button: return QStringLiteral("Button direction pair");
    }
    return QString();
}

QString knobOutputId(KnobOutput output) { return outputName(output); }

QString knobOutputDescription(KnobOutput output) {
    switch (output) {
    case KnobOutput::WheelA:
        return QStringLiteral(
            "Relative CC centered on 64: values above 64 are positive, values below 64 are negative, and the distance from 64 preserves acceleration. Use when that CTR2 knob mode is configured as Wheel A. Map 1 uses this for CC100.");
    case KnobOutput::WheelB:
        return QStringLiteral(
            "Relative CC direction: value 1 is a positive step and value 126 is a negative step. Use when that CTR2 knob mode is configured as Wheel B.");
    case KnobOutput::WheelBReverse:
        return QStringLiteral(
            "Reversed Wheel B direction: value 1 is a negative step and value 126 is a positive step.");
    case KnobOutput::SliderA:
        return QStringLiteral(
            "Absolute CC values 0 through 127 from a CTR2 Slider A output. The first report establishes position; later movement produces fine signed steps, including across 0/127 wrap. Map 1 uses this for CC101 through CC107.");
    case KnobOutput::SliderB:
        return QStringLiteral(
            "Absolute CC values 0 through 127 from a CTR2 Slider B output. It is decoded like sliderA; use it when that CTR2 knob mode is configured as Slider B.");
    case KnobOutput::Button:
        return QStringLiteral(
            "Directional NoteOn pair from a CTR2 MIDI Button output, not a CC value. The cc field selects the pair: CC100 is notes 40/41 through CC107 at notes 54/55; the even note is negative and the odd note is positive.");
    }
    return QString();
}

bool knobOutputFromId(const QString &id, KnobOutput *output) { return parseOutput(id, output); }

QJsonObject toJson(const DeviceMapping &mapping) {
    QJsonObject root;
    QJsonArray comments;
    comments.append(QStringLiteral(
        "This is a user-editable QK4 CTR2 mapping. Action keywords are case-sensitive."));
    comments.append(QStringLiteral(
        "For a predefined button function, copy a keyword from _buttonActions into the button's action field and remove its macro field."));
    comments.append(QStringLiteral(
        "For a custom K4 programmer command, use action \"macro\", set the button's macro field to an id, and define that id in macros. Commands must end with a semicolon."));
    comments.append(QStringLiteral(
        "The adjust_* button actions select the function controlled by a knob whose action is selected_adjustment."));
    comments.append(QStringLiteral(
        "A knob's output describes the MIDI messages emitted by that CTR2 knob mode; it does not select the radio action. The output keyword must match the output configured on the CTR2. See _knobOutputs."));
    comments.append(QStringLiteral(
        "Loading replaces the complete CTR2 mapping; mappings are never merged."));
    root.insert(QStringLiteral("_comments"), comments);

    QJsonArray knobOutputGuide;
    knobOutputGuide.append(QStringLiteral(
        "Stock K4-Control Map 1: leave CC100 set to wheelA and CC101 through CC107 set to sliderA. No output selection is required unless you reprogram those modes in CTR2-MIDI."));
    knobOutputGuide.append(QStringLiteral(
        "To choose an output, inspect that knob mode in the CTR2-MIDI map/setup and copy its output type here: Wheel A = wheelA, Wheel B = wheelB, Slider A = sliderA, Slider B = sliderB, or MIDI Button = button."));
    knobOutputGuide.append(QStringLiteral(
        "Use wheelA when the CTR2 mode emits relative values centered on 64. It is the best choice for VFO tuning or another action where faster turns should produce larger changes, but only when the CTR2 mode itself is configured as Wheel A."));
    knobOutputGuide.append(QStringLiteral(
        "Use wheelB when the CTR2 mode emits relative direction values 1 and 126. It produces one fine step per report. Use wheelB-r instead only when Wheel B moves the selected QK4 action in the wrong direction."));
    knobOutputGuide.append(QStringLiteral(
        "Use sliderA or sliderB when the CTR2 mode emits absolute values from 0 through 127. QK4 converts changes in those values into fine directional steps; it does not jump the radio control to an absolute position."));
    knobOutputGuide.append(QStringLiteral(
        "Do not choose slider merely because the QK4 control is drawn as a slider, and do not choose wheel merely because the CTR2 has a physical knob. The MIDI message format configured in CTR2-MIDI is what determines this field."));
    root.insert(QStringLiteral("_knobOutputGuide"), knobOutputGuide);

    QJsonObject buttonActionReference;
    for (const QString &action : supportedButtonActions()) {
        if (action != QStringLiteral("macro"))
            buttonActionReference.insert(action, buttonActionLabel(action));
    }
    root.insert(QStringLiteral("_buttonActions"), buttonActionReference);

    QJsonObject knobActionReference;
    for (const QString &action : supportedKnobActions())
        knobActionReference.insert(action, knobActionLabel(action));
    root.insert(QStringLiteral("_knobActions"), knobActionReference);

    QJsonObject knobOutputReference;
    for (int value = static_cast<int>(KnobOutput::WheelA);
         value <= static_cast<int>(KnobOutput::Button); ++value) {
        const auto output = static_cast<KnobOutput>(value);
        knobOutputReference.insert(knobOutputId(output), knobOutputDescription(output));
    }
    root.insert(QStringLiteral("_knobOutputs"), knobOutputReference);

    root.insert(QStringLiteral("format"), QStringLiteral("qk4-ctr2-midi-mapping"));
    root.insert(QStringLiteral("version"), FileVersion);
    root.insert(QStringLiteral("defaultsRevision"), Ctr2DefaultsRevision);
    root.insert(QStringLiteral("name"), mapping.name);
    root.insert(QStringLiteral("profile"), static_cast<int>(mapping.profile));
    root.insert(QStringLiteral("keyingMode"), static_cast<int>(mapping.keyingMode));
    root.insert(QStringLiteral("straightKeyInput"), static_cast<int>(mapping.straightKeyInput));
    root.insert(QStringLiteral("extendedButtons"), mapping.extendedButtons);
    root.insert(QStringLiteral("tipRingSwapped"), mapping.tipRingSwapped);
    root.insert(QStringLiteral("cwInputEnabled"), mapping.cwInputEnabled);
    if (mapping.profile == Profile::Custom) {
        root.insert(QStringLiteral("customDitStatus"), mapping.customDitStatus);
        root.insert(QStringLiteral("customDitData1"), mapping.customDitData1);
        root.insert(QStringLiteral("customDahStatus"), mapping.customDahStatus);
        root.insert(QStringLiteral("customDahData1"), mapping.customDahData1);
    }

    QJsonArray knobs;
    for (auto it = mapping.knobs.cbegin(); it != mapping.knobs.cend(); ++it) {
        QJsonObject entry;
        entry.insert(QStringLiteral("cc"), it.key());
        entry.insert(QStringLiteral("action"), it->action);
        entry.insert(QStringLiteral("output"), outputName(it->output));
        knobs.append(entry);
    }
    root.insert(QStringLiteral("knobs"), knobs);

    QJsonArray buttons;
    for (auto it = mapping.buttons.cbegin(); it != mapping.buttons.cend(); ++it) {
        QJsonObject entry;
        entry.insert(QStringLiteral("note"), it.key());
        entry.insert(QStringLiteral("action"), it->action);
        if (!it->macroId.isEmpty())
            entry.insert(QStringLiteral("macro"), it->macroId);
        buttons.append(entry);
    }
    root.insert(QStringLiteral("buttons"), buttons);

    QJsonArray macros;
    for (auto it = mapping.macros.cbegin(); it != mapping.macros.cend(); ++it) {
        QJsonObject entry;
        entry.insert(QStringLiteral("id"), it.key());
        entry.insert(QStringLiteral("label"), it->label);
        entry.insert(QStringLiteral("command"), it->command);
        macros.append(entry);
    }
    root.insert(QStringLiteral("macros"), macros);
    return root;
}

bool fromJson(const QJsonObject &root, DeviceMapping *mapping, QString *error) {
    const auto fail = [error](const QString &message) {
        if (error)
            *error = message;
        return false;
    };
    if (!mapping)
        return fail(QStringLiteral("No destination mapping was provided"));
    if (root.value(QStringLiteral("format")).toString() != QStringLiteral("qk4-ctr2-midi-mapping"))
        return fail(QStringLiteral("Not a QK4 CTR2 mapping file"));
    if (root.value(QStringLiteral("version")).toInt(-1) != FileVersion)
        return fail(QStringLiteral("Unsupported CTR2 mapping version"));

    DeviceMapping parsed;
    parsed.name = root.value(QStringLiteral("name")).toString(QStringLiteral("Imported Mapping"));
    const int profile = root.value(QStringLiteral("profile")).toInt(static_cast<int>(Profile::Ctr2));
    if (profile < static_cast<int>(Profile::TinyMidi) || profile > static_cast<int>(Profile::Ctr2))
        return fail(QStringLiteral("Invalid MIDI profile"));
    parsed.profile = static_cast<Profile>(profile);
    parsed.keyingMode = root.value(QStringLiteral("keyingMode")).toInt() == 1
                             ? KeyingMode::StraightKey
                             : KeyingMode::Paddles;
    parsed.straightKeyInput = root.value(QStringLiteral("straightKeyInput")).toInt() == 1
                                  ? PhysicalInput::Right
                                  : PhysicalInput::Left;
    parsed.extendedButtons = root.value(QStringLiteral("extendedButtons")).toBool(false);
    parsed.tipRingSwapped = root.value(QStringLiteral("tipRingSwapped")).toBool(false);
    parsed.cwInputEnabled = root.value(QStringLiteral("cwInputEnabled")).toBool(true);
    parsed.customDitStatus = root.value(QStringLiteral("customDitStatus")).toInt(0x90) & 0xf0;
    parsed.customDitData1 = qBound(0, root.value(QStringLiteral("customDitData1")).toInt(20), 127);
    parsed.customDahStatus = root.value(QStringLiteral("customDahStatus")).toInt(0x90) & 0xf0;
    parsed.customDahData1 = qBound(0, root.value(QStringLiteral("customDahData1")).toInt(21), 127);

    for (const QJsonValue &value : root.value(QStringLiteral("knobs")).toArray()) {
        const QJsonObject entry = value.toObject();
        const int cc = entry.value(QStringLiteral("cc")).toInt(-1);
        const QString actionId = entry.value(QStringLiteral("action")).toString();
        KnobOutput output;
        if (cc < 0 || cc > 127 || !isSupportedKnobAction(actionId) ||
            !parseOutput(entry.value(QStringLiteral("output")).toString(), &output))
            return fail(QStringLiteral("Invalid knob mapping"));
        parsed.knobs.insert(cc, KnobBinding{actionId, output});
    }

    for (const QJsonValue &value : root.value(QStringLiteral("buttons")).toArray()) {
        const QJsonObject entry = value.toObject();
        const int note = entry.value(QStringLiteral("note")).toInt(-1);
        const QString actionId = entry.value(QStringLiteral("action")).toString();
        const QString macroId = entry.value(QStringLiteral("macro")).toString();
        if (note < 0 || note > 127 || !isSupportedButtonAction(actionId) ||
            (actionId == QStringLiteral("macro") && macroId.isEmpty()))
            return fail(QStringLiteral("Invalid button mapping"));
        parsed.buttons.insert(note, ButtonBinding{actionId, macroId});
    }

    for (const QJsonValue &value : root.value(QStringLiteral("macros")).toArray()) {
        const QJsonObject entry = value.toObject();
        const QString id = entry.value(QStringLiteral("id")).toString();
        const QString command = entry.value(QStringLiteral("command")).toString();
        QString commandError;
        if (id.isEmpty() || !isValidK4Command(command, &commandError))
            return fail(commandError.isEmpty() ? QStringLiteral("Invalid macro definition")
                                                : commandError);
        parsed.macros.insert(id, MacroDefinition{entry.value(QStringLiteral("label")).toString(), command});
    }
    for (auto it = parsed.buttons.cbegin(); it != parsed.buttons.cend(); ++it) {
        if (it->action == QStringLiteral("macro") && !parsed.macros.contains(it->macroId))
            return fail(QStringLiteral("Button references a missing macro"));
    }

    // A legacy built-in map may contain customized buttons or macros while its
    // unchanged factory knob bindings still need the SliderA correction.
    // Current-revision maps preserve an explicit operator choice of WheelA.
    if (root.value(QStringLiteral("defaultsRevision")).toInt(0) < Ctr2DefaultsRevision)
        upgradeKnownCtr2Default(&parsed);
    *mapping = parsed;
    if (error)
        error->clear();
    return true;
}

KnobValue interpretKnobValue(KnobOutput output, int midiValue) {
    KnobValue result;
    if (midiValue < 0 || midiValue > 127)
        return result;
    switch (output) {
    case KnobOutput::WheelA:
        result.valid = midiValue != 64;
        result.value = midiValue - 64;
        break;
    case KnobOutput::WheelB:
        result.valid = midiValue == 1 || midiValue == 126;
        result.value = midiValue == 1 ? 1 : -1;
        break;
    case KnobOutput::WheelBReverse:
        result.valid = midiValue == 1 || midiValue == 126;
        result.value = midiValue == 1 ? -1 : 1;
        break;
    case KnobOutput::SliderA:
    case KnobOutput::SliderB:
        result.valid = true;
        result.absolute = true;
        result.value = midiValue;
        break;
    case KnobOutput::Button:
        break;
    }
    return result;
}

bool &InputAggregator::stateRef(SourceState &state, LogicalInput input) {
    switch (input) {
    case LogicalInput::Dit: return state.dit;
    case LogicalInput::Dah: return state.dah;
    case LogicalInput::StraightKey: return state.straight;
    case LogicalInput::Ptt: return state.ptt;
    }
    return state.dit;
}

bool InputAggregator::stateValue(const SourceState &state, LogicalInput input) {
    switch (input) {
    case LogicalInput::Dit: return state.dit;
    case LogicalInput::Dah: return state.dah;
    case LogicalInput::StraightKey: return state.straight;
    case LogicalInput::Ptt: return state.ptt;
    }
    return false;
}

bool InputAggregator::aggregateState(LogicalInput input) const {
    for (const SourceState &state : m_sources) {
        if (stateValue(state, input))
            return true;
    }
    return false;
}

QVector<InputTransition> InputAggregator::setInput(const QString &sourceId, LogicalInput input, bool pressed) {
    const bool before = aggregateState(input);
    SourceState &source = m_sources[sourceId];
    stateRef(source, input) = pressed;
    const bool after = aggregateState(input);
    if (before == after)
        return {};
    return {{input, after}};
}

QVector<InputTransition> InputAggregator::clearSource(const QString &sourceId) {
    QVector<InputTransition> transitions;
    if (!m_sources.contains(sourceId))
        return transitions;
    const bool before[] = {aggregateState(LogicalInput::Dit), aggregateState(LogicalInput::Dah),
                           aggregateState(LogicalInput::StraightKey), aggregateState(LogicalInput::Ptt)};
    m_sources.remove(sourceId);
    const LogicalInput inputs[] = {LogicalInput::Dit, LogicalInput::Dah, LogicalInput::StraightKey,
                                   LogicalInput::Ptt};
    for (int i = 0; i < 4; ++i) {
        const bool after = aggregateState(inputs[i]);
        if (before[i] != after)
            transitions.append({inputs[i], after});
    }
    return transitions;
}

} // namespace MidiMapping
