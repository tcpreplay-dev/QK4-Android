#include "models/radiostate.h"

#include <QSignalSpy>
#include <QtTest>

class RadioStateTest : public QObject {
    Q_OBJECT

private slots:
    void transmitQueryConfirmsState();
    void malformedTransmitQueryIsIgnored();
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

QTEST_MAIN(RadioStateTest)
#include "test_radiostate.moc"
