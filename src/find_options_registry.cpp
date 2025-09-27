// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_options.h"


void Find_Options::Registry_Load( HKEY hKey, DWORD cfgVersion )
{
//Free any old resources.
    m_Items.DestroyMembers();

//Load all previously stored items.
    TCHAR   key_name[30];
    HKEY    key_sub;

    for ( int i = 1; ; ++i )
    {
    //Define registry key name.
        _itot( i, key_name, 10 );

    //Open key.
        const auto      status = OpenKey( hKey, key_name, key_sub );

        if ( !status )
        {
        //No more stores items -> stop execution.
            break;
        }

    //Create new item.
        Find_Options_Item* item = new Find_Options_Item();

        if ( item == NULL )
        {
        //Close opened key.
            CloseKey( key_sub );

        //Log.
            TRACE_E( LOW_MEMORY );

            break;
        }

    //Load values from registry.
        item->Registry_Load( key_sub, cfgVersion );

    //Close opened registry key.
        CloseKey( key_sub );

    //Append item to list.
        m_Items.Add( item );

        if ( !m_Items.IsGood() )
        {
        //Free resources.
            delete item;

        //Reset list state.
            m_Items.ResetState();
            break;
        }
    }
}

void Find_Options::Registry_Save( HKEY hKey )
{
//Clear old key.
    ClearKey( hKey );

//Define currently defined search options.
    TCHAR   key_name[30];
    HKEY    key_sub;

    for ( int i = 0; i < m_Items.Count; i++)
    {
    //Define registry key name.
        _itot( i, key_name, 10 );

    //Create registry key.
        const auto      status = CreateKey( hKey, key_name, key_sub );

        if ( !status )
        {
        //Failed to create a subkey -> stop execution.
            break;
        }

    //Save value.
        m_Items[i]->Registry_Save( key_sub );

    //Close key.
        CloseKey( key_sub );
    }
}


