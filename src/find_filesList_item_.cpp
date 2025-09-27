// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_filesList_item.h"


Find_FileList_Item::Find_FileList_Item()
{
//Initialize variables.
    ZeroMemory( &LastWrite, sizeof( LastWrite ) );
}
Find_FileList_Item::~Find_FileList_Item()
{
//Free resources.
    if ( Name != NULL )
    {
        free( Name );
    }
    if ( Path != NULL )
    {
        free( Path );
    }
}
