// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_options_item.h"

const TCHAR* FINDOPTIONSITEM_ITEMNAME_REG = TEXT( "ItemName" );
const TCHAR* FINDOPTIONSITEM_SUBDIRS_REG = TEXT( "SubDirectories" );
const TCHAR* FINDOPTIONSITEM_WHOLEWORDS_REG = TEXT( "WholeWords" );
const TCHAR* FINDOPTIONSITEM_CASESENSITIVE_REG = TEXT( "CaseSensitive" );
const TCHAR* FINDOPTIONSITEM_HEXMODE_REG = TEXT( "HexMode" );
const TCHAR* FINDOPTIONSITEM_REGULAR_REG = TEXT( "RegularExpresions" );
const TCHAR* FINDOPTIONSITEM_AUTOLOAD_REG = TEXT( "AutoLoad" );
const TCHAR* FINDOPTIONSITEM_NAMED_REG = TEXT( "Named" );
const TCHAR* FINDOPTIONSITEM_LOOKIN_REG = TEXT( "LookIn" );
const TCHAR* FINDOPTIONSITEM_GREP_REG = TEXT( "Grep" );

//We used the following variable until Altap Salamander 2.5,
//where we switched to CFilterCriteria and its Save/Load
const TCHAR* OLD_FINDOPTIONSITEM_EXCLUDEMASK_REG = TEXT( "ExcludeMask" );

void Find_Options_Item::Registry_Load( HKEY hKey, DWORD cfgVersion )
{
    GetValue( hKey, FINDOPTIONSITEM_ITEMNAME_REG, REG_SZ, ItemName, ITEMNAME_TEXT_LEN );
    GetValue( hKey, FINDOPTIONSITEM_SUBDIRS_REG, REG_DWORD, &SubDirectories, sizeof(DWORD) );
    GetValue( hKey, FINDOPTIONSITEM_WHOLEWORDS_REG, REG_DWORD, &WholeWords, sizeof(DWORD) );
    GetValue( hKey, FINDOPTIONSITEM_CASESENSITIVE_REG, REG_DWORD, &CaseSensitive, sizeof(DWORD) );
    GetValue( hKey, FINDOPTIONSITEM_HEXMODE_REG, REG_DWORD, &HexMode, sizeof(DWORD) );
    GetValue( hKey, FINDOPTIONSITEM_REGULAR_REG, REG_DWORD, &RegularExpresions, sizeof(DWORD) );
    GetValue( hKey, FINDOPTIONSITEM_AUTOLOAD_REG, REG_DWORD, &AutoLoad, sizeof(DWORD) );
    GetValue( hKey, FINDOPTIONSITEM_NAMED_REG, REG_SZ, NamedText, NAMED_TEXT_LEN );
    GetValue( hKey, FINDOPTIONSITEM_LOOKIN_REG, REG_SZ, LookInText, LOOKIN_TEXT_LEN );
    GetValue( hKey, FINDOPTIONSITEM_GREP_REG, REG_SZ, GrepText, GREP_TEXT_LEN );

    if (cfgVersion <= 13)
    {
        // konverze starych hodnot

        // exclude mask
        BOOL excludeMask = FALSE;
        GetValue(hKey, OLD_FINDOPTIONSITEM_EXCLUDEMASK_REG, REG_DWORD, &excludeMask, sizeof(DWORD));
        if (excludeMask)
        {
            memmove(NamedText + 1, NamedText, NAMED_TEXT_LEN - 1);
            NamedText[0] = '|';
        }

        Criteria.LoadOld(hKey);
    }
    else
        Criteria.Load(hKey);
}

void Find_Options_Item::Registry_Save( HKEY hKey )
{
// optimization for size in Registry: we only store "non-default values";
// therefore, before storing, we need to delete the key we will store in
    Find_Options_Item def;

    if (strcmp(ItemName, def.ItemName) != 0)
        SetValue(hKey, FINDOPTIONSITEM_ITEMNAME_REG, REG_SZ, ItemName, -1);
    if (SubDirectories != def.SubDirectories)
        SetValue(hKey, FINDOPTIONSITEM_SUBDIRS_REG, REG_DWORD, &SubDirectories, sizeof(DWORD));
    if (WholeWords != def.WholeWords)
        SetValue(hKey, FINDOPTIONSITEM_WHOLEWORDS_REG, REG_DWORD, &WholeWords, sizeof(DWORD));
    if (CaseSensitive != def.CaseSensitive)
        SetValue(hKey, FINDOPTIONSITEM_CASESENSITIVE_REG, REG_DWORD, &CaseSensitive, sizeof(DWORD));
    if (HexMode != def.HexMode)
        SetValue(hKey, FINDOPTIONSITEM_HEXMODE_REG, REG_DWORD, &HexMode, sizeof(DWORD));
    if (RegularExpresions != def.RegularExpresions)
        SetValue(hKey, FINDOPTIONSITEM_REGULAR_REG, REG_DWORD, &RegularExpresions, sizeof(DWORD));
    if (AutoLoad != def.AutoLoad)
        SetValue(hKey, FINDOPTIONSITEM_AUTOLOAD_REG, REG_DWORD, &AutoLoad, sizeof(DWORD));
    if (strcmp(NamedText, def.NamedText) != 0)
        SetValue(hKey, FINDOPTIONSITEM_NAMED_REG, REG_SZ, NamedText, -1);
    if (strcmp(LookInText, def.LookInText) != 0)
        SetValue(hKey, FINDOPTIONSITEM_LOOKIN_REG, REG_SZ, LookInText, -1);
    if (strcmp(GrepText, def.GrepText) != 0)
        SetValue(hKey, FINDOPTIONSITEM_GREP_REG, REG_SZ, GrepText, -1);

    // advanced options
    Criteria.Save(hKey);
}
