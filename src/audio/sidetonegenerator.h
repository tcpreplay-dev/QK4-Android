#ifndef SIDETONEGENERATOR_H
#define SIDETONEGENERATOR_H

#include <QObject>
#include <QAudioSink>
#include <QIODevice>
#include <QByteArray>
#include <QPointer>
#include <QtMath>
#include <atomic>

class QTimer;
class QMediaDevices;

class SidetoneGenerator : public QObject {
    Q_OBJECT
public:
    explicit SidetoneGenerator(QObject *parent = nullptr);
    ~SidetoneGenerator();

    void setFrequency(int hz);
    void setVolume(float volume);
    void setKeyerSpeed(int wpm);

    // Initialize/shutdown audio (called on sidetone thread via invokeMethod)
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    // Start repeating element while paddle is held (V14 modem-line interface)
    Q_INVOKABLE void startDit();
    Q_INVOKABLE void startDah();
    Q_INVOKABLE void stopElement(); // Call when paddle is released

    // Play a single element without repeat (MIDI interface — K4 keyer handles repeat)
    Q_INVOKABLE void playSingleDit();
    Q_INVOKABLE void playSingleDah();

signals:
    // Emitted when repeat timer fires (for sending KZ commands)
    void ditRepeated();
    void dahRepeated();

private slots:
    void onRepeatTimer();
    void scheduleAudioRouteRefresh();
    void refreshAudioRoute();
#ifdef Q_OS_ANDROID
    void pollAndroidAudioRoute();
#endif

private:
    void initAudio();
    void destroyAudio();
    bool ensureAudioReady();
    void playElement(int durationMs);
    int ditDurationMs() const;
    int dahDurationMs() const;

    enum Element { ElementNone, ElementDit, ElementDah };

    QAudioSink *m_audioSink = nullptr;
    // QAudioSink owns the push-mode QIODevice. Android audio-route and app
    // lifecycle changes can invalidate that child independently of this
    // generator, so a guarded pointer is required here.
    QPointer<QIODevice> m_pushDevice;
    QTimer *m_repeatTimer = nullptr;
    QMediaDevices *m_mediaDevices = nullptr;
    QTimer *m_routeRefreshTimer = nullptr;
#ifdef Q_OS_ANDROID
    QTimer *m_androidRoutePollTimer = nullptr;
    int m_lastAndroidRouteGeneration = -1;
    int m_pendingAndroidOutputId = -1;
#endif
    bool m_running = false;
    std::atomic<int> m_frequency{600};
    std::atomic<float> m_volume{0.3f};
    std::atomic<int> m_keyerWpm{20};
    double m_phase = 0.0;
    Element m_currentElement = ElementNone;
};

#endif // SIDETONEGENERATOR_H
