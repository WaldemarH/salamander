// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

//Includes


//Class
    class Find_String_Search  // synchronizovany buffer pro "Searching" text ve status bare Find dialogu
    {
    //Constructor/Desctructo
        public: Find_String_Search();
        public: ~Find_String_Search();

        protected: CRITICAL_SECTION     m_CriticalSection;

    //Redraw
        public: BOOL Redraw_Get();
        public: void Redraw_Set( BOOL redraw );

        protected: BOOL                 m_Redraw = FALSE;

    //String
        public: void String_Get( TCHAR* pText, int size_buffer );
        public: void String_Set( const TCHAR* pText );
        public: void String_Set_Base( const TCHAR* pText );

        protected: TCHAR                m_String[MAX_PATH + 50] = { 0 };
        protected: int                  m_String_baseLength = 0;
    };
