// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

//Includes


//Class
    struct Find_Log_Item
    {
        friend class Find_Log;
        friend class Find_Log_Dialog;

    //Defines
        public: struct Flags        //Bit flags.
        {
            public: enum Value
            {
                button_ignore       = 0x00000002,    //Enable Ignore button on this item.

                type_info           = 0x00000000,
                type_error          = 0x00000001,
            };
        };

    //Function
        private: Find_Log_Item() : flags( Flags::type_info ), path( NULL ), text( NULL ) {};
        private: Find_Log_Item( const Flags::Value flags, TCHAR* path, TCHAR* text ) : flags( flags ), path( path ), text( text ) {};

    //Data
        Flags::Value    flags;

        TCHAR*          path;
        TCHAR*          text;
    };
    struct Find_Log_Item_VM    //WM_USER_ADDLOG - Strings will be copied when adding log to list, so they can be deallocated after return.
    {
    //Function
        public: Find_Log_Item_VM( const Find_Log_Item::Flags::Value flags, const TCHAR* path, const TCHAR* text ) : flags( flags ), path( path ), text( text ) {};

    //Data
        Find_Log_Item::Flags::Value    flags;

        const TCHAR*                    path;
        const TCHAR*                    text;
    };
