// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_filesList.h"
#include "find_dialog.h"
#include "find.old.h"
#include "plugins.h"
#include "cfgdlg.h"
#include "mainwnd.h"


LRESULT Find_FilesList_View::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SLOW_CALL_STACK_MESSAGE4( "Find_FilesList_View::WindowProc(0x%X, 0x%IX, 0x%IX)", uMsg, wParam, lParam );

    switch (uMsg)
    {
    case WM_GETDLGCODE:
    {
        if (lParam != NULL)
        {
            // pokud jde o Enter, tak ho chceme zpracovat (jinak se Enter nedoruci)
            MSG* msg = (LPMSG)lParam;
            if (msg->message == WM_KEYDOWN && msg->wParam == VK_RETURN &&
                ListView_GetItemCount(HWindow) > 0)
                return DLGC_WANTMESSAGE;
        }

        return DLGC_WANTCHARS | DLGC_WANTARROWS;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        BOOL altPressed = (GetKeyState(VK_MENU) & 0x8000) != 0;
        BOOL shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        /* vypada to, ze tenhle kod uz je zbytecnej, resime to ve wndproc dialogu
      if (wParam == VK_RETURN)
      {
        if (altPressed)
        {
          FindDialog->OnProperties();
          FindDialog->SkipCharacter = TRUE;
        }
        else
          FindDialog->OnOpen();
        return TRUE;
      }
*/
        if ((wParam == VK_F10 && shiftPressed || wParam == VK_APPS))
        {
            POINT p;
            GetListViewContextMenuPos(HWindow, &p);
            m_FindDialog.OnContextMenu(p.x, p.y);
            return TRUE;
        }

    //Let base class handle others.
        break;
    }
    case WM_KILLFOCUS:
    {
        HWND next = (HWND)wParam;
        BOOL nextIsButton;
        if (next != NULL)
        {
            char className[30];
            WORD wl = LOWORD(GetWindowLongPtr(next, GWL_STYLE)); // jen BS_...
            nextIsButton = (GetClassName(next, className, 30) != 0 &&
                            StrICmp(className, "BUTTON") == 0 &&
                            (wl == BS_PUSHBUTTON || wl == BS_DEFPUSHBUTTON));
        }
        else
            nextIsButton = FALSE;
        SendMessage(GetParent(HWindow), WM_USER_BUTTONS, nextIsButton ? wParam : 0, 0);
        break;
    }
    case WM_MOUSEACTIVATE:
    {
        // pokud je Find neaktivni a uzivatel chce pres drag&drop odtahnout
        // nekterou z polozek, nesmi Find vyskocit nahoru
        return MA_NOACTIVATE;
    }
    case WM_SETFOCUS:
    {
        SendMessage(GetParent(HWindow), WM_USER_BUTTONS, 0, 0);
        break;
    }
    case WM_USER_ENUMFILENAMES: // hledani dalsiho/predchoziho jmena pro viewer
    {
        HANDLES(EnterCriticalSection(&FileNamesEnumDataSect));
        if (
            ( (int)wParam /* reqUID */ == FileNamesEnumData.RequestUID )  // nedoslo k zadani dalsiho pozadaku (tento by pak byl k nicemu)
            &&
            ( EnumFileNamesSourceUID == FileNamesEnumData.SrcUID )       // nedoslo ke zmene zdroje
            &&
            ( !FileNamesEnumData.TimedOut )                                // na vysledek jeste nekdo ceka
        )
        {
            HANDLES(EnterCriticalSection(&m_CriticalSection));

            BOOL selExists = FALSE;
            if (FileNamesEnumData.PreferSelected) // je-li to treba, zjistime jestli existuje selectiona
            {
                int i = -1;
                int selCount = 0; // musime ignorovat stav, kdy je jedina oznacena polozka fokus (to logicky nelze povazovat za oznacene polozky)
                while (1)
                {
                    i = ListView_GetNextItem(HWindow, i, LVNI_SELECTED);
                    if (i == -1)
                        break;
                    else
                    {
                        selCount++;
                        if (!m_Items[i]->IsDir)
                            selExists = TRUE;
                        if (selCount > 1 && selExists)
                            break;
                    }
                }
                if (selExists && selCount <= 1)
                    selExists = FALSE;
            }

            int index = FileNamesEnumData.LastFileIndex;
            int count = m_Items.Count;
            BOOL indexNotFound = TRUE;
            if (index == -1) // hledame od prvniho nebo od posledniho
            {
                if (FileNamesEnumData.RequestType == fnertFindPrevious)
                    index = count; // hledame predchozi + mame zacit na poslednim
                                   // else  // hledame nasledujici + mame zacit na prvnim
            }
            else
            {
                if (FileNamesEnumData.LastFileName[0] != 0) // zname plne jmeno souboru na 'index', zkontrolujeme jestli nedoslo k rozesunuti/sesunuti pole + pripadne dohledame novy index
                {
                    BOOL ok = FALSE;
                    Find_FileList_Item* f = (index >= 0 && index < count) ? m_Items[index] : NULL;
                    char fileName[MAX_PATH];
                    if (f != NULL && f->Path != NULL && f->Name != NULL)
                    {
                        lstrcpyn(fileName, f->Path, MAX_PATH);
                        SalPathAppend(fileName, f->Name, MAX_PATH);
                        if (StrICmp(fileName, FileNamesEnumData.LastFileName) == 0)
                        {
                            ok = TRUE;
                            indexNotFound = FALSE;
                        }
                    }
                    if (!ok)
                    { // jmeno na indexu 'index' neni FileNamesEnumData.LastFileName, zkusime najit novy index tohoto jmena
                        int i;
                        for (i = 0; i < count; i++)
                        {
                            f = m_Items[i];
                            if (f->Path != NULL && f->Name != NULL)
                            {
                                lstrcpyn(fileName, f->Path, MAX_PATH);
                                SalPathAppend(fileName, f->Name, MAX_PATH);
                                if (StrICmp(fileName, FileNamesEnumData.LastFileName) == 0)
                                    break;
                            }
                        }
                        if (i != count) // novy index nalezen
                        {
                            index = i;
                            indexNotFound = FALSE;
                        }
                    }
                }
                if (index >= count)
                {
                    if (FileNamesEnumData.RequestType == fnertFindNext)
                        index = count - 1;
                    else
                        index = count;
                }
                if (index < 0)
                    index = 0;
            }

            int wantedViewerType = 0;
            BOOL onlyAssociatedExtensions = FALSE;
            if (FileNamesEnumData.OnlyAssociatedExtensions) // preje si viewer filtrovani podle asociovanych pripon?
            {
                if (FileNamesEnumData.Plugin != NULL) // viewer z pluginu
                {
                    int pluginIndex = Plugins.GetIndex(FileNamesEnumData.Plugin);
                    if (pluginIndex != -1) // "always true"
                    {
                        wantedViewerType = -1 - pluginIndex;
                        onlyAssociatedExtensions = TRUE;
                    }
                }
                else // interni viewer
                {
                    wantedViewerType = VIEWER_INTERNAL;
                    onlyAssociatedExtensions = TRUE;
                }
            }

            BOOL preferSelected = selExists && FileNamesEnumData.PreferSelected;
            switch (FileNamesEnumData.RequestType)
            {
            case fnertFindNext: // dalsi
            {
                String_TChar  strViewerMasks;

                if ( MainWindow->GetViewersAssoc(wantedViewerType, strViewerMasks) )
                {
                    CMaskGroup masks;
                    int errorPos;
                    if (masks.PrepareMasks(errorPos, strViewerMasks.Text_Get()))
                    {
                        while (index + 1 < count)
                        {
                            index++;
                            if (preferSelected)
                            {
                                int i = ListView_GetNextItem(HWindow, index - 1, LVNI_SELECTED);
                                if (i != -1)
                                {
                                    index = i;
                                    if (!m_Items[index]->IsDir) // hledame jen soubory
                                    {
                                        if (!onlyAssociatedExtensions || masks.AgreeMasks(m_Items[index]->Name, NULL))
                                        {
                                            FileNamesEnumData.Found = TRUE;
                                            break;
                                        }
                                    }
                                }
                                else
                                    index = count - 1;
                            }
                            else
                            {
                                if (!m_Items[index]->IsDir)
                                {
                                    if (!onlyAssociatedExtensions || masks.AgreeMasks(m_Items[index]->Name, NULL))
                                    {
                                        FileNamesEnumData.Found = TRUE;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    else
                        TRACE_E("Unexpected situation in Find::WM_USER_ENUMFILENAMES: grouped viewer's masks can't be prepared for use!");
                }
                break;
            }

            case fnertFindPrevious: // predchozi
            {
                String_TChar strViewerMasks;
                if (MainWindow->GetViewersAssoc(wantedViewerType, strViewerMasks))
                {
                    CMaskGroup masks;
                    int errorPos;
                    if (masks.PrepareMasks(errorPos, strViewerMasks.Text_Get()))
                    {
                        while (index - 1 >= 0)
                        {
                            index--;
                            if (!m_Items[index]->IsDir &&
                                (!preferSelected ||
                                 (ListView_GetItemState(HWindow, index, LVIS_SELECTED) & LVIS_SELECTED)))
                            {
                                if (!onlyAssociatedExtensions || masks.AgreeMasks(m_Items[index]->Name, NULL))
                                {
                                    FileNamesEnumData.Found = TRUE;
                                    break;
                                }
                            }
                        }
                    }
                    else
                        TRACE_E("Unexpected situation in Find::WM_USER_ENUMFILENAMES: grouped viewer's masks can't be prepared for use!");
                }
                break;
            }

            case fnertIsSelected: // zjisteni oznaceni
            {
                if (!indexNotFound && index >= 0 && index < m_Items.Count)
                {
                    FileNamesEnumData.IsFileSelected = (ListView_GetItemState(HWindow, index, LVIS_SELECTED) & LVIS_SELECTED) != 0;
                    FileNamesEnumData.Found = TRUE;
                }
                break;
            }

            case fnertSetSelection: // nastaveni oznaceni
            {
                if (!indexNotFound && index >= 0 && index < m_Items.Count)
                {
                    ListView_SetItemState(HWindow, index, FileNamesEnumData.Select ? LVIS_SELECTED : 0, LVIS_SELECTED);
                    FileNamesEnumData.Found = TRUE;
                }
                break;
            }
            }

            if (FileNamesEnumData.Found)
            {
                Find_FileList_Item* f = m_Items[index];
                if (f->Path != NULL && f->Name != NULL)
                {
                    lstrcpyn(FileNamesEnumData.FileName, f->Path, MAX_PATH);
                    SalPathAppend(FileNamesEnumData.FileName, f->Name, MAX_PATH);
                    FileNamesEnumData.LastFileIndex = index;
                }
                else // nikdy by nemelo nastat
                {
                    TRACE_E("Unexpected situation in Find_FilesList_View::WindowProc(): handling of WM_USER_ENUMFILENAMES");
                    FileNamesEnumData.Found = FALSE;
                    FileNamesEnumData.NoMoreFiles = TRUE;
                }
            }
            else
                FileNamesEnumData.NoMoreFiles = TRUE;

            HANDLES(LeaveCriticalSection(&m_CriticalSection));
            SetEvent(FileNamesEnumDone);
        }
        HANDLES(LeaveCriticalSection(&FileNamesEnumDataSect));
        return 0;
    }
    }

//Call base class.
    return CWindow::WindowProc( uMsg, wParam, lParam );
}
