// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "registry.h"


bool Registry::Noisy_Size_Get( HWND hWnd_parent, const HKEY hKey, const String_TChar_View& name, DWORD& size_inBytes )
{
//Execute request.
    LSTATUS     status_system = ERROR_SUCCESS;

    auto        status = Silent_Size_Get( hKey, name, size_inBytes, &status_system );

    if ( status == false )
    {
    //Request failed -> make some noise.
        //Get error description.
        const auto      text_description = System::Error_GetErrorDescription( status_system );

        //Show message box.
        if ( HLanguage == NULL )
        {
            MessageBox( hWnd_parent, text_description.Text_Get(), TEXT( "Error loading configuration" ), MB_OK | MB_ICONEXCLAMATION );
        }
        else
        {
            SalMessageBox( hWnd_parent, text_description.Text_Get(), LoadStr( IDS_ERRORLOADCONFIG ), MB_OK | MB_ICONEXCLAMATION );
        }
    }

    return status;
}
