#pragma once

#define _WIN32_WINNT 0x0501

#include <memory>
#include <mutex>
#include <iostream>

#include "boost/asio.hpp"
#include "boost/array.hpp"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <Windows.h>
#include <Psapi.h>
#pragma comment(lib, "psapi.lib")

#include "VMTHooking.h"
#include "CSGO_SDK.hpp"
#include "NetvarManager.hpp"
#include "CSGO_ESP.hpp"
#include "SyncData.hpp"
