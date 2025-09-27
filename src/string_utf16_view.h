// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//Includes
    #include "string_base_view.h"

//Class
    namespace OS::String
    {
    //Notice:
    //Windows uses wchar_t as a UTF16 representation, but we'll use proper UTF16 characters and convert on the need to do basis to wchar_t.
    //
    //Notice 2:
    //Be careful as Windows allows even partial surrogate pairs to be stored as a path (clearly a bug but it can not be fixed anymore.. I'm 100% some software uses that).
        class String_Utf16_View : public String_Base_View<char16_t>
        {
        //Constructor/Destructor
            public: using String_Base_View::String_Base_View;

            //Since Windows 2000 these two represent the same string... UCS-2 is represented by a single 16bit container where UTF16 can be represented by two 16bit containers.
            public: constexpr String_Utf16_View( const std::u16string_view& from ) : String_Base_View<char16_t>( from.data(), (unsigned long)from.length() ){};
            public: constexpr String_Utf16_View( const std::wstring_view& from ) : String_Base_View<char16_t>( (const char16_t*)from.data(), (unsigned long)from.length() ){};

        //Size
            public: constexpr unsigned long Size_CodePoints() const         //Does not include NUL character.
            {
            //'Code point' is encoded with one or two 16-bit 'code units' -> get number of 'code point'-s.
            //
            //Encoding form    Code unit size   Code unit
            //UTF-16           16-bit           Code point is encoded with one or two 16-bit code units
            //                                  - U+000000 – U+00FFFF (1 unit)
            //                                  - U+010000 – U+10FFFF (2 units, surrogate pair)
            //
                bool                isPair = false;
                unsigned long       size_codePoints = 0;
                const auto*         text = m_Text;

                for ( unsigned long i = 0, i_max = m_Size_codeUnits; i < i_max; ++i )
                {
                //Get code unit.
                    const auto  codeUnit = text[i];

                //Is it surrogate pair?
                //
                //Ranges:
                //1. U+0000<->U+D7FF and U+E000<->U+FFFF are valid UTF-16 and UCS-2 code points
                //2. U+010000<->U+10FFFF are valid UTF-16 code points -> converted to D800<->DFFF surrogate pairs
                //
                //   Notice:
                //   Windows allows unpaired surrogate values in filenames. They don't have any visual representation,
                //   but can still occure inside the string.
                //
                    if (
                        ( codeUnit <= 0xD7FF )
                        ||
                        ( codeUnit >= 0xE000 )
                    )
                    {
                    //No, a normal character -> was previous code unit part of a surrogate pair?
                        if ( isPair == false )
                        {
                        //No, it was a valid code point -> count only current codeUnit/codePoint.
                            size_codePoints += 1;
                        }
                        else
                        {
                        //Yes, it was an ILLEGAL code unit -> count previous 'code point' also as if it was a valid character.
                            size_codePoints += 2;

                        //Reset pair flag.
                            isPair = false;
                        }
                    }
                    else
                    {
                    //Yes -> is it a second part of the surrogate pair?
                        if ( isPair == false )
                        {
                        //No it's the first one -> signal that we have a possible surrogate pair.
                            isPair = true;
                        }
                        else
                        {
                        //Yes it's the second one -> count as one 'code point' (i.e. a real character).
                            size_codePoints += 1;

                        //Reset pair flag.
                            isPair = false;
                        }
                    }
                }

                return size_codePoints;
            };
        };

    ////Global operators.
    ////
    ////Notice:
    ////Defined in a separate namespace, so that we can define using namespace with minimal items and not clash with other namespaces.
    //    namespace String_View_Literals
    //    {
    //        inline constexpr String_Utf16_View operator "" _str_utf16( const wchar_t* pText, std::size_t size )
    //        {
    //        //Define string view.
    //        //
    //        //Notice:
    //        //From Windows 2000 wchar_t behaves the same way as char16_t.
    //            return String_Utf16_View( (const char16_t*)pText, (unsigned long)size );
    //        };
    //        inline constexpr String_Utf16_View operator "" _str_utf16( const char16_t* pText, std::size_t size )
    //        {
    //        //Define string view.
    //            return String_Utf16_View( pText, (unsigned long)size );
    //        };
    //    }
    }
