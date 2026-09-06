#!/bin/bash
set -e

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

echo "==> Starting build..."
cmake ..
make

cd ..

if [ ! -d "$HOME/.local/share/cachy-rs" ]; then
    mkdir -p ~/.local/share/cachy-rs
fi

if [ ! -d "$HOME/.local/share/cachy-rs/plugins" ]; then
    mkdir ~/.local/share/cachy-rs/plugins
fi

echo "==> Updating file at $HOME/.local/share/cachy-rs..."
cp -r config/* ~/.local/share/cachy-rs
cp build/mod/libmod.so ~/.local/share/cachy-rs/libmod.so
cp build/updater/updater ~/.local/share/cachy-rs/updater

if [ -d "$HOME/.local/share/bolt-launcher/Jagex/launcher/" ]; then
    echo "==> Moving rs2client from bolt launcher to $HOME/.local/share/cachy-rs..."
    cp ~/.local/share/bolt-launcher/Jagex/launcher/rs2client ~/.local/share/cachy-rs/rs2client
fi

echo "==> Done!"