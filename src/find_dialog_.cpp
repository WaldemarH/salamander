// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_dialog.h"
#include "cfgdlg.h"
#include "execute.h"

//****************************************************************************
//
// Find_Dialog
//

Find_Dialog::Find_Dialog( HWND hCenterAgainst, const char* initPath ) : CCommonDialog( HLanguage, IDD_FIND, NULL, ooStandard, hCenterAgainst ), SearchForData( 50, 10 )
{
    // data potrebna pro layoutovani dialogu

    char buf[100];
    sprintf(buf, "%s ", LoadStr(IDS_FF_SEARCHING));
    SearchingText.String_Set_Base(buf);

    EditLine = new CComboboxEdit();

    FileNameFormat = Configuration.FileNameFormat;
    SkipCharacter = FALSE;

    // pokud ma nektery z options nastavany AutoLoad, nahraju ho
    int i;
    for (i = 0; i < FindOptions.Items_Count(); i++)
        if (FindOptions.Items_At(i)->AutoLoad)
        {
            Data = *FindOptions.Items_At(i);
            Data.AutoLoad = FALSE;
            break;
        }

    // data pro controly
    if (Data.NamedText[0] == 0)
        lstrcpy(Data.NamedText, "*.*");
    if (Data.LookInText[0] == 0)
    {
        const char* s = initPath;
        char* d = Data.LookInText;
        char* end = Data.LookInText + LOOKIN_TEXT_LEN - 1; // -1 je prostor na null na konci retezce
        while (*s != 0 && d < end)
        {
            if (*s == ';')
            {
                *d++ = ';';
                if (d >= end)
                    break;
            }
            *d++ = *s++;
        }
        *d++ = 0;
    }
}

Find_Dialog::~Find_Dialog()
{
    if (CacheBitmap != NULL)
        delete CacheBitmap;
}
