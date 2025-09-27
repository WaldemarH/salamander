// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

//Include

//Class
    struct Find_FileList_Item
    {
    //Constructor/Destructor
        public: Find_FileList_Item();
        public: ~Find_FileList_Item();

    //Text
        /// if Name or Path return a pointer to the corresponding variable
        // otherwise fill the 'text' buffer (must be at least 50 characters long) with the corresponding value
        // and return a pointer to 'text'
        // 'fileNameFormat' specifies the formatting of the names of the found items
        public: TCHAR* Text_Get( const int index_column, TCHAR* text, int fileNameFormat );
        public: BOOL Text_Set( const TCHAR* path, const TCHAR* name, const CQuadWord& size, DWORD attr, const FILETIME* lastWrite, BOOL isDir );

        public: TCHAR*      Name = NULL;
        public: TCHAR*      Path = NULL;

    //[TODO: unorganized... get the meaning of the members]
    //Data

        public: CQuadWord   Size;
        public: DWORD       Attr = 0;
        public: FILETIME    LastWrite;
    
        // 'Group' is used in two ways:
        // 1) during the duplicate file search, if the contents are compared,
        // it contains a pointer to the CMD5Digest with the computed MD5 for the file
        // 2) before passing the duplicate file search result to the ListView
        // it contains a number that connects multiple files into an equivalent group
        public: DWORD_PTR   Group = 0;
    
        public: unsigned    Different : 1 = 0; // 0 - polozka ma klasicke bile pozadi, 1 - polozka me jine pozadi (pouziva se pri hledani diferenci)
        public: unsigned    Focused : 1 = 0;  // 0 - polozka je focused, 1 - polozka neni focused
        public: unsigned    IsDir : 1 = 0; // 0 - polozka je soubor, 1 - polozka je adresar
        // Selected a Focused maji vyznam pouze lokalne pro StoreItemsState/RestoreItemsState
        public: unsigned    Selected : 1 = 0; // 0 - polozka neoznacena, 1 - polozka oznacena
        // Different se pouziva k rozliseni skupin souboru pri hledani duplicit
    };
