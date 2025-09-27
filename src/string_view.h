//// SPDX-FileCopyrightText: 2023 Open Salamander Authors
//// SPDX-License-Identifier: GPL-2.0-or-later
//
//#pragma once
//
////Includes
//    #include <type_traits>
//    #include "string_char_view.h"
//
////Class
//    class String_View
//    {
//    //Constructor/Destructor
//    //
//    //Notice:
//    //Do not include std::initializer_list in view as std::initializer_list gets destructed right after the creation.
//    //i.e. you always need to make a copy of initializer_list data.
//        public: constexpr String_View() : m_Text( nullptr ), m_Size_chars( 0 ), m_Size_codeUnits( 0 ) {};
//        public: constexpr String_View( const code_unit* text ) : m_Text( const_cast<code_unit*>( text ) )
//        {
//        //Get the number of code-units and real characters.
//            if constexpr ( std::is_same< code_unit, char >::value == true )
//            {
//            //Code-unit is char (locale) -> every byte is a real character.
//                unsigned long       size = 0;
//                const code_unit*    text_size = m_Text;
//    
//                while ( *text_size++ != 0 )
//                {
//                    ++size;
//                }
//    
//                m_Size_chars = m_Size_codeUnits = size;
//            }
//            //TODO (if ever needed)
//            //else if constexpr ( std::is_same< code_unit, char8_t >::value == true )
//            //{
//            ////Code-unit is UTF8 char -> multi-code-unit can represent a real character.
//            ////
//            ////Notice:
//            ////Invalid code-unit sequences will be interpreted as a single real character... so depending
//            ////on the bytes sequences up to 4 bytes can be interpreted as a single real character.
//            //    //TODO
//            //    Bytes that never appear in UTF-8: 0xC0, 0xC1, 0xF5–0xFF
//            //    A "continuation byte" (0x80–0xBF) at the start of a character
//            //    A non-continuation byte (or the string ending) before the end of a character
//            //    An overlong encoding (0xE0 followed by less than 0xA0, or 0xF0 followed by less than 0x90)
//            //    A 4-byte sequence that decodes to a value greater than U+10FFFF (0xF4 followed by 0x90 or greater)
//            //    
//            //    signed long       size_nChars = 0;
//            //    signed long       size_nCodeUnits = 0;
//            //    const code_unit*    text_size = m_Text;
//            //    
//            //    while ( *text_size++ != 0 )
//            //    {
//            //        ++size_nCodeUnits;
//            //    }
//            //    
//            //    m_Size_chars = size_nChars;
//            //    m_Size_codeUnits = size_nCodeUnits;
//            //}
//            else if constexpr ( std::is_same< code_unit, wchar_t >::value == true )
//            {
//            //Code-unit is wchar_t (UTF-16) -> multi-code-unit can represent a real character.
//                signed long         size_nChars = 0;
//                signed long         size_nCodeUnits = 0;
//                const code_unit*    text_size = m_Text;
//                
//                while ( *text_size++ != 0 )
//                {
//                    ++size_nCodeUnits;
//                }
//                
//                m_Size_chars = size_nChars;
//                m_Size_codeUnits = size_nCodeUnits;
//
//            }
//            else
//            {
//                static_assert( false, "Invalid code-unit" );
//            }
//        };
//        public: constexpr String_View( const code_unit* text, const unsigned long size_chars, const unsigned long size_codeUnits ) : m_Text( const_cast<code_unit*>( text ) ), m_Size_chars( size_chars ), m_Size_codeUnits( size_codeUnits ) {};
//        public: constexpr String_View( const String_View& from ) : m_Text( from.m_Text ), m_Size_chars( from.m_Size_chars ), m_Size_codeUnits( from.m_Size_codeUnits )
//        {
//        //Copy constructor.
//        };
//    
//        public: constexpr String_View& operator=( const String_View& from )
//        {
//        //Copy assignment operator.
//            m_Size_chars = from.m_Size_chars;
//            m_Size_codeUnits = from.m_Size_codeUnits;
//            m_Text = const_cast<code_unit*>( from.m_Text );
//    
//            return *this;
//        };
//    
//        public: constexpr  bool operator!=( const String_View& compare_to )
//        {
//        //Difference operator.
//            return !IsEqual( compare_to );
//        };
//        public: constexpr bool operator==( const String_View& compare_to )
//        {
//        //Equal operator.
//            return IsEqual( compare_to );
//        };
//    
//        protected: unsigned long    m_Size_chars;           //Notice: Should behave as if its const!
//        protected: unsigned long    m_Size_codeUnits;       //Notice: Should behave as if its const!
//        protected: code_unit*       m_Text;                 //Notice: Should behave as if its const! Notice 2: Must not be const or compiler can do all sorts of optimization, even removing it from class if it wants.
//    
//    //At
//        //[W: At would be very slow as we would have to traverse whole string until index to properly return a code-point.
//        //public: constexpr const Char_View& At( const unsigned long index ) const
//        //{
//        ////Validate.
//        //#ifdef DEBUG
//        //    if (
//        //      ( m_pText == nullptr )
//        //      ||
//        //      ( index >= m_Size )
//        //    )
//        //    {
//        //    //Empty or index out of range.
//        //        throw;			//Notice: even if exceptions are disabled the debugger will break.
//        //    }
//        //#endif
//        //    
//        ////Get item.
//        //    return m_pText[index];
//        //};
//        //public: constexpr const itemType& At_First() const
//        //{
//        ////Get item.
//        //    return At( 0 );
//        //};
//        //public: constexpr const itemType& At_Last() const
//        //{
//        ////Get item.
//        //    return At( m_Length-1 );
//        //};
//    
//    //Clear
//        protected: constexpr void Clear()
//        {
//        //Remove view.
//            m_Size_chars = 0;
//            m_Size_codeUnits = 0;
//            m_Text = nullptr;
//        };
//    
//    //Compare
//        protected: constexpr bool IsEqual( const itemType* pText, const unsigned long length ) const
//        {
//        //Do they have the same length?
//            unsigned long		length_compareTo = length;
//            unsigned long		length_object = m_Length;
//    
//            if ( length_compareTo != length_object )
//            {
//            //No -> they are not the same.
//                return false;
//            }
//        
//        //Compare their data.
//        //
//        //Notice:
//        //Empty or uninitialized streams are the same too.
//            const itemType*		pText_compareTo = pText;
//            const itemType*		pText_object = m_pText;
//        
//            for ( unsigned long i = 0, i_max = length; i < i_max; ++i )
//            {
//                if ( pText_compareTo[i] != pText_object[i] )
//                {
//                //Not the same.
//                    return false;
//                }
//            }
//    
//        //If we came to here they the streams are the same.
//            return true;
//        };
//        protected: constexpr bool IsEqual( const String_View& compare_to ) const
//        {
//        //Forward request.
//            return IsEqual( compare_to.Text(), compare_to.Length() );
//        };
//    
//    //Length
//        public: constexpr unsigned long Length() const
//        {
//        //Return number of characters.
//            return m_Length;
//        };
//        public: constexpr unsigned long Length_withNul() const
//        {
//        //Return number of characters with NUL character included.
//            return ( m_Length > 0 ) ? ( m_Length + 1 ) : 0;
//        };
//        public: constexpr bool Length_IsEmpty() const
//        {
//        //Is string empty?
//            return ( m_Length == 0 );
//        };
//    
//    //Size
//        public: constexpr unsigned long Size() const
//        {
//        //Return number of bytes.
//            return ( m_Length * sizeof( itemType ) );
//        };
//        public: constexpr unsigned long Size_withNul() const
//        {
//        //Return number of bytes with NUL character included.
//            return ( m_Length > 0 ) ? ( ( m_Length + 1 ) * sizeof( itemType ) ) : 0;
//        };
//    
//    //Text
//        public: constexpr const itemType* Text() const
//        {
//            return m_pText;
//        };
//    };
//
//    inline constexpr String_View<char> operator "" _str( const char* pText, std::size_t size )
//    {
//    //Define string view.
//        return String_View<char>( pText, size );
//    };
//    inline constexpr String_View<wchar_t> operator "" _wstr( const wchar_t* pText, std::size_t size )
//    {
//    //Define string view.
//        return String_View<wchar_t>( pText, size );
//    };
