// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//Includes
    #include "string_iterators.h"

//Class
    namespace OS::String
    {
        template <typename codeUnitType> class String_Base_View
        {
        //Constructor/Destructor
        //
        //Notice:
        //Do not include std::initializer_list in view as std::initializer_list gets destructed right after the creation.
        //i.e. you always need to make a copy of initializer_list data.
            public: constexpr String_Base_View() : m_Text( nullptr ), m_Size_codeUnits( 0 ) {};
            public: constexpr String_Base_View( const codeUnitType* text, const unsigned long size_codeUnits ) : m_Text( const_cast<codeUnitType*>( text ) ), m_Size_codeUnits( size_codeUnits ) {};
            public: constexpr String_Base_View( const String_Base_View& from ) : m_Text( from.m_Text ), m_Size_codeUnits( from.m_Size_codeUnits )
            {
            //Copy constructor.
            };

            public: constexpr String_Base_View& operator=( const String_Base_View& from )
            {
            //Copy assignment operator.
                m_Text = const_cast<codeUnitType*>( from.m_Text );
                m_Size_codeUnits = from.m_Size_codeUnits;

                return *this;
            };

            public: constexpr bool operator!=( const String_Base_View& compare_to )
            {
            //Difference operator.
                return !IsEqual( compare_to );
            };
            public: constexpr bool operator==( const String_Base_View& compare_to )
            {
            //Equal operator.
                return IsEqual( compare_to );
            };

            public: typedef codeUnitType    codeUnit;

            protected: unsigned long        m_Size_codeUnits;       //Must behave as it was a const.
            protected: codeUnitType*        m_Text;                 //Must behave as it was a const.

        //At
            public: constexpr const codeUnitType& At( const unsigned long index ) const
            {
            //Validate.
            #ifdef DEBUG
                if (
                  ( m_Text == nullptr )
                  ||
                  ( index >= m_Length )
                )
                {
                //Empty or index out of range.
                    throw;
                }
            #endif
            
            //Get item.
                return m_Text[index];
            };
            public: constexpr const codeUnitType& At_First() const
            {
            //Get item.
                return At( 0 );
            };
            public: constexpr const codeUnitType& At_Last() const
            {
            //Get item.
                return At( m_Size_codeUnits-1 );
            };

        //Clear
            protected: constexpr void Clear()
            {
            //Remove view.
                m_Text = nullptr;
                m_Size_codeUnits = 0;
            };

        //Compare
            public: constexpr bool IsEqual( const codeUnitType* text, const unsigned long size_codeUnits ) const
            {
            //Do they have the same size?
                if ( m_Size_codeUnits != size_codeUnits )
                {
                //No, they are not the same -> stop execution.
                    return false;
                }
        
            //Compare their code units.
            //
            //Notice:
            //Empty or uninitialized streams are the same too.
                const codeUnitType*        text_this = m_Text;
        
                for ( unsigned long i = 0, i_max = size_codeUnits; i < i_max; ++i )
                {
                //Compare code unit.
                    if ( text_this[i] != text[i] )
                    {
                    //Not the same.
                        return false;
                    }
                }

            //If we came to here the texts are the same.
                return true;
            };
            public: constexpr bool IsEqual( const String_Base_View& compare_to ) const
            {
            //Forward request.
                return IsEqual( compare_to.Text_Get(), compare_to.Size_CodeUnits() );
            };

        //Convert
            //[W: the most naive version of constexpr atoi.. we should do better.. faster]
            protected: constexpr bool Convert_ToUint64_Internal( uint64& value_unsigned, bool& value_isNegative ) const
            {
            //Validate.
                if ( m_Size_codeUnits == 0 )
                {
                    return false;
                }

            //Initialize values.
                value_isNegative = false;

            //Convert value.
                const auto*		        text = m_Text;
                unsigned long long      value_internal = 0;

                switch ( *text )
                {
                case '-':
                {
                //Define that we have a negative value.
                    value_isNegative = true;
            
                //Continue to next case.
                    [[fallthrough]];
                }
                case '+':
                {
                //Move index one character forward.
                    ++text;
            
                //Continue to next case.
                    [[fallthrough]];
                }
                default:
                {
                    const auto*		text_end = m_Text + m_Size_codeUnits;

                    while ( text < text_end )
                    {
                    //Get character.
                        auto		character = *text++;

                    //Are we beyond the number?
                        if (
                          ( character < '0' )
                          ||
                          ( character > '9' )
                        )
                        {
                        //Yes -> exit loop.
                            break;
                        }

                    //Convert to decimal number.
                        if constexpr ( std::is_same<char, codeUnitType>::value == true )
                        {
                        //Character
                            character -= '0';       //Local
                        }
                        else if constexpr ( std::is_same<char16_t, codeUnitType>::value == true )
                        {
                        //Wide character
                            character -= u'0';      //UTF16
                        }
                        else
                        {
                        //Extend if needed.
                            static_assert( true, "Unsupported codeUnitType" );
                        }

                    //"Append" it to value.
                        value_internal = 10 * value_internal + character;
                    }
                    break;
                }
                }
                
            //Return value.
                value_unsigned = value_internal;

                return true;
            };
            public: constexpr long long Convert_ToInt64() const
            {
            //Forward request.
                unsigned long long      value_internal;
                bool                    value_isNegative;

                auto    status = Convert_ToUint64_Internal( value_internal, value_isNegative );

                if ( status == false )
                {
                    return 0;
                }

            //Is value in limits?
                if ( value_isNegative == false )
                {
                //A positive number -> is it in limits?
                    if ( value_internal <= std::numeric_limits<long long>::max() )
                    {
                    //Value is in limits -> return it.
                        return (long long)value_internal;
                    }
                    else
                    {
                    //Value is out of limits -> return max that we can.
                        status = std::numeric_limits<long long>::max();
                    }
                }
                else
                {
                //A negative number -> is it in limits?
                    if ( value_internal <= (unsigned long long)( 1 + std::numeric_limits<long long>::max() ) )
                    {
                    //Value is in limits -> convert and return it.
                        value = -(long long)value_internal;
                    }
                    else
                    {
                    //Value is out of limits -> return min that we can.
                        status = std::numeric_limits<long long>::min();
                    }
                }
            };
            public: constexpr unsigned long long Convert_ToUInt64() const
            {
            //Forward request.
                unsigned long long      value_internal;
                bool                    value_isNegative;

                auto    status = Convert_ToUint64_Internal( value_internal, value_isNegative );

                if ( status == false )
                {
                    return 0;
                }

            //Return value.
                return value_internal;
            }

        //Size
            public: constexpr unsigned long Size_Bytes() const                      //Does not include NUL character.
            {
            //Return number of bytes.
                return ( m_Size_codeUnits * sizeof( codeUnitType ) );
            }
            //public: constexpr unsigned long Size_CodePoints() const               //Implement in derived classes.

            public: constexpr unsigned long Size_CodeUnits() const                  //Does not include NUL character.
            {
            //Return number of code units.
                return m_Size_codeUnits;
            };

        //STL and range for loop support.
        //
        //Notice:
        //Only const iterators are allowed.
            public: constexpr typename Iterators<codeUnitType>::Iterator_Forward_Const begin() const
            {
                return typename Iterators<codeUnitType>::Iterator_Forward_Const( m_Text );
            };
            public: constexpr typename Iterators<codeUnitType>::Iterator_Forward_Const end() const
            {
                return typename Iterators<codeUnitType>::Iterator_Forward_Const( m_Text + m_Size_codeUnits );
            };

            public: constexpr typename Iterators<codeUnitType>::Iterator_Reverse_Const rbegin() const
            {
                return typename Iterators<codeUnitType>::Iterator_Reverse_Const( m_Text + m_Size_codeUnits - 1 );
            };
            public: constexpr typename Iterators<codeUnitType>::Iterator_Reverse_Const rend() const
            {
                return typename Iterators<codeUnitType>::Iterator_Reverse_Const( m_Text - 1 );
            };

        //Text
            public: constexpr const codeUnitType* Text_Get() const
            {
                return m_Text;
            };
        };
    }
