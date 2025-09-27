// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_string_search.h"
#include "common\handles.h"


void Find_String_Search::Redraw_Set( BOOL redraw )
{
//Enter critical section.
    HANDLES( EnterCriticalSection( &m_CriticalSection ) );

    //Set redraw state.
        m_Redraw = redraw;

//Leave critical section.
    HANDLES( LeaveCriticalSection( &m_CriticalSection ) );
}

BOOL Find_String_Search::Redraw_Get()
{
//Enter critical section.
    HANDLES( EnterCriticalSection( &m_CriticalSection ) );

    //Retrieve current redraw state.
        auto    redraw = m_Redraw;

//Leave critical section.
    HANDLES( LeaveCriticalSection( &m_CriticalSection ) );

    return redraw;
}
