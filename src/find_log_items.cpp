// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_log.h"


BOOL Find_Log::Items_Add( Find_Log_Item::Flags::Value flags, const TCHAR* text, const TCHAR* path )
{
//Increase counters.
    if ( ( flags & Find_Log_Item::Flags::type_error ) != 0 )
    {
        m_Items_Count_Error++;
    }
    else
    {
        m_Items_Count_Info++;
    }

//Did we reach the list limit?
    if ( m_Items.Count > m_Items_Count_Max )
    {
    //Yes -> skip item.
        m_Items_Count_Skipped++;

        return TRUE;
    }

//Duplicate strings.
    auto*   path_clone = DupStr( ( path != NULL ) ? path : "" );
    auto*   text_clone = DupStr( ( text != NULL ) ? text : "" );

    if (
        ( path_clone == NULL )
        ||
        ( text_clone == NULL )
    )
    {
    //Out of memory.
        //Free resources.
        if ( path_clone != NULL )
        {
            free( path_clone );
        }
        if ( text_clone != NULL )
        {
            free( text_clone );
        }

        //Log
        TRACE_E( LOW_MEMORY );

        return FALSE;
    }

//Add item.
    Find_Log_Item   item( flags, path_clone, text_clone );

    m_Items.Add( item );

    if ( !m_Items.IsGood() )
    {
    //Out of memory.
        TRACE_E(LOW_MEMORY);

    //Free resources.
        free( path_clone );
        free( text_clone );

    //Reset list state.
        m_Items.ResetState();

        return FALSE;
    }

    return TRUE;
}

void Find_Log::Items_Clean()
{
//Free log texts.
    for ( int i = 0; i < m_Items.Count; ++i )
    {
    //Get item
        Find_Log_Item*  item = &m_Items[i];

    //Free resources.
        free( item->path );
        free( item->text );
    }
    m_Items.DetachMembers();

//Reset variables.
    m_Items_Count_Error = 0;
    m_Items_Count_Info = 0;
    m_Items_Count_Skipped = 0;
}

const Find_Log_Item* Find_Log::Items_Get( const int index )
{
//Validate.
    if (
        ( index < 0 )
        ||
        ( index >= m_Items.Count )
    )
    {
        TRACE_E("Index is out of range");
        return NULL;
    }

//Return item
    return &m_Items[index];
}

