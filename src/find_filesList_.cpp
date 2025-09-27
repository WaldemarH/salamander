// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_filesList.h"


Find_FilesList_View::Find_FilesList_View( HWND dlg, int ctrlID, Find_Dialog& findDialog ) : m_FindDialog( findDialog ), m_Items( 1000, 500 ), m_Items_ForRefine( 1, 1000 ), CWindow( dlg, ctrlID )
{
//Initialize variables.
    HANDLES( InitializeCriticalSection( &m_CriticalSection ) );

//[TODO: ??? what does this mean -> define proper comment]
// adding this panel to the source field for file enumeration in viewers
    EnumFileNamesAddSourceUID( HWindow, &EnumFileNamesSourceUID );
}

Find_FilesList_View::~Find_FilesList_View()
{
//[TODO: ??? what does this mean -> define proper comment]
// removing this panel from the source field for file enumeration in viewers
    EnumFileNamesRemoveSourceUID( HWindow );

//Free resources.
    HANDLES( DeleteCriticalSection( &m_CriticalSection ) );
}
