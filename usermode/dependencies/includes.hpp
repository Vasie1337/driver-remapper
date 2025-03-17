#pragma once

#define WIN32_LEAN_AND_MEAN  
#define _CRT_SECURE_NO_WARNINGS_


#pragma warning(disable  : 4996).
#pragma warning(disable  : 4530).

#include <Windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <dwmapi.h>
#include <winioctl.h>
#include <bcrypt.h>
#include <vector>
#include <array>
#include <thread>
#include <mutex>
#include <string>
#include <list>
#include <algorithm>
#include <TlHelp32.h>

#define m_log(fmt, ...) printf("[%s] " fmt, __FUNCTION__, ##__VA_ARGS__)

#include <dependencies/driver/driver.h>