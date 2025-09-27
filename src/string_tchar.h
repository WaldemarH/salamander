// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//Includes
    #include "string_local.h"
    #include "string_utf16.h"

//Declerations
    #ifdef UNICODE
    //Define string type.
        #define String_TChar        OS::String::String_Utf16

    #else
    //Define string type.
        #define String_TChar        OS::String::String_Local

    #endif
