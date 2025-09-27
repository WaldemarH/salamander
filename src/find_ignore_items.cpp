// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_ignore.h"


BOOL FindIgnore::Items_Add( BOOL enabled, const TCHAR* path, const unsigned long path_length )
{
//Duplicate string.
    auto*   path_clone = DupStr( ( path != NULL ) ? path : "", path_length );

    if ( path_clone == NULL )
    {
    //Out of memory.
        TRACE_E( LOW_MEMORY );

        return FALSE;
    }

//Create new item.
    Find_Ignore_Item*   item = new Find_Ignore_Item;

    if ( item == NULL )
    {
        TRACE_E( LOW_MEMORY );
        return FALSE;
    }

    item->Enabled = enabled;
    item->Path = path_clone;

//Append item
    m_Items.Add( item );

    if ( !m_Items.IsGood() )
    {
    //Free resources.
        delete item;

    //Reset list state.
        m_Items.ResetState();

        return FALSE;
    }

    return TRUE;
}

BOOL FindIgnore::Items_Move( const int index_source, const int index_destination )
{
//Move items.
    auto*   item_source = m_Items[index_source];

    if ( index_source < index_destination )
    {
    //Move all upper items down one item.
        for ( int i = index_source; i < index_destination; ++i )
        {
            m_Items[i] = m_Items[i + 1];
        }
    }
    else
    {
    //Move all lower items up one item.
        for ( int i = index_source; i > index_destination; --i )
        {
            m_Items[i] = m_Items[i - 1];
        }
    }

//Set item to destination slot.
    m_Items[index_destination] = item_source;

    return TRUE;
}

void FindIgnore::Items_Reset()
{
//Free old resources.
    m_Items.DestroyMembers();

//Insert default items.
    Items_Add( TRUE, TEXT_VIEW( "\\System Volume Information" ) );
    Items_Add( FALSE, TEXT_VIEW( "Local Settings\\Temporary Internet Files" ) );
}

BOOL FindIgnore::Items_Set( int index, BOOL enabled, const TCHAR* path )
{
//Validate.
    if (
        ( index < 0 )
        ||
        ( index >= m_Items.Count )
    )
    {
        TRACE_E( "Index is out of range" );

        return FALSE;
    }

//Duplicate path.
    TCHAR*  path_clone = DupStr( path );

    if ( path_clone == NULL)
    {
    //Failed to duplicate string -> stop execution.
        TRACE_E( LOW_MEMORY );

        return FALSE;
    }

//Update item state.
    //Free old path.
    Find_Ignore_Item*   item = m_Items[index];

    if ( item->Path != nullptr )
    {
        free( item->Path );
    }

    //Update state.
    item->Enabled = enabled;
    item->Path = path_clone;

    return TRUE;
}
