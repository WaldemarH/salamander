// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_dialog.h"
#include "find_filesList.h"
#include "fileswnd.h"


void Find_Dialog::Clipboard_CopyTo( const Clipboard_CopyMode::Value mode )
{
    CALL_STACK_MESSAGE1("Find_Dialog::FocusButton()");
    
    DWORD selectedCount = ListView_GetSelectedCount( FoundFilesListView->HWindow );

    if (selectedCount != 1)
        return;

    int index = ListView_GetNextItem( FoundFilesListView->HWindow, -1, LVNI_SELECTED );
    if (index < 0)
        return;

    Find_FileList_Item* data = FoundFilesListView->Items_At(index);

    char buff[2 * MAX_PATH];
    buff[0] = 0;

    switch (mode)
    {
    case Clipboard_CopyMode::name:
    {
        AlterFileName(buff, data->Name, -1, FileNameFormat, 0, data->IsDir);
        break;
    }
    case Clipboard_CopyMode::name_full:
    {
        strcpy(buff, data->Path);
        int len = (int)strlen(buff);
        if (len > 0 && buff[len - 1] != '\\')
            strcat(buff, "\\");
        AlterFileName(buff + strlen(buff), data->Name, -1, FileNameFormat, 0, data->IsDir);
        break;
    }
    case Clipboard_CopyMode::name_unc:
    {
        AlterFileName(buff, data->Name, -1, FileNameFormat, 0, data->IsDir);
        CopyUNCPathToClipboard(data->Path, buff, data->IsDir, HWindow);
        break;
    }
    case Clipboard_CopyMode::path_full:
    {
        strcpy(buff, data->Path);
        break;
    }
    }
    if ( mode != Clipboard_CopyMode::name_unc )
    {
        CopyTextToClipboard( buff );
    }
}
