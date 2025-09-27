// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_options_item.h"


Find_Options_Item& Find_Options_Item::operator=( const Find_Options_Item& s )
{
    // Internal
    lstrcpy( ItemName, s.ItemName );

    memmove(&Criteria, &s.Criteria, sizeof(Criteria));

    // Find dialog
    SubDirectories = s.SubDirectories;
    WholeWords = s.WholeWords;
    CaseSensitive = s.CaseSensitive;
    HexMode = s.HexMode;
    RegularExpresions = s.RegularExpresions;

    AutoLoad = s.AutoLoad;

    lstrcpy(NamedText, s.NamedText);
    lstrcpy(LookInText, s.LookInText);
    lstrcpy(GrepText, s.GrepText);

    return *this;
}
