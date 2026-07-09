#include "injector.h"
#include <zero/log.h>
#include <zero/cmdline.h>

#include <filesystem>

static bool is_pid_dir(const std::string& entry)
{
    for (auto p : entry)
    {
        if (!isdigit(p))
        {
            return false;
        }
    }
    return true;
}

static pid_t get_pid_by_name(const std::string &process_name)
{
    for (const auto &entry : std::filesystem::directory_iterator("/proc"))
    {
        if (!entry.is_directory())
            continue;

        std::string dir_name = entry.path().filename().string();
        if (!is_pid_dir(dir_name))
        {
            continue;
        }

        std::ifstream comm_file(entry.path() / "comm");
        std::string comm_name;

        if (std::getline(comm_file, comm_name))
        {
            if (comm_name == process_name)
            {
                return std::stoul(dir_name);
            }
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    INIT_CONSOLE_LOG(zero::INFO);

    Injector injector;

    auto pid = get_pid_by_name("rs2client");
    if (pid == 0)
    {
        LOG_ERROR("Failed to find process");
        return -1;
    }

    if (!injector.open(pid))
    {
        LOG_ERROR("process injector open failed");
        return -1;
    }

    auto exe_path = std::filesystem::read_symlink("/proc/self/exe");
    auto exe_dir = exe_path.parent_path();
    return injector.inject(exe_dir.string() + "/libmod.so");
}
