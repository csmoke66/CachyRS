#include "cachy.h"
#include <thread>

namespace crs
{
    void init_async()
    {
        RS.init();
    }

    // TODO FIXME replace this with a global object to support
    // other compilers
    __attribute__((constructor)) void entry()
    {
        ::std::thread thr(init_async);
        thr.detach();
    }
}