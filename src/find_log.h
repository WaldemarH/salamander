// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

//Includes
    #include "find_log_item.h"

//Class
    class Find_Log          //Contains search errors.
    {
    //Constructor/Destructor
        public: Find_Log();
        public: ~Find_Log();

    //Items
        public: BOOL Items_Add( Find_Log_Item::Flags::Value flags, const TCHAR* path, const TCHAR* text );
        public: inline BOOL Items_Add( const Find_Log_Item_VM& item )
        {
        //Forward event.
            return Items_Add( item.flags, item.path, item.text );
        };
        public: void Items_Clean(); // uvolni vsechny drzene polozky
        public: int Items_Count() const
        {
        //Get number of items.
            return m_Items.Count;
        }
        public: int Items_Count_Error() const
        {
        //Get number of error items.
            return m_Items.Count;
        }
        public: int Items_Count_Info() const
        {
        //Get number of info items.
            return m_Items_Count_Info;
        }
        public: int Items_Count_Skipped() const
        {
        //Get number of skipped items.
            return m_Items_Count_Skipped;
        }
        public: const Find_Log_Item* Items_Get( const int index );

        protected: TDirectArray<Find_Log_Item>      m_Items;
        protected: int                              m_Items_Count_Error = 0;
        protected: int                              m_Items_Count_Info = 0;
        protected: const int                        m_Items_Count_Max = 300;
        protected: int                              m_Items_Count_Skipped = 0;
    };
