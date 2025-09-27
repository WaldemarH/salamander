// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//Includes
    #include "string_base.h"
    #include "string_local_view.h"
    #include "string_utf16_view.h"

//Class
    namespace OS::String
    {
    //Notice:
    //Windows uses wchar_t as a UTF16 representation, but we'll use proper UTF16 characters and convert on the need to do basis to wchar_t.
    //
    //Notice 2:
    //Be careful as Windows allows even partial surrogate pairs to be stored as a path (clearly a bug but it can not be fixed anymore.. I'm 100% some software uses that).
        class String_Utf16 : public String_Base<char16_t>
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

                return std::wcstoll( (wchar_t*)m_Text, nullptr, 10 );
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
                return std::wcstoull( (wchar_t*)m_Text, nullptr, 10 );
            };

        //Size
            public: unsigned long Size_CodePoints() const                       //Does not include NUL character. Notice: It's expensive.
            {
            //Get number of code points.
                return View_Get().Size_CodePoints();
            };

        //Text
            public: bool Text_Append( const std::wstring& text )
            {
            //Forward to base class.
                return String_Base::Text_Append( String_Utf16_View( text ) );
            };

            public: bool Text_Append( const String_Utf16& text )
            {
            //Forward to base class.
                return String_Base::Text_Append( text.View_Get() );
            };
            public: bool Text_Append( const String_Utf16_View& text )
            {
            //Forward to base class.
                return String_Base::Text_Append( text );
            };

            public: bool Text_Append( const String_Local_View& text_local )     //It's expensive.
            {
            //Is it a zero length text?
                if ( text_local.Size_CodeUnits() == 0 )
                {
                //Yes -> ignore.
                    return true;
                }

            //Convert and append local to Utf16.
                //Resize memory (prepare space after current string).
                //
                //Notice:
                //As I don't know if any local character can be converted to surrogate pair we'll just assume that it can happen and we'll reserve double the memory.
                const auto      size_codeUnits_original = Size_CodeUnits();
                auto            status = Size_CodeUnits_Resize( size_codeUnits_original + ( 2 * text_local.Size_CodeUnits() ) );

                //Convert local page to Utf16.
                int     size_utf16_codeUnits_written = 0;

                if ( status )
                {
                    size_utf16_codeUnits_written = MultiByteToWideChar(
                        CP_ACP,
                        0,
                        text_local.Text_Get(),
                        -1,                                                         //Process NUL character also.
                        (LPWSTR)( Text_Get() + size_codeUnits_original ),           //Move to the end of current string.
                        Size_CodeUnits_Allocated() - size_codeUnits_original        //Size in characters.
                    );
                    if ( size_utf16_codeUnits_written == 0 )
                    {
                    //Failed to convert the string.
                        status = false;
                    }
                }

                //Resize string.
                if ( status )
                {
                    status = Size_CodeUnits_Resize( size_codeUnits_original + size_utf16_codeUnits_written - 1 );       //-1 Exclude NUL character from size.
                }

                return status;
            };

        //View
            public: String_Utf16_View View_Get() const
            {
            //Return view to string.
                return String_Utf16_View( m_Text, m_Size_codeUnits );
            };
        };
    }
