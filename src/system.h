// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//Includes
    #include "string_tchar.h"

//Class
    class System
    {
    //Error
        public: static String_TChar Error_GetErrorDescription( const LSTATUS error );
        public: static OS::String::String_Utf16 Error_GetErrorDescription_Utf16( const LSTATUS error );
    };
