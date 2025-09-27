// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later
#include "precomp.h"
#include "menu_queue.h"

//Globals
Menu_Queue  MenuQueue;

//Class
Menu_Queue::Menu_Queue() : m_Menus( 5, 5 )
{
    CALL_STACK_MESSAGE_NONE
    HANDLES( InitializeCriticalSection( &m_Menus_CriticalSection ) );
}
Menu_Queue::~Menu_Queue()
{
    CALL_STACK_MESSAGE_NONE
    HANDLES( DeleteCriticalSection( &m_Menus_CriticalSection ) );
}
