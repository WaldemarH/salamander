// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"

#include "menu.h"
#include "cfgdlg.h"
#include "mainwnd.h"
#include "plugins.h"
#include "fileswnd.h"
#include "viewer.h"
#include "shellib.h"
#include "find.old.h"
#include "gui.h"
#include "usermenu.h"
#include "execute.h"
#include "tasklist.h"
#include "find_data_umd.h"
#include "find_filesList.h"


#include <Shlwapi.h>

BOOL FindManageInUse = FALSE;
BOOL FindIgnoreInUse = FALSE;

//****************************************************************************
//
// CFindTBHeader
//

// popis viz mainwnd.h
BOOL GetNextItemFromFind(int index, char* path, char* name, void* param)
{
    CALL_STACK_MESSAGE2("GetNextItemFromFind(%d, , ,)", index);
    CUMDataFromFind* data = (CUMDataFromFind*)param;

    Find_FilesList_View* listView = (Find_FilesList_View*)WindowsManager.GetWindowPtr(data->hWindow);
    if (listView == NULL)
    {
        TRACE_E("Unable to find object for ListView");
        return FALSE;
    }

    LV_ITEM item;
    item.mask = LVIF_PARAM;
    item.iSubItem = 0;
    if (data->count == -1)
    {
        data->count = ListView_GetSelectedCount(data->hWindow);
        if (data->count == 0)
            return FALSE;
        data->index = new int[data->count];
        if (data->index == NULL)
            return FALSE; // chyba
        int i = 0;
        int findItem = -1;
        while (i < data->count)
        {
            findItem = ListView_GetNextItem(data->hWindow, findItem, LVNI_SELECTED);
            data->index[i++] = findItem;
        }
    }

    if (index >= 0 && index < data->count)
    {
        Find_FileList_Item* file = listView->Items_At(data->index[index]);
        strcpy(path, file->Path);
        strcpy(name, file->Name);
        return TRUE;
    }
    if (data->index != NULL)
    {
        delete[] (data->index);
        data->index = NULL;
    }
    return FALSE;
}

//****************************************************************************
//
// CFindDialogQueue
//

void CFindDialogQueue::AddToArray(TDirectArray<HWND>& arr)
{
    CS.Enter();
    CWindowQueueItem* item = Head;
    while (item != NULL)
    {
        arr.Add(item->HWindow);
        item = item->Next;
    }
    CS.Leave();
}

