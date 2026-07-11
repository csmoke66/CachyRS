# CachyRS
A third party client for RS3 (and eventually, OSRS.) This is under active development.

**THIS PROJECT IS A WIP AND ACTIVELY UNDER DEVELOPMENT**
## Compiling
### Linux
`git clone https://github.com/csmoke66/CachyRS`  
`cd CachyRS && mkdir build && cd build`  
`cmake ..`  
`make`  
`cd ..`  
`mkdir ~/.local/share/cachy-rs`  
`cp -r config/* ~/.local/share/cachy-rs`
``

### Windows
I do not intend to add support for windows. If you would like, you can make a pull request to add support.
## Usage
1. Run the native RS3 linux client
2. Place `injector` and `libmod.so` into the same directories
3. Run `injector` with root permissions

This will load the mod into the game client, and automatically enable everything.

![Crabussy](media/crabussy.png)