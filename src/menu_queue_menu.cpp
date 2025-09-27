// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later
#include "precomp.h"
#include "menu_queue.h"

//Class
bool Menu_Queue::Menu_Add( HWND hWindow )
{
    CALL_STACK_MESSAGE1( "Menu_Queue::Menu_Add()" );

//Add menu to the list.
    HANDLES( EnterCriticalSection( &m_Menus_CriticalSection ) );

    //Add to list.
        auto&   menus = m_Menus;

        menus.Add( hWindow );

    //Is all ok with the list?
        const auto  status = m_Menus.IsGood();

        if ( !status )
        {
        //No, an error occured -> reset state.
            m_Menus.ResetState();
        }

    HANDLES( LeaveCriticalSection( &m_Menus_CriticalSection ) );

    return status;
}
void Menu_Queue::Menu_Remove( HWND hWindow )
{
    CALL_STACK_MESSAGE1( "Menu_Queue::Menu_Remove()" );

//Remove menu from list.
    HANDLES( EnterCriticalSection( &m_Menus_CriticalSection ) );

    //Remove item.
        auto&   menus = m_Menus;

        for ( int i = menus.Count - 1; i >= 0; i-- )
        {
        //Get menu.
            const auto  menu = menus[i];

        //Is it the one we are searching for?
            if ( menu != hWindow )
            {
            //No -> skip it.
                continue;
            }

        //Found menu -> remove it.
            menus.Detach(i);
            break;
        }

    HANDLES( LeaveCriticalSection( &m_Menus_CriticalSection ) );
}

void Menu_Queue::Menus_CloseAll_Blocking()
{
    CALL_STACK_MESSAGE1( "Menu_Queue::Menus_CloseAll_Blocking()" );

//Send close message to all opened menus.
    HANDLES( EnterCriticalSection( &m_Menus_CriticalSection ) );

    //Execute close messages on menus.
        auto&   menus = m_Menus;

        for ( int i = menus.Count - 1; i >= 0; i--)
        {
            CWindow::CWindowProc( menus[i], WM_USER_CLOSEMENU, 0, 0 );
        }

    //Free resources.
        menus.DetachMembers();

    HANDLES( LeaveCriticalSection( &m_Menus_CriticalSection ) );
}
void Menu_Queue::Menus_CloseAll_NonBlocking()
{
    CALL_STACK_MESSAGE1( "Menu_Queue::Menus_CloseAll_NonBlocking()" );

//Send close message to all opened menus.
    HANDLES( EnterCriticalSection( &m_Menus_CriticalSection ) );

    //Execute close messages on menus.
        auto&   menus = m_Menus;

        for ( int i = menus.Count - 1; i >= 0; i--)
        {
            PostMessage( menus[i], WM_USER_CLOSEMENU, 0, 0 );
        }

    //Free resources.
        menus.DetachMembers();

    HANDLES( LeaveCriticalSection( &m_Menus_CriticalSection ) );
}

void Menu_Queue::Menus_PostToAll( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CALL_STACK_MESSAGE1( "Menu_Queue::Menus_CloseAll_NonBlocking()" );

//Send close message to all opened menus.
    HANDLES( EnterCriticalSection( &m_Menus_CriticalSection ) );

    //Post close messages on menus.
        auto&   menus = m_Menus;

        for ( int i = menus.Count - 1; i >= 0; i--)
        {
            PostMessage( menus[i], uMsg, wParam, lParam );
        }

    //Free resources.
        menus.DetachMembers();

    HANDLES( LeaveCriticalSection( &m_Menus_CriticalSection ) );
}
void Menu_Queue::Menus_SendToAll( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CALL_STACK_MESSAGE1( "Menu_Queue::Menus_CloseAll_NonBlocking()" );

//Send close message to all opened menus.
    HANDLES( EnterCriticalSection( &m_Menus_CriticalSection ) );

    //Post close messages on menus.
        auto&   menus = m_Menus;

        for ( int i = menus.Count - 1; i >= 0; i--)
        {
            SendMessage( menus[i], uMsg, wParam, lParam );
        }

    //Free resources.
        menus.DetachMembers();

    HANDLES( LeaveCriticalSection( &m_Menus_CriticalSection ) );
}
