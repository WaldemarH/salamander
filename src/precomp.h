// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//[W: when UNICODE will be used define UNICODE and _UNICODE defines... for C++ and C functions types... since W2000 Windows are using UTF16 internally].
//


//#define WIN32_LEAN_AND_MEAN // exclude rarely-used stuff from Windows headers

// SDK nam potlacuje nektere warningy, napriklad C4244, proto ho obklicime do PUSH/POP
#pragma warning(push)

#ifdef _WIN64
#ifdef _DEBUG
#define X64_STRESS_TEST
#endif
#endif

#include <windows.h>
#include <tchar.h>
#include <shlobj.h>
#include <dbt.h>
#include <crtdbg.h>
#include <winioctl.h>
#include <winnetwk.h>
#include <ostream>
#include <limits.h>
#include <commctrl.h>
#include <stdio.h>
#include <olectl.h>
#include <process.h>
#include <zmouse.h>
//#include <shlwapi.h>   // do not use, use our COMMON/STR.H instead
#include <exdisp.h>
#if (_MSC_VER < 1700)
#include <wfext.h>
#endif
#include <math.h>

typedef unsigned __int64 QWORD;

//Standard library
#include <string>
#include <string_view>
#include <iostream>
#include <sstream>

using namespace std::literals;

// opatreni proti runtime check failure v debug verzi: puvodni verze makra pretypovava rgb na WORD,
// takze hlasi ztratu dat (RED slozky)
#undef GetGValue
#define GetGValue(rgb) ((BYTE)(((rgb) >> 8) & 0xFF))

#pragma warning(pop)

#pragma warning(3 : 4706) // warning C4706: assignment within conditional expression

#if defined(_DEBUG) && defined(_MSC_VER) // without passing file+line to 'new' operator, list of memory leaks shows only 'crtdbg.h(552)'
#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

//Use as VS_OUTPUT( "The value of x is " << x );
#define VS_OUTPUT( s )                      \
{                                           \
   std::ostringstream os_;                  \
   os_ << s;                                \
   OutputDebugString( os_.str().c_str() );  \
}

#include "trace.h"
#include "messages.h"
#include "handles.h"
#include "heap.h"
#include "array.h"
#include "string_tchar.h"
#include "string_tchar_view.h"
//using namespace OS::String::String_View_Literals;
#include "system.h"
#include "winlib.h"
#include "multimon.h"
#include "sheets.h"
#include "strutils.h"
#include "spl_com.h"
#include "spl_base.h"
#include "spl_crypt.h"
#include "spl_zlib.h"
#include "spl_bzip2.h"
#include "spl_gen.h"
#include "spl_arc.h"
#include "spl_view.h"
#include "spl_menu.h"
#include "spl_file.h"
#include "spl_fs.h"
#include "spl_thum.h"
#include "spl_gui.h"
#include "spl_vers.h"
#include "bitmap.h"
#include "iconlist.h"
#include "consts.h"
#include "icncache.h"
#include "salamand.h"
#include "sort.h"
#include "masks.h"
#include "str.h"
#include "callstk.h"
#include "moore.h"
#include "regexp.h"
#include "filter.h"
#include "registry_worker_thread.h"
#include "registry.h"
#include "texts.rh2"
#include "lang\lang.rh"
#include "salamand.rh"
#include "resource.rh2"
