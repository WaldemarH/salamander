// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_string_search.h"


void Find_String_Search::String_Get( TCHAR* pText, int size_buffer )
{
//Enter critical section.
    HANDLES( EnterCriticalSection( &m_CriticalSection ) );

    //Copy string to requestor.
        lstrcpyn( pText, m_String, size_buffer );

//Leave critical section.
    HANDLES( LeaveCriticalSection( &m_CriticalSection ) );
}

void Find_String_Search::String_Set( const TCHAR* pText )
{
//Enter critical section.
    HANDLES( EnterCriticalSection( &m_CriticalSection ) );

    //Append string to internal buffer.
        lstrcpyn( m_String + m_String_baseLength, pText, sizeof( m_String ) - m_String_baseLength );

    //Set redraw state.
        m_Redraw = TRUE;

//Leave critical section.
    HANDLES( LeaveCriticalSection( &m_CriticalSection ) );
}

void Find_String_Search::String_Set_Base( const TCHAR* pText )
{
//Enter critical section.
    HANDLES( EnterCriticalSection( &m_CriticalSection ) );

    //Append string to internal buffer.
        lstrcpyn( m_String, pText, sizeof( m_String ) );

    //Define length of base string.
        m_String_baseLength = (int)_tcslen( m_String );

    //[W: shouldn't it be marked as "dirty"?]
    ////Set redraw state.
    //    m_Redraw = TRUE;

//Leave critical section.
    HANDLES( LeaveCriticalSection( &m_CriticalSection ) );
}

