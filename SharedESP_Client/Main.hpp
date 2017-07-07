#pragma once

#define _WIN32_WINNT 0x0501

#include <memory>
#include <mutex>

#include "boost/asio.hpp"

#include <Windows.h>

#include "VMTHooking.h"
#include "CSGO_SDK.hpp"
#include "CSGO_ESP.hpp"
#include "SyncData.hpp"