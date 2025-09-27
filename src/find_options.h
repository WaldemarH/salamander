// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once


//Includes
    #include "find_options_item.h"

//Class
    class CMenuPopup;

    class Find_Options
    {
    //Constructor/Destructor
        public: Find_Options() : m_Items( 20, 10 ) {};

        public: BOOL Load( Find_Options& source );

    //Items
        public: BOOL Items_Add( Find_Options_Item* item )
        {
        //Add item.
            m_Items.Add( item );

            if ( !m_Items.IsGood() )
            {
            //Failed to add item -> reset list state.
                m_Items.ResetState();

                return FALSE;
            }

            return TRUE;
        };
        public: Find_Options_Item* Items_At( int index )
        {
        //Get item.
            return m_Items[index];
        };
        public: int Items_Count() const
        {
        //Get number of items.
            return m_Items.Count;
        };
        public: void Items_Delete( int index )
        {
        //Remove item.
            m_Items.Delete( index );
        };

        protected: TIndirectArray<Find_Options_Item>    m_Items;

    //Menu
        public: void Menu_Set( CMenuPopup& popup, BOOL enabled, int originalCount );

    //Registry
        public: void Registry_Load( HKEY hKey, DWORD cfgVersion );   // nacte cele pole
        public: void Registry_Save( HKEY hKey );                     // ulozi cele pole
    };
    extern Find_Options FindOptions;
