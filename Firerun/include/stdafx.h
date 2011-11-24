// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Disable warnings about unsecure functions
#define _CRT_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE

#include <assert.h>

// Windows Header Files
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>

// Direct3D
#include <D3D9.h>
#include <d3dx9.h>

// Mathematics
#include <math.h>

// There's only a macro for PI in math.h (M_PI)
const float g_PI = 3.141592653589f;
const float g_PI_4 = 0.785398163397f;

// some useful globals
extern float g_PassedTime;
extern float g_FrameTime;

	// Test-Case
//#define TEXTURE_TEST
#ifdef TEXTURE_TEST
extern int g_TexIndex;
#endif