// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_options.h"


Find_Options FindOptions;

BOOL Find_Options::Load( Find_Options& source )
{
//Free any old resources.
    m_Items.DestroyMembers();

//Insert items to list.
    for ( int i = 0; i < source.m_Items.Count; i++ )
    {
    //Create new item.
        auto*    item = new Find_Options_Item( *source.m_Items[i] );

        if ( item == NULL )
        {
            TRACE_E( LOW_MEMORY );

            return FALSE;
        }

    //Add item to list.
        m_Items.Add( item );

        if ( !m_Items.IsGood() )
        {
        //Free resources.
            delete item;

        //Reset list state.
            m_Items.ResetState();

            return FALSE;
        }
    }

    return TRUE;
}
