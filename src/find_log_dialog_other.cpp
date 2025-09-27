// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_log_dialog.h"
#include "find.old.h"
#include "fileswnd.h"
#include "mainwnd.h"
#include "dialogs.h"


//TODO
void Find_Log_Dialog::Transfer(CTransferInfo& ti)
{
    if (ti.Type == ttDataToWindow)
    {
        HListView = GetDlgItem(HWindow, IDC_FINDLOG_LIST);

        int iconSize = IconSizes[IconSize::size_16x16];
        HIMAGELIST hIcons = ImageList_Create(iconSize, iconSize, ILC_MASK | GetImageListColorFlags(), 2, 0); // o destrukci se postara listview

        HICON hWarning;
        LoadIconWithScaleDown(NULL, (PCWSTR)IDI_EXCLAMATION, iconSize, iconSize, &hWarning);
        HICON hInfo;
        LoadIconWithScaleDown(NULL, (PCWSTR)IDI_INFORMATION, iconSize, iconSize, &hInfo);
        ImageList_SetImageCount(hIcons, 2);
        ImageList_ReplaceIcon(hIcons, 0, hWarning);
        ImageList_ReplaceIcon(hIcons, 1, hInfo);
        DestroyIcon(hWarning);
        DestroyIcon(hInfo);
        HIMAGELIST hOldIcons = ListView_SetImageList(HListView, hIcons, LVSIL_SMALL);
        if (hOldIcons != NULL)
            ImageList_Destroy(hOldIcons);

        DWORD exFlags = LVS_EX_FULLROWSELECT;
        DWORD origFlags = ListView_GetExtendedListViewStyle(HListView);
        ListView_SetExtendedListViewStyle(HListView, origFlags | exFlags); // 4.71

        // inicializace sloupcu
        int header[] = {IDS_FINDLOG_TYPE, IDS_FINDLOG_TEXT, IDS_FINDLOG_PATH, -1};

        LV_COLUMN lvc;
        lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
        lvc.fmt = LVCFMT_LEFT;
        int i;
        for (i = 0; header[i] != -1; i++)
        {
            lvc.pszText = LoadStr(header[i]);
            lvc.iSubItem = i;
            ListView_InsertColumn(HListView, i, &lvc);
        }

        // pridani polozek
        LVITEM lvi;
        lvi.mask = LVIF_IMAGE;
        lvi.iSubItem = 0;
        char buff[4000];
        const Find_Log_Item* item;
        for (i = 0; i < Log->Items_Count(); i++)
        {
            item = Log->Items_Get(i);

            lvi.iItem = i;
            lvi.iSubItem = 0;
            lvi.iImage = ( ( item->flags & Find_Log_Item::Flags::type_error ) != 0 ) ? 0 : 1;
            ListView_InsertItem(HListView, &lvi);
            ListView_SetItemText(HListView, i, 0, LoadStr( ( ( item->flags & Find_Log_Item::Flags::type_error ) != 0 ) ? IDS_FINDLOG_ERROR : IDS_FINDLOG_INFO ) );

            // odstranim z textu znaky '\r' a '\n'
            lstrcpyn(buff, item->text, 4000);
            int j;
            for (j = 0; buff[j] != 0; j++)
                if (buff[j] == '\r' || buff[j] == '\n')
                    buff[j] = ' ';

            ListView_SetItemText(HListView, i, 1, buff);
            ListView_SetItemText(HListView, i, 2, item->path);
        }

        if (Log->Items_Count_Skipped() > 0)
        {
            wsprintf(buff, LoadStr(IDS_FINDERRORS_SKIPPING), Log->Items_Count_Skipped());

            lvi.iItem = i;
            lvi.iSubItem = 0;
            lvi.iImage = 1; // question
            ListView_InsertItem(HListView, &lvi);
            ListView_SetItemText(HListView, i, 0, LoadStr(IDS_FINDLOG_INFO));
            ListView_SetItemText(HListView, i, 1, buff);
        }

        // nulta polozka bude vybrana
        DWORD state = LVIS_SELECTED | LVIS_FOCUSED;
        ListView_SetItemState(HListView, 0, state, state);

        // nastavim sirky sloupcu
        ListView_SetColumnWidth(HListView, 0, LVSCW_AUTOSIZE_USEHEADER);
        ListView_SetColumnWidth(HListView, 1, LVSCW_AUTOSIZE_USEHEADER);
        ListView_SetColumnWidth(HListView, 2, LVSCW_AUTOSIZE_USEHEADER);
    }
}

void Find_Log_Dialog::OnFocusFile()
{
    CALL_STACK_MESSAGE1("Find_Log_Dialog::OnFocusFile()");
    const Find_Log_Item* item = GetSelectedItem();
    if (item == NULL)
        return;

    if (SalamanderBusy)
    {
        Sleep(200); // dame Salamu cas - pokud slo o prepnuti z hlavniho okna, mohla
                    // by jeste dobihat message queue od menu
        if (SalamanderBusy)
        {
            SalMessageBox(HWindow, LoadStr(IDS_SALAMANDBUSY2),
                          LoadStr(IDS_INFOTITLE), MB_OK | MB_ICONINFORMATION);
            return;
        }
    }
    static char FocusPath[2 * MAX_PATH];
    lstrcpyn(FocusPath, item->path, _countof(FocusPath));
    char buffEmpty[] = "";
    char* p = buffEmpty;
    if (FocusPath[0] != 0)
    {
        if (FocusPath[strlen(FocusPath) - 1] == '\\')
            FocusPath[strlen(FocusPath) - 1] = 0;
        p = strrchr(FocusPath, '\\');
        if (p == NULL)
        {
            TRACE_E("p == NULL");
            return;
        }
        *p = 0;
    }

    SendMessage(MainWindow->GetActivePanel()->HWindow, WM_USER_FOCUSFILE, (WPARAM)p + 1, (LPARAM)FocusPath);
}

void Find_Log_Dialog::OnIgnore()
{
    CALL_STACK_MESSAGE1("Find_Log_Dialog::OnIgnore()");
    const Find_Log_Item* item = GetSelectedItem();
    if (item == NULL)
        return;

    CTruncatedString str;
    str.Set(LoadStr(IDS_FINDLOG_IGNORE), item->path);
    CMessageBox msgBox(HWindow, MSGBOXEX_OKCANCEL | MSGBOXEX_ICONQUESTION, LoadStr(IDS_QUESTION), &str, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL);
    if (msgBox.Execute() == IDOK)
    {
        FindIgnore.AddUnique(TRUE, item->path);
    }
}

const Find_Log_Item* Find_Log_Dialog::GetSelectedItem()
{
    CALL_STACK_MESSAGE1("Find_Log_Dialog::OnIgnore()");
    int index = ListView_GetNextItem(HListView, -1, LVNI_SELECTED);
    if (index < 0 || index >= Log->Items_Count())
        return NULL;
    return Log->Items_Get(index);
}

void Find_Log_Dialog::EnableControls()
{
    const Find_Log_Item* item = GetSelectedItem();
    BOOL path = item != NULL && item->path != NULL && item->path[0] != 0;
    TRACE_I("path=" << path);
    EnableWindow(GetDlgItem(HWindow, IDC_FINDLOG_FOCUS), path);
    EnableWindow(GetDlgItem(HWindow, IDC_FINDLOG_IGNORE), path && ( ( item->flags & Find_Log_Item::Flags::button_ignore ) != 0 ) );
}

