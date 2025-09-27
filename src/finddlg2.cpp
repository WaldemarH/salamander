// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"

#include "find_dialog.h"
#include "find_filesList.h"
#include "find_log_dialog.h"
#include "cfgdlg.h"
#include "edtlbwnd.h"
#include "mainwnd.h"
#include "find.old.h"
#include "toolbar.h"
#include "menu.h"
#include "shellib.h"
#include "gui.h"
#include "plugins.h"
#include "fileswnd.h"
#include "dialogs.h"

//****************************************************************************
//
// Find_Dialog (continued finddlg1.cpp)
//

void Find_Dialog::OnHideSelection()
{
    HWND hListView = FoundFilesListView->HWindow;
    DWORD totalCount = ListView_GetItemCount(hListView);
    DWORD selCount = ListView_GetSelectedCount(hListView);
    if (selCount == 0)
        return;

    HCURSOR hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    DWORD newSelectIndex = 0;
    int i;
    for (i = 0; i < (int)totalCount; i++)
    {
        if (ListView_GetItemState(hListView, i, LVIS_SELECTED) & LVIS_SELECTED)
            break;
        else
            newSelectIndex = i + 1;
    }

    int deletedCount = 0;
    for (i = totalCount - 1; i >= 0; i--)
    {
        if (ListView_GetItemState(hListView, i, LVIS_SELECTED) & LVIS_SELECTED)
        {
            FoundFilesListView->Items_Delete(i);
            deletedCount++;
        }
    }
    if (deletedCount > 0)
    {
        // hack hack: https://forum.altap.cz/viewtopic.php?f=2&t=3112&p=14801#p14801
        // bez resetu na nulu si listview pamatuje predchozi stav vyberu
        ListView_SetItemCount(FoundFilesListView->HWindow, 0);
        // reknu listview novy pocet polozek
        ListView_SetItemCount(FoundFilesListView->HWindow, totalCount - deletedCount);
    }

    if (totalCount - deletedCount > 0)
    {
        if (newSelectIndex >= totalCount - deletedCount)
            newSelectIndex = totalCount - deletedCount - 1;

        ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED | LVIS_FOCUSED); // -1: all items
        ListView_SetItemState(hListView, newSelectIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        ListView_EnsureVisible(hListView, newSelectIndex, FALSE);
    }
    else
        UpdateStatusBar = TRUE;
    UpdateListViewItems();
    SetCursor(hOldCursor);
}

void Find_Dialog::OnHideDuplicateNames()
{
    HWND hListView = FoundFilesListView->HWindow;
    DWORD totalCount = ListView_GetItemCount(hListView);
    if (totalCount == 0)
        return;

    HCURSOR hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    // seradime podle cesty
    FoundFilesListView->SortItems(1);

    // pokud polozka bude mit stejnou cestu a jmeno jako predesla polozka, vyradime ji
    int deletedCount = 0;
    Find_FileList_Item* lastData = NULL;
    int i;
    for (i = totalCount - 1; i >= 0; i--)
    {
        if (lastData == NULL)
            lastData = FoundFilesListView->Items_At(i);
        else
        {
            Find_FileList_Item* data = FoundFilesListView->Items_At(i);
            if (lastData->IsDir == data->IsDir &&
                RegSetStrICmp(lastData->Path, data->Path) == 0 &&
                RegSetStrICmp(lastData->Name, data->Name) == 0)
            {
                FoundFilesListView->Items_Delete(i);
                deletedCount++;
            }
            else
                lastData = data;
        }
    }
    if (deletedCount > 0)
    {
        // reknu listview novy pocet polozek
        ListView_SetItemCount(FoundFilesListView->HWindow, totalCount - deletedCount);
    }

    if (totalCount - deletedCount > 0)
    {
        // hloupe nastavim selected na nultou polozku... zde by to casem stalo za vylepseni
        ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED);
        ListView_SetItemState(hListView, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        ListView_EnsureVisible(hListView, 0, FALSE);
    }
    else
        UpdateStatusBar = TRUE;
    UpdateListViewItems();
    SetCursor(hOldCursor);
}

void Find_Dialog::OnDelete(BOOL toRecycle)
{
    CALL_STACK_MESSAGE1("Find_Dialog::OnDelete()");
    HWND hListView = FoundFilesListView->HWindow;
    DWORD selCount = ListView_GetSelectedCount(hListView);
    if (selCount == 0)
        return;

    // necham napocitat vyslednou velikost listu
    DWORD listSize = FoundFilesListView->GetSelectedListSize();
    if (listSize <= 2)
        return;

    char* list = (char*)malloc(listSize);
    if (list == NULL)
    {
        TRACE_E(LOW_MEMORY);
        return;
    }

    // necham naplnit list
    if (!FoundFilesListView->GetSelectedList(list, listSize))
    {
        free(list);
        return;
    }

    // ulozime focused polozku a index
    Find_FileList_Item lastFocusedItem;
    int lastFocusedIndex = ListView_GetNextItem(FoundFilesListView->HWindow, 0, LVIS_FOCUSED);
    if (lastFocusedIndex != -1)
    {
        Find_FileList_Item* lastItem = FoundFilesListView->Items_At(lastFocusedIndex);
        lastFocusedItem.Text_Set(lastItem->Path, lastItem->Name, lastItem->Size, lastItem->Attr, &lastItem->LastWrite, lastItem->IsDir);
    }

    CShellExecuteWnd shellExecuteWnd;
    SHFILEOPSTRUCT fo;
    fo.hwnd = shellExecuteWnd.Create(HWindow, "SEW: Find_Dialog::OnDelete toRecycle=%d", toRecycle);
    fo.wFunc = FO_DELETE;
    fo.pFrom = list;
    fo.pTo = NULL;
    fo.fFlags = toRecycle ? FOF_ALLOWUNDO : 0;
    fo.fAnyOperationsAborted = FALSE;
    fo.hNameMappings = NULL;
    fo.lpszProgressTitle = "";
    // provedeme samotne mazani - uzasne snadne, bohuzel jim sem tam pada ;-)
    CALL_STACK_MESSAGE1("Find_Dialog::OnDelete::SHFileOperation");
    SHFileOperation(&fo);
    free(list);

    // necham aktualizovat seznam
    FoundFilesListView->CheckAndRemoveSelectedItems(FALSE, lastFocusedIndex, &lastFocusedItem);
}

void Find_Dialog::OnSelectAll()
{
    HCURSOR hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    ListView_SetItemState(FoundFilesListView->HWindow, -1, LVIS_SELECTED, LVIS_SELECTED);
    SetCursor(hOldCursor);
}

void Find_Dialog::OnInvertSelection()
{
    HCURSOR hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    HWND hListView = FoundFilesListView->HWindow;
    int count = ListView_GetItemCount(hListView);
    int i;
    for (i = 0; i < count; i++)
    {
        DWORD state = ListView_GetItemState(hListView, i, LVIS_SELECTED);
        ListView_SetItemState(hListView, i, (state & LVIS_SELECTED) ? 0 : LVIS_SELECTED, LVIS_SELECTED)
    }
    SetCursor(hOldCursor);
}

void Find_Dialog::OnShowLog()
{
    if (Log.Items_Count() == 0)
        return;

    Find_Log_Dialog dlg(HWindow, &Log);
    dlg.Execute();
}

void Find_Dialog::OnEnterIdle()
{
    if (UpdateStatusBar && !IsSearchInProgress())
    {
        UpdateStatusText();
        UpdateStatusBar = FALSE;
    }
}

void Find_Dialog::UpdateStatusText()
{
    int count = ListView_GetSelectedCount(FoundFilesListView->HWindow);
    if (count != 0)
    {
        CQuadWord selectedSize;
        selectedSize = CQuadWord(0, 0);
        int totalCount = ListView_GetItemCount(FoundFilesListView->HWindow);
        int files = 0;
        int dirs = 0;

        int index = -1;
        do
        {
            index = ListView_GetNextItem(FoundFilesListView->HWindow, index, LVNI_SELECTED);
            if (index != -1)
            {
                Find_FileList_Item* item = FoundFilesListView->Items_At(index);
                if (item->IsDir)
                    dirs++;
                else
                {
                    files++;
                    selectedSize += item->Size;
                }
            }
        } while (index != -1);

        char text[200];
        if (dirs > 0)
            ExpandPluralFilesDirs(text, 200, files, dirs, epfdmSelected, FALSE);
        else
            ExpandPluralBytesFilesDirs(text, 200, selectedSize, files, dirs, FALSE);
        SendMessage(HStatusBar, SB_SETTEXT, 1 | SBT_NOBORDERS, (LPARAM)text);
    }
    else
    {
        SendMessage(HStatusBar, SB_SETTEXT, 1 | SBT_NOBORDERS, (LPARAM) "");
    }
}

void Find_Dialog::OnColorsChange()
{
    if (MainMenu != NULL)
    {
        MainMenu->SetImageList(HGrayToolBarImageList, TRUE);
        MainMenu->SetHotImageList(HHotToolBarImageList, TRUE);
    }
    if (TBHeader != NULL)
        TBHeader->OnColorsChange();
    if (FoundFilesListView != NULL && FoundFilesListView->HWindow != NULL)
        ListView_SetImageList(FoundFilesListView->HWindow, HFindSymbolsImageList, LVSIL_SMALL);
    if (MenuBar != NULL)
    {
        MenuBar->SetFont();
        MenuBarHeight = MenuBar->GetNeededHeight();
        LayoutControls();
    }
}

//****************************************************************************
//
// CFindTBHeader
//

#define FINDTBHDR_BUTTONS 9

CFindTBHeader::CFindTBHeader(HWND hDlg, int ctrlID)
    : CWindow(hDlg, ctrlID, ooAllocated)
{
    CALL_STACK_MESSAGE2("CFindTBHeader::CFindTBHeader(, %d)", ctrlID);
    HNotifyWindow = hDlg;
    Text[0] = 0;
    ErrorsCount = 0;
    InfosCount = 0;
    FoundCount = -1; // abychom si vynutili nastaveni
    FlashIconCounter = 0;
    StopFlash = FALSE;

    DWORD exStyle = (DWORD)GetWindowLongPtr(HWindow, GWL_EXSTYLE);
    exStyle |= WS_EX_STATICEDGE;
    SetWindowLongPtr(HWindow, GWL_EXSTYLE, exStyle);

    LogToolBar = NULL;
    HWarningIcon = NULL;
    HInfoIcon = NULL;
    HEmptyIcon = NULL;

    ToolBar = new CToolBar(HWindow);

    ToolBar->CreateWnd(HWindow);

    ToolBar->SetImageList(HGrayToolBarImageList);
    ToolBar->SetHotImageList(HHotToolBarImageList);
    ToolBar->SetStyle(TLB_STYLE_IMAGE | TLB_STYLE_TEXT);

    int ids[FINDTBHDR_BUTTONS] = {CM_FIND_FOCUS, CM_FIND_VIEW, CM_FIND_EDIT, CM_FIND_DELETE, CM_FIND_USERMENU, CM_FIND_PROPERTIES, CM_FIND_CLIPCUT, CM_FIND_CLIPCOPY, IDC_FIND_STOP};
    int idx[FINDTBHDR_BUTTONS] = {IDX_TB_FOCUS, IDX_TB_VIEW, IDX_TB_EDIT, IDX_TB_DELETE, IDX_TB_USERMENU, IDX_TB_PROPERTIES, IDX_TB_CLIPBOARDCUT, IDX_TB_CLIPBOARDCOPY, IDX_TB_STOP};

    TLBI_ITEM_INFO2 tii;
    tii.Mask = TLBI_MASK_STYLE | TLBI_MASK_ID | TLBI_MASK_IMAGEINDEX | TLBI_MASK_TEXT;
    tii.Style = TLBI_STYLE_SHOWTEXT;
    int buttonsCount = 0;
    int i;
    for (i = 0; i < FINDTBHDR_BUTTONS; i++)
    {
        tii.ImageIndex = idx[i];
        tii.Text = NULL;
        if (i == 0)
            tii.Text = LoadStr(IDS_FINDTB_FOCUS);
        tii.ID = ids[i];
        ToolBar->InsertItem2(i, TRUE, &tii);
        buttonsCount++;
    }
    ShowWindow(ToolBar->HWindow, SW_SHOW);
}

BOOL CFindTBHeader::CreateLogToolbar(BOOL errors, BOOL infos)
{
    BOOL justCreated = FALSE;
    if (LogToolBar == NULL)
    {
        LogToolBar = new CToolBar(HWindow);
        if (LogToolBar == NULL)
        {
            TRACE_E(LOW_MEMORY);
            return FALSE;
        }

        LogToolBar->CreateWnd(HWindow);
        LogToolBar->SetStyle(TLB_STYLE_IMAGE);
        WarningDisplayed = FALSE;
        InfoDisplayed = FALSE;

        justCreated = TRUE;
    }

    BOOL change = FALSE;
    TLBI_ITEM_INFO2 tii;
    tii.Mask = TLBI_MASK_STYLE | TLBI_MASK_ID | TLBI_MASK_ICON;
    tii.Style = TLBI_STYLE_SHOWTEXT;
    if (errors && !WarningDisplayed)
    {
        if (InfoDisplayed)
            LogToolBar->RemoveItem(0, TRUE);
        if (HWarningIcon == NULL)
        {
            int iconSize = IconSizes[IconSize::size_16x16];
            LoadIconWithScaleDown(NULL, (PCWSTR)IDI_EXCLAMATION, iconSize, iconSize, &HWarningIcon);
        }
        tii.HIcon = HWarningIcon;
        tii.ID = CM_FIND_MESSAGES;
        LogToolBar->InsertItem2(0, TRUE, &tii);
        WarningDisplayed = TRUE;
        change = TRUE;
    }
    if (infos && !errors && !InfoDisplayed)
    {
        if (HInfoIcon == NULL)
        {
            int iconSize = IconSizes[IconSize::size_16x16];
            LoadIconWithScaleDown(NULL, (PCWSTR)IDI_INFORMATION, iconSize, iconSize, &HInfoIcon);
        }
        tii.HIcon = HInfoIcon;
        tii.ID = CM_FIND_MESSAGES;
        LogToolBar->InsertItem2(0, TRUE, &tii);
        InfoDisplayed = TRUE;
        change = TRUE;
    }

    if (justCreated)
        ShowWindow(LogToolBar->HWindow, SW_SHOW);

    return change;
}

void CFindTBHeader::OnColorsChange()
{
    if (ToolBar != NULL)
    {
        ToolBar->SetImageList(HGrayToolBarImageList);
        ToolBar->SetHotImageList(HHotToolBarImageList);
        ToolBar->OnColorsChanged();
    }
}

int CFindTBHeader::GetNeededHeight()
{
    return ToolBar->GetNeededHeight() + 2;
}

BOOL CFindTBHeader::EnableItem(DWORD position, BOOL byPosition, BOOL enabled)
{
    return ToolBar->EnableItem(position, byPosition, enabled);
}

void CFindTBHeader::SetFoundCount(int foundCount)
{
    if (foundCount != FoundCount)
    {
        char buff[200];
        GetWindowText(HWindow, buff, 200);
        sprintf(Text, buff, foundCount);

        RECT r;
        GetClientRect(HWindow, &r);
        InvalidateRect(HWindow, &r, TRUE);
    }
}

#define FLASH_ICON_COUNT 20
#define FLASH_ICON_DELAY 333

void CFindTBHeader::StartFlashIcon()
{
    StopFlash = FALSE;
    if (GetForegroundWindow() != HNotifyWindow)
        SendMessage(HNotifyWindow, WM_USER_FLASHICON, 0, 0); // pokud nejsme aktivni, odlozime blikani na pozdeji
    else
        SetTimer(HWindow, IDT_FLASHICON, FLASH_ICON_DELAY, NULL);
    FlashIconCounter = FLASH_ICON_COUNT;
}

void CFindTBHeader::StopFlashIcon()
{
    StopFlash = TRUE;
}

void CFindTBHeader::SetFont()
{
    if (ToolBar != NULL && ToolBar->HWindow != NULL)
        ToolBar->SetFont();
}

void CFindTBHeader::SetErrorsInfosCount(int errorsCount, int infosCount)
{
    if (ErrorsCount != errorsCount || InfosCount != infosCount)
    {
        BOOL change = FALSE;
        if (errorsCount != 0 || infosCount != 0)
        {
            change = CreateLogToolbar(errorsCount != 0, infosCount != 0);
            if (change)
                StartFlashIcon(); // pokud prave neblikame, pak zablikame
        }
        if (errorsCount == 0 && infosCount == 0 && LogToolBar != NULL)
        {
            DestroyWindow(LogToolBar->HWindow);
            LogToolBar = NULL;
            change = TRUE;
        }
        ErrorsCount = errorsCount;
        InfosCount = infosCount;
        if (change)
        {
            RECT r;
            GetClientRect(HWindow, &r);
            SendMessage(HWindow, WM_SIZE, SIZE_RESTORED,
                        MAKELONG(r.right - r.left, r.bottom - r.top));
            UpdateWindow(HWindow);
        }
    }
}

LRESULT
CFindTBHeader::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CALL_STACK_MESSAGE4("CFindTBHeader::WindowProc(0x%X, 0x%IX, 0x%IX)", uMsg, wParam, lParam);
    switch (uMsg)
    {
    case WM_DESTROY:
    {
        if (FlashIconCounter > 0)
            KillTimer(HWindow, WM_TIMER);
        if (ToolBar != NULL)
            DestroyWindow(ToolBar->HWindow);
        if (LogToolBar != NULL)
            DestroyWindow(LogToolBar->HWindow);
        if (HWarningIcon != NULL)
            DestroyIcon(HWarningIcon);
        if (HInfoIcon != NULL)
            DestroyIcon(HInfoIcon);
        if (HEmptyIcon != NULL)
            DestroyIcon(HEmptyIcon);
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = HANDLES(BeginPaint(HWindow, &ps));
        HANDLES(EndPaint(HWindow, &ps));
        return 0;
    }

    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)wParam;
        RECT r;
        GetClientRect(HWindow, &r);
        RECT tr = r;
        tr.right -= 7;
        if (LogToolBar != NULL)
            tr.right -= LogToolBar->GetNeededWidth();

        HFONT hOldFont = (HFONT)SelectObject(hdc, (HFONT)SendMessage(HWindow, WM_GETFONT, 0, 0));
        int oldBkMode = SetBkMode(hdc, TRANSPARENT);
        FillRect(hdc, &r, (HBRUSH)(COLOR_3DFACE + 1));
        DrawText(hdc, Text, -1, &tr, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
        SetBkMode(hdc, oldBkMode);
        SelectObject(hdc, hOldFont);

        return TRUE;
    }

    case WM_TIMER:
    {
        BOOL stopped = FALSE;
        if (wParam == IDT_FLASHICON)
        {
            if (FlashIconCounter > 0 && LogToolBar != NULL && LogToolBar->HWindow != NULL)
            {
                if (HEmptyIcon == NULL)
                {
                    int iconSize = IconSizes[IconSize::size_16x16];
                    LoadIconWithScaleDown(HInstance, (PCWSTR)IDI_EMPTY, iconSize, iconSize, &HEmptyIcon);
                }
                TLBI_ITEM_INFO2 tii;
                tii.Mask = TLBI_MASK_ICON;
                if ((FlashIconCounter & 0x00000001) == 0)
                    tii.HIcon = HEmptyIcon;
                else
                {
                    tii.HIcon = ErrorsCount > 0 ? HWarningIcon : HInfoIcon;
                    if (StopFlash)
                    {
                        FlashIconCounter = 1;
                        StopFlash = FALSE;
                        stopped = TRUE; // abychom se zase nerozjeli
                    }
                }
                LogToolBar->SetItemInfo2(0, TRUE, &tii);
                UpdateWindow(LogToolBar->HWindow);
                FlashIconCounter--;
            }
            if (FlashIconCounter <= 0)
            {
                KillTimer(HWindow, IDT_FLASHICON);
                if (!stopped && GetForegroundWindow() != HNotifyWindow)
                {
                    // uzivatel asi nevidel blikani, zopakujem si ho pri pristi aktivaci Findu
                    SendMessage(HNotifyWindow, WM_USER_FLASHICON, 0, 0);
                }
            }
            return 0;
        }
        break;
    }

    case WM_SIZE:
    {
        InvalidateRect(HWindow, NULL, TRUE);
        int tbW = ToolBar->GetNeededWidth();
        int tbH = ToolBar->GetNeededHeight();
        SetWindowPos(ToolBar->HWindow, NULL, 0, 0, tbW, tbH, SWP_NOZORDER | SWP_NOACTIVATE);

        if (LogToolBar != NULL)
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            tbW = LogToolBar->GetNeededWidth();
            tbH = LogToolBar->GetNeededHeight();
            SetWindowPos(LogToolBar->HWindow, NULL, width - tbW, 0, tbW, tbH, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    }

    case WM_COMMAND:
    {
        if ((HWND)lParam == ToolBar->HWindow)
            PostMessage(HNotifyWindow, WM_COMMAND, MAKEWPARAM(LOWORD(wParam), 0), (LPARAM)HWindow);
        else if (LogToolBar != NULL && (HWND)lParam == LogToolBar->HWindow)
            PostMessage(HNotifyWindow, WM_COMMAND, MAKEWPARAM(LOWORD(wParam), 0), (LPARAM)HWindow);
        break;
    }

    case WM_USER_TBGETTOOLTIP:
    {
        TOOLBAR_TOOLTIP* tt = (TOOLBAR_TOOLTIP*)lParam;
        if (tt->HToolBar == ToolBar->HWindow)
        {
            int tooltips[FINDTBHDR_BUTTONS] = {IDS_FINDTBTT_FOCUS, IDS_FINDTBTT_VIEW, IDS_FINDTBTT_EDIT, IDS_FINDTBTT_DELETE, IDS_FINDTBTT_USERMENU, IDS_FINDTBTT_PROPERTIES, IDS_FINDTBTT_CUT, IDS_FINDTBTT_COPY, IDS_FINDTBTT_STOP};
            lstrcpy(tt->Buffer, LoadStr(tooltips[tt->Index]));
            PrepareToolTipText(tt->Buffer, FALSE);
        }
        else
        {
            wsprintf(tt->Buffer, LoadStr(IDS_FINDTBTT_MESSAGES), ErrorsCount, InfosCount);
        }
        return TRUE;
    }
    }
    return CWindow::WindowProc(uMsg, wParam, lParam);
}

//*********************************************************************************
//
// CFindManageDialog
//

CFindManageDialog::CFindManageDialog(HWND hParent, const Find_Options_Item* currenOptionsItem)
    : CCommonDialog(HLanguage, IDD_FINDSETTINGS, IDD_FINDSETTINGS, hParent)
{
    CurrenOptionsItem = currenOptionsItem;
    EditLB = NULL;
    FO = new Find_Options();
    if (FO == NULL)
    {
        TRACE_E(LOW_MEMORY);
        return;
    }
    FO->Load(FindOptions); // nacucnu do pracovni promenne globalni konfiguraci
}

CFindManageDialog::~CFindManageDialog()
{
    if (FO != NULL)
        delete FO;
}

void CFindManageDialog::Transfer(CTransferInfo& ti)
{
    CALL_STACK_MESSAGE1("CFindManageDialog::Transfer()");
    if (ti.Type == ttDataToWindow)
    {
        int i;
        for (i = 0; i < FO->Items_Count(); i++)
            EditLB->AddItem((INT_PTR)FO->Items_At(i));
        EditLB->SetCurSel(0);
        LoadControls();
        //    EnableButtons();
    }
    else
    {
        FindOptions.Load(*FO);
    }
}

void CFindManageDialog::LoadControls()
{
    CALL_STACK_MESSAGE1("CFindManageDialog::LoadControls()");
    INT_PTR itemID;
    EditLB->GetCurSelItemID(itemID);
    BOOL empty = FALSE;
    if (itemID == -1)
        empty = TRUE;

    Find_Options_Item* item = NULL;
    if (!empty)
        item = (Find_Options_Item*)itemID;
    else
        item = (Find_Options_Item*)CurrenOptionsItem;
    SetDlgItemText(HWindow, IDC_FFS_NAMED, item->NamedText);
    SetDlgItemText(HWindow, IDC_FFS_LOOKIN, item->LookInText);
    SetDlgItemText(HWindow, IDC_FFS_SUBDIRS, item->SubDirectories ? LoadStr(IDS_INFODLGYES) : LoadStr(IDS_INFODLGNO));
    SetDlgItemText(HWindow, IDC_FFS_CONTAINING, item->GrepText);
    char buff[200];
    BOOL dirty;
    item->Criteria.GetAdvancedDescription(buff, 200, dirty);
    SetDlgItemText(HWindow, IDC_FFS_ADVANCED, buff);

    EnableWindow(GetDlgItem(HWindow, IDC_FFS_AUTOLOAD), !empty);
    CheckDlgButton(HWindow, IDC_FFS_AUTOLOAD, item->AutoLoad);
}

INT_PTR
CFindManageDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CALL_STACK_MESSAGE4("CFindManageDialog::DialogProc(0x%X, 0x%IX, 0x%IX)", uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        EditLB = new CEditListBox(HWindow, IDC_FFS_NAMES);
        if (EditLB == NULL)
            TRACE_E(LOW_MEMORY);
        EditLB->MakeHeader(IDC_FFS_NAMESLABEL);
        EditLB->EnableDrag(HWindow);
        break;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_FFS_AUTOLOAD:
        {
            if (HIWORD(wParam) == BN_CLICKED)
            {
                int index;
                EditLB->GetCurSel(index);
                if (index != -1)
                {
                    BOOL checked = IsDlgButtonChecked(HWindow, IDC_FFS_AUTOLOAD);
                    int i;
                    for (i = 0; i < FO->Items_Count(); i++)
                        if (FO->Items_At(i)->AutoLoad)
                            FO->Items_At(i)->AutoLoad = FALSE;
                    if (checked)
                        FO->Items_At(index)->AutoLoad = TRUE;
                    InvalidateRect(EditLB->HWindow, NULL, TRUE);
                    UpdateWindow(EditLB->HWindow);
                }
            }
            break;
        }

        case IDC_FFS_NAMES:
        {
            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                EditLB->OnSelChanged();
                LoadControls();
            }
            break;
        }

        case IDC_FFS_ADD:
        {
            Find_Options_Item* item = new Find_Options_Item();
            if (item != NULL)
            {
                *item = *CurrenOptionsItem;
                item->BuildItemName();
                FO->Items_Add(item);
                int index = EditLB->AddItem((INT_PTR)item);
                EditLB->SetCurSel(index);
            }
            else
                TRACE_E(LOW_MEMORY);
            break;
        }
        }
        break;
    }

    case WM_NOTIFY:
    {
        NMHDR* nmhdr = (NMHDR*)lParam;
        switch (nmhdr->idFrom)
        {
        case IDC_FFS_NAMES:
        {
            switch (nmhdr->code)
            {
            case EDTLBN_GETDISPINFO:
            {
                EDTLB_DISPINFO* dispInfo = (EDTLB_DISPINFO*)lParam;
                if (dispInfo->ToDo == edtlbGetData)
                {
                    strcpy(dispInfo->Buffer, ((Find_Options_Item*)dispInfo->ItemID)->ItemName);
                    dispInfo->Bold = ((Find_Options_Item*)dispInfo->ItemID)->AutoLoad;
                    SetWindowLongPtr(HWindow, DWLP_MSGRESULT, FALSE);
                    return TRUE;
                }
                else
                {
                    Find_Options_Item* item;
                    if (dispInfo->ItemID == -1)
                    {
                        item = new Find_Options_Item();
                        if (item == NULL)
                        {
                            TRACE_E(LOW_MEMORY);
                            SetWindowLongPtr(HWindow, DWLP_MSGRESULT, TRUE);
                            return TRUE;
                        }
                        *item = *CurrenOptionsItem;
                        FO->Items_Add(item);
                        lstrcpyn(item->ItemName, dispInfo->Buffer, ITEMNAME_TEXT_LEN);
                        EditLB->SetItemData((INT_PTR)item);
                        LoadControls();
                    }
                    else
                    {
                        item = (Find_Options_Item*)dispInfo->ItemID;
                        lstrcpyn(item->ItemName, dispInfo->Buffer, ITEMNAME_TEXT_LEN);
                    }
                    SetWindowLongPtr(HWindow, DWLP_MSGRESULT, TRUE);
                    return TRUE;
                }
                break;
            }
                /*
            case EDTLBN_MOVEITEM:
            {
              EDTLB_DISPINFO *dispInfo = (EDTLB_DISPINFO *)lParam;
              int index;
              EditLB->GetCurSel(index);
              int srcIndex = index;
              int dstIndex = index + (dispInfo->Up ? -1 : 1);

              Find_Options_Item tmp;
              tmp = *FO->At(srcIndex);
              *FO->At(srcIndex) = *FO->At(dstIndex);
              *FO->At(dstIndex) = tmp;

              SetWindowLongPtr(HWindow, DWLP_MSGRESULT, FALSE);  // povolim prohozeni
              return TRUE;
            }
*/
            case EDTLBN_MOVEITEM2:
            {
                EDTLB_DISPINFO* dispInfo = (EDTLB_DISPINFO*)lParam;
                int index;
                EditLB->GetCurSel(index);
                int srcIndex = index;
                int dstIndex = dispInfo->NewIndex;

                Find_Options_Item tmp;
                tmp = *FO->Items_At(srcIndex);
                if (srcIndex < dstIndex)
                {
                    int i;
                    for (i = srcIndex; i < dstIndex; i++)
                        *FO->Items_At(i) = *FO->Items_At(i + 1);
                }
                else
                {
                    int i;
                    for (i = srcIndex; i > dstIndex; i--)
                        *FO->Items_At(i) = *FO->Items_At(i - 1);
                }
                *FO->Items_At(dstIndex) = tmp;

                SetWindowLongPtr(HWindow, DWLP_MSGRESULT, FALSE); // povolime zmenu
                return TRUE;
            }

            case EDTLBN_DELETEITEM:
            {
                int index;
                EditLB->GetCurSel(index);
                FO->Items_Delete(index);
                SetWindowLongPtr(HWindow, DWLP_MSGRESULT, FALSE); // povolim smazani
                return TRUE;
            }
            }
            break;
        }
        }
        break;
    }

    case WM_DRAWITEM:
    {
        int idCtrl = (int)wParam;
        if (idCtrl == IDC_FFS_NAMES)
        {
            EditLB->OnDrawItem(lParam);
            return TRUE;
        }
        break;
    }
    }
    return CCommonDialog::DialogProc(uMsg, wParam, lParam);
}

//*********************************************************************************
//
// CFindIgnoreDialog
//

CFindIgnoreDialog::CFindIgnoreDialog(HWND hParent, FindIgnore* globalIgnoreList)
    : CCommonDialog(HLanguage, IDD_FINDIGNORE, IDD_FINDIGNORE, hParent)
{
    EditLB = NULL;
    DisableNotification = FALSE;
    GlobalIgnoreList = globalIgnoreList;
    HChecked = NULL;
    HUnchecked = NULL;

    IgnoreList = new FindIgnore;
    if (IgnoreList == NULL)
    {
        TRACE_E(LOW_MEMORY);
        return;
    }
    IgnoreList->Load(GlobalIgnoreList); // nacucnu do pracovni promenne globalni konfiguraci
}

CFindIgnoreDialog::~CFindIgnoreDialog()
{
    if (HChecked != NULL)
        DestroyIcon(HChecked);
    if (HUnchecked != NULL)
        DestroyIcon(HUnchecked);
    if (IgnoreList != NULL)
        delete IgnoreList;
}

void CFindIgnoreDialog::Transfer(CTransferInfo& ti)
{
    CALL_STACK_MESSAGE1("CFindIgnoreDialog::Transfer()");
    if (ti.Type == ttDataToWindow)
    {
        FillList();
    }
    else
    {
        GlobalIgnoreList->Load(IgnoreList);
    }
}

BOOL IsIgnorePathValid(const char* path)
{
    // cesta nesmi obsahovat znaky '?' a '*' -- nepodporujeme masky
    BOOL foundChar = FALSE;
    const char* p = path;
    while (*p != 0)
    {
        if (*p == '?' || *p == '*')
            return FALSE;
        if (*p == '\\')
            foundChar = TRUE;
        if (LowerCase[*p] >= 'a' && LowerCase[*p] <= 'z')
            foundChar = TRUE;
        p++;
    }
    if (!foundChar)
        return FALSE;
    return TRUE;
}

void CFindIgnoreDialog::Validate(CTransferInfo& ti)
{
    int i;
    for (i = 0; i < IgnoreList->GetCount(); i++)
    {
        Find_Ignore_Item* item = IgnoreList->At(i);
        if (item->Enabled && !IsIgnorePathValid(item->Path))
        {
            SalMessageBox(HWindow, LoadStr(IDS_ACBADDRIVE), LoadStr(IDS_ERRORTITLE),
                          MB_OK | MB_ICONEXCLAMATION);
            ti.ErrorOn(IDC_FFI_NAMES);
            EditLB->SetCurSel(i);
            PostMessage(HWindow, WM_USER_EDIT, 0, 0);
            return;
        }
    }
}

void CFindIgnoreDialog::FillList()
{
    EditLB->DeleteAllItems();
    int i;
    for (i = 0; i < IgnoreList->GetCount(); i++)
        EditLB->AddItem((INT_PTR)IgnoreList->At(i));
    EditLB->SetCurSel(0);
}

INT_PTR
CFindIgnoreDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CALL_STACK_MESSAGE4("CFindIgnoreDialog::DialogProc(0x%X, 0x%IX, 0x%IX)", uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        EditLB = new CEditListBox(HWindow, IDC_FFI_NAMES, ELB_ITEMINDEXES | ELB_ENABLECOMMANDS | ELB_SHOWICON | ELB_SPACEASICONCLICK); // potrebujeme enabler a ikony
        if (EditLB == NULL)
            TRACE_E(LOW_MEMORY);
        EditLB->MakeHeader(IDC_FFI_NAMESLABEL);
        EditLB->EnableDrag(HWindow);
        int iconSize = IconSizes[IconSize::size_16x16];
        HIMAGELIST hIL = CreateCheckboxImagelist(iconSize);
        HUnchecked = ImageList_GetIcon(hIL, 0, ILD_NORMAL);
        HChecked = ImageList_GetIcon(hIL, 1, ILD_NORMAL);
        ImageList_Destroy(hIL);

        break;
    }

    case WM_DRAWITEM:
    {
        int idCtrl = (int)wParam;
        if (idCtrl == IDC_FFI_NAMES)
        {
            EditLB->OnDrawItem(lParam);
            return TRUE;
        }
        break;
    }

    case WM_USER_EDIT:
    {
        SetFocus(EditLB->HWindow);
        EditLB->OnBeginEdit();
        return 0;
    }

    case WM_COMMAND:
    {
        if (DisableNotification)
            return 0;

        switch (LOWORD(wParam))
        {
        case IDC_FFI_RESET:
        {
            if (SalMessageBox(HWindow, LoadStr(IDS_FINDIGNORE_RESET), LoadStr(IDS_QUESTION),
                              MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
            {
                IgnoreList->Reset();
                FillList();
            }
            return 0;
        }

        case IDC_FFI_NAMES:
        {
            if (HIWORD(wParam) == LBN_SELCHANGE)
                EditLB->OnSelChanged();
            break;
        }
        }
        break;
    }

    case WM_NOTIFY:
    {
        NMHDR* nmhdr = (NMHDR*)lParam;
        switch (nmhdr->idFrom)
        {
        case IDC_FFI_NAMES:
        {
            switch (nmhdr->code)
            {
            case EDTLBN_GETDISPINFO:
            {
                EDTLB_DISPINFO* dispInfo = (EDTLB_DISPINFO*)lParam;
                if (dispInfo->ToDo == edtlbGetData)
                {
                    Find_Ignore_Item* item = IgnoreList->At((int)dispInfo->ItemID);
                    strcpy(dispInfo->Buffer, item->Path);
                    dispInfo->HIcon = item->Enabled ? HChecked : HUnchecked;
                    SetWindowLongPtr(HWindow, DWLP_MSGRESULT, FALSE);
                    return TRUE;
                }
                else
                {
                    if (dispInfo->ItemID == -1)
                    {
                        if (!IgnoreList->Add(TRUE, dispInfo->Buffer))
                        {
                            TRACE_E(LOW_MEMORY);
                            SetWindowLongPtr(HWindow, DWLP_MSGRESULT, TRUE);
                            return TRUE;
                        }
                        EditLB->SetItemData(IgnoreList->GetCount() - 1);
                    }
                    else
                    {
                        int index = (int)dispInfo->ItemID;
                        Find_Ignore_Item* item = IgnoreList->At(index);
                        IgnoreList->Set(index, item->Enabled, dispInfo->Buffer);
                    }

                    SetWindowLongPtr(HWindow, DWLP_MSGRESULT, TRUE);
                    return TRUE;
                }
                break;
            }

            case EDTLBN_ENABLECOMMANDS:
            {
                EDTLB_DISPINFO* dispInfo = (EDTLB_DISPINFO*)lParam;
                int index;
                EditLB->GetCurSel(index);
                dispInfo->Enable = TLBHDRMASK_NEW | TLBHDRMASK_MODIFY;
                if (index > 0 && index < IgnoreList->GetCount())
                    dispInfo->Enable |= TLBHDRMASK_UP;
                if (index < IgnoreList->GetCount())
                    dispInfo->Enable |= TLBHDRMASK_DELETE;
                if (index < IgnoreList->GetCount() - 1)
                    dispInfo->Enable |= TLBHDRMASK_DOWN;
                return TRUE;
            }

            case EDTLBN_MOVEITEM2:
            {
                EDTLB_DISPINFO* dispInfo = (EDTLB_DISPINFO*)lParam;
                int index;
                EditLB->GetCurSel(index);
                IgnoreList->Move(index, dispInfo->NewIndex);
                SetWindowLongPtr(HWindow, DWLP_MSGRESULT, FALSE); // povolime zmenu
                return TRUE;
            }

            case EDTLBN_ICONCLICKED:
            {
                EDTLB_DISPINFO* dispInfo = (EDTLB_DISPINFO*)lParam;
                if (dispInfo->ItemID >= 0 && dispInfo->ItemID < IgnoreList->GetCount())
                {
                    Find_Ignore_Item* item = IgnoreList->At((int)dispInfo->ItemID);
                    item->Enabled = item->Enabled ? FALSE : TRUE;
                    EditLB->RedrawFocusedItem();
                }
                return TRUE;
            }

            case EDTLBN_DELETEITEM:
            {
                int index;
                EditLB->GetCurSel(index);
                IgnoreList->Delete(index);
                SetWindowLongPtr(HWindow, DWLP_MSGRESULT, FALSE); // povolim smazani
                return TRUE;
            }
            }
            break;
        }
        }
        break;
    }
    }
    return CCommonDialog::DialogProc(uMsg, wParam, lParam);
}

//****************************************************************************
//
// Drag&Drop + Clipboard Copy&Cut
//

BOOL Find_Dialog::InitializeOle()
{
    if (!OleInitialized)
    {
        OleInitialized = (OleInitialize(NULL) == S_OK);
    }
    return OleInitialized;
}

void Find_Dialog::UninitializeOle()
{
    if (OleInitialized)
    {
        __try
        {
            OleFlushClipboard(); // predame systemu data z IDataObjectu, ktery jsme na clipboardu nechali lezet (tento IDataObject se tim uvolni)
            OleUninitialize();
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            OCUExceptionHasOccured++;
        }
        OleInitialized = FALSE;
    }
}

const char*
Find_Dialog::GetName(int index)
{
    if (index < 0 || index >= FoundFilesListView->Items_Count())
        return FALSE;
    return FoundFilesListView->Items_At(index)->Name;
}

const char*
Find_Dialog::GetPath(int index)
{
    if (index < 0 || index >= FoundFilesListView->Items_Count())
        return FALSE;
    return FoundFilesListView->Items_At(index)->Path;
}

BOOL Find_Dialog::GetCommonPrefixPath(char* buffer, int bufferMax, int& commonPrefixChars)
{
    HWND hListView = FoundFilesListView->HWindow;
    DWORD selCount = ListView_GetSelectedCount(hListView);
    if (selCount == 0)
    {
        TRACE_E("Selected count = 0");
        return FALSE;
    }

    char path[MAX_PATH];
    int pathLen = 0;
    path[0] = 0;

    int index = -1;
    do
    {
        index = ListView_GetNextItem(FoundFilesListView->HWindow, index, LVNI_SELECTED);
        if (index != -1)
        {
            Find_FileList_Item* file = FoundFilesListView->Items_At(index);
            if (path[0] == 0)
            {
                lstrcpy(path, file->Path); // v prvnim kroku pouze zapiseme cestu
                pathLen = lstrlen(path);
            }
            else
            {
                int count = CommonPrefixLength(path, file->Path);
                if (count < pathLen)
                {
                    // doslo ke zkraceni cesty
                    path[count] = 0;
                    pathLen = count;
                }
                if (count == 0)
                    return FALSE; // neexistuje spolecna cast cesty
            }
        }
    } while (index != -1);

    if (pathLen == 0)
        return FALSE;
    if (pathLen + 1 > bufferMax)
    {
        TRACE_E("Buffer is small. " << pathLen + 1 << " bytes is needed");
        return FALSE;
    }
    lstrcpy(buffer, path);
    commonPrefixChars = pathLen;
    return TRUE;
}

struct CMyEnumFileNamesData
{
    Find_Dialog* FindDialog;
    HWND HListView;
    int CommonPrefixChars; // pocet znaku spolecne cesty
    int LastIndex;
};

static char MyEnumFileNamesBuffer[MAX_PATH]; // funkce je volana z GUI => nemuze byt zavolana ve vice threadech => muzeme si dovolit staticky buffer
const char* MyEnumFileNames(int index, void* param)
{
    CMyEnumFileNamesData* data = (CMyEnumFileNamesData*)param;
    int foundIndex = ListView_GetNextItem(data->HListView, data->LastIndex, LVNI_SELECTED);
    if (foundIndex != -1)
    {
        data->LastIndex = foundIndex;
        const char* p = data->FindDialog->GetPath(foundIndex) + data->CommonPrefixChars;
        while (*p == '\\')
            p++;
        if (*p != 0)
        {
            lstrcpy(MyEnumFileNamesBuffer, p);
            SalPathAddBackslash(MyEnumFileNamesBuffer, MAX_PATH);
        }
        else
            MyEnumFileNamesBuffer[0] = 0;
        lstrcat(MyEnumFileNamesBuffer, data->FindDialog->GetName(foundIndex));
        return MyEnumFileNamesBuffer;
    }
    TRACE_E("Next item was not found");
    return NULL;
}

void ContextMenuInvoke(IContextMenu2* contextMenu, CMINVOKECOMMANDINFO* ici)
{
    CALL_STACK_MESSAGE_NONE

    // docasne snizime prioritu threadu, aby nam nejaka zmatena shell extension nesezrala CPU
    HANDLE hThread = GetCurrentThread(); // pseudo-handle, neni treba uvolnovat
    int oldThreadPriority = GetThreadPriority(hThread);
    SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);

    __try
    {
        contextMenu->InvokeCommand(ici);
    }
    __except (CCallStack::HandleException(GetExceptionInformation(), 8))
    {
        ICExceptionHasOccured++;
    }

    SetThreadPriority(hThread, oldThreadPriority);
}

void ContextMenuQuery(IContextMenu2* contextMenu, HMENU h)
{
    CALL_STACK_MESSAGE_NONE

    // docasne snizime prioritu threadu, aby nam nejaka zmatena shell extension nesezrala CPU
    HANDLE hThread = GetCurrentThread(); // pseudo-handle, neni treba uvolnovat
    int oldThreadPriority = GetThreadPriority(hThread);
    SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);

    __try
    {
        UINT flags = CMF_NORMAL | CMF_EXPLORE;
        // osetrime stisknuty shift - rozsirene kontextove menu, pod W2K je tam napriklad Run as...
#define CMF_EXTENDEDVERBS 0x00000100 // rarely used verbs
        BOOL shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        if (shiftPressed)
            flags |= CMF_EXTENDEDVERBS;

        contextMenu->QueryContextMenu(h, 0, 0, -1, flags);
    }
    __except (CCallStack::HandleException(GetExceptionInformation(), 9))
    {
        QCMExceptionHasOccured++;
    }

    SetThreadPriority(hThread, oldThreadPriority);
}

void Find_Dialog::OnDrag(BOOL rightMouseButton)
{
    if (!InitializeOle())
        return;

    HWND hListView = FoundFilesListView->HWindow;
    DWORD selCount = ListView_GetSelectedCount(hListView);
    if (selCount < 1)
        return;

    char commonPrefixPath[MAX_PATH];
    int commonPrefixChars;
    if (!GetCommonPrefixPath(commonPrefixPath, MAX_PATH, commonPrefixChars))
    {
        SalMessageBox(HWindow, LoadStr(IDS_COMMONPREFIXNOTFOUND), LoadStr(IDS_ERRORTITLE),
                      MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    // ulozime focused polozku a index
    Find_FileList_Item lastFocusedItem;
    int lastFocusedIndex = ListView_GetNextItem(FoundFilesListView->HWindow, 0, LVIS_FOCUSED);
    if (lastFocusedIndex != -1)
    {
        Find_FileList_Item* lastItem = FoundFilesListView->Items_At(lastFocusedIndex);
        lastFocusedItem.Text_Set(lastItem->Path, lastItem->Name, lastItem->Size, lastItem->Attr, &lastItem->LastWrite, lastItem->IsDir);
    }

    CMyEnumFileNamesData data;
    data.FindDialog = this;
    data.HListView = hListView;
    data.CommonPrefixChars = commonPrefixChars;
    data.LastIndex = -1;

    CImpIDropSource* dropSource = new CImpIDropSource(FALSE);
    if (dropSource == NULL)
    {
        TRACE_E(LOW_MEMORY);
        return;
    }

    IDataObject* dataObject = CreateIDataObject(MainWindow->HWindow, commonPrefixPath,
                                                selCount, MyEnumFileNames, &data);
    if (dataObject == NULL)
    {
        SalMessageBox(HWindow, LoadStr(IDS_FOUNDITEMNOTFOUND), LoadStr(IDS_ERRORTITLE),
                      MB_ICONEXCLAMATION | MB_OK);
        dropSource->Release();
        return;
    }

    DWORD dwEffect;
    HRESULT hr = DoDragDrop(dataObject, dropSource,
                            DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK,
                            &dwEffect);

    if (hr == DRAGDROP_S_DROP)
    {
        // vraci dwEffect == 0 pri MOVE, viz "Handling Shell Data Transfer Scenarios" sekce
        // "Handling Optimized Move Operations": http://msdn.microsoft.com/en-us/library/windows/desktop/bb776904%28v=vs.85%29.aspx
        // (zkracene: dela se optimalizovany Move, coz znamena ze se nedela kopie do cile nasledovana mazanim
        //            originalu, aby zdroj nechtene nesmazal original (jeste nemusi byt presunuty), dostane
        //            vysledek operace DROPEFFECT_NONE nebo DROPEFFECT_COPY),
        // proto zavadime tuto obezlicku
        if (!rightMouseButton && // prave tlacitko ma menu: tedy dropSource->LastEffect obsahuje effect pred zobrazenim menu a ne vysledny effect: ten je krome Move v dwEffect - pri Move je to 0 stejne jako pri Cancel - tedy nejsme schopni rozlisit Move a Cancel, tedy radsi nebudeme nic mazat
            (dropSource->LastEffect & DROPEFFECT_MOVE))
        {
            // protoze operace move bezi ve vlastnim threadu, nejsou jeste soubory smazany
            // proto forcneme remove (prasarna, ale nemusime dalsi tyden programovat...)
            FoundFilesListView->CheckAndRemoveSelectedItems(TRUE, lastFocusedIndex, &lastFocusedItem);
        }
    }

    dropSource->Release();
    dataObject->Release();
}

void Find_Dialog::OnContextMenu(int x, int y)
{
    if (!InitializeOle())
        return;

    HWND hListView = FoundFilesListView->HWindow;
    DWORD selCount = ListView_GetSelectedCount(hListView);
    if (selCount < 1)
        return;

    char commonPrefixPath[MAX_PATH];
    int commonPrefixChars;
    if (!GetCommonPrefixPath(commonPrefixPath, MAX_PATH, commonPrefixChars))
    {
        SalMessageBox(HWindow, LoadStr(IDS_COMMONPREFIXNOTFOUND), LoadStr(IDS_ERRORTITLE),
                      MB_ICONEXCLAMATION | MB_OK);
        return;
    }
    CMyEnumFileNamesData data;
    data.FindDialog = this;
    data.HListView = hListView;
    data.CommonPrefixChars = commonPrefixChars;
    data.LastIndex = -1;
    ContextMenu = CreateIContextMenu2(HWindow, commonPrefixPath,
                                      selCount, MyEnumFileNames, &data);

    if (ContextMenu != NULL)
    {
        HMENU hMenu = CreatePopupMenu();
        ContextMenuQuery(ContextMenu, hMenu);
        RemoveUselessSeparatorsFromMenu(hMenu);

        CMenuPopup popup;
        popup.SetTemplateMenu(hMenu);
        DWORD cmd = popup.Track(MENU_TRACK_RETURNCMD | MENU_TRACK_RIGHTBUTTON,
                                x, y, HWindow, NULL);
        if (cmd != 0)
        {
            char cmdName[2000]; // schvalne mame 2000 misto 200, shell-extensiony obcas zapisuji dvojnasobek (uvaha: unicode = 2 * "pocet znaku"), atp.
            if (AuxGetCommandString(ContextMenu, cmd, GCS_VERB, NULL, cmdName, 200) != NOERROR)
            {
                cmdName[0] = 0;
            }
            BOOL cmdCut = stricmp(cmdName, "cut") == 0;
            BOOL cmdCopy = stricmp(cmdName, "copy") == 0;
            BOOL cmdDelete = stricmp(cmdName, "delete") == 0;

            CShellExecuteWnd shellExecuteWnd;
            CMINVOKECOMMANDINFOEX ici;
            ZeroMemory(&ici, sizeof(CMINVOKECOMMANDINFOEX));
            ici.cbSize = sizeof(CMINVOKECOMMANDINFOEX);
            ici.fMask = CMIC_MASK_PTINVOKE;
            if (CanUseShellExecuteWndAsParent(cmdName))
                ici.hwnd = shellExecuteWnd.Create(HWindow, "SEW: Find_Dialog::OnContextMenu cmd=%d cmdName=%s", cmd, cmdName);
            else
                ici.hwnd = HWindow;
            ici.lpVerb = MAKEINTRESOURCE(cmd);
            ici.nShow = SW_SHOWNORMAL;
            ici.ptInvoke.x = x;
            ici.ptInvoke.y = y;

            if (!cmdDelete)
            {
                ContextMenuInvoke(ContextMenu, (CMINVOKECOMMANDINFO*)&ici); // delete zavolame vlastni, at se to chova stejne na klavesu i z kontextoveho menu
            }

            if (cmdCut || cmdCopy)
            {
                // nastavime jeste preffered drop effect + puvod ze Salama
                SetClipCutCopyInfo(HWindow, cmdCopy, TRUE);
            }

            // postneme si Delete command
            if (cmdDelete)
                PostMessage(HWindow, WM_COMMAND, CM_FIND_DELETE, 0);
        }
        ContextMenu->Release();
        ContextMenu = NULL;
        DestroyMenu(hMenu);
    }
    else
        SalMessageBox(HWindow, LoadStr(IDS_FOUNDITEMNOTFOUND), LoadStr(IDS_ERRORTITLE),
                      MB_ICONEXCLAMATION | MB_OK);
}

BOOL Find_Dialog::InvokeContextMenu(const char* lpVerb)
{
    BOOL ret = FALSE;
    HWND hListView = FoundFilesListView->HWindow;
    DWORD selCount = ListView_GetSelectedCount(hListView);
    if (selCount < 1)
        return FALSE;
    HCURSOR hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    if (InitializeOle())
    {
        char commonPrefixPath[MAX_PATH];
        int commonPrefixChars;
        if (GetCommonPrefixPath(commonPrefixPath, MAX_PATH, commonPrefixChars))
        {
            CMyEnumFileNamesData data;
            data.FindDialog = this;
            data.HListView = hListView;
            data.CommonPrefixChars = commonPrefixChars;
            data.LastIndex = -1;
            IContextMenu2* menu = CreateIContextMenu2(HWindow, commonPrefixPath,
                                                      selCount, MyEnumFileNames, &data);

            if (menu != NULL)
            {
                CShellExecuteWnd shellExecuteWnd;
                CMINVOKECOMMANDINFOEX ici;
                ZeroMemory(&ici, sizeof(CMINVOKECOMMANDINFOEX));
                ici.cbSize = sizeof(CMINVOKECOMMANDINFOEX);
                ici.fMask = CMIC_MASK_PTINVOKE;
                ici.hwnd = shellExecuteWnd.Create(HWindow, "SEW: Find_Dialog::InvokeContextMenu lpVerb=%s", lpVerb);
                ici.lpVerb = lpVerb;
                ici.nShow = SW_SHOWNORMAL;
                GetListViewContextMenuPos(FoundFilesListView->HWindow, &ici.ptInvoke);

                ContextMenuInvoke(menu, (CMINVOKECOMMANDINFO*)&ici);
                menu->Release();

                ret = TRUE;
            }
            else
                SalMessageBox(HWindow, LoadStr(IDS_FOUNDITEMNOTFOUND), LoadStr(IDS_ERRORTITLE),
                              MB_ICONEXCLAMATION | MB_OK);
        }
        else
            SalMessageBox(HWindow, LoadStr(IDS_COMMONPREFIXNOTFOUND), LoadStr(IDS_ERRORTITLE),
                          MB_ICONEXCLAMATION | MB_OK);
    }
    SetCursor(hOldCursor);
    return TRUE;
}

void Find_Dialog::OnCutOrCopy(BOOL cut)
{
    if (InvokeContextMenu(cut ? "cut" : "copy"))
    {
        // nastavime jeste preffered drop effect + puvod ze Salama
        SetClipCutCopyInfo(HWindow, !cut, TRUE);
    }
}

void Find_Dialog::OnProperties()
{
    InvokeContextMenu("properties");
}

void Find_Dialog::OnOpen(BOOL onlyFocused)
{
    int count = ListView_GetSelectedCount(FoundFilesListView->HWindow);
    if (count == 0)
        return;

    if (!InitializeOle())
        return;

    int index = -1;
    do
    {
        // Petr: nahradil jsem LVNI_SELECTED za LVNI_FOCUSED, protoze si lidi stezovali, ze
        // si oznaci hafo souboru a pak j enechtene double-clickem vsechny otevrou, coz
        // znamena start mnoha procesu, proste bordel ... navrhovali konfirmaci pro otevreni
        // vice nez peti souboru, ale logictejsi reseni mi prijde otevirat jen fokuslej
        // (jako se to dela v panelu; nicmene Explorer otevira vsechny oznaceny, takze v tom
        // se ted lisime)
        // Honza 4/2014: pokud jsou vyhledane soubory z ruznych submenu, nefunguje pro ne
        // prikaz Otevrit z context menu, takze jsme lidem zrusili starou funkcionalitu
        // kdy otevreli vsechny soubory jednim Enterem. Zavadime proto novy prikaz Open Selected.
        // Bug na foru: https://forum.altap.cz/viewtopic.php?f=6&t=7449
        index = ListView_GetNextItem(FoundFilesListView->HWindow, index, onlyFocused ? LVNI_FOCUSED : LVNI_SELECTED);
        if (index != -1)
        {
            Find_FileList_Item* file = FoundFilesListView->Items_At(index);
            BOOL setWait = (GetCursor() != LoadCursor(NULL, IDC_WAIT)); // ceka uz ?
            HCURSOR oldCur;
            if (setWait)
                oldCur = SetCursor(LoadCursor(NULL, IDC_WAIT));

            char fullPath[MAX_PATH];
            lstrcpy(fullPath, file->Path);
            if (SalPathAppend(fullPath, file->Name, MAX_PATH))
                MainWindow->FileHistory->AddFile(fhitOpen, 0, fullPath);

            ExecuteAssociation(HWindow, file->Path, file->Name);
            if (setWait)
                SetCursor(oldCur);
        }
    } while (!onlyFocused && index != -1);
}

//****************************************************************************
//
// CFindDuplicatesDialog
//

BOOL CFindDuplicatesDialog::SameName = TRUE;
BOOL CFindDuplicatesDialog::SameSize = TRUE;
BOOL CFindDuplicatesDialog::SameContent = TRUE;

CFindDuplicatesDialog::CFindDuplicatesDialog(HWND hParent)
    : CCommonDialog(HLanguage, IDD_FIND_DUPLICATE, IDD_FIND_DUPLICATE, hParent)
{
}

void CFindDuplicatesDialog::Validate(CTransferInfo& ti)
{
    BOOL sameName = IsDlgButtonChecked(HWindow, IDC_FD_SAME_NAME);
    BOOL sameSize = IsDlgButtonChecked(HWindow, IDC_FD_SAME_SIZE);
    if (!sameName && !sameSize)
    {
        SalMessageBox(HWindow, LoadStr(IDS_FIND_DUPS_NO_OPTION), LoadStr(IDS_ERRORTITLE),
                      MB_OK | MB_ICONEXCLAMATION);
        ti.ErrorOn(IDC_FD_SAME_NAME);
    }
}

void CFindDuplicatesDialog::Transfer(CTransferInfo& ti)
{
    ti.CheckBox(IDC_FD_SAME_NAME, SameName);
    ti.CheckBox(IDC_FD_SAME_SIZE, SameSize);
    ti.CheckBox(IDC_FD_SAME_CONTENT, SameContent);

    if (ti.Type == ttDataToWindow)
        EnableControls();
}

void CFindDuplicatesDialog::EnableControls()
{
    BOOL sameSize = IsDlgButtonChecked(HWindow, IDC_FD_SAME_SIZE);
    if (!sameSize)
        CheckDlgButton(HWindow, IDC_FD_SAME_CONTENT, BST_UNCHECKED);
    EnableWindow(GetDlgItem(HWindow, IDC_FD_SAME_CONTENT), sameSize);
}

INT_PTR
CFindDuplicatesDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CALL_STACK_MESSAGE4("CFindDuplicatesDialog::DialogProc(0x%X, 0x%IX, 0x%IX)", uMsg, wParam, lParam);
    switch (uMsg)
    {
    case WM_COMMAND:
    {
        if (HIWORD(wParam) == BN_CLICKED)
            EnableControls();
        break;
    }
    }

    return CCommonDialog::DialogProc(uMsg, wParam, lParam);
}

