#pragma once

// Minimal Windows type forward declarations
// Use this in headers instead of #include <Windows.h>
// Include full Windows.h (via stdafx.h) in .cpp files

#ifndef _WINDOWS_ // Only declare if Windows.h not already included

//=============================================================================
// Opaque Handle Types (Forward Declarations)
//=============================================================================

struct HWND__;
struct HINSTANCE__;
struct HDC__;
struct HICON__;
struct HCURSOR__;
struct HBRUSH__;
struct HMENU__;

typedef struct HWND__* HWND;
typedef struct HINSTANCE__* HINSTANCE;
typedef struct HDC__* HDC;
typedef struct HICON__* HICON;
typedef struct HCURSOR__* HCURSOR;
typedef struct HBRUSH__* HBRUSH;
typedef struct HMENU__* HMENU;

//=============================================================================
// String Types
//=============================================================================

typedef char* LPSTR;
typedef const char* LPCSTR;
typedef wchar_t* LPWSTR;
typedef const wchar_t* LPCWSTR;

//=============================================================================
// Basic Integer Types
//=============================================================================

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned int UINT;
typedef int BOOL;
typedef long LONG;
typedef unsigned long ULONG;

//=============================================================================
// 64-bit Safe Types
//=============================================================================

#if defined(_WIN64)
typedef unsigned long long WPARAM;
typedef long long LPARAM;
typedef long long LRESULT;
typedef long long LONG_PTR;
typedef unsigned long long ULONG_PTR;
typedef unsigned long long UINT_PTR;
#else
typedef unsigned int WPARAM;
typedef long LPARAM;
typedef long LRESULT;
typedef long LONG_PTR;
typedef unsigned long ULONG_PTR;
typedef unsigned long UINT_PTR;
#endif

//=============================================================================
// DirectX/COM Types
//=============================================================================

typedef long HRESULT;
typedef WORD ATOM;

//=============================================================================
// Structure Definitions (Need Full Definition, Not Just Forward Declaration)
//=============================================================================

struct tagRECT
{
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
};
typedef struct tagRECT RECT;

struct tagPOINT
{
    LONG x;
    LONG y;
};
typedef struct tagPOINT POINT;

struct tagMSG
{
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
};
typedef struct tagMSG MSG;

//=============================================================================
// Calling Conventions
//=============================================================================

#ifndef CALLBACK
#define CALLBACK __stdcall
#endif

#ifndef WINAPI
#define WINAPI __stdcall
#endif

#ifndef APIENTRY
#define APIENTRY WINAPI
#endif

//=============================================================================
// Common Constants
//=============================================================================

#ifndef NULL
#ifdef __cplusplus
#define NULL nullptr
#else
#define NULL ((void*)0)
#endif
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

//=============================================================================
// Common Macros (Minimal Set)
//=============================================================================

#ifndef LOWORD
#define LOWORD(l) ((WORD)(((ULONG_PTR)(l)) & 0xffff))
#endif

#ifndef HIWORD
#define HIWORD(l) ((WORD)((((ULONG_PTR)(l)) >> 16) & 0xffff))
#endif

#ifndef MAKEWORD
#define MAKEWORD(a, b)                                                         \
    ((WORD)(((BYTE)(((ULONG_PTR)(a)) & 0xff)) |                                \
            ((WORD)((BYTE)(((ULONG_PTR)(b)) & 0xff))) << 8))
#endif

#ifndef MAKELONG
#define MAKELONG(a, b)                                                         \
    ((LONG)(((WORD)(((ULONG_PTR)(a)) & 0xffff)) |                              \
            ((DWORD)((WORD)(((ULONG_PTR)(b)) & 0xffff))) << 16))
#endif

//=============================================================================
// HRESULT Macros (for DirectX error checking)
//=============================================================================

#ifndef SUCCEEDED
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#endif

#ifndef FAILED
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif

#endif // _WINDOWS_
