// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

//Includes
    #include "find_ignore_item.h"

//Class
    // The FindIgnore object serves two purposes:
    // 1. A global object holding a list of paths, editable from Find/Options/Ignore Directory List
    // 2. A temporary copy of this array for search purposes -- containing only Enabled items, which are additionally modified (backslashes added) and qualified (Find_Ignore_Item::Type set)
    class FindIgnore
    {
        friend class CFindIgnoreDialog;

    //Constructor/Destructor
        public: FindIgnore();

        public: BOOL Load( FindIgnore* source );

    //Items
        protected: BOOL Items_Add( BOOL enabled, const TCHAR* path, const unsigned long path_length = -1 );
        protected: inline BOOL Items_Add( BOOL enabled, const String_TChar_View& path )
        {
        //Forward request.
            return Items_Add( enabled, path.Text_Get(), path.Size_CodeUnits() );
        };
        protected: Find_Ignore_Item* Items_At( const int index )
        {
        //Return item.
            return m_Items[index];
        }
        protected: void Items_Delete( const int index )
        {
        //Remove item at position.
            m_Items.Delete( index );
        }
        protected: int Items_GetCount() const
        {
        //Return number of items.
            return m_Items.Count;
        };
        protected: BOOL Items_Move( const int index_source, const int index_destination );
        protected: void Items_Reset(); // sestreli existujici polozky, prida default hodnoty
        protected: BOOL Items_Set( int index, BOOL enabled, const TCHAR* path );

        TIndirectArray<Find_Ignore_Item>        m_Items;

    //Registry
        public: void Registry_Save( HKEY hKey );                        // ulozi cele pole
        public: void Registry_Load( HKEY hKey, DWORD cfgVersion );      // nacte cele pole


    public:

        // called for a local copy of the object, which is then used for searching
        // must be called before calling Contains()
        // copies items from 'source' and prepares them for searching
        // returns TRUE if successful, otherwise returns FALSE
        //
        // vola se pro lokalni kopii objekt, ktera se nasledne pouziva pro hledani
        // je treba zavolat pred volanim Contains()
        // z 'source' nakopiruje polozky a pripravi pro hledani
        // vraci TRUE, pokud se zadarilo, jinak vraci FALSE
        BOOL Prepare(FindIgnore* source);

        // returns TRUE if the list contains an item corresponding to the path 'path'
        // only 'Enabled'==TRUE items are evaluated
        // if no such item is found, returns FALSE
        // !attention! the method must be entered with a full path with a slash at the end
        //
        // vraci TRUE, pokud seznam obashuje polozku odpovidajici ceste 'path'
        // vyhodnocuji se pouze 'Enabled'==TRUE polozky
        // pokud takovou polozku nenajde, vraci FALSE
        // !pozor! do metody musi vstupovat plna cesta s lomitkem na konci
        BOOL Contains(const char* path, int startPathLen);


        // add the path only if it doesn't already exist in the list
        BOOL AddUnique(BOOL enabled, const char* path);
    };


