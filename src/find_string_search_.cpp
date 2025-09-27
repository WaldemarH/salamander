// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_string_search.h"
#include "common\handles.h"


Find_String_Search::Find_String_Search()
{
//Initialize variables.
    HANDLES( InitializeCriticalSection( &m_CriticalSection ) );
}
Find_String_Search::~Find_String_Search()
{
//Free resources.
    HANDLES( DeleteCriticalSection( &m_CriticalSection ) );
}

