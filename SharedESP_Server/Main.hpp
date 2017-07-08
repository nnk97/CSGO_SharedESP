#pragma once

#define _WIN32_WINNT 0x0501

#include <memory>
#include <mutex>
#include <thread>
#include <iostream>
#include <iomanip>

#include "boost/asio.hpp"
#include "boost/array.hpp"
#include <boost/bind.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "Server.hpp"
