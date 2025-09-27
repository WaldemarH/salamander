// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//Includes
    #include "string_base_view.h"

//Class
    namespace OS::String
    {
        class String_Local_View : public String_Base_View<char>
        {
        //Constructor/Destructor
            public: using String_Base_View::String_Base_View;

            public: constexpr String_Local_View( const std::string_view& from ) : String_Base_View<char>( from.data(), (unsigned long)from.length() ){};

        //Size
            public: constexpr unsigned long Size_CodePoints() const         //Does not include NUL character.
            {
            //For local codepage the 'code unit' and 'code point' have the same size -> just return the number of code units.
                return m_Size_codeUnits;
            };
        };

    ////Global operators.
    ////
    ////Notice:
    ////Defined in a separate namespace, so that we can define using namespace with minimal items and not clash with other namespaces.
    //    namespace String_View_Literals
    //    {
    //        inline constexpr String_Local_View operator "" _str_local( const char* pText, std::size_t size )
    //        {
    //        //Define string view.
    //            return String_Local_View( pText, (unsigned long)size );
    //        };
    //    }
    }
