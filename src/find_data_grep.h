// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

//Includes
    #include ".\common\moore.h"
    #include "find_data_search.h"
    #include "find_string_search.h"

//Class
    class Find_FilesList_View;
    class Find_String_Search;

    struct Find_Data_Grep
    {
        BOOL FindDuplicates; // hledame duplikaty?
        DWORD FindDupFlags;  // FIND_DUPLICATES_xxx; ma vyznam je-li 'FindDuplicates' TRUE
        int Refine;          // 0: hledat nova data, 1 & 2: hledat nad nalezenyma datama; 1: intersect with old data; 2: subtract from old data
        BOOL Grep;           // grepovat ?
        BOOL WholeWords;     // jen cela slova ?
        BOOL Regular;        // regularni vyraz ?
        BOOL EOL_CRLF,       // EOLy pri hledani regularnich vyrazu
            EOL_CR,
            EOL_LF;
        //       EOL_NULL;              // na to nemam regexp :(
    
        CSearchData SearchData;
        CRegularExpression RegExp;
        // advanced search
        DWORD AttributesMask;  // nejprve vymaskujem
        DWORD AttributesValue; // pak porovname
        CFilterCriteria Criteria;
        // rizeni + data
        BOOL StopSearch;    // nastavuje hl. thread pro ukonceni grep threadu
        BOOL SearchStopped; // byl terminovan nebo ne ?
        HWND HWindow;       // s kym grep thread komunikuje
        TIndirectArray<Find_Data_Search>* Data;
        Find_FilesList_View* FoundFilesListView; // sem ladujeme nalezene soubory
        // dve kriteria pro update listview
        int FoundVisibleCount;  // pocet polozek zobrazenych v listview
        DWORD FoundVisibleTick; // kdy bylo naposled zobrazovano
        BOOL NeedRefresh;       // je potreba refreshnout zobrazeni (pribyla polozka bez zobrazeni)
    
        Find_String_Search* SearchingText;  // synchronizovany text "Searching" ve status-bare Findu
        Find_String_Search* SearchingText2; // [optional] druhy text vpravo; pouzivame pro "Total: 35%"
    };

// flags for searching for identical files
// at least _NAME or _SIZE will be specified
// _CONTENT can only be set if _SIZE is also set
#define FIND_DUPLICATES_NAME 0x00000001    // same name
#define FIND_DUPLICATES_SIZE 0x00000002    // same size
#define FIND_DUPLICATES_CONTENT 0x00000004 // same content
