#include "sstvorientation.h"

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QtCore/qcoreapplication_platform.h>

void setSstvOrientationEnabled(bool enabled) {
    const jint requested = enabled ? 10 /* SCREEN_ORIENTATION_FULL_SENSOR */
                                   : 6  /* SCREEN_ORIENTATION_SENSOR_LANDSCAPE */;
    const QJniObject activity = QNativeInterface::QAndroidApplication::context();
    if (activity.isValid())
        activity.callMethod<void>("setRequestedOrientation", "(I)V", requested);
}
#else
void setSstvOrientationEnabled(bool) {}
#endif
