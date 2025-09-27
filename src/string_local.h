// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//Includes
    #include "string_base.h"
    #include "string_local_view.h"
    #include "string_utf16_view.h"

//Class
    namespace OS::String
    {
        class String_Local : public String_Base<char>
        {
        //Constructor/Destructor
            public: using String_Base::String_Base;

        //Convert
            public: long long ToInt64() const
            {
            //Validate.
                if ( m_Size_codeUnits == 0 )
                {
                //Nothing to convert.
                    return 0;
                }

            //Convert to int64.
                char*   pEnd = nullptr;

                return std::strtoll( m_Text, nullptr, 10 );
            };
            public: unsigned long long ToUInt64() const
            {
            //Validate.
                if ( m_Size_codeUnits == 0 )
                {
                //Nothing to convert.
                    return 0;
                }

            //Convert to int64.
                return std::strtoull( m_Text, nullptr, 10 );
            };

        //Size
            public: inline unsigned long Size_CodePoints() const                //Does not include NUL character.
            {
            //For local codepage the 'code unit' and 'code point' have the same size -> just return the number of code units.
                return m_Size_codeUnits;
            };

        //Text
            public: bool Text_Append( const std::string& text )
            {
            //Forward to base class.
                return String_Base::Text_Append( String_Local_View( text ) );
            };

            public: bool Text_Append( const String_Local& text )
            {
            //Forward to base class.
                return String_Base::Text_Append( text.View_Get() );
            };
            public: bool Text_Append( const String_Local_View& text )
            {
            //Forward to base class.
                return String_Base::Text_Append( text );
            };

            public: bool Text_Append( const String_Utf16_View& text_utf16 )     //It's expensive.
            {
            //Is it a zero length text?
                if ( text_utf16.Size_CodeUnits() == 0 )
                {
                //Yes -> ignore.
                    return true;
                }

            //Convert and append Utf16 to locale.
            //
            //Notice:
            //Locale string will always contain less 'code points' (real characters) then there are 'code units' in UTF16 string.
                //Resize memory (prepare space after current string).
                const auto      size_codeUnits_original = Size_CodeUnits();
                auto            status = Size_CodeUnits_Resize( size_codeUnits_original + text_utf16.Size_CodeUnits() );

                //Convert Utf16 to local page.
                int     size_local_bytes_written = 0;

                if ( status )
                {
                    size_local_bytes_written = WideCharToMultiByte(
                        CP_ACP,
                        0,
                        (LPCWCH)text_utf16.Text_Get(),
                        -1,                                                                                 //Process NUL character also.
                        (LPSTR)( Text_Get() + size_codeUnits_original ),                                    //Move to the end of current string.
                        ( Size_CodeUnits_Allocated() - size_codeUnits_original ) * sizeof( codeUnit ),      //Size in bytes.
                        NULL,
                        NULL
                    );
                    if ( size_local_bytes_written == 0 )
                    {
                    //Failed to convert the string.
                        status = false;
                    }
                }

                //Resize string.
                if ( status )
                {
                    status = Size_CodeUnits_Resize( size_codeUnits_original + ( size_local_bytes_written / sizeof( codeUnit ) ) - 1 );      //-1 Exclude NUL character from size.
                }

                return status;
            };

        //View
            public: String_Local_View View_Get() const
            {
            //Return view to string.
                return String_Local_View( m_Text, m_Size_codeUnits );
            };
        };
    }
