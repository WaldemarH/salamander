// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//Includes
    #include "string_local_view.h"
    #include "string_utf16_view.h"

//Declerations
    #ifdef UNICODE
    //Define view type.
        #define String_TChar_View       OS::String::String_Utf16_View

    //Define TEXT string view literal macro (encode as UTF16).
    //
    //Notice:
    //As views use constexpr hardcoded text will be converted to a view at compile time.
        #define TEXT_VIEW( text ) String_TChar_View( u##text )

    #else
    //Define view type.
        #define String_TChar_View       OS::String::String_Local_View

    //Define TEXT string view literal macro.
    //
    //Notice:
    //As views use constexpr hardcoded text will be converted to a view at compile time.
        #define TEXT_VIEW( text ) String_TChar_View( text )
    #endif
