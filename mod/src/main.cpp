#include "cachy.h"
#include <iostream>
#include <thread>

#include <dlfcn.h>
#include <pci/pci.h>

static void redirect_output()
{
  freopen("/tmp/cachy-rs-stdout.txt", "w", stdout);
  freopen("/tmp/cachy-rs-stderr.txt", "a", stderr);
}

static bool is_nvidia_wayland()
{
  void *nvml = dlopen("libnvidia-ml.so.1", RTLD_LAZY | RTLD_LOCAL);
  auto nvml_init = nvml ? (uint32_t (*)())dlsym(nvml, "nvmlInit_v2") : nullptr;

  return nvml_init && nvml_init() == 0 && !!getenv("WAYLAND_DISPLAY");
}

extern "C" int __libc_start_main(
    int (*main)(int, char **, char **),
    int argc,
    char **argv,
    void (*init)(void),
    void (*fini)(void),
    void (*rtld_fini)(void),
    void *stack_end)
{
  auto real_libc_start_main = (decltype(__libc_start_main) *)dlsym(RTLD_NEXT, "__libc_start_main");

  if (std::string(program_invocation_short_name) == "rs2client")
  {
    redirect_output();

    auto no_graphics = getenv("NO_GRAPHICS") != nullptr;
    if (no_graphics)
    {
      setenv("VK_DRIVER_FILES", "/usr/share/vulkan/icd.d/lvp_icd.x86_64.json", 1);
      setenv("MESA_VK_DEVICE_SELECT", "cpu", 1);
    }
    else if (is_nvidia_wayland())
    {
      setenv("MESA_LOADER_DRIVER_OVERRIDE", "zink", 1);
    }

    crs::RS.init(no_graphics);
  }

  return real_libc_start_main(main, argc, argv, init, fini, rtld_fini, stack_end);
}
