#pragma once
#include <stdarg.h>
#include <orbis/libkernel.h>

// Forward declare nanosleep for libc++ threading support
struct timespec;
extern "C" int nanosleep(const struct timespec* req, struct timespec* rem);

#include <string>
#include <memory>
#include <mutex>

#include "DriverDefinitions.h"
#include "FusionUtils.h"
#include "Embed.h"
#include "RemoteThread.h"
#include "RemoteCaller.h"
#include "RemoteCallerManager.h"