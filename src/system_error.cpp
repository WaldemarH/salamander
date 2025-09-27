// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "system.h"


String_TChar System::Error_GetErrorDescription( const LSTATUS error )
{
//Allocate memory for error description.
    String_TChar      text;

    auto    status = text.Size_CodeUnits_Resize( 2048 );    //2k characters should be more then enough.

//Retrieve error description.
    if ( status == false )
    {
    //Define error id.
    //
    //Notice:
    //In winerror.h some errors are defined as normal numbers and the negative ones are defined as hex.
        auto*           pText = (LPTSTR)text.Text_Get();
        const auto      size_codeUnits = text.Size_CodeUnits();
        const auto      size_codeUnits_used_sprintf = _stprintf_s( pText, size_codeUnits, ( error < 0 ) ? TEXT( "(0x%08X) " ) : TEXT( "(%i) " ), (int)error );

    //Get error description.
        auto            size_codeUnits_used_format = FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            error,
            MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
            pText + size_codeUnits_used_sprintf,
            size_codeUnits - size_codeUnits_used_sprintf,
            NULL
        );

    //Set text length.
        const auto      size_codeUnits_full = size_codeUnits_used_sprintf + ( ( size_codeUnits_used_format > 0 ) ? size_codeUnits_used_format : 0 );
    
        text.Size_CodeUnits_Resize( size_codeUnits_full );

    //Was all ok?
        if ( size_codeUnits_used_format == 0 )
        {
        //No, formatMessage failed -> define generic message.
            text.Text_Append( TEXT_VIEW( "[System error description is not available.]" ) );
        }
    }

    return text;
}
OS::String::String_Utf16 System::Error_GetErrorDescription_Utf16( const LSTATUS error )
{
#ifdef UNICODE
//Forward to TChar version.
    return Error_GetErrorDescription( error );
#else
//Allocate memory for error description.
    OS::String::String_Utf16      text;

    auto    status = text.Size_CodeUnits_Resize( 2048 );    //2k characters should be more then enough.

//Retrieve error description.
    if ( status == false )
    {
    //Define error id.
    //
    //Notice:
    //In winerror.h some errors are defined as normal numbers and the negative ones are defined as hex.
        auto*           pText = (LPWSTR)text.Text_Get();
        const auto      size_codeUnits = text.Size_CodeUnits();
        const auto      size_codeUnits_used_sprintf = swprintf_s( pText, size_codeUnits, ( error < 0 ) ? L"(0x%08X) " : L"(%i) ", (int)error );

    //Get error description.
        auto            size_codeUnits_used_format = FormatMessageW(
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            error,
            MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
            pText + size_codeUnits_used_sprintf,
            size_codeUnits - size_codeUnits_used_sprintf,
            NULL
        );

    //Set text length.
        const auto      size_codeUnits_full = size_codeUnits_used_sprintf + ( ( size_codeUnits_used_format > 0 ) ? size_codeUnits_used_format : 0 );
    
        text.Size_CodeUnits_Resize( size_codeUnits_full );

    //Was all ok?
        if ( size_codeUnits_used_format == 0 )
        {
        //FormatMessage failed -> define generic message.
            text.Text_Append( u"[System error description is not available.]"sv );
        }
    }

    return text;
#endif
}
