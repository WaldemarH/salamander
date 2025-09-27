// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "settings.h"


bool Settings::Initialize()
{
//Remove MAX_PATH limitation.
    //Enable long paths.
    //HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\FileSystem LongPathsEnabled (Type: REG_DWORD) must exist and be set to 1

    //Get the size of executable.
    //
    //Why?
    //Once a "MAX_PATH limited WIN32 call" is made the LongPathsEnabled flag is read and locked to specific process.
    //So even if some other process sets the LongPathsEnabled to 0, MAX_PATH limitation will not be influenced for current app.

}
