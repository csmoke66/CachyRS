mkdir build && cd build
cmake ..
make
cd ..
mkdir ~/.local/share/cachy-rs` 
cp -r config/* ~/.local/share/cachy-rs
mkdir bin
cp build/injector/injector bin/
cp build/mod/libmod.so bin/
cp build/updater/updater bin/
