// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

//Includes

//Class

//TODO - cleanup
    #define ITEMNAME_TEXT_LEN MAX_PATH + MAX_PATH + 10
    #define NAMED_TEXT_LEN MAX_PATH  // maximalni velikost textu v comboboxu
    #define LOOKIN_TEXT_LEN MAX_PATH // maximalni velikost textu v comboboxu
    #define GREP_TEXT_LEN 201        // maximalni velikost textu v comboboxu; POZOR: melo by byt shodne s FIND_TEXT_LEN
    #define GREP_LINE_LEN 10000      // max. delka radky pro reg. expr. (viewer ma jine makro)

    class Find_Options_Item
    {
    //Constructor/Destructor
        public: Find_Options_Item() {};
        public: Find_Options_Item( const Find_Options_Item& s )
        {
        //Initialize variables.
            *this = s;
        };

        // !POZOR! jakmile bude objekt obsahovat alokovana data, prestane fungovat
        // kod pro presouvani polozek v Options Manageru, kde dochazi k destrukci tmp prvku
        //
        // !WARNING! once the object contains allocated data, it will stop working
        // code for moving items in Options Manager, where the tmp element is destroyed
        public: Find_Options_Item& operator=(const Find_Options_Item& s);

    //Registry
        // !!! POZOR: ukladani optimalizovano, ukladaji se pouze zmenene hodnoty; pred ulozenim do klice musi by tento klic napred promazan
        //
        // !!! ATTENTION: saving optimized, only changed values are saved; before saving to the key, this key must first be erased
        public: void Registry_Save( HKEY hKey );                   // ulozi itemu
        public: void Registry_Load( HKEY hKey, DWORD cfgVersion ); // nacte itemu

    //TODO cleanup
    //Data
        // na zaklade NamedText a LookInText postavi nazev polozky (ItemName)
        //
        // based on NamedText and LookInText builds the item name (ItemName)
        public: void BuildItemName()
        {
            _stprintf(ItemName, "\"%s\" %s \"%s\"", NamedText, LoadStr(IDS_FF_IN), LookInText);
        };


        public: TCHAR ItemName[ITEMNAME_TEXT_LEN] = { 0 };
    
        CFilterCriteria Criteria;
    
        // Find dialog
        int SubDirectories = TRUE;
        int WholeWords = FALSE;
        int CaseSensitive = FALSE;
        int HexMode = FALSE;
        int RegularExpresions = FALSE;
    
        BOOL AutoLoad = FALSE;
    
        TCHAR NamedText[NAMED_TEXT_LEN] = { 0 };
        TCHAR LookInText[LOOKIN_TEXT_LEN] = { 0 };
        TCHAR GrepText[GREP_TEXT_LEN] = { 0 };
    
    };
