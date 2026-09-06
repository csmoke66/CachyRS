#!/bin/bash
set -e
INSTALL_LOCATION="$HOME/.local/share/cachy-rs"
PLUGIN_LOCATION="$INSTALL_LOCATION/plugins"
BOLT_LOCATION="$HOME/.local/share/bolt-launcher/Jagex/launcher"

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

echo "==> Starting build..."
cmake ..
make

cd ..

if [ ! -d "$INSTALL_LOCATION" ]; then
    mkdir -p "$INSTALL_LOCATION"
fi

if [ ! -d "$PLUGIN_LOCATION" ]; then
    mkdir "$PLUGIN_LOCATION"
fi

echo "==> Updating file at $HOME/.local/share/cachy-rs..."
cp -r config/* ~/.local/share/cachy-rs
cp build/mod/libmod.so "$INSTALL_LOCATION/libmod.so"
cp build/updater/updater "$INSTALL_LOCATION/updater"

if [ -f "$BOLT_LOCATION/rs2client" ]; then
    echo "==> Copying rs2client from $BOLT_LOCATION to $INSTALL_LOCATION"
    cp "$BOLT_LOCATION/rs2client" "$INSTALL_LOCATION/rs2client"
fi

if [ -f "$INSTALL_LOCATION/rs2client" ]; then
    echo "==> Generating reversed_generated.h"
    cd "$INSTALL_LOCATION"
    ./updater
    echo "==> If your reversed_generated.h is outdated, a new one can be found at $INSTALL_LOCATION/reversed_generated.h"
fi

echo "==> Done!"
