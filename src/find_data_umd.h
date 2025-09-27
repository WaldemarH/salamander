// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

//Includes


//Class
    struct CUMDataFromFind
    {
    //Functions
        public: CUMDataFromFind( HWND hWindow ) : hWindow( hWindow ), index( NULL ), count( -1 ) {};
        public: ~CUMDataFromFind()
        {
        //Free resources.
            if ( index != NULL )
            {
                delete[] ( index );
            }
        }

    //Data
        HWND    hWindow;
        int*    index;
        int     count;
    };

