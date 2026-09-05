# iOS third-party dependencies

Static libraries for the iOS build. Each `lib/*.a` is a fat archive holding
an `x86_64` slice (iOS Simulator on Intel Macs) and an `arm64` slice (iPhone /
iPad devices). Apple Silicon simulators need an additional `arm64` simulator
slice, which cannot share a fat file with the device slice; build an
`.xcframework` instead if that becomes a target.

Expected layout:

```text
openssl/include/openssl/*.h
openssl/lib/libssl.a
openssl/lib/libcrypto.a
opus/include/opus/*.h
opus/lib/libopus.a
```

`CMakeLists.txt` points at these paths when `IOS` is set. Override with
`-DQK4_OPENSSL_ROOT=<prefix>` and `-DQK4_OPUS_INCLUDE_DIR=... -DQK4_OPUS_LIBRARY=...`.

## Why OpenSSL

The K4 remote link uses TLS 1.2 with pre-shared keys. Qt's iOS kit only ships
the Secure Transport TLS plugin, which has no PSK support, so
`src/network/psktlssocket_openssl.cpp` drives OpenSSL directly over a
`QTcpSocket`. The Qt OpenSSL TLS plugin is not used on iOS.

## Building OpenSSL (3.6.3)

```bash
curl -sSLO https://github.com/openssl/openssl/releases/download/openssl-3.6.3/openssl-3.6.3.tar.gz
tar xzf openssl-3.6.3.tar.gz

build() { # $1=Configure target  $2=min-version flag  $3=prefix
  cp -R openssl-3.6.3 "build-$1" && cd "build-$1"
  ./Configure "$1" no-shared no-dso no-tests no-apps no-docs no-engine "$2" --prefix="$3"
  make -j"$(sysctl -n hw.ncpu)" build_libs && make install_dev
  cd ..
}
build iossimulator-x86_64-xcrun -mios-simulator-version-min=16.0 "$PWD/out/sim"
build ios64-xcrun               -miphoneos-version-min=16.0      "$PWD/out/dev"

lipo -create out/sim/lib/libssl.a    out/dev/lib/libssl.a    -output openssl/lib/libssl.a
lipo -create out/sim/lib/libcrypto.a out/dev/lib/libcrypto.a -output openssl/lib/libcrypto.a
cp -R out/sim/include openssl/include
```

`include/openssl/configuration.h` is generated per target; the copy here
merges the two with `__x86_64__` / `__aarch64__` guards where they differ.

## Building Opus

See `opus/README.md` once populated. Same recipe: configure Opus with CMake
for `iphonesimulator` and `iphoneos`, `BUILD_SHARED_LIBS=OFF`, then `lipo`.

## Debug builds crash in the panadapter — use RelWithDebInfo

A `Debug` iOS build aborts on the first frame with a Qt Metal assertion:

```
ASSERT: "...->recordingPass == QMetalCommandBuffer::NoPass" in qrhimetal.mm
```

`PanadapterRhiWidget::render()` and `MiniPanRhiWidget::render()` call
`QRhiCommandBuffer::resourceUpdate()` between `beginPass()` and `endPass()`.
Qt only allows that outside a pass; its Metal backend enforces it with a
`Q_ASSERT`, so the check fires only in debug Qt. This predates the iOS work
(inherited from upstream QK4) and affects every platform — it is simply
compiled out of non-debug builds, which is why desktop Release and Android
never trip it.

Until the renderers are restructured to upload all resources before the
pass, build the iOS app with `RelWithDebInfo`. Qt's assertions are off, so
it runs, and it keeps debug symbols for breakpoints and stepping:

```bash
cmake --build build-ios --config RelWithDebInfo \
    -- -sdk iphonesimulator -arch x86_64 CODE_SIGNING_ALLOWED=NO ONLY_ACTIVE_ARCH=YES
```

Plain `Release` also runs; it just has no debug symbols.
