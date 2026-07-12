git clone https://github.com/csmoke66/CachyRS
mkdir build && cd build
cmake ..
make
cd ..
mkdir ~/.local/share/cachy-rs` 
cp -r config/* ~/.local/share/cachy-rs
