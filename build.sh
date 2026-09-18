#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
PRIV="$(cd "$ROOT/../CachyRS-Private" 2>/dev/null && pwd || true)"
INSTALL_LOCATION="${HOME}/.local/share/cachy-rs"
BOLT_LOCATION="${HOME}/.local/share/bolt-launcher/Jagex/launcher"
BUILD_DIR="$ROOT/build"

CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"
CACHYRS_PLUGIN_API_VALIDATE="${CACHYRS_PLUGIN_API_VALIDATE:-OFF}"

echo "==> Configuring CachyRS (${CMAKE_BUILD_TYPE}, ASAN=OFF, PLUGIN_API_VALIDATE=${CACHYRS_PLUGIN_API_VALIDATE})"
# Explicitly force ASAN off so a prior asan configure can't stick in the cache.
cmake -S "$ROOT" -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
  -DCACHYRS_ASAN=OFF \
  -DCACHYRS_PLUGIN_API_VALIDATE="$CACHYRS_PLUGIN_API_VALIDATE"

echo "==> Building CachyRS"
cmake --build "$BUILD_DIR" -j"$(nproc)"

mkdir -p "$INSTALL_LOCATION"

echo "==> Installing libmod + config (plugins dir left alone)"
cp -r "$ROOT/config/"* "$INSTALL_LOCATION/"
cp -f "$BUILD_DIR/mod/libmod.so" "$INSTALL_LOCATION/libmod.so"
cp -f "$BUILD_DIR/updater/updater" "$INSTALL_LOCATION/updater"

if [[ -n "$PRIV" && -f "$PRIV/CMakeLists.txt" ]]; then
  echo "==> Configuring CachyRS-Private"
  cmake -S "$PRIV" -B "$PRIV/build" \
    -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
    -DCACHYRS_ASAN=OFF \
    -DCACHYRS_PLUGIN_API_VALIDATE="$CACHYRS_PLUGIN_API_VALIDATE"

  echo "==> Building CachyRS-Private"
  cmake --build "$PRIV/build" -j"$(nproc)"
fi

if [[ -f "$BOLT_LOCATION/rs2client" ]]; then
  echo "==> Copying rs2client from $BOLT_LOCATION"
  cp -f "$BOLT_LOCATION/rs2client" "$INSTALL_LOCATION/rs2client"
fi

if [[ -f "$INSTALL_LOCATION/rs2client" ]]; then
  echo "==> Generating reversed_generated.h"
  (
    cd "$INSTALL_LOCATION"
    ./updater
  )
  echo "==> If reversed_generated.h is outdated, a new one is at $INSTALL_LOCATION/reversed_generated.h"
fi

echo "==> Done — libmod at $INSTALL_LOCATION/libmod.so"
