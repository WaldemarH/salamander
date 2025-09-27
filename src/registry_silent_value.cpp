// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "registry.h"


bool Registry::Silent_Value_Delete( HKEY hKey, const String_TChar_View& name )
{
//Delete value.
    auto    status_reg =  RegDeleteValue( hKey, name.Text_Get() );

    if ( status_reg != ERROR_SUCCESS )
    {
        TRACE_E( "Unable to delete value from registry." );

        return false;
    }

    return true;
}

bool Registry::Silent_Value_Get( HKEY hKey, const String_TChar_View& name, BYTE* buffer, DWORD buffer_size )
{
//Retrieve binary data.
//
//Notice:
//Read the value as binary data and will not check type in anyway.
    auto    status_reg = RegQueryValueEx( hKey, name.Text_Get(), 0, NULL, buffer, &buffer_size );

    if ( status_reg != ERROR_SUCCESS )
    {
        TRACE_E( "Unable to retrieve registry binary value." );

        return false;
    }

    return true;
}
bool Registry::Silent_Value_Get( HKEY hKey, const String_TChar_View& name, String_TChar& text )
{
//Get size of text.
    LONG    size_inBytes = 0;        //May not include NUL terminated character... old Windows issues.

    auto    status_reg = RegQueryValue( hKey, name.Text_Get(), nullptr, &size_inBytes );

    if ( status_reg != ERROR_SUCCESS )
    {
        TRACE_E( "Unable to retrieve registry string length." );

        return false;
    }

//Does text contain any data?
    if ( size_inBytes <= 0 )
    {
    //No -> nothing to do.
        return true;
    }

//Reallocate memory.
    //Reallocate memory.
    //
    //Notice (from MSDN):
    //If data has the REG_SZ, REG_MULTI_SZ, or REG_EXPAND_SZ type, the string may not have been stored with the proper terminating null characters.
    //Therefore, when reading a string from the registry, you must ensure that the string is properly terminated before using it; otherwise, it may overwrite a buffer.
    //Note that REG_MULTI_SZ strings should have two terminating null characters.
    const unsigned long     size_codeUnits = size_inBytes / sizeof( TCHAR );

    if ( text.Size_CodeUnits_Resize( size_codeUnits + 2 ) == false )            //+2 is for optional NUL terminators (+1 for normal string, +2 for REG_MULTI_SZ).
    {
        TRACE_E( "Out of memory." );

        return false;
    }

    //Set last two characters to NUL.
    auto*   pText = text.Text_Get();

    pText[size_codeUnits] = 0;
    pText[size_codeUnits+1] = 0;

//Retrieve text.
    status_reg = RegQueryValue( hKey, name.Text_Get(), pText, &size_inBytes );

    if ( status_reg != ERROR_SUCCESS )
    {
        TRACE_E( "Unable to retrieve registry string." );

        return false;
    }

//Set proper string length (not including NUL characters.. -1 for normal strings, -2 for REG_MULTI_SZ).
    text.Size_CodeUnits_Resize( size_codeUnits - ( ( pText[size_codeUnits - 1] == 0 ) ? 1 : 0 ) - ( ( pText[size_codeUnits - 2] == 0 ) ? 1 : 0 ) );

    return true;
}
bool Registry::Silent_Value_Get( HKEY hKey, const String_TChar_View& name, DWORD& value )
{
//Retrieve dword.
    DWORD   size_inBytes = (LONG)sizeof( value );
    DWORD   type = 0;

    auto    status_reg = RegGetValue( hKey, NULL, name.Text_Get(), RRF_RT_REG_DWORD, &type, &value, &size_inBytes );

    if ( status_reg != ERROR_SUCCESS )
    {
        TRACE_E( "Unable to retrieve registry dword." );

        return false;
    }

//Validate type.
    if ( type != REG_DWORD )
    {
        TRACE_E( "Invalid value type returned." );

        return false;
    }

    return true;
}
bool Registry::Silent_Value_Get( HKEY hKey, const String_TChar_View& name, QWORD& value )
{
//Retrieve dword.
    DWORD   size_inBytes = (LONG)sizeof( value );
    DWORD   type = 0;

    auto    status_reg = RegGetValue( hKey, NULL, name.Text_Get(), RRF_RT_REG_DWORD, &type, &value, &size_inBytes );

    if ( status_reg != ERROR_SUCCESS )
    {
        TRACE_E( "Unable to retrieve registry dword." );

        return false;
    }

//Validate type.
    if ( type != REG_QWORD )
    {
        TRACE_E( "Invalid value type returned." );

        return false;
    }

    return true;
}

bool Registry::Silent_Value_Set( HKEY hKey, const String_TChar_View& name, const String_TChar_View& text )
{
//Store text.
    auto    status_reg = RegSetValueEx( hKey, name.Text_Get(), 0, REG_SZ, (const BYTE*)text.Text_Get(), text.Size_Bytes() + sizeof( String_TChar_View::codeUnit ) );    //+ sizeof -> include NUL character.

    if ( status_reg != ERROR_SUCCESS )
    {
        TRACE_E( "Unable to set text to registry." );

        return false;
    }

    return true;
}
bool Registry::Silent_Value_Set( HKEY hKey, const String_TChar_View& name, const DWORD value )
{
//Store value.
    auto    status_reg = RegSetValueEx( hKey, name.Text_Get(), 0, REG_DWORD, (const BYTE*)&value, sizeof( value ) );

    if ( status_reg != ERROR_SUCCESS )
    {
        TRACE_E( "Unable to set value to registry." );

        return false;
    }

    return true;
}
bool Registry::Silent_Value_Set( HKEY hKey, const String_TChar_View& name, const QWORD value )
{
//Store value.
    auto    status_reg = RegSetValueEx( hKey, name.Text_Get(), 0, REG_QWORD, (const BYTE*)&value, sizeof( value ) );

    if ( status_reg != ERROR_SUCCESS )
    {
        TRACE_E( "Unable to set value to registry." );

        return false;
    }

    return true;
}


//BOOL GetValue2Aux(HWND parent, HKEY hKey, const char* name, DWORD type1, DWORD type2, DWORD* returnedType, void* buffer, DWORD bufferSize)
//{
//    DWORD gettedType;
//    LONG res = SalRegQueryValueEx(hKey, name, 0, &gettedType, (BYTE*)buffer, &bufferSize);
//    if (res == ERROR_SUCCESS)
//        if (gettedType == type1 || gettedType == type2)
//        {
//            *returnedType = gettedType;
//            return TRUE;
//        }
//        else
//        {
//            if (HLanguage == NULL)
//            {
//                MessageBox(parent, "Unexpected value type.","Error Loading Configuration", MB_OK | MB_ICONEXCLAMATION);
//            }
//            else
//            {
//                SalMessageBox(parent, LoadStr(IDS_UNEXPECTEDVALUETYPE),LoadStr(IDS_ERRORLOADCONFIG), MB_OK | MB_ICONEXCLAMATION);
//            }
//            return FALSE;
//        }
//    else
//    {
//        if (res != ERROR_FILE_NOT_FOUND)
//        {
//            if (HLanguage == NULL)
//            {
//                MessageBox(parent, GetErrorText(res), "Error Loading Configuration", MB_OK | MB_ICONEXCLAMATION);
//            }
//            else
//            {
//                SalMessageBox(parent, GetErrorText(res), LoadStr(IDS_ERRORLOADCONFIG), MB_OK | MB_ICONEXCLAMATION);
//            }
//        }
//        return FALSE;
//    }
//}

//BOOL SetValueAux(HWND parent, HKEY hKey, const char* name, DWORD type, const void* data, DWORD dataSize, BOOL quiet)
//{
//    if (dataSize == -1)
//        dataSize = (DWORD)strlen((char*)data) + 1;
//    LONG res = RegSetValueEx(hKey, name, 0, type, (CONST BYTE*)data, dataSize);
//    if (res == ERROR_SUCCESS)
//        return TRUE;
//    else
//    {
//        if (!quiet)
//        {
//            if (HLanguage == NULL)
//            {
//                MessageBox(parent, GetErrorText(res), "Error Saving Configuration", MB_OK | MB_ICONEXCLAMATION);
//            }
//            else
//            {
//                SalMessageBox(parent, GetErrorText(res), LoadStr(IDS_ERRORSAVECONFIG), MB_OK | MB_ICONEXCLAMATION);
//            }
//        }
//        return FALSE;
//    }
//}
//
