// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#ifdef WINDOWS
#include <tchar.h>
#include <Windows.h>
#include "CyAPI.h"
#include "cyioctl.h"
#else
#include <string.h>
typedef unsigned char BYTE;
typedef unsigned int DWORD;
#endif
#include "HexFileParser.h"
