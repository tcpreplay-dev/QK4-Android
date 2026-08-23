#include "models/radiostate.h"

#include <QSignalSpy>
#include <QtTest>

class RadioStateTest : public QObject {
    Q_OBJECT

private slots:
    void transmitQueryConfirmsState();
    void malformedTransmitQueryIsIgnored();
    void reverseDataSubModesUseDistinctDisplayLabels();
};

void RadioStateTest::transmitQueryConfirmsState() {
    RadioState state;
    QSignalSpy spy(&state, &RadioState::transmitStateChanged);

    state.parseCATCommand(QStringLiteral("TQ1;"));
    QVERIFY(state.isTransmitting());
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.takeFirst().at(0).toBool(), true);

    // Repeated confirmation must not create a false state transition.
    state.parseCATCommand(QStringLiteral("TQ1;"));
    QCOMPARE(spy.size(), 0);

    state.parseCATCommand(QStringLiteral("TQ0;"));
    QVERIFY(!state.isTransmitting());
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.takeFirst().at(0).toBool(), false);
}

void RadioStateTest::malformedTransmitQueryIsIgnored() {
    RadioState state;
    QSignalSpy spy(&state, &RadioState::transmitStateChanged);

    state.parseCATCommand(QStringLiteral("TQ;"));
    state.parseCATCommand(QStringLiteral("TQX;"));
    state.parseCATCommand(QStringLiteral("TQ2;"));
    QVERIFY(!state.isTransmitting());
    QCOMPARE(spy.size(), 0);
}

void RadioStateTest::reverseDataSubModesUseDistinctDisplayLabels() {
    RadioState state;

    state.parseCATCommand(QStringLiteral("MD6;"));
    state.parseCATCommand(QStringLiteral("DT0;"));
    QCOMPARE(state.modeStringFull(), QStringLiteral("DATA"));

    state.parseCATCommand(QStringLiteral("MD9;"));
    QCOMPARE(state.modeStringFull(), QStringLiteral("DATA-R"));
    state.parseCATCommand(QStringLiteral("DT1;"));
    QCOMPARE(state.modeStringFull(), QStringLiteral("AFSK-R"));
    state.parseCATCommand(QStringLiteral("DT2;"));
    QCOMPARE(state.modeStringFull(), QStringLiteral("FSK-R"));
    state.parseCATCommand(QStringLiteral("DT3;"));
    QCOMPARE(state.modeStringFull(), QStringLiteral("PSK-R"));

    state.parseCATCommand(QStringLiteral("MD$9;"));
    state.parseCATCommand(QStringLiteral("DT$1;"));
    QCOMPARE(state.modeStringFullB(), QStringLiteral("AFSK-R"));
    state.parseCATCommand(QStringLiteral("MD$6;"));
    QCOMPARE(state.modeStringFullB(), QStringLiteral("AFSK"));
}

QTEST_MAIN(RadioStateTest)
#include "test_radiostate.moc"
