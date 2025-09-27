// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

//Includes


//Class
    struct Find_Ignore_Item
    {
        public: struct ItemType
        {
            public: enum Value
            {
                unknow,
    
                full,           //Full path including root: 'C:\' 'D:\TMP\' \\server\share\'
                relative,       //Path without root: 'aaa' 'aaa\bbbb\ccc'
                rooted,         //Path starting at any root
            };
        };
    
    //Functions
        public: ~Find_Ignore_Item()
        {
        //Free resources.
            if ( Path != NULL )
            {
                free( Path );
            }
        };

    //Data
        public: BOOL                Enabled = TRUE;
        public: ItemType::Value     Type = ItemType::unknow;        // the following data is not saved, it is initialized in Prepare()

        public: TCHAR*              Path = NULL;
        public: int                 Path_Length = 0;
    };
