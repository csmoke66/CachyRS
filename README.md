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
## Usage
1. Run the native RS3 linux client
2. Place `injector` and `libmod.so` into the same directories
3. Run `injector` with root permissions

This will load the mod into the game client, and automatically enable everything.

![Crabussy](media/crabussy.png)
## Feature Requests
I expect people to request things like Windows support, or language bindings. This is currently outside of the scope of the project as I only run Linux on my machines. I will happily work with people to add support and accept pull requests.