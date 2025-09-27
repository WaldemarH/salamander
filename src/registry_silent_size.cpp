// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "registry.h"


bool Registry::Silent_Size_Get( HKEY hKey, const String_TChar_View& name, DWORD& size_inBytes, LSTATUS* status = nullptr )
{
//Retrieve size of the variable.
    DWORD   type = 0;

    auto    status_reg = RegGetValue( hKey, NULL, name.Text_Get(), RRF_RT_ANY, &type, NULL, &size_inBytes );

    //Return status?
    if ( status != nullptr )
    {
        *status = status_reg;
    }

    //Was all ok?
    if ( status_reg != ERROR_SUCCESS )
    {
        TRACE_E( "Unable to retrieve size of registry value." );

        return false;
    }

//Account for NUL character.
//
//Notice:
//We'll always extend the size_inBytes to include additional NUL characters even if not needed.
//There shouldn't be an issue as if string will be retrieved 'in string' or our additional NUL character/s will stop the string processing.
    switch ( type )
    {
    case REG_EXPAND_SZ:
    case REG_SZ:
    {
    //Single string -> "append" one NUL character.
        size_inBytes += sizeof( TCHAR );
        break;
    }
    case REG_MULTI_SZ:
    {
    //A sequence of strings -> "append" two NUL characters.
        size_inBytes += 2 * sizeof( TCHAR );
        break;
    }
    }

    return true;
}
