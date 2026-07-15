#pragma once

namespace crs
{
    struct Engine;
    struct CpuState;
    
    typedef uint64_t (*CApiGameGetBase)();
    typedef const Engine *(*CApiGameGetEngine)();

    typedef void (*CApiSubSystemRegister)(const char *name, void *subsystem);

    typedef void (*CApiEventBusReceiver)(void *event);
    typedef void (*CApiEventBusAddReceiver)(const char *id, CApiEventBusReceiver receiver, void* context);
    typedef void (*CApiEventBusDispatch)(const char *id, void *event);

    typedef void (*CApiCachyRSGetVersion)(uint32_t *major, uint32_t *minor, uint32_t *patch, uint32_t *api);
    typedef void (*CApiCachyRSHookHandler)(CpuState *cpu_state);

    struct CApiGame
    {
        CApiGameGetBase get_base;
        CApiGameGetEngine get_engine;
    };

    struct CApiSubSystem
    {
        CApiSubSystemRegister register_new;
    };

    struct CApiEventBus
    {
        CApiEventBusAddReceiver add_receiver;
        CApiEventBusDispatch dispatch;
    };

    struct CApiCachyRS
    {
        CApiGame game;
        CApiSubSystem sub_system;
        CApiEventBus event_bus;
    };
}