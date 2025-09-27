// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"

#include "tooltip.h"
#include "stswnd.h"
#include "plugins.h"
#include "fileswnd.h"
#include "filesbox.h"
#include "mainwnd.h"
#include "editwnd.h"
#include "cfgdlg.h"
#include "dialogs.h"
#include "usermenu.h"
#include "toolbar.h"
#include "shellib.h"
#include "menu.h"
#include "pack.h"
#include "gui.h"
#include "execute.h"
#include "jumplist.h"

#include "versinfo.rh2"

const char* SALAMANDER_TEXT_VERSION = "Open Salamander " VERSINFO_VERSION;

//****************************************************************************
//
// ToolTip's calls redirection
//

void SetCurrentToolTip(HWND hNotifyWindow, DWORD id, int showDelay)
{
    if (MainWindow != NULL && MainWindow->ToolTip != NULL)
        MainWindow->ToolTip->SetCurrentToolTip(hNotifyWindow, id, showDelay);
}

void SuppressToolTipOnCurrentMousePos()
{
    if (MainWindow != NULL && MainWindow->ToolTip != NULL)
        MainWindow->ToolTip->SuppressToolTipOnCurrentMousePos();
}

void RefreshToolTip()
{
    if (MainWindow != NULL && MainWindow->ToolTip != NULL)
        PostMessage(MainWindow->ToolTip->HWindow, WM_USER_REFRESHTOOLTIP, 0, 0); // pozadame okenko, aby nasalo novy text a znovu se vykreslilo
}

//****************************************************************************
//
// CHotPathItems
//

const char* SALAMANDER_HOTPATHS_NAME = "Name";
const char* SALAMANDER_HOTPATHS_PATH = "Path";
const char* SALAMANDER_HOTPATHS_VISIBLE = "Visible";

BOOL CHotPathItems::SwapItems(int index1, int index2)
{
    // CHotPathItem nema destruktor, muzeme takto primo priradit do lokani promenne
    CHotPathItem item = Items[index1];
    Items[index1] = Items[index2];
    Items[index2] = item;
    return TRUE;
}

void CHotPathItems::FillHotPathsMenu(CMenuPopup* menu, int minCommand, BOOL emptyItems, BOOL emptyEcho, BOOL customize, BOOL topSeparator, BOOL forAssign)
{
    CALL_STACK_MESSAGE1("CHotPathItems::FillMenu()");
    char root[MAX_PATH + 100];
    BOOL menuIsEmpty = TRUE;
    int firstIndex = menu->GetItemCount();
    MENU_ITEM_INFO mii;
    mii.Mask = MENU_MASK_TYPE | MENU_MASK_ID | MENU_MASK_STATE | MENU_MASK_STRING | MENU_MASK_ICON;
    mii.Type = MENU_TYPE_STRING;
    mii.State = 0;
    char name[MAX_PATH];
    int i;
    for (i = 0; i < HOT_PATHS_COUNT; i++)
    {
        BOOL assigned = GetPathLen(i) > 0 && GetNameLen(i) > 0;
        if (i >= 10)
        {
            if (emptyItems)
                emptyItems = FALSE; // od desate hot paths uz zobrazime pole setrepane
        }
        if (emptyItems || assigned)
        {
            menuIsEmpty = FALSE;
            GetName(i, name, MAX_PATH);
            DuplicateAmpersands(name, MAX_PATH);
            if (i < 10)
            {
                int key = (i == 9 ? 0 : i + 1);
                if (emptyItems)
                    sprintf(root, "%s\t%s+%s+%d", name, LoadStr(IDS_CTRL), LoadStr(IDS_SHIFT), key);
                else
                    sprintf(root, "%s\t%s+%d", name, LoadStr(IDS_CTRL), key);
            }
            else
                sprintf(root, "%s", name);
            mii.ID = minCommand + i;
            mii.String = root;
            mii.HIcon = assigned ? HFavoritIcon : NULL;
            menu->InsertItem( 0xffffffff, TRUE, &mii);
        }
    }
    int unassignedIndex = GetUnassignedHotPathIndex();
    if (forAssign && unassignedIndex != -1)
    {
        mii.ID = minCommand + unassignedIndex;
        mii.String = LoadStr(IDS_NEWHOTPATH);
        mii.HIcon = NULL;
        menu->InsertItem( 0xffffffff, TRUE, &mii);
    }

    if (!menuIsEmpty && topSeparator)
    {
        mii.Mask = MENU_MASK_TYPE;
        mii.Type = MENU_TYPE_SEPARATOR;
        menu->InsertItem(firstIndex, TRUE, &mii);
    }

    if (emptyEcho && !emptyItems && menuIsEmpty)
    {
        mii.Mask = MENU_MASK_TYPE | MENU_MASK_STATE | MENU_MASK_STRING;
        mii.Type = MENU_TYPE_STRING;
        mii.State = MENU_STATE_GRAYED;
        mii.String = LoadStr(IDS_EMPTYHOTPATHS);
        menu->InsertItem( 0xffffffff, TRUE, &mii);
    }

    if (customize)
    {
        // pripojime separator a moznost konfigurace
        mii.Mask = MENU_MASK_TYPE;
        mii.Type = MENU_TYPE_SEPARATOR;
        mii.HIcon = NULL;
        menu->InsertItem( 0xffffffff, TRUE, &mii);

        mii.Mask = MENU_MASK_TYPE | MENU_MASK_ID | MENU_MASK_STATE | MENU_MASK_STRING;
        mii.Type = MENU_TYPE_STRING;
        mii.State = 0;
        mii.ID = CM_CUSTOMIZE_HOTPATHS;
        mii.String = LoadStr(IDS_CUSTOMIZE_HOTPATHS);
        mii.HIcon = NULL;
        menu->InsertItem( 0xffffffff, TRUE, &mii);
    }
}

int CHotPathItems::GetUnassignedHotPathIndex()
{
    for (int i = 10; i < HOT_PATHS_COUNT; i++)
    {
        BOOL assigned = GetPathLen(i) > 0 && GetNameLen(i) > 0;
        if (!assigned)
            return i;
    }
    return -1;
}

BOOL CHotPathItems::CleanName(char* name)
{
    char* start = name;
    char* end = name + strlen(name) - 1;
    while (*start != 0 && *start == ' ')
        start++;
    while (end >= name && *end == ' ')
        end--;
    end++;
    *end = 0;
    if (start > name && start < end)
        memmove(name, start, end - start + 1);
    return strlen(name) > 0;
}

BOOL CHotPathItems::Save(HKEY hKey)
{
    char keyName[5];
    int i;
    for (i = 0; i < HOT_PATHS_COUNT; i++)
    {
        itoa(i + 1, keyName, 10);
        HKEY actKey;
        if (CreateKey(hKey, keyName, actKey))
        {
            const char* name = "";
            if (GetNameLen(i) > 0)
                name = Items[i].Name;
            const char* path = "";
            if (GetPathLen(i) > 0)
                path = Items[i].Path;
            DWORD visible = Items[i].Visible;

            if (*name == 0 && *path == 0 && visible == TRUE)
            {
                // optimalizace, nebudeme spinit registry pokud to neni nutne
                // neni pripravene na merge konfiguraci, ale to neni ani zbytek nasi konfigurace
                ClearKey(actKey);
                CloseKey(actKey);
                DeleteKey(hKey, keyName);
            }
            else
            {
                SetValue(actKey, SALAMANDER_HOTPATHS_NAME, REG_SZ, name, -1);
                SetValue(actKey, SALAMANDER_HOTPATHS_PATH, REG_SZ, path, -1);
                SetValue(actKey, SALAMANDER_HOTPATHS_VISIBLE, REG_DWORD, &visible, sizeof(DWORD));
                CloseKey(actKey);
            }
        }
    }
    return TRUE;
}

BOOL CHotPathItems::Load(HKEY hKey)
{
    char keyName[5];
    int i;
    for (i = 0; i <= HOT_PATHS_COUNT; i++)
    {
        itoa(i, keyName, 10);
        HKEY actKey;
        if (OpenKey(hKey, keyName, actKey))
        {
            // dokud bylo 10 hot paths, byla desata v registry pod klicem '0', takze ji zkusime nacist
            int index = (i == 0) ? 9 : i - 1;
            char name[MAX_PATH];
            char path[HOTPATHITEM_MAXPATH];
            DWORD visible;
            name[0] = 0;
            path[0] = 0;
            visible = TRUE;
            GetValue(actKey, SALAMANDER_HOTPATHS_NAME, REG_SZ, name, MAX_PATH);
            CleanName(name);
            if (GetValue(actKey, SALAMANDER_HOTPATHS_PATH, REG_SZ, path, HOTPATHITEM_MAXPATH))
            {
                if (Configuration.ConfigVersion < 47)            // stara cesta byla limitovana na MAX_PATH, takze se vejde i s expanzi
                    DuplicateDollars(path, HOTPATHITEM_MAXPATH); // pokud je cesta dlouha a obsahuje '$', muze dojit k oriznuti konce; neresim
            }
            GetValue(actKey, SALAMANDER_HOTPATHS_VISIBLE, REG_DWORD, &visible, sizeof(DWORD));

            Set(index, name, path, visible);
            CloseKey(actKey);
        }
    }
    return TRUE;
}

BOOL CHotPathItems::Load1_52(HKEY hKey)
{
    // konvert z verze 1.52 na 1.6
    char keyName[5];
    int i;
    for (i = 0; i < HOT_PATHS_COUNT; i++)
    {
        char name[MAX_PATH];
        char path[MAX_PATH];
        DWORD visible;
        name[0] = 0;
        path[0] = 0;
        visible = FALSE; // zkonvertovane cesty ukazovat nebudeme - jsou dlouhe

        itoa(i, keyName, 10);
        if (GetValue(hKey, keyName, REG_SZ, path, MAX_PATH))
        {
            DuplicateDollars(path, MAX_PATH); // pokud je cesta dlouha a obsahuje '$', muze dojit k oriznuti konce; neresim
            strcpy(name, path);
        }

        Set(i == 0 ? 9 : i - 1, name, path, visible);
    }
    return TRUE;
}

//
// ****************************************************************************
// CMainWindow
//

CMainWindow::CMainWindow() : ChangeNotifArray(3, 5)
{
    HANDLES(InitializeCriticalSection(&DispachChangeNotifCS));
    LastDispachChangeNotifTime = 0;
    NeedToResentDispachChangeNotif = FALSE;
    DoNotLoadAnyPlugins = FALSE;

    ActivateSuspMode = 0;
    FirstActivateApp = TRUE;
    UserMenuItems = new CUserMenuItems(10, 5);
    ViewerMasks = new CViewerMasks(10, 5);
    HANDLES(InitializeCriticalSection(&ViewerMasksCS));
    AltViewerMasks = new CViewerMasks(10, 5);
    EditorMasks = new CEditorMasks(10, 5);
    HighlightMasks = new CHighlightMasks(10, 5);
    DetachedFSList = new CDetachedFSList;
    DirHistory = new CPathHistory(TRUE);
    CanAddToDirHistory = FALSE;
    FileHistory = new CFileHistory;
    LeftPanel = RightPanel = NULL;
    SetActivePanel(NULL);
    EditWindow = NULL;
    EditMode = FALSE;
    HelpMode = HELP_INACTIVE;
    EditPermanentVisible = FALSE;
    TopToolBar = NULL;
    PluginsBar = NULL;
    MiddleToolBar = NULL;
    UMToolBar = NULL;
    HPToolBar = NULL;
    DriveBar = NULL;
    DriveBar2 = NULL;
    BottomToolBar = NULL;
    //AnimateBar = NULL;
    //  TipOfTheDayDialog = NULL;
    HTopRebar = NULL;
    MenuBar = NULL;
    WindowWidth = WindowHeight = EditHeight = 0;
    SplitPosition = 0.5; // split je v pulce
    BeforeZoomSplitPosition = 0.5;
    DragMode = FALSE;
    ContextMenuNew = new CMenuNew;
    ContextMenuChngDrv = NULL;
    TaskbarRestartMsg = 0;
    CanClose = FALSE;
    CanCloseButInEndSuspendMode = FALSE;
    SaveCfgInEndSession = FALSE;
    WaitInEndSession = FALSE;
    DisableIdleProcessing = FALSE;
    strcpy(SelectionMask, "*.*");
    Created = FALSE;
    //  DrivesControlHWnd = NULL;
    HDisabledKeyboard = NULL;
    CmdShow = SW_SHOWNORMAL;
    IdleStatesChanged = TRUE;
    PluginsStatesChanged = FALSE;
    CaptionIsActive = FALSE;
    SHChangeNotifyRegisterID = 0;
    IgnoreWM_SETTINGCHANGE = FALSE;
    LockedUI = FALSE;
    LockedUIToolWnd = NULL;
    LockedUIReason = NULL;

    ToolTip = new CToolTip(ooStatic);

    // viewers
    CViewerMasksItem* item;

    item = new CViewerMasksItem();
    if (ViewerMasks != NULL && item != NULL)
    {
        ViewerMasks->Add(item); // kriticka sekce neni treba, jsme v konstruktoru
        item->Set("*.htm;*.html;*.xml;*.mht", "", "", "");
        item->ViewerType = -4; // IE viewer (4. plug-in v def. konfiguraci)
    }

    item = new CViewerMasksItem();
    if (ViewerMasks != NULL && item != NULL)
    {
        ViewerMasks->Add(item); // kriticka sekce neni treba, jsme v konstruktoru
        item->Set("*.rpm", "", "", "");
        item->ViewerType = -2; // TAR (2. plug-in v def. konfiguraci)
    }

    item = new CViewerMasksItem();
    if (ViewerMasks != NULL && item != NULL)
    {
        ViewerMasks->Add(item); // kriticka sekce neni treba, jsme v konstruktoru
        item->Set("*.*", "", "", "");
        item->ViewerType = VIEWER_INTERNAL; // interni viewer
    }

    item = new CViewerMasksItem();
    if (AltViewerMasks != NULL && item != NULL)
    {
        AltViewerMasks->Add(item);
        item->Set("*.*", "", "", "");
        item->ViewerType = VIEWER_INTERNAL; // interni viewer
    }

    CEditorMasksItem* eItem;
    eItem = new CEditorMasksItem();
    if (EditorMasks != NULL && eItem != NULL)
    {
        EditorMasks->Add(eItem);
        eItem->Set("*.*", "notepad.exe", "\"$(Name)\"", "$(FullPath)");
    }

    if (HighlightMasks != NULL)
    {
        CHighlightMasksItem* hItem = new CHighlightMasksItem();
        if (hItem != NULL)
        {
            HighlightMasks->Add(hItem);
            hItem->Set("*.*");
            int errPos;
            hItem->Masks->PrepareMasks(errPos);
            hItem->NormalFg = RGBF(0, 0, 255, 0);
            hItem->FocusedFg = RGBF(0, 0, 255, 0);
            hItem->ValidAttr = FILE_ATTRIBUTE_COMPRESSED;
            hItem->Attr = FILE_ATTRIBUTE_COMPRESSED;
        }

        hItem = new CHighlightMasksItem();
        if (hItem != NULL)
        {
            HighlightMasks->Add(hItem);
            hItem->Set("*.*");
            int errPos;
            hItem->Masks->PrepareMasks(errPos);
            hItem->NormalFg = RGBF(19, 143, 13, 0); // barva vzata z Windows XP
            hItem->FocusedFg = RGBF(19, 143, 13, 0);
            hItem->ValidAttr = FILE_ATTRIBUTE_ENCRYPTED;
            hItem->Attr = FILE_ATTRIBUTE_ENCRYPTED;
        }
    }
}

BOOL CMainWindow::IsGood()
{
    return LeftPanel != NULL && LeftPanel->IsGood() &&
           RightPanel != NULL && RightPanel->IsGood() &&
           EditWindow != NULL && EditWindow->IsGood() &&
           UserMenuItems != NULL && ViewerMasks != NULL &&
           AltViewerMasks != NULL && EditorMasks != NULL &&
           HighlightMasks != NULL && ContextMenuNew != NULL &&
           ToolTip != NULL && DetachedFSList != NULL &&
           FileHistory != NULL && DirHistory != NULL;
}

CMainWindow::~CMainWindow()
{
    HANDLES(DeleteCriticalSection(&DispachChangeNotifCS));
    if (FileHistory != NULL)
        delete FileHistory;
    if (DetachedFSList != NULL)
        delete DetachedFSList;
    if (DirHistory != NULL)
        delete DirHistory;
    if (UserMenuItems != NULL)
        delete UserMenuItems;
    if (ViewerMasks != NULL)
        delete ViewerMasks;
    HANDLES(DeleteCriticalSection(&ViewerMasksCS));
    if (AltViewerMasks != NULL)
        delete AltViewerMasks;
    if (EditorMasks != NULL)
        delete EditorMasks;
    if (HighlightMasks != NULL)
        delete HighlightMasks;
    if (ContextMenuNew != NULL)
        delete ContextMenuNew;
    if (ToolTip != NULL)
        delete ToolTip;
    MainWindow = NULL;
}

void CMainWindow::ClearHistory()
{
    if (FileHistory != NULL)
        FileHistory->ClearHistory();

    if (DirHistory != NULL)
        DirHistory->ClearHistory();
    // zmizime sipcicky v DirLine
    if (LeftPanel != NULL)
        LeftPanel->DirectoryLine->SetHistory(FALSE);
    if (RightPanel != NULL)
        RightPanel->DirectoryLine->SetHistory(FALSE);
}

void CMainWindow::UpdateDefaultDir(BOOL activePrefered)
{
    CPanelWindow *active, *nonactive;
    if (activePrefered)
    {
        nonactive = GetActivePanel();
        active = GetNonActivePanel();
    }
    else
    {
        active = GetActivePanel();
        nonactive = GetNonActivePanel();
    }
    const char* pathActive = active->GetPath();
    if (!active->Is(ptPluginFS) && pathActive[0] != '\\')
    {
        strcpy(DefaultDir[LowerCase[pathActive[0]] - 'a'], pathActive);
    }
    const char* pathPasive = nonactive->GetPath();
    if (!nonactive->Is(ptPluginFS) && pathPasive[0] != '\\')
    {
        strcpy(DefaultDir[LowerCase[pathPasive[0]] - 'a'], pathPasive);
    }
}

BOOL CMainWindow::ToggleTopToolBar(BOOL storePos)
{
    CALL_STACK_MESSAGE2("CMainWindow::ToggleTopToolBar(%d)", storePos);
    if (TopToolBar == NULL)
        return FALSE;

    LockWindowUpdate(HWindow);

    if (TopToolBar->HWindow != NULL)
    {
        TopToolBar->Save(Configuration.TopToolBar);
        int index = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_TOPTOOLBAR, 0);
        SendMessage(HTopRebar, RB_DELETEBAND, index, 0);
        //    InvalidateRect(HTopRebar, NULL, TRUE);
        //    UpdateWindow(HTopRebar);
        DestroyWindow(TopToolBar->HWindow);
        Configuration.TopToolBarVisible = FALSE;
        if (storePos)
            StoreBandsPos();
    }
    else
    {
        if (!TopToolBar->CreateWnd(HTopRebar))
            return FALSE;
        TopToolBar->Load(Configuration.TopToolBar);
        IdleForceRefresh = TRUE;  // forcneme update
        IdleRefreshStates = TRUE; // pri pristim Idle vynutime kontrolu stavovych promennych
        InsertTopToolbarBand();
        ShowWindow(TopToolBar->HWindow, SW_SHOW);
        Configuration.TopToolBarVisible = TRUE;
        if (storePos)
            StoreBandsPos();
    }

    LockWindowUpdate(NULL);

    return TRUE;
}

BOOL CMainWindow::TogglePluginsBar(BOOL storePos)
{
    CALL_STACK_MESSAGE2("CMainWindow::TogglePluginsBar(%d)", storePos);
    if (PluginsBar == NULL)
        return FALSE;

    LockWindowUpdate(HWindow);

    if (PluginsBar->HWindow != NULL)
    {
        int index = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_PLUGINSBAR, 0);
        SendMessage(HTopRebar, RB_DELETEBAND, index, 0);
        DestroyWindow(PluginsBar->HWindow);
        Configuration.PluginsBarVisible = FALSE;
        if (storePos)
            StoreBandsPos();
    }
    else
    {
        if (!PluginsBar->CreateWnd(HTopRebar))
            return FALSE;
        //    IdleForceRefresh = TRUE;   // forcneme update
        //    IdleRefreshStates = TRUE;  // pri pristim Idle vynutime kontrolu stavovych promennych
        PluginsBar->CreatePluginButtons();
        InsertPluginsBarBand();
        ShowWindow(PluginsBar->HWindow, SW_SHOW);
        Configuration.PluginsBarVisible = TRUE;
        if (storePos)
            StoreBandsPos();
    }

    LockWindowUpdate(NULL);

    return TRUE;
}

BOOL CMainWindow::ToggleMiddleToolBar()
{
    CALL_STACK_MESSAGE1("CMainWindow::ToggleMiddleToolBar()");
    if (MiddleToolBar == NULL)
        return FALSE;

    LockWindowUpdate(HWindow);

    if (MiddleToolBar->HWindow != NULL)
    {
        MiddleToolBar->Save(Configuration.MiddleToolBar);
        DestroyWindow(MiddleToolBar->HWindow);
        Configuration.MiddleToolBarVisible = FALSE;
    }
    else
    {
        if (!MiddleToolBar->CreateWnd(HWindow))
            return FALSE;
        MiddleToolBar->Load(Configuration.MiddleToolBar);
        IdleForceRefresh = TRUE;  // forcneme update
        IdleRefreshStates = TRUE; // pri pristim Idle vynutime kontrolu stavovych promennych
        ShowWindow(MiddleToolBar->HWindow, SW_SHOW);
        Configuration.MiddleToolBarVisible = TRUE;
    }

    LockWindowUpdate(NULL);

    return TRUE;
}

BOOL CMainWindow::ToggleUserMenuToolBar(BOOL storePos)
{
    CALL_STACK_MESSAGE2("CMainWindow::ToggleUserMenuToolBar(%d)", storePos);
    if (UMToolBar == NULL)
        return FALSE;

    LockWindowUpdate(HWindow);

    if (UMToolBar->HWindow != NULL)
    {
        int index = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_UMTOOLBAR, 0);
        SendMessage(HTopRebar, RB_DELETEBAND, index, 0);
        //    InvalidateRect(HTopRebar, NULL, TRUE);
        //    UpdateWindow(HTopRebar);
        DestroyWindow(UMToolBar->HWindow);
        Configuration.UserMenuToolBarVisible = FALSE;
        if (storePos)
            StoreBandsPos();
    }
    else
    {
        if (!UMToolBar->CreateWnd(HTopRebar))
            return FALSE;
        UMToolBar->CreateButtons();
        UMToolBar->SetFont();
        InsertUMToolbarBand();
        ShowWindow(UMToolBar->HWindow, SW_SHOW);
        Configuration.UserMenuToolBarVisible = TRUE;
        if (storePos)
            StoreBandsPos();
    }

    LockWindowUpdate(NULL);

    return TRUE;
}

BOOL CMainWindow::ToggleHotPathsBar(BOOL storePos)
{
    CALL_STACK_MESSAGE2("CMainWindow::ToggleHotPathsBar(%d)", storePos);
    if (HPToolBar == NULL)
        return FALSE;

    LockWindowUpdate(HWindow);

    if (HPToolBar->HWindow != NULL)
    {
        int index = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_HPTOOLBAR, 0);
        SendMessage(HTopRebar, RB_DELETEBAND, index, 0);
        DestroyWindow(HPToolBar->HWindow);
        Configuration.HotPathsBarVisible = FALSE;
        if (storePos)
            StoreBandsPos();
    }
    else
    {
        if (!HPToolBar->CreateWnd(HTopRebar))
            return FALSE;
        HPToolBar->CreateButtons();
        HPToolBar->SetFont();
        InsertHPToolbarBand();
        ShowWindow(HPToolBar->HWindow, SW_SHOW);
        Configuration.HotPathsBarVisible = TRUE;
        if (storePos)
            StoreBandsPos();
    }

    LockWindowUpdate(NULL);

    return TRUE;
}

BOOL CMainWindow::ToggleDriveBar(BOOL twoDriveBars, BOOL storePos)
{
    CALL_STACK_MESSAGE3("CMainWindow::ToggleDriveBar(%d, %d)", twoDriveBars, storePos);
    if (DriveBar == NULL || DriveBar2 == NULL)
        return FALSE;

    LockWindowUpdate(HWindow);

    BOOL forceShow = TRUE;
    BOOL twoBarsVisible = (DriveBar2->HWindow != NULL);

    if (DriveBar->HWindow != NULL)
    {
        forceShow = FALSE;
        int index = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_DRIVEBAR, 0);
        SendMessage(HTopRebar, RB_DELETEBAND, index, 0);
        DestroyWindow(DriveBar->HWindow);
        Configuration.DriveBarVisible = FALSE;
    }
    if (DriveBar2->HWindow != NULL)
    {
        forceShow = FALSE;
        int index = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_DRIVEBAR2, 0);
        SendMessage(HTopRebar, RB_DELETEBAND, index, 0);
        DestroyWindow(DriveBar2->HWindow);
        Configuration.DriveBar2Visible = FALSE;
    }

    if (forceShow || twoBarsVisible != twoDriveBars)
    {
        if (!DriveBar->CreateWnd(HTopRebar))
            return FALSE;
        if (twoDriveBars && !DriveBar2->CreateWnd(HTopRebar))
            return FALSE;

        DriveBar->CreateDriveButtons(NULL);
        DriveBar->SetFont();

        if (twoDriveBars)
        {
            DriveBar2->CreateDriveButtons(DriveBar);
            DriveBar2->SetFont();
        }

        InsertDriveBarBand(twoDriveBars);

        ShowWindow(DriveBar->HWindow, SW_SHOW);
        Configuration.DriveBarVisible = TRUE;

        if (twoDriveBars)
        {
            ShowWindow(DriveBar2->HWindow, SW_SHOW);
            Configuration.DriveBar2Visible = TRUE;
        }
        else
            Configuration.DriveBar2Visible = FALSE;
        if (storePos)
            StoreBandsPos();
    }

    LockWindowUpdate(NULL);
    //  InvalidateRect(HTopRebar, NULL, TRUE);
    //  UpdateWindow(HTopRebar);
    return TRUE;
}

BOOL CMainWindow::ToggleBottomToolBar()
{
    CALL_STACK_MESSAGE1("CMainWindow::ToggleBottomToolBar()");
    if (BottomToolBar == NULL)
        return FALSE;
    if (BottomToolBar->HWindow != NULL)
    {
        DestroyWindow(BottomToolBar->HWindow);
        BottomToolBar->SetState(btbsCount); // pri pristim zobrazeni nalejeme nejaky validni stav
        Configuration.BottomToolBarVisible = FALSE;
        return TRUE;
    }
    else
    {
        if (!CBottomToolBar::InitDataFromResources())
            return FALSE;
        if (!BottomToolBar->CreateWnd(HWindow))
            return FALSE;
        BottomToolBar->SetFont();
        UpdateBottomToolBar();
        ShowWindow(BottomToolBar->HWindow, SW_SHOW);
        Configuration.BottomToolBarVisible = TRUE;
        return TRUE;
    }
}

void CMainWindow::ToggleToolBarGrips()
{
    CALL_STACK_MESSAGE1("CMainWindow::ToggleToolBarGrips()");

    Configuration.GripsVisible = !Configuration.GripsVisible;

    LockWindowUpdate(HWindow);

    // menu
    int index = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_MENU, 0);
    SendMessage(HTopRebar, RB_DELETEBAND, index, 0);
    InsertMenuBand();

    // top toolbar
    index = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_TOPTOOLBAR, 0);
    if (index != -1)
    {
        SendMessage(HTopRebar, RB_DELETEBAND, index, 0);
        InsertTopToolbarBand();
    }

    // plugin bar
    index = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_PLUGINSBAR, 0);
    if (index != -1)
    {
        SendMessage(HTopRebar, RB_DELETEBAND, index, 0);
        InsertPluginsBarBand();
    }

    // user menu bar
    index = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_UMTOOLBAR, 0);
    if (index != -1)
    {
        SendMessage(HTopRebar, RB_DELETEBAND, index, 0);
        InsertUMToolbarBand();
    }

    // hot path bar
    index = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_HPTOOLBAR, 0);
    if (index != -1)
    {
        SendMessage(HTopRebar, RB_DELETEBAND, index, 0);
        InsertHPToolbarBand();
    }

    // drive bar
    // pouze pokud je jeden bar; jinak gripy nemaji
    if (DriveBar->HWindow != NULL && DriveBar2->HWindow == NULL)
    {
        index = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_DRIVEBAR, 0);
        SendMessage(HTopRebar, RB_DELETEBAND, index, 0);
        InsertDriveBarBand(FALSE);
    }

    LockWindowUpdate(NULL);
}

void CMainWindow::StoreBandsPos()
{
    CALL_STACK_MESSAGE1("CMainWindow::StoreBandsPos()");
    // ulozim rozlozeni v rebaru
    REBARBANDINFO rbbi;

    rbbi.cbSize = sizeof(rbbi);
    rbbi.fMask = RBBIM_STYLE | RBBIM_SIZE;
    Configuration.MenuIndex = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_MENU, 0);
    SendMessage(HTopRebar, RB_GETBANDINFO,
                Configuration.MenuIndex, (LPARAM)&rbbi);
    Configuration.MenuBreak = (rbbi.fStyle & RBBS_BREAK) != 0;
    Configuration.MenuWidth = rbbi.cx;

    if (TopToolBar->HWindow != NULL)
    {
        rbbi.cbSize = sizeof(rbbi);
        rbbi.fMask = RBBIM_STYLE | RBBIM_SIZE;
        Configuration.TopToolbarIndex = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_TOPTOOLBAR, 0);
        SendMessage(HTopRebar, RB_GETBANDINFO,
                    Configuration.TopToolbarIndex, (LPARAM)&rbbi);
        Configuration.TopToolbarBreak = (rbbi.fStyle & RBBS_BREAK) != 0;
        Configuration.TopToolbarWidth = rbbi.cx;
    }
    if (PluginsBar->HWindow != NULL)
    {
        rbbi.cbSize = sizeof(rbbi);
        rbbi.fMask = RBBIM_STYLE | RBBIM_SIZE;
        Configuration.PluginsBarIndex = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_PLUGINSBAR, 0);
        SendMessage(HTopRebar, RB_GETBANDINFO,
                    Configuration.PluginsBarIndex, (LPARAM)&rbbi);
        Configuration.PluginsBarBreak = (rbbi.fStyle & RBBS_BREAK) != 0;
        Configuration.PluginsBarWidth = rbbi.cx;
    }
    if (UMToolBar->HWindow != NULL)
    {
        rbbi.cbSize = sizeof(rbbi);
        rbbi.fMask = RBBIM_STYLE | RBBIM_SIZE;
        Configuration.UserMenuToolbarIndex = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_UMTOOLBAR, 0);
        SendMessage(HTopRebar, RB_GETBANDINFO,
                    Configuration.UserMenuToolbarIndex, (LPARAM)&rbbi);
        Configuration.UserMenuToolbarBreak = (rbbi.fStyle & RBBS_BREAK) != 0;
        Configuration.UserMenuToolbarWidth = rbbi.cx;
    }
    if (HPToolBar->HWindow != NULL)
    {
        rbbi.cbSize = sizeof(rbbi);
        rbbi.fMask = RBBIM_STYLE | RBBIM_SIZE;
        Configuration.HotPathsBarIndex = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_HPTOOLBAR, 0);
        SendMessage(HTopRebar, RB_GETBANDINFO,
                    Configuration.HotPathsBarIndex, (LPARAM)&rbbi);
        Configuration.HotPathsBarBreak = (rbbi.fStyle & RBBS_BREAK) != 0;
        Configuration.HotPathsBarWidth = rbbi.cx;
    }
    if (DriveBar->HWindow != NULL && DriveBar2->HWindow == NULL)
    {
        rbbi.cbSize = sizeof(rbbi);
        rbbi.fMask = RBBIM_STYLE | RBBIM_SIZE;
        Configuration.DriveBarIndex = (int)SendMessage(HTopRebar, RB_IDTOINDEX, BANDID_DRIVEBAR, 0);
        SendMessage(HTopRebar, RB_GETBANDINFO,
                    Configuration.DriveBarIndex, (LPARAM)&rbbi);
        Configuration.DriveBarBreak = (rbbi.fStyle & RBBS_BREAK) != 0;
        Configuration.DriveBarWidth = rbbi.cx;
    }
}

BOOL CMainWindow::InsertMenuBand()
{
    CALL_STACK_MESSAGE1("CMainWindow::InsertMenuBand()");

    REBARBANDINFO rbbi;
    ZeroMemory(&rbbi, sizeof(rbbi));

    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE |
                 RBBIM_ID;
    rbbi.cxMinChild = 10;
    rbbi.cyMinChild = MenuBar->GetNeededHeight();
    rbbi.cx = Configuration.MenuWidth;
    if (Configuration.MenuBreak)
        rbbi.fStyle |= RBBS_BREAK;
    if (Configuration.GripsVisible)
        rbbi.fStyle |= RBBS_GRIPPERALWAYS;
    else
    {
        rbbi.fStyle |= RBBS_NOGRIPPER;
        // abychom nebyli tak nalepeni na strane
        rbbi.fMask |= RBBIM_HEADERSIZE;
        rbbi.cxHeader = 2;
    }
    rbbi.hwndChild = MenuBar->HWindow;
    rbbi.wID = BANDID_MENU;

    int count = (int)SendMessage(HTopRebar, RB_GETBANDCOUNT, 0, 0);
    if (count >= 2 && DriveBar2 != NULL && DriveBar2->HWindow != NULL)
        count -= 2;
    if (Configuration.MenuIndex > count)
        Configuration.MenuIndex = count;

    SendMessage(HTopRebar, RB_INSERTBAND,
                (WPARAM)Configuration.MenuIndex, (LPARAM)&rbbi);

    return TRUE;
}

BOOL CMainWindow::CreateAndInsertWorkerBand()
{
    /*
  CALL_STACK_MESSAGE1("CMainWindow::CreateAndInsertWorkerBand()");

  REBARBANDINFO  rbbi;
  ZeroMemory(&rbbi, sizeof(rbbi));

  rbbi.cbSize       = sizeof(REBARBANDINFO);
  rbbi.fMask        = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE | RBBIM_ID;
  SIZE sz;
  AnimateBar->GetFrameSize(&sz);
  rbbi.cxMinChild   = sz.cx + 10;
  rbbi.cyMinChild   = sz.cy;
  rbbi.fStyle = RBBS_FIXEDSIZE | RBBS_NOGRIPPER | RBBS_VARIABLEHEIGHT;

  AnimateBar->Create(CWINDOW_CLASSNAME2, "",
                     WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                     0, 0, 0, 0,
                     HTopRebar,
                     NULL,
                     HInstance,
                     AnimateBar);

  rbbi.hwndChild    = AnimateBar->HWindow;
  rbbi.wID          = BANDID_WORKER;

//  SendMessage(HTopRebar, RB_INSERTBAND, (WPARAM)1, (LPARAM)&rbbi);
*/
    return TRUE;
}

BOOL CMainWindow::InsertTopToolbarBand()
{
    CALL_STACK_MESSAGE1("CMainWindow::InsertTopToolbarBand()");
    REBARBANDINFO rbbi;
    ZeroMemory(&rbbi, sizeof(rbbi));

    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE |
                 RBBIM_ID;
    rbbi.cxMinChild = 10;
    rbbi.cyMinChild = TopToolBar->GetNeededHeight();
    rbbi.cx = Configuration.TopToolbarWidth;
    if (Configuration.TopToolbarBreak)
        rbbi.fStyle |= RBBS_BREAK;
    if (Configuration.GripsVisible)
        rbbi.fStyle |= RBBS_GRIPPERALWAYS;
    else
    {
        rbbi.fStyle |= RBBS_NOGRIPPER;
        // abychom nebyli tak nalepeni na strane
        rbbi.fMask |= RBBIM_HEADERSIZE;
        rbbi.cxHeader = 2;
    }
    rbbi.hwndChild = TopToolBar->HWindow;
    rbbi.wID = BANDID_TOPTOOLBAR;

    int count = (int)SendMessage(HTopRebar, RB_GETBANDCOUNT, 0, 0);
    if (count >= 2 && DriveBar2 != NULL && DriveBar2->HWindow != NULL)
        count -= 2;
    if (Configuration.TopToolbarIndex > count)
        Configuration.TopToolbarIndex = count;
    SendMessage(HTopRebar, RB_INSERTBAND,
                (WPARAM)Configuration.TopToolbarIndex, (LPARAM)&rbbi);
    return TRUE;
}

BOOL CMainWindow::InsertPluginsBarBand()
{
    CALL_STACK_MESSAGE1("CMainWindow::InsertPluginsBarBand()");
    REBARBANDINFO rbbi;
    ZeroMemory(&rbbi, sizeof(rbbi));

    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE |
                 RBBIM_ID;
    rbbi.cxMinChild = 10;
    rbbi.cyMinChild = PluginsBar->GetNeededHeight();
    rbbi.cx = Configuration.PluginsBarWidth;
    if (Configuration.PluginsBarBreak)
        rbbi.fStyle |= RBBS_BREAK;
    if (Configuration.GripsVisible)
        rbbi.fStyle |= RBBS_GRIPPERALWAYS;
    else
    {
        rbbi.fStyle |= RBBS_NOGRIPPER;
        // abychom nebyli tak nalepeni na strane
        rbbi.fMask |= RBBIM_HEADERSIZE;
        rbbi.cxHeader = 2;
    }
    rbbi.hwndChild = PluginsBar->HWindow;
    rbbi.wID = BANDID_PLUGINSBAR;

    int count = (int)SendMessage(HTopRebar, RB_GETBANDCOUNT, 0, 0);
    if (count >= 2 && DriveBar2 != NULL && DriveBar2->HWindow != NULL)
        count -= 2;
    if (Configuration.PluginsBarIndex > count)
        Configuration.PluginsBarIndex = count;
    SendMessage(HTopRebar, RB_INSERTBAND,
                (WPARAM)Configuration.PluginsBarIndex, (LPARAM)&rbbi);
    return TRUE;
}

BOOL CMainWindow::InsertUMToolbarBand()
{
    CALL_STACK_MESSAGE1("CMainWindow::InsertUMToolbarBand()");
    REBARBANDINFO rbbi;
    ZeroMemory(&rbbi, sizeof(rbbi));

    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE |
                 RBBIM_ID;
    rbbi.cxMinChild = 10;
    rbbi.cyMinChild = UMToolBar->GetNeededHeight();
    rbbi.cx = Configuration.UserMenuToolbarWidth;
    if (Configuration.UserMenuToolbarBreak)
        rbbi.fStyle |= RBBS_BREAK;
    if (Configuration.GripsVisible)
        rbbi.fStyle |= RBBS_GRIPPERALWAYS;
    else
    {
        rbbi.fStyle |= RBBS_NOGRIPPER;
        // abychom nebyli tak nalepeni na strane
        rbbi.fMask |= RBBIM_HEADERSIZE;
        rbbi.cxHeader = 2;
    }
    rbbi.hwndChild = UMToolBar->HWindow;
    rbbi.wID = BANDID_UMTOOLBAR;

    int count = (int)SendMessage(HTopRebar, RB_GETBANDCOUNT, 0, 0);
    if (count >= 2 && DriveBar2 != NULL && DriveBar2->HWindow != NULL)
        count -= 2;
    if (Configuration.UserMenuToolbarIndex > count)
        Configuration.UserMenuToolbarIndex = count;

    SendMessage(HTopRebar, RB_INSERTBAND,
                (WPARAM)Configuration.UserMenuToolbarIndex, (LPARAM)&rbbi);
    return TRUE;
}

BOOL CMainWindow::InsertHPToolbarBand()
{
    CALL_STACK_MESSAGE1("CMainWindow::InsertHPToolbarBand()");
    REBARBANDINFO rbbi;
    ZeroMemory(&rbbi, sizeof(rbbi));

    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE |
                 RBBIM_ID;
    rbbi.cxMinChild = 10;
    rbbi.cyMinChild = HPToolBar->GetNeededHeight();
    rbbi.cx = Configuration.HotPathsBarWidth;
    if (Configuration.HotPathsBarBreak)
        rbbi.fStyle |= RBBS_BREAK;
    if (Configuration.GripsVisible)
        rbbi.fStyle |= RBBS_GRIPPERALWAYS;
    else
    {
        rbbi.fStyle |= RBBS_NOGRIPPER;
        // abychom nebyli tak nalepeni na strane
        rbbi.fMask |= RBBIM_HEADERSIZE;
        rbbi.cxHeader = 2;
    }
    rbbi.hwndChild = HPToolBar->HWindow;
    rbbi.wID = BANDID_HPTOOLBAR;

    int count = (int)SendMessage(HTopRebar, RB_GETBANDCOUNT, 0, 0);
    if (count >= 2 && DriveBar2 != NULL && DriveBar2->HWindow != NULL)
        count -= 2;
    if (Configuration.HotPathsBarIndex > count)
        Configuration.HotPathsBarIndex = count;

    SendMessage(HTopRebar, RB_INSERTBAND,
                (WPARAM)Configuration.HotPathsBarIndex, (LPARAM)&rbbi);
    return TRUE;
}

BOOL CMainWindow::InsertDriveBarBand(BOOL twoDriveBars)
{
    CALL_STACK_MESSAGE2("CMainWindow::InsertDriveBarBand(%d)", twoDriveBars);
    REBARBANDINFO rbbi;
    ZeroMemory(&rbbi, sizeof(rbbi));

    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE |
                 RBBIM_ID;
    rbbi.cyMinChild = DriveBar->GetNeededHeight();
    rbbi.cx = Configuration.DriveBarWidth;
    if (twoDriveBars)
    {
        rbbi.fMask |= RBBIM_HEADERSIZE;
        rbbi.fStyle = RBBS_NOGRIPPER | RBBS_BREAK;
        rbbi.cxHeader = 0; // pozor, tato hodnota se nastavuje jeste na jednom miste
        rbbi.cxMinChild = 0;
    }
    else
    {
        if (Configuration.DriveBarBreak)
            rbbi.fStyle |= RBBS_BREAK;
        if (Configuration.GripsVisible)
            rbbi.fStyle |= RBBS_GRIPPERALWAYS;
        else
            rbbi.fStyle |= RBBS_NOGRIPPER;
        rbbi.cxMinChild = 10;
    }
    rbbi.hwndChild = DriveBar->HWindow;
    rbbi.wID = BANDID_DRIVEBAR;

    int count = (int)SendMessage(HTopRebar, RB_GETBANDCOUNT, 0, 0);

    if (twoDriveBars)
    {
        SendMessage(HTopRebar, RB_INSERTBAND, (WPARAM)count, (LPARAM)&rbbi);
        rbbi.wID = BANDID_DRIVEBAR2;
        rbbi.fStyle = RBBS_NOGRIPPER;
        rbbi.hwndChild = DriveBar2->HWindow;
        SendMessage(HTopRebar, RB_INSERTBAND, (WPARAM)count + 1, (LPARAM)&rbbi);
    }
    else
    {
        if (Configuration.DriveBarIndex > count)
            Configuration.DriveBarIndex = count;
        SendMessage(HTopRebar, RB_INSERTBAND,
                    (WPARAM)Configuration.DriveBarIndex, (LPARAM)&rbbi);
    }
    return TRUE;
}

void CMainWindow::FocusLeftPanel()
{
    FocusPanel(LeftPanel);
    LeftPanel->SetCaretIndex(0, FALSE);
}

BOOL CMainWindow::EditWindowKnowHWND(HWND hwnd)
{
    return EditWindow->KnowHWND(hwnd);
}

void CMainWindow::EditWindowSetDirectory()
{
    SetWindowTitle(); // aktualni adresar do title bar
    CPanelWindow* panel = GetActivePanel();
    if (panel != NULL &&
        (panel->Is(ptDisk) ||
         panel->Is(ptPluginFS) && panel->GetPluginFS()->NotEmpty() &&
             panel->GetPluginFS()->IsServiceSupported(FS_SERVICE_COMMANDLINE)))
    {
        char dir[2 * MAX_PATH];
        panel->GetGeneralPath(dir, 2 * MAX_PATH);
        EditWindow->Enable(TRUE); // cached in EditWindow
        EditWindow->SetDirectory(dir);
    }
    else // disable/hide edit-line
    {
        if (EditMode && panel != NULL) // sysvobodime focus z commanline pred jejim disablenim
            FocusPanel(panel, TRUE);
        EditWindow->Enable(FALSE); // cached in EditWindow
        EditWindow->SetDirectory("");
    }
}

HWND CMainWindow::GetEditLineHWND(BOOL disableSkip)
{
    if (EditWindow == NULL || EditWindow->EditLine == NULL)
        return NULL;
    if (disableSkip)
        EditWindow->EditLine->SkipCharacter = FALSE;
    return EditWindow->EditLine->HWindow;
}

HWND CMainWindow::GetActivePanelHWND()
{
    return GetActivePanel()->HWindow;
}

int CMainWindow::GetDirectoryLineHeight()
{
    if (GetActivePanel()->DirectoryLine != NULL &&
        GetActivePanel()->DirectoryLine->HWindow != NULL)
    {
        RECT r;
        GetClientRect(GetActivePanel()->DirectoryLine->HWindow, &r);
        return r.bottom - r.top;
    }
    return 0;
}

void CMainWindow::RefreshDiskFreeSpace()
{
    LeftPanel->RefreshDiskFreeSpace(TRUE, TRUE);
    RightPanel->RefreshDiskFreeSpace(TRUE, TRUE);
}

void CMainWindow::RefreshDirs()
{
    LeftPanel->ChangePathToDisk(LeftPanel->HWindow, LeftPanel->GetPath());
    RightPanel->ChangePathToDisk(RightPanel->HWindow, RightPanel->GetPath());
}

// pro predani cesty do konfiguracniho dialogu
char HotPathSetBufferName[MAX_PATH];
char HotPathSetBufferPath[HOTPATHITEM_MAXPATH];

void CMainWindow::SetUnescapedHotPath(int index, const char* path)
{
    if (Configuration.HotPathAutoConfig)
    {
        // prejdeme na buffer, aby slapal Cancel
        lstrcpyn(HotPathSetBufferName, path, MAX_PATH);
        lstrcpyn(HotPathSetBufferPath, path, HOTPATHITEM_MAXPATH);
        DuplicateDollars(HotPathSetBufferPath, HOTPATHITEM_MAXPATH);
        // nechame vybalit stranku HotPaths a rozeditovat polozku index
        PostMessage(HWindow, WM_USER_CONFIGURATION, 1, index);
    }
    else
    {
        // napereme hodnotu primo
        char buff[HOTPATHITEM_MAXPATH];
        lstrcpyn(buff, path, HOTPATHITEM_MAXPATH);
        char nameBuff[MAX_PATH];
        lstrcpyn(nameBuff, path, MAX_PATH);
        DuplicateDollars(buff, HOTPATHITEM_MAXPATH);
        HotPaths.Set(index, nameBuff, buff);
        // doslo ke zmene, nechame prestavet Hot Path Bar
        if (HPToolBar != NULL && HPToolBar->HWindow != NULL)
            HPToolBar->CreateButtons();
        if (Windows7AndLater)
            CreateJumpList();
    }
}

BOOL CMainWindow::GetExpandedHotPath(HWND hParent, int index, char* buffer, int bufferSize)
{
    // buffer by mel byt 2 * MAX_PATH veliky
    if (bufferSize != 2 * MAX_PATH)
        TRACE_E("CMainWindow::GetExpandedHotPath: invalid buffer size!");

    // pokud neni cesta definovana, muzeme rovnou vypadnout
    int pathLen = HotPaths.GetPathLen(index);
    if (pathLen == 0)
        return FALSE;

    // vytahneme cestu k nam
    char* path = (char*)malloc(pathLen + 1);
    HotPaths.GetPath(index, path, pathLen + 1);

    // provedeme validaci
    int errorPos1, errorPos2;
    if (!ValidateHotPath(hParent, path, errorPos1, errorPos2))
    {
        free(path);
        return FALSE;
    }

    // na zaver provedeme expanzi
    BOOL ret = ExpandHotPath(hParent, path, buffer, bufferSize, FALSE);
    free(path);
    return ret;
}

int CMainWindow::GetUnassignedHotPathIndex()
{
    return HotPaths.GetUnassignedHotPathIndex();
}

// font pro nase GUI (panel muze mit font definovatelny v konfiguraci)
BOOL GetSystemGUIFont(LOGFONT* lf)
{
    if (!SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), lf, 0))
    {
        // kdyby nahodou selhalo SystemParametersInfo, pouzijeme nahradni reseni
        NONCLIENTMETRICS ncm;
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
        *lf = ncm.lfMessageFont;
        lf->lfWeight = FW_NORMAL;
    }
    return TRUE;
}

// font pro tooltips
BOOL GetSystemTooltipFont(LOGFONT* lf)
{
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
    *lf = ncm.lfStatusFont;
    return TRUE;
}

BOOL CreatePanelFont()
{
    CALL_STACK_MESSAGE1("CreatePanelFont()");

    if (Font != NULL)
        HANDLES(DeleteObject(Font));

    LOGFONT lf;
    if (UseCustomPanelFont)
        lf = LogFont; // uzivatel nastavil vlastni font
    else
        GetSystemGUIFont(&lf); // vytahneme font ze systemu

    Font = HANDLES(CreateFontIndirect(&lf));
    if (Font == NULL)
    {
        TRACE_E("Unable to create panel font.");
        return FALSE;
    }

    // vytvorim podtrzenou variantu
    BYTE oldUnderline = lf.lfUnderline;
    lf.lfUnderline = TRUE;
    if (FontUL != NULL)
        HANDLES(DeleteObject(FontUL));
    FontUL = HANDLES(CreateFontIndirect(&lf));
    lf.lfUnderline = oldUnderline;
    if (FontUL == NULL)
    {
        TRACE_E("Unable to create underlined panel font.");
        return FALSE;
    }

    HDC dc = HANDLES(GetDC(NULL));
    HFONT oldFont = (HFONT)SelectObject(dc, Font);
    TEXTMETRIC tm;
    GetTextMetrics(dc, &tm);
    FontCharHeight = tm.tmHeight;
    SIZE sz;
    GetTextExtentPoint32(dc, "...", 3, &sz);
    TextEllipsisWidth = sz.cx;
    SelectObject(dc, oldFont);
    HANDLES(ReleaseDC(NULL, dc));
    return TRUE;
}

BOOL CreateEnvFonts()
{
    LOGFONT lf;
    GetSystemGUIFont(&lf);

    if (EnvFont != NULL)
        HANDLES(DeleteObject(EnvFont));
    EnvFont = HANDLES(CreateFontIndirect(&lf));
    if (EnvFont == NULL)
    {
        TRACE_E("Unable to create font.");
        return FALSE;
    }

    // vytvorim podtrzenou variantu
    lf.lfUnderline = TRUE;
    if (EnvFontUL != NULL)
        HANDLES(DeleteObject(EnvFontUL));
    EnvFontUL = HANDLES(CreateFontIndirect(&lf));
    if (EnvFontUL == NULL)
    {
        TRACE_E("Unable to create font.");
        return FALSE;
    }

    HDC dc = HANDLES(GetDC(NULL));
    HFONT oldFont = (HFONT)SelectObject(dc, EnvFont);
    TEXTMETRIC tm;
    GetTextMetrics(dc, &tm);
    EnvFontCharHeight = tm.tmHeight;
    SIZE sz;
    GetTextExtentPoint32(dc, "...", 3, &sz);
    TextEllipsisWidthEnv = sz.cx;
    SelectObject(dc, oldFont);
    HANDLES(ReleaseDC(NULL, dc));

    LOGFONT toolLF;
    GetSystemTooltipFont(&toolLF);
    if (TooltipFont != NULL)
        HANDLES(DeleteObject(TooltipFont));
    TooltipFont = HANDLES(CreateFontIndirect(&toolLF));
    if (TooltipFont == NULL)
    {
        TRACE_E("Unable to create font.");
        return FALSE;
    }

    return TRUE;
}

void CMainWindow::SetFont()
{
    CALL_STACK_MESSAGE1("CMainWindow::SetFont()");

    CreatePanelFont();

    if (IsWindowVisible(HWindow))
    {
        RECT r;
        GetClientRect(HWindow, &r);
        PostMessage(HWindow, WM_SIZE, SIZE_RESTORED,
                    MAKELONG(r.right - r.left, r.bottom - r.top));
        HANDLES(EnterCriticalSection(&TimeCounterSection));
        int t1 = MyTimeCounter++;
        int t2 = MyTimeCounter++;
        HANDLES(LeaveCriticalSection(&TimeCounterSection));
        if (LeftPanel != NULL)
            PostMessage(LeftPanel->HWindow, WM_USER_REFRESH_DIR, 0, t1);
        if (RightPanel != NULL)
            PostMessage(RightPanel->HWindow, WM_USER_REFRESH_DIR, 0, t2);
        InvalidateRect(HWindow, NULL, FALSE);
    }
}

void CMainWindow::SetEnvFont()
{
    CALL_STACK_MESSAGE1("CMainWindow::SetEnvFont()");

    CreateEnvFonts();

    if (MenuBar != NULL)
        MenuBar->SetFont();
    if (EditWindow != NULL)
        EditWindow->SetFont();
    if (UMToolBar != NULL)
        UMToolBar->SetFont();
    if (HPToolBar != NULL)
        HPToolBar->SetFont();
    if (DriveBar != NULL)
        DriveBar->SetFont();
    if (DriveBar2 != NULL)
        DriveBar2->SetFont();
    if (BottomToolBar != NULL)
        BottomToolBar->SetFont();

    if (HTopRebar != NULL)
    {
        REBARBANDINFO rbbi;
        rbbi.cbSize = sizeof(REBARBANDINFO);
        rbbi.fMask = RBBIM_CHILDSIZE;
        rbbi.cxMinChild = 10;
        rbbi.cxMinChild = 10;
        if (MenuBar != NULL)
        {
            rbbi.cyMinChild = MenuBar->GetNeededHeight();
            SendMessage(HTopRebar, RB_SETBANDINFO, (WPARAM)Configuration.MenuIndex, (LPARAM)&rbbi);
        }
        if (DriveBar != NULL && DriveBar->HWindow != NULL)
        {
            rbbi.cyMinChild = DriveBar->GetNeededHeight();
            int index;
            if (DriveBar2 != NULL && DriveBar2->HWindow != NULL)
                index = (int)SendMessage(HTopRebar, RB_GETBANDCOUNT, 0, 0) - 2;
            else
                index = Configuration.DriveBarIndex;
            SendMessage(HTopRebar, RB_SETBANDINFO, (WPARAM)index, (LPARAM)&rbbi);
        }
        if (DriveBar2 != NULL && DriveBar2->HWindow != NULL)
        {
            rbbi.cyMinChild = DriveBar2->GetNeededHeight();
            int index = (int)SendMessage(HTopRebar, RB_GETBANDCOUNT, 0, 0) - 1;
            SendMessage(HTopRebar, RB_SETBANDINFO, (WPARAM)index, (LPARAM)&rbbi);
        }
        if (UMToolBar != NULL && UMToolBar->HWindow != NULL)
        {
            rbbi.cyMinChild = UMToolBar->GetNeededHeight();
            SendMessage(HTopRebar, RB_SETBANDINFO, (WPARAM)Configuration.UserMenuToolbarIndex, (LPARAM)&rbbi);
        }
        if (HPToolBar != NULL && HPToolBar->HWindow != NULL)
        {
            rbbi.cyMinChild = HPToolBar->GetNeededHeight();
            SendMessage(HTopRebar, RB_SETBANDINFO, (WPARAM)Configuration.HotPathsBarIndex, (LPARAM)&rbbi);
        }
    }

    if (LeftPanel != NULL)
        LeftPanel->SetFont();
    if (RightPanel != NULL)
        RightPanel->SetFont();

    if (IsWindowVisible(HWindow))
    {
        RECT r;
        GetClientRect(HWindow, &r);
        PostMessage(HWindow, WM_SIZE, SIZE_RESTORED,
                    MAKELONG(r.right - r.left, r.bottom - r.top));
        HANDLES(EnterCriticalSection(&TimeCounterSection));
        int t1 = MyTimeCounter++;
        int t2 = MyTimeCounter++;
        HANDLES(LeaveCriticalSection(&TimeCounterSection));
        if (LeftPanel != NULL)
        {
            LeftPanel->SleepIconCacheThread();
            LeftPanel->IconCache->Release();
            LeftPanel->EndOfIconReadingTime = GetTickCount() - 10000;
            LeftPanel->UseThumbnails = FALSE;
            PostMessage(LeftPanel->HWindow, WM_USER_REFRESH_DIR, 0, t1);
        }
        if (RightPanel != NULL)
        {
            RightPanel->SleepIconCacheThread();
            RightPanel->IconCache->Release();
            RightPanel->EndOfIconReadingTime = GetTickCount() - 10000;
            RightPanel->UseThumbnails = FALSE;
            PostMessage(RightPanel->HWindow, WM_USER_REFRESH_DIR, 0, t2);
        }
        InvalidateRect(HWindow, NULL, FALSE);
    }
}

void CMainWindow::FillUserMenu2(CMenuPopup* menu, int* iterator, int max)
{
    MENU_ITEM_INFO mii;
    int added = 0;
    for (; *iterator < max; (*iterator)++)
    {
        switch (UserMenuItems->At(*iterator)->Type)
        {
        case umitSeparator:
        {
            mii.Mask = MENU_MASK_TYPE;
            mii.Type = MENU_TYPE_SEPARATOR;
            menu->InsertItem( 0xffffffff, TRUE, &mii);
            break;
        }

        case umitItem:
        {
            mii.Mask = MENU_MASK_TYPE | MENU_MASK_ID | MENU_MASK_STATE | MENU_MASK_STRING | MENU_MASK_ICON;
            mii.State = 0;
            mii.Type = MENU_TYPE_STRING;
            mii.ID = CM_USERMENU_MIN + *iterator;
            mii.String = UserMenuItems->At(*iterator)->ItemName;
            mii.HIcon = UserMenuItems->At(*iterator)->UMIcon;
            menu->InsertItem( 0xffffffff, TRUE, &mii);
            added++;
            break;
        }

        case umitSubmenuBegin:
        {
            CMenuPopup* popup = new CMenuPopup();
            if (popup == NULL)
            {
                TRACE_E(LOW_MEMORY);
                return;
            }
            mii.Mask = MENU_MASK_TYPE | MENU_MASK_STRING | MENU_MASK_ICON | MENU_MASK_SUBMENU;
            mii.Type = MENU_TYPE_STRING;
            mii.SubMenu = popup;
            mii.String = UserMenuItems->At(*iterator)->ItemName;
            mii.HIcon = UserMenuItems->At(*iterator)->UMIcon;
            menu->InsertItem( 0xffffffff, TRUE, &mii);
            // rekurze
            (*iterator)++;
            FillUserMenu2(popup, iterator, max);
            added++;
            break;
        }

        case umitSubmenuEnd:
        {
            goto ESCAPE;
        }
        }
    }
ESCAPE:
    if (added == 0)
    {
        mii.Mask = MENU_MASK_TYPE | MENU_MASK_STATE | MENU_MASK_STRING;
        mii.Type = MENU_TYPE_STRING;
        mii.State = MENU_STATE_GRAYED;
        mii.String = LoadStr(IDS_EMPTYUSERMENU);
        menu->InsertItem( 0xffffffff, TRUE, &mii);
    }
}

void CMainWindow::FillUserMenu(CMenuPopup* menu, BOOL customize)
{
    int max = min(CM_USERMENU_MAX - CM_USERMENU_MIN, UserMenuItems->Count);

    int iterator = 0;
    FillUserMenu2(menu, &iterator, max);

    if (customize)
    {
        // pripojime separator a moznost konfigurace
        MENU_ITEM_INFO mii;
        mii.Mask = MENU_MASK_TYPE;
        mii.Type = MENU_TYPE_SEPARATOR;
        menu->InsertItem( 0xffffffff, TRUE, &mii);
        mii.Mask = MENU_MASK_TYPE | MENU_MASK_ID | MENU_MASK_STATE | MENU_MASK_STRING | MENU_MASK_ICON;
        mii.Type = MENU_TYPE_STRING;
        mii.State = 0;
        mii.ID = CM_CUSTOMIZE_USERMENU;
        mii.String = LoadStr(IDS_CUSTOMIZE_HOTPATHS);
        mii.HIcon = NULL;
        menu->InsertItem( 0xffffffff, TRUE, &mii);
    }
}

/*
void
CMainWindow::ReleaseMenuNew()
{
  CALL_STACK_MESSAGE1("CMainWindow::ReleaseMenuNew()");
  if (ContextMenuNew != NULL) ContextMenuNew->Release();

  MENUITEMINFO mi;
  mi.cbSize = sizeof(mi);
  mi.fMask = MIIM_STATE | MIIM_SUBMENU;
  mi.fState = MFS_GRAYED;
  mi.hSubMenu = NULL;
  SetMenuItemInfo(GetSubMenu(Menu->HMenu, FILES_MENU_INDEX), CM_NEWMENU, FALSE, &mi);
}
*/

void CMainWindow::LayoutWindows()
{
    RECT r;
    GetClientRect(HWindow, &r);

    SendMessage(HWindow, WM_SIZE, SIZE_RESTORED, MAKELONG(r.right - r.left, r.bottom - r.top));
}

void CMainWindow::AddTrayIcon(BOOL updateIcon)
{
    CALL_STACK_MESSAGE1("CMainWindow::AddTrayIcon()");

    NOTIFYICONDATA tnid;
    tnid.cbSize = sizeof(NOTIFYICONDATA);
    tnid.hWnd = HWindow;
    tnid.uID = TASKBAR_ICON_ID;
    tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    tnid.uCallbackMessage = WM_USER_ICON_NOTIFY;
    int resID = MainWindowIcons[Configuration.GetMainWindowIconIndex()].IconResID;
    tnid.hIcon = SalLoadIcon(HInstance, resID, IconSizes[IconSize::size_16x16]);
    lstrcpyn(tnid.szTip, MAINWINDOW_NAME, sizeof(tnid.szTip));
    Shell_NotifyIcon(updateIcon ? NIM_MODIFY : NIM_ADD, &tnid);
    HANDLES(DestroyIcon(tnid.hIcon));
}

void CMainWindow::RemoveTrayIcon()
{
    CALL_STACK_MESSAGE1("CMainWindow::RemoveTrayIcon()");
    NOTIFYICONDATA tnid;
    tnid.cbSize = sizeof(NOTIFYICONDATA);
    tnid.hWnd = HWindow;
    tnid.uID = TASKBAR_ICON_ID;
    tnid.uFlags = 0;
    Shell_NotifyIcon(NIM_DELETE, &tnid);
}

void CMainWindow::SetTrayIconText(const char* text)
{
    CALL_STACK_MESSAGE1("CMainWindow::SetTrayIconText()");
    if (!Configuration.StatusArea)
    {
        TRACE_E("CMainWindow::SetTrayIconText(): !Configuration.StatusArea");
        return;
    }
    NOTIFYICONDATA tnid;
    tnid.cbSize = sizeof(NOTIFYICONDATA);
    tnid.hWnd = HWindow;
    tnid.uID = TASKBAR_ICON_ID;
    tnid.uFlags = NIF_TIP;
    lstrcpyn(tnid.szTip, text, sizeof(tnid.szTip));
    Shell_NotifyIcon(NIM_MODIFY, &tnid);
}

void CMainWindow::GetFormatedPathForTitle(char* path)
{
    path[0] = 0;
    int titleBarMode = Configuration.TitleBarMode;
    // plugin FS bez podpory pro ziskavani cesty do titulku okna umi zobrazit jen Full Path
    CPanelWindow* panel = GetActivePanel();
    if (panel == NULL)
    {
        path[0] = 0;
        return;
    }
    if (panel->Is(ptPluginFS) &&
        !panel->GetPluginFS()->IsServiceSupported(FS_SERVICE_GETPATHFORMAINWNDTITLE))
    {
        titleBarMode = TITLE_BAR_MODE_FULLPATH;
    }
    switch (titleBarMode)
    {
    case TITLE_BAR_MODE_COMPOSITE:
    {
        if (!panel->Is(ptPluginFS) ||
            !panel->GetPluginFS()->GetPathForMainWindowTitle(panel->GetPluginFS()->GetPluginFSName(),
                                                             2, path, 2 * MAX_PATH))
        {
            // mame zobrazit "root\...\aktualni adresar"
            panel->GetGeneralPath(path, 2 * MAX_PATH);
            if (path[0] != 0)
            {
                char* trimStart = NULL; // misto kam vlozim "...", za ktere pripojim 'trimEnd'
                char* trimEnd = NULL;
                if (panel->Is(ptDisk) || panel->Is(ptZIPArchive))
                {
                    char rootPath[MAX_PATH];
                    GetRootPath(rootPath, path);
                    int chars = (int)strlen(rootPath);
                    // izolovali jsme root
                    trimStart = path + chars;
                    while (path[chars] != 0)
                    {
                        // hledame posledni komponentu, kterou chceme zachovat
                        if (path[chars] == '\\' && path[chars + 1] != 0)
                            trimEnd = path + chars;
                        chars++;
                    }
                }
                else
                {
                    if (panel->Is(ptPluginFS))
                    {
                        int chars = 0;
                        int pathLen = (int)strlen(path);
                        // pokud FS nepodporuje FS_SERVICE_GETNEXTDIRLINEHOTPATH, dostaneme FALSE a zobrazime plnou cestu
                        if (panel->GetPluginFS()->GetNextDirectoryLineHotPath(path, pathLen, chars) &&
                            chars < pathLen) // konec cesty neni delici bod, chyba implementace GetNextDirectoryLineHotPath
                        {
                            // izolovali jsme root
                            trimStart = path + chars;
                            int lastChars = chars;
                            while (panel->GetPluginFS()->GetNextDirectoryLineHotPath(path, pathLen, chars))
                            {
                                // hledame posledni komponentu, kterou chceme zachovat
                                if (chars < pathLen)
                                    lastChars = chars;
                                else
                                    break; // konec cesty neni delici bod, chyba implementace GetNextDirectoryLineHotPath
                            }
                            trimEnd = path + lastChars;
                        }
                    }
                }
                // orezeme cestu
                if (trimStart != NULL && trimEnd != NULL && trimEnd > trimStart)
                {
                    memmove(trimStart + 3, trimEnd, strlen(trimEnd) + 1);
                    memcpy(trimStart, "...", 3);
                }
            }
        }
        break;
    }

    case TITLE_BAR_MODE_DIRECTORY:
    {
        if (!panel->Is(ptPluginFS) ||
            !panel->GetPluginFS()->GetPathForMainWindowTitle(panel->GetPluginFS()->GetPluginFSName(),
                                                             1, path, MAX_PATH))
        {
            // mame zobrazit pouze aktualni adresar
            panel->GetGeneralPath(path, 2 * MAX_PATH);
            if (path[0] != 0)
            {
                if (panel->Is(ptDisk) || panel->Is(ptZIPArchive))
                {
                    char rootPath[MAX_PATH];
                    GetRootPath(rootPath, path);
                    int chars = (int)strlen(rootPath);
                    char* p = path + strlen(path);
                    if (*p == '\\')
                        p--;
                    while (p > path && *p != '\\')
                        p--;
                    if (*(p + 1) != 0 && p + 1 >= path + chars)
                        memmove(path, p + 1, strlen(p + 1) + 1);
                }
                else
                {
                    if (panel->Is(ptPluginFS))
                    {
                        int chars = 0;
                        int pathLen = (int)strlen(path);
                        int lastChars = 0;
                        // pokud FS nepodporuje FS_SERVICE_GETNEXTDIRLINEHOTPATH, dostaneme FALSE a zobrazime plnou cestu
                        while (panel->GetPluginFS()->GetNextDirectoryLineHotPath(path, pathLen, chars))
                        {
                            if (chars < pathLen)
                                lastChars = chars;
                            else
                                break; // konec cesty neni delici bod, chyba implementace GetNextDirectoryLineHotPath
                        }
                        if (lastChars > 0)
                        {
                            char* p = path + lastChars;
                            if (*p == '/' || *p == '\\')
                                p++;
                            memmove(path, p, strlen(p) + 1);
                        }
                    }
                }
            }
        }
        break;
    }

    case TITLE_BAR_MODE_FULLPATH:
    {
        // vracime plnou cestu
        panel->GetGeneralPath(path, 2 * MAX_PATH);
        break;
    }

    default:
    {
        TRACE_E("Configuration.TitleBarMode = " << Configuration.TitleBarMode);
    }
    }
}

void CMainWindow::SetWindowTitle(const char* text)
{
    CALL_STACK_MESSAGE2("CMainWindow::SetWindowTitle(%s)", text);
    char buff[1000];
    ::GetWindowText(HWindow, buff, 1000);
    buff[999] = 0;

    char stdWndName[2 * MAX_PATH + 300];
    if (text == NULL)
    {
        // dodame implicitni obsah
        stdWndName[0] = 0;

        // prefix
        if (Configuration.UseTitleBarPrefixForced)
        {
            strcpy(stdWndName, Configuration.TitleBarPrefixForced);
            if (stdWndName[0] != 0)
                strcat(stdWndName, " - ");
        }
        else
        {
            if (Configuration.UseTitleBarPrefix)
            {
                strcpy(stdWndName, Configuration.TitleBarPrefix);
                if (stdWndName[0] != 0)
                    strcat(stdWndName, " - ");
            }
        }

        // path
        if (Configuration.TitleBarShowPath)
        {
            GetFormatedPathForTitle(stdWndName + strlen(stdWndName));
            if (stdWndName[0] != 0)
                strcat(stdWndName, " - ");
        }

        // Open Salamander name + ver
        lstrcat(stdWndName, MAINWINDOW_NAME);
        lstrcat(stdWndName, " " VERSINFO_VERSION);

        if (RunningAsAdmin)
            sprintf(stdWndName + lstrlen(stdWndName), " (%s)", LoadStr(IDS_AS_ADMIN_TITLE));

#ifdef X64_STRESS_TEST
        lstrcat(stdWndName, " ST");
#endif //X64_STRESS_TEST

#ifdef USE_BETA_EXPIRATION_DATE
        // beta version Expires on
        lstrcat(stdWndName, " - Expires on ");
        char expire[100];
        if (GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &BETA_EXPIRATION_DATE, NULL, expire, 100) == 0)
            sprintf(expire, "%u.%u.%u", BETA_EXPIRATION_DATE.wDay, BETA_EXPIRATION_DATE.wMonth, BETA_EXPIRATION_DATE.wYear);
        lstrcat(stdWndName, expire);
#endif // USE_BETA_EXPIRATION_DATE

        text = stdWndName;
    }

    if (strcmp(text, buff) != 0)
    {
        ::SetWindowText(HWindow, text);
        if (Configuration.StatusArea)
            SetTrayIconText(text);
    }
}

void CMainWindow::SetWindowIcon()
{
    int resID = MainWindowIcons[Configuration.GetMainWindowIconIndex()].IconResID;
    // priradim oknu ikonku
    SendMessage(HWindow, WM_SETICON, ICON_BIG,
                (LPARAM)HANDLES(LoadIcon(HInstance, MAKEINTRESOURCE(resID))));

    if (Configuration.StatusArea)
        AddTrayIcon(TRUE);
}

// btbsCount == prazdna toolbara
CBottomTBStateEnum VirtKeyStateTable[2][2][2] =
    {
        {
            {btbsNormal, btbsCtrl},
            {btbsAlt, btbsCount},
        },
        {
            {btbsShift, btbsCtrlShift},
            {btbsAltShift, btbsCount},
        }};

void CMainWindow::UpdateBottomToolBar()
{
    if (BottomToolBar == NULL || BottomToolBar->HWindow == NULL)
        return;

    DWORD shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0 ? 1 : 0;
    DWORD altPressed = (GetKeyState(VK_MENU) & 0x8000) != 0 ? 1 : 0;
    DWORD ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0 ? 1 : 0;

    CBottomTBStateEnum newState = btbsCount;
    if (CaptionIsActive)
    {
        if (MenuBar != NULL && MenuBar->IsInMenuLoop())
            newState = btbsMenu;
        else
            newState = VirtKeyStateTable[shiftPressed][altPressed][ctrlPressed];
    }
    else
        newState = btbsNormal;

    BottomToolBar->SetState(newState);
    BottomToolBar->UpdateItemsState();
}

CMainWindow::MouseLocation::Value CMainWindow::MouseLocation( POINT point )
{
//Was it clicked inside the main window?
    //Get main window rect.
    //
    //Notice:
    //Upper-left is always (0, 0).
    RECT    rect_main;

    GetClientRect( HWindow, &rect_main );

    //Convert main window rect to screen coordinates.
    RECT    rect_main_screen = rect_main;

    MapWindowPoints( HWindow, NULL, (POINT*)&rect_main_screen, 2 );

    //Was point clicked inside the main window area?
    MouseLocation::Value    location = MouseLocation::Value::none;

    if ( PtInRect( &rect_main_screen, point ) == 0 )
    {
    //Mouse was NOT clicked inside main window -> ignore.
        return location;
    }

//Which window was clicked?
    //Convert position relative to client area.
    ScreenToClient( HWindow, &point );

    //Which window was clicked?
    //
    //Notice:
    //Order of tests is on assumption was is clicked most of the time e.g. file context menu is probably the most often clicked,...
    if ( PtInChild( LeftPanel->HWindow, point ) != 0 )
    {
    //Left panel -> which windows was clicked?
        if ( PtInChild( LeftPanel->DirectoryLine->HWindow, point ) != 0 )
        {
        //Directory line
            location = MouseLocation::panel_left_dirLine;
        }
        else if ( PtInChild( LeftPanel->GetHeaderLineHWND(), point) != 0 )
        {
        //Header line
            location = MouseLocation::panel_left_headerLine;
        }
        else if ( PtInChild( LeftPanel->StatusLine->HWindow, point) != 0 )
        {
        //Status line
            location = MouseLocation::panel_left_statusLine;
        }
        else
        {
        //Working area
            location = MouseLocation::panel_left_workingArea;
        }
    }
    else if ( PtInChild( RightPanel->HWindow, point ) != 0 )
    {
    //Right panel -> which windows was clicked?
        if ( PtInChild( RightPanel->DirectoryLine->HWindow, point ) != 0 )
        {
        //Directory line
            location = MouseLocation::panel_right_dirLine;
        }
        else if ( PtInChild( RightPanel->GetHeaderLineHWND(), point) != 0 )
        {
        //Header line
            location = MouseLocation::panel_right_headerLine;
        }
        else if ( PtInChild( RightPanel->StatusLine->HWindow, point) != 0 )
        {
        //Status line
            location = MouseLocation::panel_right_statusLine;
        }
        else
        {
        //Working area
            location = MouseLocation::panel_right_workingArea;
        }
    }
    else if ( PtInChild( HTopRebar, point ) != 0 )
    {
    //Top rebar -> was it one of the toolbars or a rebar it self?
        RBHITTESTINFO    hti = { .pt = point };

        if (
            ( SendMessage( HTopRebar, RB_HITTEST, 0, (LPARAM)&hti ) == -1 )
            ||
            ( hti.flags == RBHT_NOWHERE )
            ||
            ( hti.iBand == -1 )
        )
        {
        //Click was on the rebar it self.
            location = MouseLocation::rebar_top;
        }
        else
        {
        //One of the toolbars was clicked -> which one?
            //Get toolbar id.
            REBARBANDINFO   rbi = { .cbSize = sizeof( REBARBANDINFO ), .fMask = RBBIM_ID };

            SendMessage( HTopRebar, RB_GETBANDINFO, hti.iBand, (LPARAM)&rbi );

            //Convert to enum.
            switch ( rbi.wID )
            {
            case BANDID_MENU: location = MouseLocation::menu; break;
            case BANDID_TOPTOOLBAR: location = MouseLocation::toolbar_top; break;
            case BANDID_PLUGINSBAR: location = MouseLocation::toolbar_plugins; break;
            case BANDID_UMTOOLBAR: location = MouseLocation::toolbar_userMenu; break;
            case BANDID_HPTOOLBAR: location = MouseLocation::toolbar_hotPaths; break;
            case BANDID_DRIVEBAR: location = MouseLocation::toolbar_drive; break;
            case BANDID_DRIVEBAR2: location = MouseLocation::toolbar_drive; break;
            case BANDID_WORKER: location = MouseLocation::worker; break;
            default:
            {
                TRACE_E( "Unknown band in rebar id = " << rbi.wID );
                location = MouseLocation::rebar_top;
            }
            }
        }
    }
    else if ( PtInChild( MiddleToolBar->HWindow, point ) != 0 )
    {
    //Middle toolbar
        location = MouseLocation::toolbar_middle;
    }
    else if ( PtInChild( EditWindow->HWindow, point ) )
    {
    //Command line window
        location = MouseLocation::commandLine;
    }
    else if ( PtInChild( BottomToolBar->HWindow, point ) != 0 )
    {
    //Bottom toolbar
        location = MouseLocation::toolbar_bottom;
    }
    else
    {
    //Was it maybe split line?
        RECT    rect_splitLine;

        GetSplitRect( rect_splitLine );

        if ( PtInRect( &rect_splitLine, point ) != 0 )
        {
            location = MouseLocation::splitLine_middleToolbar;
        }
    }

    return location;
}

void CMainWindow::OnWmContextMenu( const HWND hWnd, const POINT point_screen )
{
    CALL_STACK_MESSAGE3( "CMainWindow::OnWmContextMenu(, %d, %d)", point_screen.x, point_screen.y );

//Which window was clicked?
    const auto      location = MouseLocation( point_screen );

    switch ( location )
    {
    case MouseLocation::commandLine:
    case MouseLocation::menu:
    case MouseLocation::rebar_top:
    case MouseLocation::toolbar_bottom:
    case MouseLocation::toolbar_drive:
    case MouseLocation::toolbar_hotPaths:
    case MouseLocation::toolbar_middle:
    case MouseLocation::toolbar_plugins:
    case MouseLocation::toolbar_userMenu:
    case MouseLocation::toolbar_top:
    case MouseLocation::worker:
    {
    //Main window child.
        OnWmContextMenu_Main( hWnd, point_screen, location );
        break;
    }
    case MouseLocation::panel_left_dirLine:
    case MouseLocation::panel_left_headerLine:
    case MouseLocation::panel_left_statusLine:
    case MouseLocation::panel_left_workingArea:
    case MouseLocation::panel_right_dirLine:
    case MouseLocation::panel_right_headerLine:
    case MouseLocation::panel_right_statusLine:
    case MouseLocation::panel_right_workingArea:
    {
    //Left or right panel.
        OnWmContextMenu_Panel( hWnd, point_screen, location );
        break;
    }
    case MouseLocation::splitLine_middleToolbar:
    {
    //Split line at middle toolbar.
        OnWmContextMenu_SplitLine( point_screen );
        break;
    }
    }
}
void CMainWindow::OnWmContextMenu_Main( const HWND hWnd, const POINT& point_screen, MouseLocation::Value location )
{
//Initialize menu.
    CMenuPopup      menu;

    menu.SetImageList( HGrayToolBarImageList, TRUE );
    menu.SetHotImageList( HHotToolBarImageList, TRUE );

    //Initialize menu item info structures.
    MENU_ITEM_INFO  mii_item =
    {
        .Mask = MENU_MASK_TYPE | MENU_MASK_ID | MENU_MASK_STATE | MENU_MASK_STRING | MENU_MASK_SUBMENU | MENU_MASK_IMAGEINDEX,
        .Type = MENU_TYPE_STRING,
        .State = 0,
        .SubMenu = NULL,
        .ImageIndex = -1
    };
    MENU_ITEM_INFO  mii_separator =
    {
        .Mask = MENU_MASK_TYPE,
        .Type = MENU_TYPE_SEPARATOR
    };

//Insert menu items.
    struct MenuItem_Id
    {
        public: enum Value
        {
            commandLine = 1,                //Has to start with 1 as 0 is used as error.

            customize_toolbar_hotPaths,
            customize_toolbar_middle,
            customize_toolbar_plugins,
            customize_toolbar_top,
            customize_toolbar_userMenu,

            lock_toolbars,
            show_labels,

            toolbar_bottom,
            toolbar_drive_double,
            toolbar_drive_single,
            toolbar_hotPaths,
            toolbar_middle,
            toolbar_plugins,
            toolbar_top,
            toolbar_userMenu,
        };
    };

    //Top toolbar checkbox.
    mii_item.String = LoadStr( IDS_TOPTOOLBAR );
    mii_item.ID = MenuItem_Id::toolbar_top;
    mii_item.State = TopToolBar->HWindow != NULL ? MENU_STATE_CHECKED : 0;
    menu.InsertItem( 0xffffffff, TRUE, &mii_item );

    //Plugins toolbar checkbox.
    mii_item.String = LoadStr(IDS_PLUGINSBAR);
    mii_item.ID = MenuItem_Id::toolbar_plugins;
    mii_item.State = PluginsBar->HWindow != NULL ? MENU_STATE_CHECKED : 0;
    menu.InsertItem( 0xffffffff, TRUE, &mii_item );

    //User menu toolbar checkbox.
    mii_item.String = LoadStr(IDS_UMTOOLBAR);
    mii_item.ID = MenuItem_Id::toolbar_userMenu;
    mii_item.State = UMToolBar->HWindow != NULL ? MENU_STATE_CHECKED : 0;
    menu.InsertItem( 0xffffffff, TRUE, &mii_item );

    //Hot paths toolbar checkbox.
    mii_item.String = LoadStr(IDS_HPTOOLBAR);
    mii_item.ID = MenuItem_Id::toolbar_hotPaths;
    mii_item.State = HPToolBar->HWindow != NULL ? MENU_STATE_CHECKED : 0;
    menu.InsertItem( 0xffffffff, TRUE, &mii_item );

    //'Single' drive toolbar checkbox.
    mii_item.String = LoadStr(IDS_DRIVEBAR);
    mii_item.ID = MenuItem_Id::toolbar_drive_single;
    mii_item.State = (DriveBar->HWindow != NULL && DriveBar2->HWindow == NULL) ? MENU_STATE_CHECKED : 0;
    menu.InsertItem( 0xffffffff, TRUE, &mii_item );

    //'Double' drive toolbar checkbox.
    mii_item.String = LoadStr(IDS_DRIVEBAR2);
    mii_item.ID = MenuItem_Id::toolbar_drive_double;
    mii_item.State = (DriveBar->HWindow != NULL && DriveBar2->HWindow != NULL) ? MENU_STATE_CHECKED : 0;
    menu.InsertItem( 0xffffffff, TRUE, &mii_item );

    //Middle toolbar checkbox.
    mii_item.String = LoadStr(IDS_MIDDLETOOLBAR);
    mii_item.ID = MenuItem_Id::toolbar_middle;
    mii_item.State = (MiddleToolBar->HWindow != NULL) ? MENU_STATE_CHECKED : 0;
    menu.InsertItem( 0xffffffff, TRUE, &mii_item );

    //Command line checkbox.
    mii_item.String = LoadStr(IDS_COMMANDLINE);
    mii_item.ID = MenuItem_Id::commandLine;
    mii_item.State = EditPermanentVisible ? MENU_STATE_CHECKED : 0;
    menu.InsertItem( 0xffffffff, TRUE, &mii_item );

    //Bottom toolbar checkbox.
    mii_item.String = LoadStr(IDS_BOTTOMTOOLBAR);
    mii_item.ID = MenuItem_Id::toolbar_bottom;
    mii_item.State = ((CWindow*)BottomToolBar)->HWindow != NULL ? MENU_STATE_CHECKED : 0;
    menu.InsertItem( 0xffffffff, TRUE, &mii_item );

    //Separator.
    menu.InsertItem( 0xffffffff, TRUE, &mii_separator );

    //Lock the toolbars checkbox.
    mii_item.String = LoadStr(IDS_GRIPSINTOOLBAR);
    mii_item.ID = MenuItem_Id::lock_toolbars;
    mii_item.State = Configuration.GripsVisible ? 0 : MENU_STATE_CHECKED;
    menu.InsertItem( 0xffffffff, TRUE, &mii_item );

    //Show text labels in user menu toolbar checkbox.
    mii_item.String = LoadStr(IDS_SHOWLABELS);
    mii_item.ID = MenuItem_Id::show_labels;
    mii_item.State = Configuration.UserMenuToolbarLabels ? MENU_STATE_CHECKED : 0;
    menu.InsertItem( 0xffffffff, TRUE, &mii_item );

    //Customize toolbar text.
    switch ( location )
    {
    case MouseLocation::toolbar_top:
    case MouseLocation::toolbar_userMenu:
    case MouseLocation::toolbar_hotPaths:
    case MouseLocation::toolbar_middle:
    case MouseLocation::toolbar_plugins:
    {
    //One of toolbars -> insert customize toolbar menu item
        //Insert separator.
        menu.InsertItem( 0xffffffff, TRUE, &mii_separator );

        //Get id.
        DWORD   id = -1;

        switch ( location )
        {
        case MouseLocation::toolbar_hotPaths: id = MenuItem_Id::customize_toolbar_hotPaths; break;
        case MouseLocation::toolbar_middle: id = MenuItem_Id::customize_toolbar_middle; break;
        case MouseLocation::toolbar_plugins: id = MenuItem_Id::customize_toolbar_plugins; break;
        case MouseLocation::toolbar_top: id = MenuItem_Id::customize_toolbar_top; break;
        case MouseLocation::toolbar_userMenu: id = MenuItem_Id::customize_toolbar_userMenu; break;
        }

        //Insert menu item.
        mii_item.String = LoadStr( IDS_CUSTOMIZE );
        mii_item.ID = id;
        mii_item.State = 0;
        menu.InsertItem( 0xffffffff, TRUE, &mii_item );
        break;
    }
    }

//Show context menu.
    const auto      cmd = menu.Track( MENU_TRACK_RETURNCMD | MENU_TRACK_RIGHTBUTTON, point_screen.x, point_screen.y, HWindow, NULL );

    if ( cmd == 0 )
    {
    //No item was clicked -> stop execution.
        return;
    }

//Handle mouse click.
    switch ( cmd )
    {
    case MenuItem_Id::commandLine:
    case MenuItem_Id::lock_toolbars:
    case MenuItem_Id::show_labels:
    case MenuItem_Id::toolbar_bottom:
    case MenuItem_Id::toolbar_drive_double:
    case MenuItem_Id::toolbar_drive_single:
    case MenuItem_Id::toolbar_hotPaths:
    case MenuItem_Id::toolbar_middle:
    case MenuItem_Id::toolbar_plugins:
    case MenuItem_Id::toolbar_top:
    case MenuItem_Id::toolbar_userMenu:
    {
    //Regular menu item -> send command.
        //Get command id.
        int     cm = 0;

        switch ( cmd )
        {
        case MenuItem_Id::commandLine: cm = CM_TOGGLEEDITLINE; break;
        case MenuItem_Id::lock_toolbars: cm = CM_TOGGLE_GRIPS; break;
        case MenuItem_Id::show_labels: cm = CM_TOGGLE_UMLABELS; break;
        case MenuItem_Id::toolbar_bottom: cm = CM_TOGGLEBOTTOMTOOLBAR; break;
        case MenuItem_Id::toolbar_drive_double: cm = CM_TOGGLEDRIVEBAR2; break;
        case MenuItem_Id::toolbar_drive_single: cm = CM_TOGGLEDRIVEBAR; break;
        case MenuItem_Id::toolbar_hotPaths: cm = CM_TOGGLEHOTPATHSBAR; break;
        case MenuItem_Id::toolbar_middle: cm = CM_TOGGLEMIDDLETOOLBAR; break;
        case MenuItem_Id::toolbar_plugins: cm = CM_TOGGLEPLUGINSBAR; break;
        case MenuItem_Id::toolbar_top: cm = CM_TOGGLETOPTOOLBAR; break;
        case MenuItem_Id::toolbar_userMenu: cm = CM_TOGGLEUSERMENUTOOLBAR; break;
        }

        //Send command
        PostMessage( HWindow, WM_COMMAND, MAKEWPARAM( cm, 0 ), 0 );
        break;
    }
    case MenuItem_Id::customize_toolbar_hotPaths:
    {
    //Customize hot paths toolbar.
        PostMessage( HWindow, WM_USER_CONFIGURATION, 1, -1 );
        break;
    }
    case MenuItem_Id::customize_toolbar_middle:
    {
    //Customize middle toolbar.
        PostMessage( MainWindow->HWindow, WM_COMMAND, CM_CUSTOMIZEMIDDLE, 0 );
        break;
    }
    case MenuItem_Id::customize_toolbar_plugins:
    {
    //Customize plugins toolbar.
        PostMessage(MainWindow->HWindow, WM_COMMAND, CM_CUSTOMIZEPLUGINS, 0);
        break;
    }
    case MenuItem_Id::customize_toolbar_top:
    {
    //Customize top toolbar.
        PostMessage( MainWindow->HWindow, WM_COMMAND, CM_CUSTOMIZETOP, 0 );
        break;
    }
    case MenuItem_Id::customize_toolbar_userMenu:
    {
    //Customize user menu toolbar.
        PostMessage( HWindow, WM_USER_CONFIGURATION, 2, 0 );
        break;
    }
    }
}
void CMainWindow::OnWmContextMenu_Panel( const HWND hWnd, const POINT& point_screen, MouseLocation::Value location )
{
//Initialize menu.
    CMenuPopup      menu;

    menu.SetImageList( HGrayToolBarImageList, TRUE );
    menu.SetHotImageList( HHotToolBarImageList, TRUE );

    //Initialize menu item info structures.
    MENU_ITEM_INFO  mii_item =
    {
        .Mask = MENU_MASK_TYPE | MENU_MASK_ID | MENU_MASK_STATE | MENU_MASK_STRING | MENU_MASK_SUBMENU | MENU_MASK_IMAGEINDEX,
        .Type = MENU_TYPE_STRING,
        .State = 0,
        .SubMenu = NULL,
        .ImageIndex = -1
    };
    MENU_ITEM_INFO  mii_separator =
    {
        .Mask = MENU_MASK_TYPE,
        .Type = MENU_TYPE_SEPARATOR
    };

//Get panel.
    struct Panel
    {
        public: enum Value
        {
            not_set,

            left,
            right
        };
    };

    Panel::Value    panel = Panel::not_set;
    CPanelWindow*   pPanel = NULL;

    switch ( location )
    {
    case MouseLocation::panel_left_dirLine:
    case MouseLocation::panel_left_headerLine:
    case MouseLocation::panel_left_statusLine:
    case MouseLocation::panel_left_workingArea: panel = Panel::left; pPanel = LeftPanel; break;
    case MouseLocation::panel_right_dirLine:
    case MouseLocation::panel_right_headerLine:
    case MouseLocation::panel_right_statusLine:
    case MouseLocation::panel_right_workingArea: panel = Panel::right; pPanel = RightPanel; break;
    default:
    {
    //Just to be 100% sure all will be good.
        return;
    }
    }

//Insert menu items.
    struct MenuItem_Id
    {
        public: enum Value
        {
            dirLine_assignHotPath               = 1,    //Has to start with 1 as 0 is used as error.
            dirLine_changeDirectory,
            dirLine_copyToClipboard,
            dirLine_toOtherPanel,

            headerLine_automaticColumnWidth,
            headerLine_chooseColumns,
            headerLine_smartColumnMode,

            show_directoryLine,
            show_headerLine,
            show_informationLine,

            statusLine_copyToClipboard,

            //--------------------------
            hotPaths_id_first                           //Root id for hot paths.
        };
    };

    char    HotText[2 * MAX_PATH] = {0};
    int     index_headerLineItem = -1;

    switch ( location )
    {
    case MouseLocation::panel_left_headerLine:
    case MouseLocation::panel_right_headerLine:
    {
    //Header line
    //
    //Get header line.
        CHeaderLine*    pHeaderLine = pPanel->GetHeaderLine();

        if ( pHeaderLine == NULL )
        {
            break;
        }

    //Which item on header line was clicked on?
        //Convert position relative to header area.
        POINT   point_header = point_screen;

        ScreenToClient( pHeaderLine->HWindow, &point_header );

        //Get item that was clicked on.
        int             index;
        BOOL            extInName;

        const auto      ht = pHeaderLine->HitTest( point_header.x, point_header.y, index, extInName );

        //Was one of the items (e.g. Name, Ext, Size,..)?
        if (
            ( index >= 0 )
            &&
            (
                ( ht == CHeaderLine::HitTest::Item )
                ||
                ( ht == CHeaderLine::HitTest::Divider )
            )
        )
        {
        //Yes -> add menu items.
            //Store index on which item user clicked.
            index_headerLineItem = index;

            //Automatic column width checkbox.
            CColumn*    pColumn = &pPanel->Columns[index];

            mii_item.String = LoadStr( IDS_HDR_ELASTIC );
            mii_item.ID = MenuItem_Id::headerLine_automaticColumnWidth;
            mii_item.State = ( pColumn->FixedWidth ) ? 0 : MENU_STATE_CHECKED;
            menu.InsertItem( 0xffffffff, TRUE, &mii_item );

            //Smart column mode checkbox.
            //
            //Notice:
            //Intended only for 'name and ext' column.
            if ( index == 0 )
            {
                mii_item.String = LoadStr( IDS_HDR_SMARTMODE );
                mii_item.ID = MenuItem_Id::headerLine_smartColumnMode;
                mii_item.State = GetSmartColumnMode( pPanel ) ? MENU_STATE_CHECKED : 0;
                menu.InsertItem( 0xffffffff, TRUE, &mii_item );
            }
        }

        //Choose columns text.
        mii_item.String = LoadStr( IDS_MENU_LEFT_VIEW );
        mii_item.ID = MenuItem_Id::headerLine_chooseColumns;
        mii_item.State = 0;
        menu.InsertItem( 0xffffffff, TRUE, &mii_item );

        //Separator.
        menu.InsertItem( 0xffffffff, TRUE, &mii_separator );
        break;
    }
    case MouseLocation::panel_left_dirLine:
    case MouseLocation::panel_right_dirLine:
    {
    //Directory line
        //Change directory text.
        mii_item.String = LoadStr( IDS_CHANGEDIRECTORY );
        mii_item.ID = MenuItem_Id::dirLine_changeDirectory;
        mii_item.State = 0;
        mii_item.ImageIndex = IDX_TB_CHANGE_DIR;
        menu.InsertItem( 0xffffffff, TRUE, &mii_item );

        mii_item.ImageIndex = -1;

        //Separator.
        menu.InsertItem( 0xffffffff, TRUE, &mii_separator );

        //Was it clicked on path?
        pPanel->DirectoryLine->GetHotText( HotText, _countof( HotText ) );

        if ( strlen( HotText ) > 0 )
        {
        //Yes -> add menu items.
            //Assign hot paths' submenu item.
            CMenuPopup*     menu_sub = new CMenuPopup();

            if ( menu_sub != NULL)
            {
            //Create 'hot paths' submenu.
                HotPaths.FillHotPathsMenu( menu_sub, MenuItem_Id::hotPaths_id_first, TRUE, FALSE, FALSE, FALSE, TRUE );

           //Assign hot paths submenu.
                mii_item.SubMenu = menu_sub;
                mii_item.String = LoadStr( IDS_SETHOTPATH );
                mii_item.ID = 0;            //No id it's a submenu.
                mii_item.State = 0;
                menu.InsertItem( 0xffffffff, TRUE, &mii_item );
                mii_item.SubMenu = NULL;
            }

        //Copy to clipboard text.
            mii_item.String = LoadStr( IDS_COPYTOCLIPBOARD );
            mii_item.ID = MenuItem_Id::dirLine_copyToClipboard;
            mii_item.State = 0;
            menu.InsertItem( 0xffffffff, TRUE, &mii_item );
        }

        //Set to other panel text.
        mii_item.String = LoadStr( IDS_TOOTHERPANEL );
        mii_item.ID = MenuItem_Id::dirLine_toOtherPanel;
        mii_item.State = 0;
        menu.InsertItem( 0xffffffff, TRUE, &mii_item );

        //Separator.
        menu.InsertItem( 0xffffffff, TRUE, &mii_separator );
        break;
    }
    case MouseLocation::panel_left_statusLine:
    case MouseLocation::panel_right_statusLine:
    {
    //Status line.
        pPanel->StatusLine->GetHotText( HotText, _countof( HotText ) );

        if ( strlen( HotText ) > 0 )
        {
        //Copy to clipboard text.
            mii_item.String = LoadStr( IDS_COPYTOCLIPBOARD );
            mii_item.ID = MenuItem_Id::statusLine_copyToClipboard;
            mii_item.State = 0;
            menu.InsertItem( 0xffffffff, TRUE, &mii_item );

        //Separator.
            menu.InsertItem( 0xffffffff, TRUE, &mii_separator );
        }
        break;
    }
    //case MouseLocation::panel_left_workingArea:
    //case MouseLocation::panel_right_workingArea:
    //{
    //    break;
    //}
    }

    //Directory line checkbox.
    mii_item.String = LoadStr( IDS_DIRECTORYLINE );
    mii_item.ID = MenuItem_Id::show_directoryLine;
    mii_item.State = ( pPanel->DirectoryLine->HWindow != NULL ) ? MENU_STATE_CHECKED : 0;
    menu.InsertItem( 0xffffffff, TRUE, &mii_item );

    //Header line checkbox.
    mii_item.String = LoadStr( IDS_HEADERLINE );
    mii_item.ID = MenuItem_Id::show_headerLine;
    mii_item.State = ( pPanel->GetViewMode() == ViewMode::detailed ? 0 : (MENU_STATE_GRAYED) ) | ( ( ( pPanel->GetViewMode() == ViewMode::detailed ) && pPanel->HeaderLineVisible ) ? MENU_STATE_CHECKED : 0 );
    menu.InsertItem( 0xffffffff, TRUE, &mii_item );

    //Information line (at bottom) checkbox.
    mii_item.String = LoadStr( IDS_INFORMATIONLINE );
    mii_item.ID = MenuItem_Id::show_informationLine;
    mii_item.State = ( pPanel->StatusLine->HWindow != NULL ) ? MENU_STATE_CHECKED : 0;
    menu.InsertItem( 0xffffffff, TRUE, &mii_item );

    //Separator
    menu.InsertItem( 0xffffffff, TRUE, &mii_separator );

    //Customize panel toolbar text.
    mii_item.String = LoadStr( IDS_CUSTOMIZEPANEL );
    mii_item.ID = 15;
    mii_item.State = 0;
    menu.InsertItem( 0xffffffff, TRUE, &mii_item );

//Show context menu.
    const auto      cmd = menu.Track( MENU_TRACK_RETURNCMD | MENU_TRACK_RIGHTBUTTON, point_screen.x, point_screen.y, HWindow, NULL );

    if ( cmd == 0 )
    {
    //No item was clicked -> stop execution.
        return;
    }

//Handle mouse click.
    int     cm = 0;

    switch ( cmd )
    {
    case MenuItem_Id::headerLine_automaticColumnWidth:
    {
    //Automatic column width.
        CColumn* column = &pPanel->Columns[index_headerLineItem];

        if (column->ID != COLUMN_ID_CUSTOM)
        {
            CColumnConfig* colCfg = &pPanel->ViewTemplate->Columns[column->ID - 1];
            if (panel == Panel::left)
                colCfg->LeftFixedWidth = column->FixedWidth ? 0 : 1;
            else
                colCfg->RightFixedWidth = column->FixedWidth ? 0 : 1;
            if (column->ID == COLUMN_ID_NAME)
            {
                if (panel == Panel::left)
                {
                    if (colCfg->LeftFixedWidth)
                        colCfg->LeftWidth = pPanel->GetResidualColumnWidth();
                    else
                        pPanel->ViewTemplate->LeftSmartMode = FALSE;
                }
                else
                {
                    if (colCfg->RightFixedWidth)
                        colCfg->RightWidth = pPanel->GetResidualColumnWidth();
                    else
                        pPanel->ViewTemplate->RightSmartMode = FALSE;
                }
            }
            else
            {
                if (panel == Panel::left)
                    colCfg->LeftWidth = column->Width;
                else
                    colCfg->RightWidth = column->Width;
            }
        }
        else
        {
            if (pPanel->PluginData.NotEmpty()) // "always true"
                pPanel->PluginData.ColumnFixedWidthShouldChange(panel == Panel::left, column, column->FixedWidth ? 0 : 1);
        }
        // user cosi menil v konfiguraci pohledu - nechame znovu sestavit sloupce
        if (panel == Panel::left)
            LeftPanel->SelectViewTemplate(LeftPanel->GetViewTemplateIndex(), TRUE, FALSE);
        else
            RightPanel->SelectViewTemplate(RightPanel->GetViewTemplateIndex(), TRUE, FALSE);
        break;
    }
    case MenuItem_Id::headerLine_chooseColumns:
    {
        PostMessage(HWindow, WM_USER_CONFIGURATION, 4, ( ( panel == Panel::left ) ? LeftPanel : RightPanel)->GetViewTemplateIndex());
        break;
    }
    case MenuItem_Id::dirLine_copyToClipboard:
    {
    //Directory line: copy to clipboard
        CopyTextToClipboard( HotText );

        pPanel->DirectoryLine->FlashText( TRUE );
        break;
    }
    case MenuItem_Id::statusLine_copyToClipboard:
    {
    //Status line: copy to clipboard
        CopyTextToClipboard( HotText );

        pPanel->StatusLine->FlashText( TRUE );
        break;
    }
    case MenuItem_Id::show_directoryLine:
    {
    //Show check: directory line
        cm = ( panel == Panel::left ) ? CM_LEFTDIRLINE : CM_RIGHTDIRLINE;
        break;
    }
    case MenuItem_Id::show_headerLine:
    {
    //Show check: header line
        cm = ( panel == Panel::left ) ? CM_LEFTHEADER : CM_RIGHTHEADER;
        break;
    }
    case MenuItem_Id::show_informationLine:
    {
    //Show check: information line
        cm = ( panel == Panel::left ) ? CM_LEFTSTATUS : CM_RIGHTSTATUS;
        break;
    }
    case 15:
    {
    //Customize panel toolbar
        cm = ( panel == Panel::left ) ? CM_CUSTOMIZELEFT : CM_CUSTOMIZERIGHT;
        break;
    }
    case MenuItem_Id::dirLine_changeDirectory:
    {
    //Change directory.
        cm = ( panel == Panel::left ) ? CM_LEFT_CHANGEDIR : CM_RIGHT_CHANGEDIR;
        break;
    }
    case MenuItem_Id::dirLine_toOtherPanel:
    {
    //Set current path to other panel.
        CPanelWindow*   pPanel_nonActive = ( panel == Panel::left ) ? RightPanel : LeftPanel;

        pPanel_nonActive->ChangePathToOtherPanelPath( pPanel, HotText );
        break;
    }
    case MenuItem_Id::headerLine_smartColumnMode:
    {
        cm = ( panel == Panel::left ) ? CM_LEFT_SMARTMODE : CM_RIGHT_SMARTMODE;
        break;
    }
    default:
    {
    //Is it hot path assignment?
        if (
            ( cmd >= MenuItem_Id::hotPaths_id_first )
            &&
            ( cmd < ( MenuItem_Id::hotPaths_id_first + HOT_PATHS_COUNT ) )
        )
        {
            SetUnescapedHotPath(cmd - 20, HotText);

            if (!Configuration.HotPathAutoConfig)
            {
                pPanel->DirectoryLine->FlashText(TRUE);
            }
        }
        break;
    }
    }
    if ( cm != 0 )
    {
        PostMessage( HWindow, WM_COMMAND, MAKEWPARAM( cm, 0 ), 0 );
        return;
    }
}
void CMainWindow::OnWmContextMenu_SplitLine( const POINT& point_screen )
{
//Initialize menu.
    CMenuPopup      menu;

    menu.SetImageList( HGrayToolBarImageList, TRUE );
    menu.SetHotImageList( HHotToolBarImageList, TRUE );

    //Initialize menu item info structures.
    MENU_ITEM_INFO  mii_item =
    {
        .Mask = MENU_MASK_TYPE | MENU_MASK_ID | MENU_MASK_STATE | MENU_MASK_STRING | MENU_MASK_SUBMENU | MENU_MASK_IMAGEINDEX,
        .Type = MENU_TYPE_STRING,
        .State = 0,
        .SubMenu = NULL,
        .ImageIndex = -1
    };

//Insert menu items.
    char    buff[20];

    for ( int i = 2; i < 9; i++ )
    {
    //Get text.
    //
    //Example: 20 / 80
        sprintf( buff, "&%d0 / %d0", i, 10 - i );

    //Insert nenu item.
        mii_item.ID = i;
        mii_item.State = ( i == 5 ) ? MENU_STATE_DEFAULT : 0;
        mii_item.String = buff;

        menu.InsertItem( 0xffffffff, TRUE, &mii_item );
    }

//Show context menu.
    const auto      cmd = menu.Track( MENU_TRACK_RETURNCMD | MENU_TRACK_RIGHTBUTTON, point_screen.x, point_screen.y, HWindow, NULL );

    if ( cmd == 0 )
    {
    //No item was clicked -> stop execution.
        return;
    }

//Update panel widths.
    SplitPosition = ( (double)cmd ) / 10.0;

//Redraw windows.
    LayoutWindows();
}

static DWORD CheckerViewMode = 0xFFFFFFFF;
static DWORD CheckerLeftViewMode = 0xFFFFFFFF;
static DWORD CheckerRightViewMode = 0xFFFFFFFF;
static DWORD CheckerSortType = 0xFFFFFFFF;
static DWORD CheckerLeftSortType = 0xFFFFFFFF;
static DWORD CheckerRightSortType = 0xFFFFFFFF;
static BOOL CheckerHelpMode = 0xFFFFFFFF;
static BOOL CheckerSmartMode = 0xFFFFFFFF;
static BOOL CheckerLeftSmartMode = 0xFFFFFFFF;
static BOOL CheckerRightSmartMode = 0xFFFFFFFF;

void CMainWindow_RefreshCommandStates(CMainWindow* obj)
{
    IdleRefreshStates = FALSE; // shodim ridici promennou

    //---  ziskani stavovych hodnot pro enablovani
    BOOL file = FALSE;                               // kurzor na souboru
    BOOL subDir = FALSE;                             // kurzor na podadresari
    BOOL files = FALSE;                              // kurzor na souboru nebo adresari nebo oznaceni
    BOOL linkOnDisk = FALSE;                         // jen disky: kurzor na linku (soubor nebo adresar s atributem FILE_ATTRIBUTE_REPARSE_POINT)
    BOOL containsFile = FALSE;                       // kurzor (nebo oznaceni) obsahuje soubory
    BOOL containsDir = FALSE;                        // kurzor (nebo oznaceni) pouze na adresarich
    BOOL compress = FALSE;                           // podporuje file-based-compression?
    BOOL encrypt = FALSE;                            // podporuje file-based-encryption?
    BOOL acls = FALSE;                               // podporuje ACL? (NTFS disky)
    BOOL archive = FALSE;                            // je v panelu archiv?
    BOOL targetArchive = FALSE;                      // je v druhem panelu archiv?
    BOOL archiveEdit = FALSE;                        // je v panelu archiv, ktery umime editovat?
    BOOL upDir = FALSE;                              // pritomnost ".."
    BOOL leftUpDir = FALSE;                          // pritomnost ".."
    BOOL rightUpDir = FALSE;                         // pritomnost ".."
    BOOL rootDir = FALSE;                            // TRUE = nejsme jeste v rootu
    BOOL leftRootDir = FALSE;                        // TRUE = nejsme jeste v rootu
    BOOL rightRootDir = FALSE;                       // TRUE = nejsme jeste v rootu
    BOOL hasForward = FALSE;                         // je mozny forward (historie cest)
    BOOL hasBackward = FALSE;                        // je mozny backward (historie cest)
    BOOL leftHasForward = FALSE;                     // je mozny forward v levem panelu (historie cest)
    BOOL leftHasBackward = FALSE;                    // je mozny backward v levem panelu (historie cest)
    BOOL rightHasForward = FALSE;                    // je mozny forward v pravem panelu (historie cest)
    BOOL rightHasBackward = FALSE;                   // je mozny backward v pravem panelu (historie cest)
    BOOL pasteFiles = EnablerPasteFiles;             // je mozne Edit/Paste? (soubory "cut" i "copy")
    BOOL pastePath = EnablerPastePath;               // je mozne Edit/Paste? (text cesty)
    BOOL pasteLinks = EnablerPasteLinks;             // je mozne Edit/Paste Shortcuts? (soubory "copy")
    BOOL pasteSimpleFiles = EnablerPasteSimpleFiles; // jsou na clipboardu soubory/adresare z jedine cesty? (aneb: je sance na Paste do archivu nebo FS?)
    DWORD pasteDefEffect = EnablerPasteDefEffect;    // jaky je defaultni paste-effect, muze byt i kombinace DROPEFFECT_COPY+DROPEFFECT_MOVE (aneb: slo o Copy nebo Cut?)
    BOOL pasteFilesToArcOrFS = FALSE;                // je mozny Paste souboru do archivu/FS v aktualnim panelu?
    BOOL onDisk = FALSE;                             // je v panelu disk?
    BOOL customizeLeftView = FALSE;                  // lze konfigurovat sloupce pro levy panel?
    BOOL customizeRightView = FALSE;                 // lze konfigurovat sloupce pro pravy panel?
    BOOL validPluginFS = FALSE;                      // je v panelu FS s inicializovanym PluginFS interfacem?
    DWORD viewMode = 0;                              // rezim zobrazeni panelu (tree/brief/detailed/...)
    DWORD leftViewMode = 0;
    DWORD rightViewMode = 0;
    DWORD sortType = 0;
    DWORD leftSortType = 0;
    DWORD rightSortType = 0;
    BOOL existPrevSel = FALSE;
    BOOL existNextSel = FALSE;

    BOOL dirHistory = FALSE; // je v directory historii dostupny adresar?
    BOOL smartMode = FALSE;
    BOOL leftSmartMode = FALSE;
    BOOL rightSmartMode = FALSE;

    int selCount = 0;
    int unselCount = 0;

    CPanelWindow* activePanel = obj->GetActivePanel();
    CPanelWindow* nonActivePanel = obj->GetNonActivePanel();
    if (activePanel != NULL && nonActivePanel != NULL && obj->LeftPanel != NULL && obj->RightPanel != NULL)
    {
        hasForward = activePanel->PathHistory->HasForward();
        hasBackward = activePanel->PathHistory->HasBackward();
        leftHasForward = obj->LeftPanel->PathHistory->HasForward();
        leftHasBackward = obj->LeftPanel->PathHistory->HasBackward();
        rightHasForward = obj->RightPanel->PathHistory->HasForward();
        rightHasBackward = obj->RightPanel->PathHistory->HasBackward();
        compress = activePanel->FileBasedCompression;
        acls = activePanel->SupportACLS;
        encrypt = activePanel->FileBasedEncryption;
        archive = activePanel->Is(ptZIPArchive);
        targetArchive = nonActivePanel->Is(ptZIPArchive);
        selCount = activePanel->GetSelectedCount();
        onDisk = activePanel->Is(ptDisk);
        dirHistory = obj->DirHistory->HasPaths();
        sortType = activePanel->SortType;
        leftSortType = obj->LeftPanel->SortType;
        rightSortType = obj->RightPanel->SortType;
        validPluginFS = activePanel->Is(ptPluginFS) && activePanel->GetPluginFS()->NotEmpty();
        smartMode = obj->GetSmartColumnMode(activePanel);
        leftSmartMode = obj->GetSmartColumnMode(obj->LeftPanel);
        rightSmartMode = obj->GetSmartColumnMode(obj->RightPanel);

        if (archive)
        {
            int format = PackerFormatConfig.PackIsArchive(activePanel->GetZIPArchive());
            if (format != 0) // nasli jsme podporovany archiv
            {
                archiveEdit = PackerFormatConfig.GetUsePacker(format - 1); // ma edit?
            }
        }

        upDir = (activePanel->Dirs->Count != 0 &&
                 strcmp(activePanel->Dirs->At(0).Name, "..") == 0);
        leftUpDir = (obj->LeftPanel->Dirs->Count != 0 &&
                     strcmp(obj->LeftPanel->Dirs->At(0).Name, "..") == 0);
        rightUpDir = (obj->RightPanel->Dirs->Count != 0 &&
                      strcmp(obj->RightPanel->Dirs->At(0).Name, "..") == 0);
        if (!leftUpDir)
            leftRootDir = FALSE; // uz jsme v rootu (neexistuje up-dir)
        else
            leftRootDir = TRUE; //!obj->LeftPanel->Is(ptDisk) || !IsUNCRootPath(obj->LeftPanel->GetPath());
        if (!rightUpDir)
            rightRootDir = FALSE; // uz jsme v rootu (neexistuje up-dir)
        else
            rightRootDir = TRUE; //!obj->RightPanel->Is(ptDisk) || !IsUNCRootPath(obj->RightPanel->GetPath());
        rootDir = activePanel == obj->LeftPanel ? leftRootDir : rightRootDir;

        unselCount = activePanel->Dirs->Count + activePanel->Files->Count - selCount;
        if (upDir)
            unselCount--; // up dir nepovazujeme za nevybrany adresar

        viewMode = activePanel->GetViewTemplateIndex();
        leftViewMode = obj->LeftPanel->GetViewTemplateIndex();
        rightViewMode = obj->RightPanel->GetViewTemplateIndex();

        customizeLeftView = leftViewMode > 1;
        customizeRightView = rightViewMode > 1;

        int caret = activePanel->GetCaretIndex();
        if (caret >= 0)
        {
            if (caret == 0)
            {
                if (!upDir)
                    files = (activePanel->Dirs->Count + activePanel->Files->Count > 0);
                else
                {
                    int count = activePanel->GetSelectedCount();
                    if (count == 1)
                    {
                        files = (activePanel->GetSelected(0) == FALSE);
                    }
                    else
                        files = (count > 0);
                }
            }
            else
                files = TRUE;
            file = files && caret >= activePanel->Dirs->Count;
            subDir = files && caret < activePanel->Dirs->Count && (caret != 0 || !upDir);

            containsFile = activePanel->SelectionContainsFile();
            containsDir = activePanel->SelectionContainsDirectory();

            int dummyIndex;
            existPrevSel = activePanel->SelectFindNext(caret, FALSE, TRUE, dummyIndex);
            existNextSel = activePanel->SelectFindNext(caret, TRUE, TRUE, dummyIndex);

            if (onDisk && (file || subDir))
            {
                if (caret < activePanel->Dirs->Count)
                    linkOnDisk = (activePanel->Dirs->At(caret).Attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
                else
                    linkOnDisk = (activePanel->Files->At(caret - activePanel->Dirs->Count).Attr & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
            }
        }

        if (IdleCheckClipboard)
        {
            IdleCheckClipboard = FALSE; // shodim ridici promennou
            pasteFiles = activePanel->ClipboardPaste(FALSE, TRUE);
            pastePath = activePanel->IsTextOnClipboard();
            pasteLinks = activePanel->ClipboardPasteLinks(TRUE);
            pasteSimpleFiles = activePanel->ClipboardPasteToArcOrFS(TRUE, &pasteDefEffect);
        }

        // napocitame hodnotu pasteFilesToArcOrFS
        if (pasteSimpleFiles)
        {
            if (archiveEdit)
                pasteFilesToArcOrFS = (pasteDefEffect & (DROPEFFECT_COPY | DROPEFFECT_MOVE)) != 0;
            else
            {
                if (validPluginFS)
                {
                    if ((pasteDefEffect & DROPEFFECT_COPY) &&
                            activePanel->GetPluginFS()->IsServiceSupported(FS_SERVICE_COPYFROMDISKTOFS) ||
                        (pasteDefEffect & DROPEFFECT_MOVE) &&
                            activePanel->GetPluginFS()->IsServiceSupported(FS_SERVICE_MOVEFROMDISKTOFS))
                    {
                        pasteFilesToArcOrFS = TRUE;
                    }
                }
            }
        }
    }

    // preneseme vysledky do globalnich promennych
    // pokud v nektere dojde ke zmene, bude nastavena promenna obj->IdleStatesChanged
    obj->CheckAndSet(&EnablerUpDir, upDir);
    obj->CheckAndSet(&EnablerRootDir, rootDir);
    obj->CheckAndSet(&EnablerForward, hasForward);
    obj->CheckAndSet(&EnablerBackward, hasBackward);
    obj->CheckAndSet(&EnablerFiles, files);
    obj->CheckAndSet(&EnablerFileOnDisk, file && onDisk);
    obj->CheckAndSet(&EnablerFileOrDirLinkOnDisk, (file || linkOnDisk) && onDisk);
    obj->CheckAndSet(&EnablerFileOnDiskOrArchive, file && (onDisk || archive));
    obj->CheckAndSet(&EnablerFilesOnDisk, onDisk && files);
    obj->CheckAndSet(&EnablerFilesOnDiskOrArchive, (onDisk || archive) && files);
    obj->CheckAndSet(&EnablerOccupiedSpace, (onDisk || archive && (activePanel->ValidFileData & VALID_DATA_SIZE)) && files);
    obj->CheckAndSet(&EnablerFilesOnDiskCompress, onDisk && compress && files);
    obj->CheckAndSet(&EnablerFilesOnDiskEncrypt, onDisk && encrypt && files);
    obj->CheckAndSet(&EnablerFileDir, (file || subDir));
    obj->CheckAndSet(&EnablerFileDirANDSelected, (file || subDir) && selCount > 0);
    obj->CheckAndSet(&EnablerOnDisk, onDisk);
    obj->CheckAndSet(&EnablerCalcDirSizes, (onDisk || archive && (activePanel->ValidFileData & VALID_DATA_SIZE)));
    obj->CheckAndSet(&EnablerPasteFiles, pasteFiles);                   // ulozime stav clipboardu pro pristi volani RefreshCommandStates()
    obj->CheckAndSet(&EnablerPastePath, pastePath);                     // ulozime stav clipboardu pro pristi volani RefreshCommandStates()
    obj->CheckAndSet(&EnablerPasteLinks, pasteLinks);                   // ulozime stav clipboardu pro pristi volani RefreshCommandStates()
    obj->CheckAndSet(&EnablerPasteSimpleFiles, pasteSimpleFiles);       // ulozime stav clipboardu pro pristi volani RefreshCommandStates()
    obj->CheckAndSet(&EnablerPasteDefEffect, pasteDefEffect);           // ulozime stav clipboardu pro pristi volani RefreshCommandStates()
    obj->CheckAndSet(&EnablerPasteFilesToArcOrFS, pasteFilesToArcOrFS); // ulozime stav pro rozliseni mezi "Paste" a "Paste (Change Directory)"
    obj->CheckAndSet(&EnablerPaste, (onDisk && pasteFiles || pasteFilesToArcOrFS || pastePath));
    obj->CheckAndSet(&EnablerPasteLinksOnDisk, onDisk && pasteLinks);
    obj->CheckAndSet(&EnablerSelected, selCount > 0);
    obj->CheckAndSet(&EnablerUnselected, unselCount > 0);
    obj->CheckAndSet(&EnablerHiddenNames, activePanel->HiddenNames.GetCount() > 0);
    obj->CheckAndSet(&EnablerSelectionStored, activePanel->OldSelection.GetCount() > 0);
    obj->CheckAndSet(&EnablerGlobalSelStored, (GlobalSelection.GetCount() > 0 || pastePath));
    obj->CheckAndSet(&EnablerSelGotoPrev, existPrevSel);
    obj->CheckAndSet(&EnablerSelGotoNext, existNextSel);
    obj->CheckAndSet(&EnablerLeftUpDir, leftUpDir);
    obj->CheckAndSet(&EnablerRightUpDir, rightUpDir);
    obj->CheckAndSet(&EnablerLeftRootDir, leftRootDir);
    obj->CheckAndSet(&EnablerRightRootDir, rightRootDir);
    obj->CheckAndSet(&EnablerLeftForward, leftHasForward);
    obj->CheckAndSet(&EnablerRightForward, rightHasForward);
    obj->CheckAndSet(&EnablerLeftBackward, leftHasBackward);
    obj->CheckAndSet(&EnablerRightBackward, rightHasBackward);
    obj->CheckAndSet(&EnablerFileHistory, obj->FileHistory->HasItem());
    obj->CheckAndSet(&EnablerDirHistory, dirHistory);
    obj->CheckAndSet(&EnablerCustomizeLeftView, customizeLeftView);
    obj->CheckAndSet(&EnablerCustomizeRightView, customizeRightView);
    obj->CheckAndSet(&EnablerDriveInfo, (onDisk || archive ||
                                         validPluginFS && activePanel->GetPluginFS()->IsServiceSupported(FS_SERVICE_SHOWINFO)));
    obj->CheckAndSet(&EnablerQuickRename, (file || subDir) &&
                                              (onDisk ||
                                               validPluginFS && activePanel->GetPluginFS()->IsServiceSupported(FS_SERVICE_QUICKRENAME)));
    obj->CheckAndSet(&EnablerCreateDir, (onDisk ||
                                         validPluginFS && activePanel->GetPluginFS()->IsServiceSupported(FS_SERVICE_CREATEDIR)));
    obj->CheckAndSet(&EnablerViewFile, file &&
                                           (onDisk || archive ||
                                            validPluginFS && activePanel->GetPluginFS()->IsServiceSupported(FS_SERVICE_VIEWFILE)));
    obj->CheckAndSet(&EnablerFilesDelete, files &&
                                              (onDisk || archiveEdit ||
                                               validPluginFS && activePanel->GetPluginFS()->IsServiceSupported(FS_SERVICE_DELETE)));
    obj->CheckAndSet(&EnablerFilesCopy, files &&
                                            (onDisk || archive ||
                                             validPluginFS && activePanel->GetPluginFS()->IsServiceSupported(FS_SERVICE_COPYFROMFS)));
    obj->CheckAndSet(&EnablerFilesMove, files &&
                                            (onDisk ||
                                             validPluginFS && activePanel->GetPluginFS()->IsServiceSupported(FS_SERVICE_MOVEFROMFS)));
    obj->CheckAndSet(&EnablerChangeAttrs, files &&
                                              (onDisk ||
                                               validPluginFS && activePanel->GetPluginFS()->IsServiceSupported(FS_SERVICE_CHANGEATTRS)));
    obj->CheckAndSet(&EnablerShowProperties, files &&
                                                 (onDisk ||
                                                  validPluginFS && activePanel->GetPluginFS()->IsServiceSupported(FS_SERVICE_SHOWPROPERTIES)));
    obj->CheckAndSet(&EnablerItemsContextMenu, files &&
                                                   (onDisk ||
                                                    validPluginFS && activePanel->GetPluginFS()->IsServiceSupported(FS_SERVICE_CONTEXTMENU)));
    obj->CheckAndSet(&EnablerOpenActiveFolder, (onDisk ||
                                                validPluginFS && activePanel->GetPluginFS()->IsServiceSupported(FS_SERVICE_OPENACTIVEFOLDER)));
    obj->CheckAndSet(&EnablerPermissions, onDisk && files && acls &&
                                              ((containsFile && !containsDir) || (!containsFile && containsDir)));

    if (obj->IdleStatesChanged || IdleForceRefresh)
    {
        // pokud doslo k nejake zmene v promennych nebo je vyrazena cache
        // pomoci IdleForceRefresh, nechame viditelne toolbary vytahnout nova data
        if (obj->TopToolBar != NULL && obj->TopToolBar->HWindow != NULL)
            obj->TopToolBar->UpdateItemsState();
        if (obj->MiddleToolBar != NULL && obj->MiddleToolBar->HWindow != NULL)
            obj->MiddleToolBar->UpdateItemsState();
        if (obj->LeftPanel->DirectoryLine->ToolBar != NULL &&
            obj->LeftPanel->DirectoryLine->ToolBar->HWindow != NULL)
            obj->LeftPanel->DirectoryLine->ToolBar->UpdateItemsState();
        if (obj->RightPanel->DirectoryLine->ToolBar != NULL &&
            obj->RightPanel->DirectoryLine->ToolBar->HWindow != NULL)
            obj->RightPanel->DirectoryLine->ToolBar->UpdateItemsState();
        if (obj->BottomToolBar != NULL && obj->BottomToolBar->HWindow != NULL)
            obj->BottomToolBar->UpdateItemsState();
        if (obj->UMToolBar != NULL && obj->UMToolBar->HWindow != NULL)
            obj->UMToolBar->UpdateItemsState();
    }

    CToolBar* topToolbar = obj->TopToolBar;
    CToolBar* midToolbar = obj->MiddleToolBar;
    if (obj->HelpMode != CheckerHelpMode || IdleForceRefresh)
    {
        CheckerHelpMode = obj->HelpMode;
        if (topToolbar != NULL && topToolbar->HWindow != NULL)
            topToolbar->CheckItem(CM_HELP_CONTEXT, FALSE, obj->HelpMode == HELP_ACTIVE);
        if (midToolbar != NULL && midToolbar->HWindow != NULL)
            midToolbar->CheckItem(CM_HELP_CONTEXT, FALSE, obj->HelpMode == HELP_ACTIVE);
    }
    if (viewMode != CheckerViewMode || IdleForceRefresh)
    {
        CheckerViewMode = viewMode;
        if (topToolbar != NULL && topToolbar->HWindow != NULL)
        {
            topToolbar->CheckItem(CM_ACTIVEMODE_2, FALSE, CheckerViewMode == 1);
            topToolbar->CheckItem(CM_ACTIVEMODE_3, FALSE, CheckerViewMode == 2);
        }
        if (midToolbar != NULL && midToolbar->HWindow != NULL)
        {
            midToolbar->CheckItem(CM_ACTIVEMODE_2, FALSE, CheckerViewMode == 1);
            midToolbar->CheckItem(CM_ACTIVEMODE_3, FALSE, CheckerViewMode == 2);
        }
    }
    if (sortType != CheckerSortType || IdleForceRefresh)
    {
        CheckerSortType = sortType;
        if (topToolbar != NULL && topToolbar->HWindow != NULL)
        {
            topToolbar->CheckItem(CM_ACTIVENAME, FALSE, CheckerSortType == stName);
            topToolbar->CheckItem(CM_ACTIVEEXT, FALSE, CheckerSortType == stExtension);
            topToolbar->CheckItem(CM_ACTIVETIME, FALSE, CheckerSortType == stTime);
            topToolbar->CheckItem(CM_ACTIVESIZE, FALSE, CheckerSortType == stSize);
        }
        if (midToolbar != NULL && midToolbar->HWindow != NULL)
        {
            midToolbar->CheckItem(CM_ACTIVENAME, FALSE, CheckerSortType == stName);
            midToolbar->CheckItem(CM_ACTIVEEXT, FALSE, CheckerSortType == stExtension);
            midToolbar->CheckItem(CM_ACTIVETIME, FALSE, CheckerSortType == stTime);
            midToolbar->CheckItem(CM_ACTIVESIZE, FALSE, CheckerSortType == stSize);
        }
    }
    if (smartMode != CheckerSmartMode || IdleForceRefresh)
    {
        CheckerSmartMode = smartMode;
        if (topToolbar != NULL && topToolbar->HWindow != NULL)
        {
            topToolbar->CheckItem(CM_ACTIVE_SMARTMODE, FALSE, CheckerSmartMode == TRUE);
        }
        if (midToolbar != NULL && midToolbar->HWindow != NULL)
        {
            midToolbar->CheckItem(CM_ACTIVE_SMARTMODE, FALSE, CheckerSmartMode == TRUE);
        }
    }

    CToolBar* toolbar = obj->LeftPanel->DirectoryLine->ToolBar;
    if (toolbar != NULL && toolbar->HWindow != NULL)
    {
        if (leftViewMode != CheckerLeftViewMode || IdleForceRefresh)
        {
            CheckerLeftViewMode = leftViewMode;
            toolbar->CheckItem(CM_LEFTMODE_2, FALSE, CheckerLeftViewMode == 1);
            toolbar->CheckItem(CM_LEFTMODE_3, FALSE, CheckerLeftViewMode == 2);
        }

        if (leftSortType != CheckerLeftSortType || IdleForceRefresh)
        {
            CheckerLeftSortType = leftSortType;
            toolbar->CheckItem(CM_LEFTNAME, FALSE, CheckerLeftSortType == stName);
            toolbar->CheckItem(CM_LEFTEXT, FALSE, CheckerLeftSortType == stExtension);
            toolbar->CheckItem(CM_LEFTTIME, FALSE, CheckerLeftSortType == stTime);
            toolbar->CheckItem(CM_LEFTSIZE, FALSE, CheckerLeftSortType == stSize);
        }

        if (leftSmartMode != CheckerLeftSmartMode || IdleForceRefresh)
        {
            CheckerLeftSmartMode = leftSmartMode;
            toolbar->CheckItem(CM_LEFT_SMARTMODE, FALSE, CheckerLeftSmartMode == TRUE);
        }
    }

    toolbar = obj->RightPanel->DirectoryLine->ToolBar;
    if (toolbar != NULL && toolbar->HWindow != NULL)
    {
        if (rightViewMode != CheckerRightViewMode || IdleForceRefresh)
        {
            CheckerRightViewMode = rightViewMode;
            toolbar->CheckItem(CM_RIGHTMODE_2, FALSE, CheckerRightViewMode == 1);
            toolbar->CheckItem(CM_RIGHTMODE_3, FALSE, CheckerRightViewMode == 2);
        }

        if (rightSortType != CheckerRightSortType || IdleForceRefresh)
        {
            CheckerRightSortType = rightSortType;
            toolbar->CheckItem(CM_RIGHTNAME, FALSE, CheckerRightSortType == stName);
            toolbar->CheckItem(CM_RIGHTEXT, FALSE, CheckerRightSortType == stExtension);
            toolbar->CheckItem(CM_RIGHTTIME, FALSE, CheckerRightSortType == stTime);
            toolbar->CheckItem(CM_RIGHTSIZE, FALSE, CheckerRightSortType == stSize);
        }

        if (rightSmartMode != CheckerRightSmartMode || IdleForceRefresh)
        {
            CheckerRightSmartMode = rightSmartMode;
            toolbar->CheckItem(CM_RIGHT_SMARTMODE, FALSE, CheckerRightSmartMode == TRUE);
        }
    }
    obj->IdleStatesChanged = FALSE;
    IdleForceRefresh = FALSE;
}

void CMainWindow::RefreshCommandStates()
{
    CMainWindow_RefreshCommandStates(this); // tahle opicarna je tu jen kvuli tomu, ze neumim zjistit adresu metody objektu (takhle je to obyc. funkce, kde to umim)
}

void CMainWindow::OnEnterIdle()
{
    CALL_STACK_MESSAGE3("CMainWindow::OnEnterIdle(R:%d, C:%d)", IdleRefreshStates, IdleCheckClipboard);

    if (DisableIdleProcessing)
        return;

    // hlavni okno uz je jiste plne nastartovane, jinak by nedoslo k idle rezimu
    FirstActivateApp = FALSE;

    // kouknu, jestli si nekdo vyzadal kontrolu stavu
    if (IdleRefreshStates)
        RefreshCommandStates();

    // update plugin bar a drives bar, je-li treba
    if (!SalamanderBusy && PluginsStatesChanged)
    {
        if (PluginsBar != NULL && PluginsBar->HWindow != NULL)
            PluginsBar->CreatePluginButtons();

        CDriveBar* copyDrivesListFrom = NULL;
        if (DriveBar != NULL && DriveBar->HWindow != NULL)
        {
            DriveBar->RebuildDrives(DriveBar); // nepotrebujeme pomalou enumeraci disku
            copyDrivesListFrom = DriveBar;
        }
        if (DriveBar2 != NULL && DriveBar2->HWindow != NULL)
            DriveBar2->RebuildDrives(copyDrivesListFrom);

        PluginsStatesChanged = FALSE;
    }

    // nechame bottom toolbar uvest do aktualniho stavu (pokud v nem uz neni)
    UpdateBottomToolBar();

    // pokud doslo k rolovani nebo resize panelu, ulozime nove pole viditelnych polozek
    if (LeftPanel != NULL)
        LeftPanel->RefreshVisibleItemsArray();
    if (RightPanel != NULL)
        RightPanel->RefreshVisibleItemsArray();
}

void CMainWindow::OnColorsChanged(BOOL reloadUMIcons)
{
    // doslo ke zmene barev nebo barevne hloubky obrazovky; uz jsou vytvorene nove imagelisty
    // pro toolbary a je treba je priradit controlum, ktere je pouzivaji

    // top toolbar
    if (TopToolBar != NULL)
    {
        TopToolBar->SetImageList(HGrayToolBarImageList);
        TopToolBar->SetHotImageList(HHotToolBarImageList);
        TopToolBar->OnColorsChanged();
    }

    // middle toolbar
    if (MiddleToolBar != NULL)
    {
        MiddleToolBar->SetImageList(HGrayToolBarImageList);
        MiddleToolBar->SetHotImageList(HHotToolBarImageList);
        MiddleToolBar->OnColorsChanged();
    }

    // plugin bar
    if (PluginsBar != NULL)
    {
        PluginsBar->OnColorsChanged();
    }

    // user menu toolbar
    if (UMToolBar != NULL)
    {
        UMToolBar->OnColorsChanged(); // nova CacheBitmap
        if (reloadUMIcons)
        {
            CUserMenuIconDataArr* bkgndReaderData = new CUserMenuIconDataArr();
            // teoreticky hrozi, ze bude user menu otevrene ve Findu a prijde nam WM_COLORCHANGE (tim bysme sestrelili
            // ikony menu ve Findu pod nohama), ale prakticky to snad nehrozi, kaslu na to ;-) Petr
            // pokud bych to casem resil: resit tez situaci, kdy je otevrena cfg a dojde ke zmene barev,
            // po zavreni cfg pres OK (nova verze user menu) se musi nacist ikony znovu
            for (int i = 0; i < UserMenuItems->Count; i++)
                UserMenuItems->At(i)->GetIconHandle(bkgndReaderData, FALSE);
            UserMenuIconBkgndReader.StartBkgndReadingIcons(bkgndReaderData); // POZOR: uvolni 'bkgndReaderData'
        }
        UMToolBar->CreateButtons(); // doslo ke zmene handlu group ikonky
    }

    // hot path bar
    if (HPToolBar != NULL)
    {
        HPToolBar->OnColorsChanged(); // nova CacheBitmap
        HPToolBar->CreateButtons();   // doslo ke zmene handlu ikonky HFavoritIcon
    }

    // drive bars
    CDriveBar* copyDrivesListFrom = NULL;
    if (DriveBar != NULL)
    {
        DriveBar->OnColorsChanged(); // nova CacheBitmap
        DriveBar->RebuildDrives();   // nechame nacist nove ikonky
        copyDrivesListFrom = DriveBar;
    }
    if (DriveBar2 != NULL)
    {
        DriveBar2->OnColorsChanged();                 // nova CacheBitmap
        DriveBar2->RebuildDrives(copyDrivesListFrom); // nechame nacist nove ikonky
    }

    // bottom toolbar
    if (BottomToolBar != NULL)
    {
        BottomToolBar->SetImageList(HBottomTBImageList);
        BottomToolBar->SetHotImageList(HHotBottomTBImageList);
        BottomToolBar->OnColorsChanged();
    }

    // main menu
    MainMenu.SetImageList(HGrayToolBarImageList, TRUE);
    MainMenu.SetHotImageList(HHotToolBarImageList, TRUE);

    // archive menu
    ArchiveMenu.SetImageList(HGrayToolBarImageList, TRUE);
    ArchiveMenu.SetHotImageList(HHotToolBarImageList, TRUE);
    ArchivePanelMenu.SetImageList(HGrayToolBarImageList, TRUE);
    ArchivePanelMenu.SetHotImageList(HHotToolBarImageList, TRUE);

    // left/right panel
    if (LeftPanel != NULL)
    {
        LeftPanel->OnColorsChanged();
    }

    if (RightPanel != NULL)
    {
        RightPanel->OnColorsChanged();
    }
}

void CMainWindow::StartAnimate()
{
    //AnimateBar->Start();
}

void CMainWindow::StopAnimate()
{
    //AnimateBar->Stop();
}
