// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//Includes

//Class
    namespace OS::String
    {
        template <typename characterType> class Iterators
        {
            public: class Iterator_Base
            {
            //Constructor/Destructor
                public: constexpr Iterator_Base( characterType* location ) : m_Location( location )
                {
                };

                protected: characterType*       m_Location;

            //Operators
                public: constexpr characterType& operator*()
                {
                //Value operator -> return reference.
                    return *m_Location;
                };
                public: constexpr bool operator!=( const Iterator_Base& iterator ) const
                {
                //Not equal operator -> compare if not at same pointer.
                    return ( m_Location != iterator.m_Location );
                };
            };
            public: class Iterator_Base_Const
            {
            //Constructor/Destructor
                public: constexpr Iterator_Base_Const( characterType* location ) : m_Location( location )
                {
                };

                protected: characterType*		m_Location;

            //Operators
                public: constexpr const characterType& operator*()
                {
                //Value operator -> return reference.
                    return *m_Location;
                };
                public: constexpr bool operator!=( const Iterator_Base_Const& iterator ) const
                {
                //Not equal operator -> compare if not at same pointer.
                    return ( m_Location != iterator.m_Location );
                };
            };

            public: class Iterator_Forward : public Iterator_Base
            {
            //Constructor/Destructor
                public: constexpr Iterator_Forward( characterType* location ) : Iterator_Base( location )
                {
                };

                protected: using Iterator_Base::m_Location;		//Define using for all variables used from base class.. gcc is very sensitive.

            //Operators
                public: constexpr Iterator_Forward& operator++()
                {
                //Increment operator -> go to next value (increase pointer).
                    ++m_Location;

                //Return reference.
                    return *this;
                };
            };
            public: class Iterator_Forward_Const : public Iterator_Base_Const
            {
            //Constructor/Destructor
                public: constexpr Iterator_Forward_Const( characterType* location ) : Iterator_Base_Const( location )
                {
                };

                protected: using Iterator_Base_Const::m_Location;

            //Operators
                public: constexpr Iterator_Forward_Const& operator++()
                {
                //Increment operator -> go to next value (increase pointer).
                    ++m_Location;

                //Return reference.
                    return *this;
                };
            };
            public: class Iterator_Reverse : public Iterator_Base
            {
            //Constructor/Destructor
                public: constexpr Iterator_Reverse( characterType* location ) : Iterator_Base( location )
                {
                };

                protected: using Iterator_Base::m_Location;

            //Operators
                public: constexpr Iterator_Reverse& operator++()
                {
                //Increment operator -> go to previous value (decrease pointer).
                    --m_Location;

                //Return reference.
                    return *this;
                };
            };
            public: class Iterator_Reverse_Const : public Iterator_Base_Const
            {
            //Constructor/Destructor
                public: constexpr Iterator_Reverse_Const( characterType* location ) : Iterator_Base_Const( location )
                {
                };

                protected: using Iterator_Base_Const::m_Location;

            //Operators
                public: constexpr Iterator_Reverse_Const& operator++()
                {
                //Increment operator -> go to previous value (decrease pointer).
                    --m_Location;

                //Return reference.
                    return *this;
                };
            };
        };
    }
