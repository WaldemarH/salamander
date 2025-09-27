// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_log.h"


Find_Log::Find_Log() : m_Items( 1, 50 )
{
//Initialize variables.
}
Find_Log::~Find_Log()
{
//Free resources.
    Items_Clean();
}
