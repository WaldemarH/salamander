// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "registry.h"
#include "common/handles.h"


void Registry::Silent_Key_Close( const HKEY hKey )
{
//Close registry key.
    HANDLES( RegCloseKey( hKey ) );
}

bool Registry::Silent_Key_Create( HKEY hKey, const String_TChar_View& name, HKEY& hKey_created, LSTATUS* status )
{
//Create registry key.
    DWORD           createType = 0;
    const auto      status_reg = HANDLES( RegCreateKeyEx( hKey, name.Text_Get(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey_created, &createType ) );

    if ( status_reg != ERROR_SUCCESS )
    {
        TRACE_E( "Unable to create registry key." );
    }

//Return status?
    if ( status != nullptr )
    {
        *status = status_reg;
    }

    return ( status_reg == ERROR_SUCCESS );
}

bool Registry::Silent_Key_Delete( const HKEY hKey, const String_TChar_View& name, LSTATUS* status )
{
//Delete registry key.
    const auto      status_reg = RegDeleteKey( hKey, name.Text_Get() );

    if ( status_reg != ERROR_SUCCESS )
    {
        TRACE_E( "Unable to delete registry key." );
    }

//Return status?
    if ( status != nullptr )
    {
        *status = status_reg;
    }

    return ( status_reg == ERROR_SUCCESS );
}

bool Registry::Silent_Key_Delete_Branch( const HKEY hKey )
{
//Remove whole tree (all sub-keys).
//
//Notice:
//We'll start from the first branch and delete everything until nothing is left.
    HKEY    hKey_sub;
    TCHAR   name[REGISTRY_LIMIT_NAME_KEY];
    DWORD   name_size_codeUnit = sizeof( name ) /sizeof( name[0] );

    while ( RegEnumKey( hKey, 0, name, name_size_codeUnit ) == ERROR_SUCCESS )
    {
    //Open sub-key.
        auto    status_reg = RegOpenKeyEx( hKey, name, 0, KEY_READ | KEY_WRITE, &hKey_sub );

        if ( status_reg != ERROR_SUCCESS )
        {
        //Something strange is going on (a returned sub-key doesn't exist anymore) -> stop execution.
            TRACE_E( "Unable to delete sub-branch in specified key (in registry)." );

            return false;
        }

    //Remove sub-branch.
        auto    status_clear = Silent_Key_Delete_Branch( hKey_sub );

    //Delete branch.
        //Close current key.
        HANDLES( RegCloseKey( hKey_sub ) );

        //Was all ok?
        if ( status_clear == false )
        {
        //No -> stop execution.
            return false;
        }

        //Delete key.
        status_reg = RegDeleteKey( hKey, name );

        if ( status_reg != ERROR_SUCCESS )
        {
        //Failed to delete the key -> stop execution.
            return false;
        }
    }

//Remove all values in current key.
    DWORD   name_size_codeUnit_copy = name_size_codeUnit;

    while ( RegEnumValue( hKey, 0, name, &name_size_codeUnit_copy, NULL, NULL, NULL, NULL ) == ERROR_SUCCESS )
    {
    //Delete value.
        auto    status_reg = RegDeleteValue( hKey, name );

        if ( status_reg != ERROR_SUCCESS )
        {
            TRACE_E( "Unable to delete values in specified key (in registry)." );

            return false;
        }

    //Clone value before next iteration.
        name_size_codeUnit_copy = name_size_codeUnit;
    }

    return true;
}

bool Registry::Silent_Key_Open( const HKEY hKey, const String_TChar_View& name, HKEY& hKey_opened, LSTATUS* status )
{
//Open registry key.
    const auto      status_reg = HANDLES_Q( RegOpenKeyEx( hKey, name.Text_Get(), 0, KEY_READ, &hKey_opened ) );

    if ( status_reg != ERROR_SUCCESS )
    {
        TRACE_E( "Unable to open registry key." );
    }

//Return status?
    if ( status != nullptr )
    {
        *status = status_reg;
    }

    return ( status_reg == ERROR_SUCCESS );
}
