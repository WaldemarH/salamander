// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//Includes
    #include "string_base_view.h"
    #include <cstdlib>

//Class
    namespace OS::String
    {
    //Define string code-unit (i.e. character) class.
    //
    //What is the issue?
    //To fully support UNICODE we need to make sure the app. understand multi-code-units encodings.
    //
    //Example of encoding the full UNICODE range:
    //
    //Encoding form    Code unit size   Code unit
    //UTF-32           32-bit           Code point is encoded with one 32-bit code unit
    //                                  - U+000000 – U+10FFFF (1 unit)
    //
    //UTF-16           16-bit           Code point is encoded with one or two 16-bit code units
    //                                  - U+000000 – U+00FFFF (1 unit)
    //                                  - U+010000 – U+10FFFF (2 units, surrogate pair)
    //
    //UTF-8            8-bit            Code point is encoded with one to four 8-bit code units
    //                                  - U+000000 – U+00007F (1 unit)
    //                                  - U+000080 – U+0007FF (2 units)
    //                                  - U+000800 – U+00FFFF (3 units)
    //                                  - U+010000 – U+10FFFF (4 units)
    //
    //From this we see that UTF-8 and UTF-16 are both multi-code-unit encodings, where locale (with it's 8bit char) and UTF-32 are not.
    //The good side of the later is that they always operate on a single real character at a time. So the algorithms can be much simpler.
    //
    //Notice:
    //Since Windows 2000 the 'wide API' is using UTF16 and not UCS-2 anymore, so we can assume that all Windows that we support use UTF16.
        template <typename codeUnitType> class String_Base
        {
        //Constructor/Destructor
            public: ~String_Base()
            {
            //Free resources.
                if ( m_Text != nullptr )
                {
                    free( m_Text );
                    //m_Text = nullptr;
                }
            }

            public: typedef codeUnitType   codeUnit;

            protected: unsigned long        m_Size_codeUnits = 0;               //Does not include NUL character (but the NUL codePoint will always be added to the text buffer).
            protected: unsigned long        m_Size_codeUnits_allocated = 0;
            protected: codeUnitType*        m_Text = nullptr;

        //Convert
        tole se naredi
		//public: Status FromFloat32( const float32 value );
		//public: Status FromFloat64( const float64 value );
		//public: Status ToFloat32( float32& value );
		//public: Status ToFloat64( float64& value );

		private: Status FromNumber_Internal( uint64 value_unsigned, const bool value_isNegative ) NO_LOG
		{
		//Clear buffer.
			Clear();

		//Test for special case.
			if ( value_unsigned == 0 )
			{
			//It's zero.
			//
			//Notice:
			//As most number would be 0, we'll just use a specialization and lower the used time drastically.
				return Copy( '0' );
			}

		//Check sign.
			uint32		size_number;

			if ( value_isNegative == false )
			{
			//No sign character prepended.
				size_number = 0;
			}
			else
			{
			//A negative value -> get a positive value.
				value_unsigned = (uint64)( -(int64)value_unsigned );

			//Include '-' sign.
				size_number = 1;
			}

		//Get number of digits.
			size_number += Math_NDigits( value_unsigned );

		//Resize memory.
			Status		status = Resize( size_number );

			if ( LST_FAILED( status ) )
			{
				return status;
			}

		//Get destination pointer.
		//
		//Notice:
		//Move destination pointer to end of number (we'll enter the characters from right to left).
		//
		//Notice 2:
		//Has to be after resize as pointer can be invalid in other case.
			auto*		pDestination = m_pData + size_number;

		//Convert.
			static const char	digits_table[200] =
			{
				'0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8','0','9',
				'1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7','1','8','1','9',
				'2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7','2','8','2','9',
				'3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7','3','8','3','9',
				'4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7','4','8','4','9',
				'5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7','5','8','5','9',
				'6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7','6','8','6','9',
				'7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9',
				'8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',
				'9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8','9','9'
			};

			while ( value_unsigned >= 100000000 )
			{
				const uint32		value_32bit_a = static_cast<uint32>( value_unsigned % 100000000 );
				value_unsigned /= 100000000;

				const uint32	value_32bit_b = value_32bit_a / 10000;
				const uint32	value_32bit_c = value_32bit_a % 10000;

				const uint32	value_32bit_b1 = (value_32bit_b / 100) << 1;
				const uint32	value_32bit_b2 = (value_32bit_b % 100) << 1;
				const uint32	value_32bit_c1 = (value_32bit_c / 100) << 1;
				const uint32	value_32bit_c2 = (value_32bit_c % 100) << 1;

				pDestination -= 8;

				pDestination[0] = (itemType)digits_table[value_32bit_b1];
				pDestination[1] = (itemType)digits_table[value_32bit_b1 + 1];
				pDestination[2] = (itemType)digits_table[value_32bit_b2];
				pDestination[3] = (itemType)digits_table[value_32bit_b2 + 1];
				pDestination[4] = (itemType)digits_table[value_32bit_c1];
				pDestination[5] = (itemType)digits_table[value_32bit_c1 + 1];
				pDestination[6] = (itemType)digits_table[value_32bit_c2];
				pDestination[7] = (itemType)digits_table[value_32bit_c2 + 1];
			}

			uint32		value_unsigned_32 = static_cast<uint32>( value_unsigned );
			while ( value_unsigned_32 >= 100 )
			{
				const unsigned i = static_cast<unsigned>( value_unsigned_32 % 100 ) << 1;
				value_unsigned_32 /= 100;

				*--pDestination = (itemType)digits_table[i + 1];
				*--pDestination = (itemType)digits_table[i];
			}

			if ( value_unsigned_32 < 10 )
			{
				*--pDestination = (itemType)( '0' + (char)value_unsigned_32 );
			}
			else
			{
				const unsigned		i = static_cast<unsigned>(value_unsigned_32) << 1;
				*--pDestination = (itemType)digits_table[i + 1];
				*--pDestination = (itemType)digits_table[i];
			}

		//Set '-' sign in case of a negative value.
			if ( value_isNegative == true )
			{
				*--pDestination = (itemType)'-';
			}

			return status;
		};
		public: template< class type_internal > typename std::enable_if<LST::IsIntegral<type_internal>::value, Status>::type FromNumber( const type_internal value ) NO_LOG
		{
		//Forward request.
			if constexpr ( std::is_signed<type_internal>::value == true )
			{
			//Signed values.
				return FromNumber_Internal( (uint64)( (int64)value ), ( value < 0 ) );
			}
			else
			{
			//Unsigned values.
				return FromNumber_Internal( (uint64)value, false );
			}
		};

        //Memory
            public: void Clear()
            {
            //Free resources.
                if ( m_Text != nullptr )
                {
                    free( m_Text );
                    m_Text = nullptr;
                }
                m_Size_codeUnits = 0;
                m_Size_codeUnits_allocated = 0;
            };

        //Size
            public: inline unsigned long Size_Bytes() const
            {
            //Return number of bytes.
                return ( m_Size_codeUnits * sizeof( codeUnitType ) );
            }
            public: inline unsigned long Size_Bytes_Allocated() const
            {
            //Return number of bytes.
                return ( m_Size_codeUnits_allocated * sizeof( codeUnitType ) );
            }
            //public: inline unsigned long Size_CodePoints() const;             //Implement in derived classes.

            public: inline unsigned long Size_CodeUnits() const
            {
            //Return number of code units.
                return m_Size_codeUnits;
            };
            public: unsigned long Size_CodeUnits_Allocated() const
            {
            //Return current allocated size.
                return m_Size_codeUnits_allocated;
            };
            public: bool Size_CodeUnits_Reserve( unsigned long size_allocate_codeUnits )
            {
            //Is it less then what is currently available?
                if ( size_allocate_codeUnits <= m_Size_codeUnits_allocated )
                {
                //Yes -> ignore.
                    return true;
                }

            //Align allocated size with 16bytes.
                const auto     size_allocate_codeUnits_execute = ( ( (unsigned long long)size_allocate_codeUnits ) + 0x0F ) & 0xFFFFFFFFFFFFFFF0;

                if ( size_allocate_codeUnits_execute > 0x00000000FFFFFFFFULL )
                {
                //Out of limits, can only allocate 32bit of code units.
                    return false;
                }

            //Reallocate.
                auto*   text_new = (codeUnitType*)realloc( (void*)m_Text, /*in bytes*/ size_allocate_codeUnits_execute * sizeof( codeUnitType ) );

                if ( text_new == nullptr )
                {
                //Out of memory.
                    //TRACE_E( LOW_MEMORY );

                    return false;
                }

                //Update buffer.
                m_Text = text_new;
                m_Size_codeUnits_allocated = (unsigned long)size_allocate_codeUnits_execute;

                return true;
            };
            public: bool Size_CodeUnits_Resize( const unsigned long size_resize_codeUnits, const bool initialize = false )
            {
            //Reallocate memory.
                const auto      size_codeUnits_original = m_Size_codeUnits;

                auto            status = Size_CodeUnits_Reserve( size_resize_codeUnits );

            //Resize.
                if ( status == true )
                {
                //Resize.
                    m_Size_codeUnits = size_resize_codeUnits;

                //Initialize (only the new memory)?
                    if (
                        ( initialize == true )                                      //Should the new memory be initialized?
                        &&
                        ( size_resize_codeUnits > size_codeUnits_original )         //Was memory extended?
                    )
                    {
                        memset( m_Text + size_codeUnits_original, 0, /*in bytes*/ ( (size_t)( size_resize_codeUnits - size_codeUnits_original ) ) * sizeof( codeUnitType ) );
                    }
                }

                return status;
            }

        //Text
            public: bool Text_Append( const String_Base_View<codeUnitType>& text )
            {
            //Get the new size.
                const auto      size_codeUnits_new = (unsigned long long)m_Size_codeUnits + text.Size_CodeUnits() + 1;    //+1 -> NUL terminated

                if ( size_codeUnits_new > 0x00000000FFFFFFFFULL )
                {
                //Out of limits, can only allocate 32bit of code units.
                    return false;
                }

            //Reallocate memory?
                if ( (unsigned long)size_codeUnits_new > m_Size_codeUnits_allocated )
                {
                //Not enough space -> reallocate.
                    if ( Size_CodeUnits_Reserve( (unsigned long)size_codeUnits_new ) == false )
                    {
                        return false;
                    }
                }

            //Append text.
                memmove( m_Text + m_Size_codeUnits, text.Text_Get(), text.Size_Bytes() );

                m_Size_codeUnits += text.Size_CodeUnits();

            //NUL terminate.
                m_Text[m_Size_codeUnits] = 0;

                return true;
            };

            public: codeUnitType* Text_Get()
            {
            //Return string buffer.
                return m_Text;
            };
            public: const codeUnitType* Text_Get() const
            {
            //Return string buffer.
                return m_Text;
            };
        };
    }



//            public: bool Text_Append( const String_Base_View<Character<char,1>>& text_local )
//            {
//            //Get text_from.
//                itemType*       text_from = nullptr;
//                bool            text_from_free = false;
//
//                if constexpr ( std::is_same_v< itemType, char >::value == f )
//                {
//                //Class uses char and view uses char -> append view normally.
//                    text_from = text_ascii.Text();
//                }
//                else
//                {
//                //Class uses wchar_t, but input view uses char -> expand input view to wide characters.
//                    //Allocate memory.
//                    text_from = (itemType*)realloc( nullptr, ( (size_t)text_ascii.Length() + 1 ) * sizeof( itemType ) );
//
//                    if ( text_from == nullptr )
//                    {
//                    //Out of memory.
//                        TRACE_E( LOW_MEMORY );
//
//                        return false;
//                    }
//
//                    //Define that text_from needs to be freed.
//                    text_from_free = true;
//
//                    //Expand input ASCII characters view to wide characters.
//                    //
//                    //Notice:
//                    //ASCII characters are the same as first 127 Unicode characters, so we just need to expan it.
//                    const auto*     text = text_ascii.Text();
//
//                    for ( int i = 0, i_max = text_ascii.Length(); i < i_max; ++i )
//                    {
//                        text_from[i] = (wchar_t)text[i];
//                    }
//                }
//
//            //Reallocate memory?
//                auto      length_new = m_Length + text.Length() + 1;    //+1 -> NUL terminated
//
//                if ( length_new > m_Length_allocated )
//                {
//                //Not enough space -> reallocate.
//                    //Set the new buffer size.
//                    length_new += 256;
//
//                    //Reallocate.
//                    if ( Length_Reserve( length_new ) == false )
//                    {
//                    //Free resources?
//                        if ( text_from_free == true )
//                        {
//                        //Yes.
//                            free( text_from );
//                            //text_from = nullptr;
//                        }
//
//                        return false;
//                    }
//                }
//
//            //Append text.
//                memmove( m_Text + m_Length, text_from, text.Length() * sizeof( itemType ) );
//
//                m_Length += text.Length();
//
//            //Nul terminate.
//                m_Text[m_Length] = 0;
//
//            //Free resources?
//                if ( text_from_free == true )
//                {
//                //Yes.
//                    free( text_from );
//                    //text_from = nullptr;
//                }
//
//                return true;
//            };
//            public: bool Text_Append( const BChar_View<wchar_t>& text )
//            {
//            //Get text_from.
//                itemType*       text_from = nullptr;
//                bool            text_from_free = false;
//
//                if constexpr ( std::is_same_v< itemType, char >::value == f )
//                {
//                //Class uses char and view uses wchar_t -> truncate input view to ASCII.
//                    //Allocate memory.
//                    text_from = (itemType*)realloc( nullptr, ( (size_t)text_ascii.Length() + 1 ) * sizeof( itemType ) );
//
//                    if ( text_from == nullptr )
//                    {
//                    //Out of memory.
//                        TRACE_E( LOW_MEMORY );
//
//                        return false;
//                    }
//
//                    //Define that text_from needs to be freed.
//                    text_from_free = true;
//
//                    //Truncate input UNICODE characters to ASCII characters view to wide characters.
//                    //
//                    //Notice:
//                    //ASCII characters are the same as first 127 Unicode characters, so we just need to expan it.
//                    const auto*     text = text_ascii.Text();
//
//                    for ( int i = 0, i_max = text_ascii.Length(); i < i_max; ++i )
//                    {
//                        text_from[i] = (wchar_t)text[i];
//                    }
//
//
//tele     popravi UTF-16 zgleda nima spodaj ascii
//
//
//
//                    text_from = text_ascii.Text();
//                }
//                else
//                {
//                //Class uses wchar_t, but input view uses char -> expand input view to wide characters.
//                }
//
//
//
//
//
//
//
//            //Reallocate memory?
//                auto      length_new = m_Length + text.Length() + 1;    //+1 -> NUL terminated
//
//                if ( length_new > m_Length_allocated )
//                {
//                //Not enough space -> reallocate.
//                    //Set the new buffer size.
//                    length_new += 256;
//
//                    //Reallocate.
//                    if ( Length_Reserve( length_new ) == false )
//                    {
//                        return false;
//                    }
//                }
//
//            //Append text.
//                memmove( m_Text + m_Length, text.Text(), text.Length() * sizeof( itemType ) );
//
//                m_Length += text.Length();
//
//            //Nul terminate.
//                m_Text[m_Length] = 0;
//
//                return true;
//            };
//
