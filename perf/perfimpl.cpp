#include "perfimpl.h"

#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#include <sys/syscall.h>
#include <sys/ioctl.h>

#include <linux/perf_event.h>

PerfImpl::PerfImpl()
{

}

