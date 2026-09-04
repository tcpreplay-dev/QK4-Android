#include "hardware/midiinputrouter.h"
#include "hardware/midimapping.h"

#include <QJsonDocument>
#include <QSignalSpy>
#include <QtTest>

class TestMidiMapping : public QObject {
    Q_OBJECT

private slots:
    void ctr2DefaultsMatchK4Control();
    void ctr2ExtendedDefaultsRepeatAcrossModes();
    void jsonRoundTrip();
    void exportedFileDocumentsEveryPredefinedAction();
    void rejectsIncompleteMacro();
    void validatesK4CommandText();
    void wheelFormats();
    void buttonActionsAreGroupedAndSorted();
    void aggregatesTwoPaddleSources();
    void aggregatesLogicalCwAndCtr2Sources();
    void ctr2StraightKeyAndPttSwap();
    void ctr2PaddleModePtt();
    void tinyMidiStraightKeySelection();
    void routesButtonMacroOnRelease();
    void routesLearnedCustomMessages();
    void sliderEstablishesBaselineBeforeMoving();
    void ctr2SliderControlsUseSingleSignedSteps();
    void cc104UsesPositionDeltasAndIgnoresDuplicates();
    void ctr2SliderDirectionContinuesAcrossEndpoints();
    void upgradesLegacyBuiltInKnobFormatsOnly();
    void routesKnobButtonDirectionPairs();
    void adjustmentSelectorsCoverTypedKnobs();
    void routesAdjustmentSelectorOnRelease();
};

void TestMidiMapping::ctr2DefaultsMatchK4Control() {
    const auto mapping = MidiMapping::ctr2Default();
    QCOMPARE(mapping.knobs.value(100).action, QStringLiteral("active_vfo_frequency"));
    QCOMPARE(mapping.knobs.value(100).output, MidiMapping::KnobOutput::WheelA);
    QCOMPARE(mapping.knobs.value(101).action, QStringLiteral("main_volume"));
    QCOMPARE(mapping.knobs.value(102).action, QStringLiteral("other_vfo_frequency"));
    QCOMPARE(mapping.knobs.value(103).action, QStringLiteral("filter_bandwidth"));
    QCOMPARE(mapping.knobs.value(104).action, QStringLiteral("rit_xit_frequency"));
    QCOMPARE(mapping.knobs.value(105).action, QStringLiteral("nr_level"));
    QCOMPARE(mapping.knobs.value(106).action, QStringLiteral("rf_power"));
    QCOMPARE(mapping.knobs.value(107).action, QStringLiteral("cw_speed"));
    for (int cc = 101; cc <= 107; ++cc)
        QCOMPARE(mapping.knobs.value(cc).output, MidiMapping::KnobOutput::SliderA);
    QCOMPARE(mapping.buttons.value(1).action, QStringLiteral("mode_next"));
    QCOMPARE(mapping.buttons.value(16).action, QStringLiteral("tune"));
    QVERIFY(MidiMapping::supportedButtonActions().contains(QStringLiteral("tx_rx_toggle")));
    QCOMPARE(MidiMapping::buttonActionLabel(QStringLiteral("tx_rx_toggle")),
             QStringLiteral("TX/RX toggle"));
    QCOMPARE(MidiMapping::buttonActionLabel(QStringLiteral("tune_step")),
             QStringLiteral("Rate"));
    QVERIFY(MidiMapping::supportedButtonActions().contains(QStringLiteral("khz")));
    QCOMPARE(MidiMapping::buttonActionLabel(QStringLiteral("khz")), QStringLiteral("KHZ"));
}

void TestMidiMapping::ctr2ExtendedDefaultsRepeatAcrossModes() {
    const auto mapping = MidiMapping::ctr2ExtendedDefault();
    QVERIFY(mapping.extendedButtons);
    QCOMPARE(mapping.buttons.size(), 48);
    QCOMPARE(mapping.buttons.value(1).action, QStringLiteral("mode_next"));
    QCOMPARE(mapping.buttons.value(7).action, QStringLiteral("mode_next"));
    QCOMPARE(mapping.buttons.value(25).action, QStringLiteral("mode_previous"));
    QCOMPARE(mapping.buttons.value(48).action, QStringLiteral("tune"));
}

void TestMidiMapping::jsonRoundTrip() {
    auto original = MidiMapping::ctr2Default();
    original.name = QStringLiteral("Portable Contest Map");
    original.keyingMode = MidiMapping::KeyingMode::StraightKey;
    original.tipRingSwapped = true;
    original.macros.insert(QStringLiteral("cq"), {QStringLiteral("CQ"), QStringLiteral("KY CQ CQ;")});
    original.buttons.insert(5, {QStringLiteral("macro"), QStringLiteral("cq")});

    const QJsonObject json = MidiMapping::toJson(original);
    MidiMapping::DeviceMapping decoded;
    QString error;
    QVERIFY2(MidiMapping::fromJson(json, &decoded, &error), qPrintable(error));
    QCOMPARE(decoded, original);
}

void TestMidiMapping::exportedFileDocumentsEveryPredefinedAction() {
    const QJsonObject json = MidiMapping::toJson(MidiMapping::ctr2Default());
    const QJsonArray comments = json.value(QStringLiteral("_comments")).toArray();
    QVERIFY(!comments.isEmpty());

    const QJsonArray knobOutputGuide = json.value(QStringLiteral("_knobOutputGuide")).toArray();
    QCOMPARE(knobOutputGuide.size(), 6);
    for (const QJsonValue &entry : knobOutputGuide)
        QVERIFY(!entry.toString().isEmpty());
    QVERIFY(knobOutputGuide.at(0).toString().contains(QStringLiteral("CC100")));
    QVERIFY(knobOutputGuide.at(0).toString().contains(QStringLiteral("CC101")));

    const QJsonObject buttonActions = json.value(QStringLiteral("_buttonActions")).toObject();
    for (const QString &action : MidiMapping::supportedButtonActions()) {
        if (action == QStringLiteral("macro"))
            continue;
        QVERIFY2(buttonActions.contains(action), qPrintable(action));
        QCOMPARE(buttonActions.value(action).toString(), MidiMapping::buttonActionLabel(action));
    }
    QCOMPARE(buttonActions.size(), MidiMapping::supportedButtonActions().size() - 1);
    QVERIFY(!buttonActions.contains(QStringLiteral("macro")));

    const QJsonObject knobActions = json.value(QStringLiteral("_knobActions")).toObject();
    for (const QString &action : MidiMapping::supportedKnobActions()) {
        QVERIFY2(knobActions.contains(action), qPrintable(action));
        QCOMPARE(knobActions.value(action).toString(), MidiMapping::knobActionLabel(action));
    }
    QCOMPARE(knobActions.size(), MidiMapping::supportedKnobActions().size());

    const QJsonObject knobOutputs = json.value(QStringLiteral("_knobOutputs")).toObject();
    for (int value = static_cast<int>(MidiMapping::KnobOutput::WheelA);
         value <= static_cast<int>(MidiMapping::KnobOutput::Button); ++value) {
        const auto output = static_cast<MidiMapping::KnobOutput>(value);
        const QString id = MidiMapping::knobOutputId(output);
        QVERIFY2(knobOutputs.contains(id), qPrintable(id));
        QCOMPARE(knobOutputs.value(id).toString(), MidiMapping::knobOutputDescription(output));
        QVERIFY(!knobOutputs.value(id).toString().isEmpty());
    }
    QCOMPARE(knobOutputs.size(), 6);
}

void TestMidiMapping::rejectsIncompleteMacro() {
    QJsonObject json = MidiMapping::toJson(MidiMapping::ctr2Default());
    QJsonArray buttons = json.value(QStringLiteral("buttons")).toArray();
    QJsonObject bad;
    bad.insert(QStringLiteral("note"), 5);
    bad.insert(QStringLiteral("action"), QStringLiteral("macro"));
    bad.insert(QStringLiteral("macro"), QStringLiteral("missing"));
    buttons.append(bad);
    json.insert(QStringLiteral("buttons"), buttons);
    MidiMapping::DeviceMapping decoded;
    QString error;
    QVERIFY(!MidiMapping::fromJson(json, &decoded, &error));
    QVERIFY(error.contains(QStringLiteral("missing"), Qt::CaseInsensitive));
}

void TestMidiMapping::validatesK4CommandText() {
    QVERIFY(MidiMapping::isValidK4Command(QStringLiteral("SWT13;")));
    QVERIFY(MidiMapping::isValidK4Command(QStringLiteral("KY CQ CQ;")));
    QVERIFY(MidiMapping::isValidK4Command(QStringLiteral("FA;FB;")));
    QVERIFY(!MidiMapping::isValidK4Command(QStringLiteral("SWT13")));
    QVERIFY(!MidiMapping::isValidK4Command(QStringLiteral("KY CQ\nCQ;")));
}

void TestMidiMapping::wheelFormats() {
    auto value = MidiMapping::interpretKnobValue(MidiMapping::KnobOutput::WheelA, 114);
    QVERIFY(value.valid);
    QVERIFY(!value.absolute);
    QCOMPARE(value.value, 50);

    value = MidiMapping::interpretKnobValue(MidiMapping::KnobOutput::WheelB, 126);
    QCOMPARE(value.value, -1);
    value = MidiMapping::interpretKnobValue(MidiMapping::KnobOutput::WheelBReverse, 126);
    QCOMPARE(value.value, 1);
    value = MidiMapping::interpretKnobValue(MidiMapping::KnobOutput::SliderA, 99);
    QVERIFY(value.absolute);
    QCOMPARE(value.value, 99);
}

void TestMidiMapping::buttonActionsAreGroupedAndSorted() {
    const QStringList actions = MidiMapping::supportedButtonActions();
    QVERIFY(!actions.isEmpty());
    QCOMPARE(actions.first(), QStringLiteral("macro"));
    QCOMPARE(MidiMapping::buttonActionLabel(actions.first()), QStringLiteral("Custom Command"));

    bool reachedAdjustments = false;
    QString previousPredefined;
    QString previousAdjustment;
    for (qsizetype index = 1; index < actions.size(); ++index) {
        const QString label = MidiMapping::buttonActionLabel(actions.at(index));
        const bool adjustment = label.startsWith(QStringLiteral("Adjust:"));
        if (adjustment) {
            reachedAdjustments = true;
            if (!previousAdjustment.isEmpty())
                QVERIFY(QString::compare(previousAdjustment, label, Qt::CaseInsensitive) <= 0);
            previousAdjustment = label;
        } else {
            QVERIFY(!reachedAdjustments);
            if (!previousPredefined.isEmpty())
                QVERIFY(QString::compare(previousPredefined, label, Qt::CaseInsensitive) <= 0);
            previousPredefined = label;
        }
    }
    QVERIFY(reachedAdjustments);
}

void TestMidiMapping::aggregatesTwoPaddleSources() {
    MidiInputRouter router;
    router.setMapping(QStringLiteral("tiny"), MidiMapping::tinyMidiDefault());
    router.setMapping(QStringLiteral("ctr2"), MidiMapping::ctr2Default());
    QSignalSpy ditSpy(&router, &MidiInputRouter::ditStateChanged);

    router.processEvent(QStringLiteral("tiny"), 0x90, 20, 127);
    router.processEvent(QStringLiteral("ctr2"), 0x90, 20, 127);
    QCOMPARE(ditSpy.count(), 1);
    QCOMPARE(ditSpy.at(0).at(0).toBool(), true);

    router.processEvent(QStringLiteral("tiny"), 0x80, 20, 0);
    QCOMPARE(ditSpy.count(), 1); // CTR2 still holds DIT.
    router.removeSource(QStringLiteral("ctr2"));
    QCOMPARE(ditSpy.count(), 2);
    QCOMPARE(ditSpy.at(1).at(0).toBool(), false);
}

void TestMidiMapping::aggregatesLogicalCwAndCtr2Sources() {
    MidiInputRouter router;
    router.setMapping(QStringLiteral("ctr2-midi"), MidiMapping::ctr2Default());
    QSignalSpy ditSpy(&router, &MidiInputRouter::ditStateChanged);

    router.setLogicalInput(QStringLiteral("cw-midi"), MidiMapping::LogicalInput::Dit, true);
    router.processEvent(QStringLiteral("ctr2-midi"), 0x90, 20, 127);
    QCOMPARE(ditSpy.count(), 1);
    QVERIFY(router.logicalInputActive(MidiMapping::LogicalInput::Dit));

    router.clearSourceState(QStringLiteral("cw-midi"));
    QCOMPARE(ditSpy.count(), 1); // CTR2 still holds DIT.
    router.clearSourceState(QStringLiteral("ctr2-midi"));
    QCOMPARE(ditSpy.count(), 2);
    QCOMPARE(ditSpy.at(1).at(0).toBool(), false);
    QVERIFY(!router.logicalInputActive(MidiMapping::LogicalInput::Dit));
}

void TestMidiMapping::ctr2StraightKeyAndPttSwap() {
    MidiInputRouter router;
    auto mapping = MidiMapping::ctr2Default();
    mapping.keyingMode = MidiMapping::KeyingMode::StraightKey;
    router.setMapping(QStringLiteral("ctr2"), mapping);
    QSignalSpy keySpy(&router, &MidiInputRouter::straightKeyStateChanged);
    QSignalSpy pttSpy(&router, &MidiInputRouter::pttStateChanged);

    router.processEvent(QStringLiteral("ctr2"), 0x90, 30, 127);
    router.processEvent(QStringLiteral("ctr2"), 0x80, 30, 0);
    QCOMPARE(keySpy.count(), 2);
    router.processEvent(QStringLiteral("ctr2"), 0x90, 31, 127);
    QCOMPARE(pttSpy.count(), 1);

    mapping.tipRingSwapped = true;
    router.setMapping(QStringLiteral("ctr2"), mapping);
    router.processEvent(QStringLiteral("ctr2"), 0x90, 31, 127);
    QCOMPARE(keySpy.count(), 3);
}

void TestMidiMapping::ctr2PaddleModePtt() {
    MidiInputRouter router;
    router.setMapping(QStringLiteral("ctr2"), MidiMapping::ctr2Default());
    QSignalSpy pttSpy(&router, &MidiInputRouter::pttStateChanged);
    router.processEvent(QStringLiteral("ctr2"), 0x90, 31, 127);
    router.processEvent(QStringLiteral("ctr2"), 0x80, 31, 0);
    QCOMPARE(pttSpy.count(), 2);
}

void TestMidiMapping::tinyMidiStraightKeySelection() {
    MidiInputRouter router;
    auto mapping = MidiMapping::tinyMidiDefault();
    mapping.keyingMode = MidiMapping::KeyingMode::StraightKey;
    mapping.straightKeyInput = MidiMapping::PhysicalInput::Right;
    router.setMapping(QStringLiteral("tiny"), mapping);
    QSignalSpy keySpy(&router, &MidiInputRouter::straightKeyStateChanged);

    router.processEvent(QStringLiteral("tiny"), 0x90, 20, 127);
    QCOMPARE(keySpy.count(), 0);
    router.processEvent(QStringLiteral("tiny"), 0x90, 21, 127);
    router.processEvent(QStringLiteral("tiny"), 0x80, 21, 0);
    QCOMPARE(keySpy.count(), 2);
}

void TestMidiMapping::routesButtonMacroOnRelease() {
    MidiInputRouter router;
    auto mapping = MidiMapping::ctr2Default();
    mapping.macros.insert(QStringLiteral("f3"), {QStringLiteral("F3"), QStringLiteral("SWT13;")});
    mapping.buttons.insert(5, {QStringLiteral("macro"), QStringLiteral("f3")});
    router.setMapping(QStringLiteral("ctr2"), mapping);
    QSignalSpy macroSpy(&router, &MidiInputRouter::macroRequested);

    router.processEvent(QStringLiteral("ctr2"), 0x90, 5, 127);
    QCOMPARE(macroSpy.count(), 0);
    router.processEvent(QStringLiteral("ctr2"), 0x80, 5, 0);
    QCOMPARE(macroSpy.count(), 1);
    QCOMPARE(macroSpy.at(0).at(1).toString(), QStringLiteral("SWT13;"));
}

void TestMidiMapping::routesLearnedCustomMessages() {
    MidiInputRouter router;
    auto mapping = MidiMapping::tinyMidiDefault();
    mapping.profile = MidiMapping::Profile::Custom;
    mapping.customDitStatus = 0xb0;
    mapping.customDitData1 = 44;
    mapping.customDahStatus = 0x90;
    mapping.customDahData1 = 45;
    router.setMapping(QStringLiteral("custom"), mapping);
    QSignalSpy ditSpy(&router, &MidiInputRouter::ditStateChanged);
    QSignalSpy dahSpy(&router, &MidiInputRouter::dahStateChanged);

    router.processEvent(QStringLiteral("custom"), 0xb0, 44, 127);
    router.processEvent(QStringLiteral("custom"), 0xb0, 44, 0);
    router.processEvent(QStringLiteral("custom"), 0x90, 45, 127);
    router.processEvent(QStringLiteral("custom"), 0x80, 45, 0);
    QCOMPARE(ditSpy.count(), 2);
    QCOMPARE(dahSpy.count(), 2);
}

void TestMidiMapping::sliderEstablishesBaselineBeforeMoving() {
    MidiInputRouter router;
    auto mapping = MidiMapping::ctr2Default();
    mapping.knobs[101].output = MidiMapping::KnobOutput::SliderA;
    router.setMapping(QStringLiteral("ctr2"), mapping);
    QSignalSpy knobSpy(&router, &MidiInputRouter::knobActionRequested);

    router.processEvent(QStringLiteral("ctr2"), 0xb0, 101, 70);
    QCOMPARE(knobSpy.count(), 0);
    router.processEvent(QStringLiteral("ctr2"), 0xb0, 101, 77);
    QCOMPARE(knobSpy.count(), 1);
    QCOMPARE(knobSpy.at(0).at(1).toInt(), 1);
    QCOMPARE(knobSpy.at(0).at(2).toBool(), false);
    router.processEvent(QStringLiteral("ctr2"), 0xb0, 101, 75);
    QCOMPARE(knobSpy.at(1).at(1).toInt(), -1);

    router.clearSourceState(QStringLiteral("ctr2"));
    router.processEvent(QStringLiteral("ctr2"), 0xb0, 101, 90);
    QCOMPARE(knobSpy.count(), 2);
}

void TestMidiMapping::ctr2SliderControlsUseSingleSignedSteps() {
    MidiInputRouter router;
    router.setMapping(QStringLiteral("ctr2"), MidiMapping::ctr2Default());
    QSignalSpy knobSpy(&router, &MidiInputRouter::knobActionRequested);

    const int controls[] = {102, 106, 107};
    const QString actions[] = {QStringLiteral("other_vfo_frequency"),
                               QStringLiteral("rf_power"),
                               QStringLiteral("cw_speed")};
    for (int index = 0; index < 3; ++index) {
        const int cc = controls[index];
        router.processEvent(QStringLiteral("ctr2"), 0xb0, cc, 20);
        router.processEvent(QStringLiteral("ctr2"), 0xb0, cc, 90);
        QCOMPARE(knobSpy.last().at(0).toString(), actions[index]);
        QCOMPARE(knobSpy.last().at(1).toInt(), 1);
        router.processEvent(QStringLiteral("ctr2"), 0xb0, cc, 30);
        QCOMPARE(knobSpy.last().at(0).toString(), actions[index]);
        QCOMPARE(knobSpy.last().at(1).toInt(), -1);
    }
    QCOMPARE(knobSpy.count(), 6);
}

void TestMidiMapping::cc104UsesPositionDeltasAndIgnoresDuplicates() {
    MidiInputRouter router;
    router.setMapping(QStringLiteral("ctr2"), MidiMapping::ctr2Default());
    QSignalSpy knobSpy(&router, &MidiInputRouter::knobActionRequested);

    router.processEvent(QStringLiteral("ctr2"), 0xb0, 104, 26);
    router.processEvent(QStringLiteral("ctr2"), 0xb0, 104, 26);
    QCOMPARE(knobSpy.count(), 0);

    router.processEvent(QStringLiteral("ctr2"), 0xb0, 104, 27);
    QCOMPARE(knobSpy.count(), 1);
    QCOMPARE(knobSpy.at(0).at(0).toString(), QStringLiteral("rit_xit_frequency"));
    QCOMPARE(knobSpy.at(0).at(1).toInt(), 1);
}

void TestMidiMapping::ctr2SliderDirectionContinuesAcrossEndpoints() {
    MidiInputRouter router;
    router.setMapping(QStringLiteral("ctr2"), MidiMapping::ctr2Default());
    QSignalSpy knobSpy(&router, &MidiInputRouter::knobActionRequested);

    // Clockwise movement remains clockwise when the absolute slider reaches
    // 127, repeats there, and then wraps to zero.
    router.processEvent(QStringLiteral("ctr2"), 0xb0, 103, 126);
    router.processEvent(QStringLiteral("ctr2"), 0xb0, 103, 127);
    router.processEvent(QStringLiteral("ctr2"), 0xb0, 103, 127);
    router.processEvent(QStringLiteral("ctr2"), 0xb0, 103, 0);
    QCOMPARE(knobSpy.count(), 3);
    for (int index = 0; index < knobSpy.count(); ++index)
        QCOMPARE(knobSpy.at(index).at(1).toInt(), 1);

    router.clearSourceState(QStringLiteral("ctr2"));
    knobSpy.clear();

    // Counterclockwise movement behaves symmetrically at the lower endpoint.
    router.processEvent(QStringLiteral("ctr2"), 0xb0, 103, 1);
    router.processEvent(QStringLiteral("ctr2"), 0xb0, 103, 0);
    router.processEvent(QStringLiteral("ctr2"), 0xb0, 103, 0);
    router.processEvent(QStringLiteral("ctr2"), 0xb0, 103, 127);
    QCOMPARE(knobSpy.count(), 3);
    for (int index = 0; index < knobSpy.count(); ++index)
        QCOMPARE(knobSpy.at(index).at(1).toInt(), -1);
}

void TestMidiMapping::upgradesLegacyBuiltInKnobFormatsOnly() {
    auto allWheelAJson = MidiMapping::toJson(MidiMapping::ctr2Default());
    allWheelAJson.remove(QStringLiteral("defaultsRevision"));
    QJsonArray knobs = allWheelAJson.value(QStringLiteral("knobs")).toArray();
    for (int index = 0; index < knobs.size(); ++index) {
        QJsonObject binding = knobs.at(index).toObject();
        binding.insert(QStringLiteral("output"), QStringLiteral("wheelA"));
        knobs[index] = binding;
    }
    allWheelAJson.insert(QStringLiteral("knobs"), knobs);

    MidiMapping::DeviceMapping decoded;
    QString error;
    QVERIFY2(MidiMapping::fromJson(allWheelAJson, &decoded, &error), qPrintable(error));
    for (int cc = 101; cc <= 107; ++cc)
        QCOMPARE(decoded.knobs.value(cc).output, MidiMapping::KnobOutput::SliderA);

    // A button customization must not block migration of the still-legacy
    // factory knob formats, and the customized button must be preserved.
    QJsonObject customizedButtonJson = allWheelAJson;
    QJsonArray buttons = customizedButtonJson.value(QStringLiteral("buttons")).toArray();
    for (int index = 0; index < buttons.size(); ++index) {
        QJsonObject binding = buttons.at(index).toObject();
        if (binding.value(QStringLiteral("note")).toInt() == 3) {
            binding.insert(QStringLiteral("action"), QStringLiteral("tx_rx_toggle"));
            buttons[index] = binding;
        }
    }
    customizedButtonJson.insert(QStringLiteral("buttons"), buttons);
    QVERIFY2(MidiMapping::fromJson(customizedButtonJson, &decoded, &error), qPrintable(error));
    QCOMPARE(decoded.buttons.value(3).action, QStringLiteral("tx_rx_toggle"));
    for (int cc = 101; cc <= 107; ++cc)
        QCOMPARE(decoded.knobs.value(cc).output, MidiMapping::KnobOutput::SliderA);

    QJsonObject interimJson = allWheelAJson;
    knobs = interimJson.value(QStringLiteral("knobs")).toArray();
    for (int index = 0; index < knobs.size(); ++index) {
        QJsonObject binding = knobs.at(index).toObject();
        if (binding.value(QStringLiteral("cc")).toInt() == 104) {
            binding.insert(QStringLiteral("output"), QStringLiteral("sliderA"));
            knobs[index] = binding;
        }
    }
    interimJson.insert(QStringLiteral("knobs"), knobs);
    QVERIFY2(MidiMapping::fromJson(interimJson, &decoded, &error), qPrintable(error));
    for (int cc = 101; cc <= 107; ++cc)
        QCOMPARE(decoded.knobs.value(cc).output, MidiMapping::KnobOutput::SliderA);

    allWheelAJson.insert(QStringLiteral("name"), QStringLiteral("Operator Custom Map"));
    QVERIFY2(MidiMapping::fromJson(allWheelAJson, &decoded, &error), qPrintable(error));
    for (int cc = 101; cc <= 107; ++cc)
        QCOMPARE(decoded.knobs.value(cc).output, MidiMapping::KnobOutput::WheelA);

    // Once saved by the corrected build, an explicit WheelA selection remains
    // user-definable even when the mapping retains the built-in name.
    QJsonObject currentJson = MidiMapping::toJson(MidiMapping::ctr2Default());
    knobs = currentJson.value(QStringLiteral("knobs")).toArray();
    for (int index = 0; index < knobs.size(); ++index) {
        QJsonObject binding = knobs.at(index).toObject();
        if (binding.value(QStringLiteral("cc")).toInt() == 102) {
            binding.insert(QStringLiteral("output"), QStringLiteral("wheelA"));
            knobs[index] = binding;
        }
    }
    currentJson.insert(QStringLiteral("knobs"), knobs);
    QVERIFY2(MidiMapping::fromJson(currentJson, &decoded, &error), qPrintable(error));
    QCOMPARE(decoded.knobs.value(102).output, MidiMapping::KnobOutput::WheelA);
}

void TestMidiMapping::routesKnobButtonDirectionPairs() {
    MidiInputRouter router;
    auto mapping = MidiMapping::ctr2ExtendedDefault();
    mapping.knobs[103].output = MidiMapping::KnobOutput::Button;
    router.setMapping(QStringLiteral("ctr2"), mapping);
    QSignalSpy knobSpy(&router, &MidiInputRouter::knobActionRequested);
    QSignalSpy buttonSpy(&router, &MidiInputRouter::buttonActionRequested);

    router.processEvent(QStringLiteral("ctr2"), 0x90, 46, 127);
    router.processEvent(QStringLiteral("ctr2"), 0x80, 46, 0);
    router.processEvent(QStringLiteral("ctr2"), 0x90, 47, 127);
    QCOMPARE(knobSpy.count(), 2);
    QCOMPARE(knobSpy.at(0).at(0).toString(), QStringLiteral("filter_bandwidth"));
    QCOMPARE(knobSpy.at(0).at(1).toInt(), -1);
    QCOMPARE(knobSpy.at(1).at(1).toInt(), 1);
    QCOMPARE(buttonSpy.count(), 0);
}

void TestMidiMapping::adjustmentSelectorsCoverTypedKnobs() {
    const QStringList knobActions = MidiMapping::supportedKnobActions();
    const QStringList buttonActions = MidiMapping::supportedButtonActions();
    for (const QString &knobAction : knobActions) {
        if (knobAction == QStringLiteral("disabled")
            || knobAction == QStringLiteral("selected_adjustment")) {
            continue;
        }
        const QString selector = QStringLiteral("adjust_") + knobAction;
        QVERIFY2(buttonActions.contains(selector), qPrintable(selector));
        QCOMPARE(MidiMapping::knobActionForButtonAction(selector), knobAction);
    }
    QVERIFY(MidiMapping::knobActionForButtonAction(QStringLiteral("nr_toggle")).isEmpty());
}

void TestMidiMapping::routesAdjustmentSelectorOnRelease() {
    MidiInputRouter router;
    auto mapping = MidiMapping::ctr2Default();
    mapping.buttons[5] = {QStringLiteral("adjust_noise_blanker_level"), QString()};
    router.setMapping(QStringLiteral("ctr2"), mapping);
    QSignalSpy buttonSpy(&router, &MidiInputRouter::buttonActionRequested);

    router.processEvent(QStringLiteral("ctr2"), 0x90, 5, 127);
    QCOMPARE(buttonSpy.count(), 0);
    router.processEvent(QStringLiteral("ctr2"), 0x80, 5, 0);
    QCOMPARE(buttonSpy.count(), 1);
    QCOMPARE(buttonSpy.at(0).at(0).toString(), QStringLiteral("adjust_noise_blanker_level"));
}

QTEST_APPLESS_MAIN(TestMidiMapping)
#include "test_midimapping.moc"
