#ifndef SSTVORIENTATION_H
#define SSTVORIENTATION_H

#include <QtGlobal>

// This helper is deliberately independent of the SSTV codec/controller so
// host tests and non-Android builds remain free of JNI dependencies.
void setSstvOrientationEnabled(bool enabled);

#endif // SSTVORIENTATION_H
