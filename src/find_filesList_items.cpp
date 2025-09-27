// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_filesList.h"


Find_FileList_Item* Find_FilesList_View::Items_At(int index)
{
//Enter critical section.
    HANDLES( EnterCriticalSection( &m_CriticalSection ) );

    //Get item.
        auto*   item = m_Items[index];

//Leave critical section.
    HANDLES( LeaveCriticalSection( &m_CriticalSection ) );

    return item;
}

int Find_FilesList_View::Items_Count()
{
//Enter critical section.
    HANDLES( EnterCriticalSection( &m_CriticalSection ) );

    //Get count.
        auto    count = m_Items.Count;

//Leave critical section.
    HANDLES( LeaveCriticalSection( &m_CriticalSection ) );

    return count;
}

void Find_FilesList_View::Items_Delete(int index)
{
//Enter critical section.
    HANDLES( EnterCriticalSection( &m_CriticalSection ) );

    //Delete item.
        m_Items.Delete( index );

//Leave critical section.
    HANDLES( LeaveCriticalSection( &m_CriticalSection ) );
}

void Find_FilesList_View::Items_DestroyMembers()
{
//TODO: why is critical section commmented out???

////Enter critical section.
//    HANDLES( EnterCriticalSection( &m_CriticalSection ) );

    //Destroy members.
        m_Items.DestroyMembers();

////Leave critical section.
//    HANDLES( LeaveCriticalSection( &m_CriticalSection ) );
}

int Find_FilesList_View::Items_Add( Find_FileList_Item* item )
{
//Enter critical section.
    HANDLES( EnterCriticalSection( &m_CriticalSection ) );

    //Add item.
        auto    index = m_Items.Add( item );

//Leave critical section.
    HANDLES( LeaveCriticalSection( &m_CriticalSection ) );

    return index;
}
