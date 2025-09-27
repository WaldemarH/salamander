// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

//Includes
    #include "masks.h"

//Class
    //TODO cleanup
    class Find_Data_Search
    {
    //Constructor/Destructor
        public: Find_Data_Search( const TCHAR* dir, const char* masksGroup, BOOL includeSubDirs )
        {
        //Initialize variables.
            Set( dir, masksGroup, includeSubDirs );
        }

        char Dir[MAX_PATH];
        CMaskGroup MasksGroup;
        BOOL IncludeSubDirs;

        protected: void Set( const char* dir, const char* masksGroup, BOOL includeSubDirs )
        {
            strcpy(Dir, dir);
            MasksGroup.SetMasksString(masksGroup);
            IncludeSubDirs = includeSubDirs;
        };

    //Text
        public: const TCHAR* Text_Get( const int i )
        {
            switch ( i )
            {
            case 0:
                return MasksGroup.GetMasksString();
            case 1:
                return Dir;
            default:
                return IncludeSubDirs ? LoadStr(IDS_INCLUDESUBDIRSYES) : LoadStr(IDS_INCLUDESUBDIRSNO);
            }
        }
    };
