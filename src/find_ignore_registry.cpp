// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_ignore.h"


const TCHAR* FINDIGNOREITEM_ENABLED_REG = TEXT( "Enabled" );
const TCHAR* FINDIGNOREITEM_PATH_REG = TEXT( "Path" );


void FindIgnore::Registry_Load( HKEY hKey, DWORD cfgVersion )
{
//Free any old resources.
    m_Items.DestroyMembers();

//Load all previously stored items.


tole preveri ali je isto in koncaj... verjetno sem na tem mestu sel delat registry in je ostalo


    TCHAR   key_name[30];
    HKEY    key_sub;
    TCHAR   path[MAX_PATH];

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
        Find_Ignore_Item* item = new Find_Ignore_Item();

        if ( item == NULL )
        {
        //Close opened key.
            CloseKey( key_sub );

        //Log.
            TRACE_E( LOW_MEMORY );

            break;
        }

    //Load path.
        if ( !GetValue(key_sub, FINDIGNOREITEM_PATH_REG, REG_SZ, path, MAX_PATH ) )
        {
        //Failed to load path -> define empty path.
            path[0] = 0;
        }

        item->Path = DupStr(path);

    }
    while (OpenKey(hKey, buf, key_sub))
    {
        if (!GetValue(key_sub, FINDIGNOREITEM_ENABLED_REG, REG_DWORD, &item->Enabled, sizeof(DWORD)))
            item->Enabled = TRUE; // ulozeno pouze je-li FALSE
        if (Configuration.ConfigVersion < 32)
        {
            // uzivatele byli zmateni, ze jim neprohledavame tento adresar
            // takze ho sice nechame v seznamu, ale vypneme checkbox
            // kdo chce, muze si ho zapnout
            if (strcmp(item->Path, "Local Settings\\Temporary Internet Files") == 0)
                item->Enabled = FALSE;
        }
        Items.Add(item);
        if (!Items.IsGood())
        {
            Items.ResetState();
            delete item;
            break;
        }
        itoa(++i, buf, 10);
        CloseKey(key_sub);
    }

    return TRUE;
}

void FindIgnore::Registry_Save( HKEY hKey )
{
//Clear old key.
    ClearKey( hKey );

//Define currently defined search options.
    TCHAR   key_name[30];
    HKEY    key_sub;

    for ( int i = 0; i < m_Items.Count; i++)
    {
    //Define registry key name.
        _itot( i+1, key_name, 10 );

    //Create registry key.
        const auto      status = CreateKey( hKey, key_name, key_sub );

        if ( !status )
        {
        //Failed to create a subkey -> stop execution.
            break;
        }

    //Save value.
        const auto*     item = m_Items[i];

        //Path
        SetValue( key_sub, FINDIGNOREITEM_PATH_REG, REG_SZ, item->Path, -1 );

        //Enabled
        //
        //Notice:
        //Save only if it is FALSE.
        if ( !item->Enabled )
        {
            SetValue( key_sub, FINDIGNOREITEM_ENABLED_REG, REG_DWORD, &item->Enabled, sizeof( DWORD ) );
        }

    //Close key.
        CloseKey( key_sub );
    }
}
