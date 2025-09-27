// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_dialog.h"
#include "find_data_umd.h"
#include "find_filesList.h"
#include "find.old.h"
#include "cfgdlg.h"
#include "execute.h"
#include "fileswnd.h"
#include "gui.h"
#include "mainwnd.h"
#include "menu.h"
#include "shellib.h"
#include "tasklist.h"
#include "viewer.h"

#include <shlwapi.h>

const char* MINIMIZED_FINDING_CAPTION = "(%d) %s [%s %s]";
const char* NORMAL_FINDING_CAPTION = "%s [%s %s]";

void Find_Dialog::GetLayoutParams()
{
    RECT wr;
    GetWindowRect(HWindow, &wr);
    MinDlgW = wr.right - wr.left;
    MinDlgH = wr.bottom - wr.top;

    RECT cr;
    GetClientRect(HWindow, &cr);

    RECT br;
    GetWindowRect(GetDlgItem(HWindow, IDC_FIND_RESULTS), &br);
    int windowMargin = ((wr.right - wr.left) - (cr.right)) / 2;
    HMargin = br.left - wr.left - windowMargin;
    VMargin = HMargin;

    int captionH = wr.bottom - wr.top - cr.bottom - windowMargin;
    ResultsY = br.top - wr.top - captionH;

    GetWindowRect(GetDlgItem(HWindow, IDOK), &br); //IDC_FIND_FINDNOW
    ButtonW = br.right - br.left;
    ButtonH = br.bottom - br.top;
    FindNowY = br.top - wr.top - captionH;

    GetWindowRect(GetDlgItem(HWindow, IDC_FIND_REGEXP_BROWSE), &br);
    RegExpButtonW = br.right - br.left;
    RegExpButtonY = br.top - wr.top - captionH;

    MenuBarHeight = MenuBar->GetNeededHeight();

    RECT r;
    GetWindowRect(HStatusBar, &r);
    StatusHeight = r.bottom - r.top;

    GetWindowRect(GetDlgItem(HWindow, IDC_FIND_NAMED), &r);
    CombosX = r.left - wr.left - windowMargin;

    GetWindowRect(GetDlgItem(HWindow, IDC_FIND_LOOKIN_BROWSE), &r);
    BrowseY = r.top - wr.top - captionH;

    GetWindowRect(GetDlgItem(HWindow, IDC_FIND_LINE2), &r);
    Line2X = r.left - wr.left - windowMargin;

    GetWindowRect(GetDlgItem(HWindow, IDC_FIND_SPACER), &r);
    SpacerH = r.bottom - r.top;

    GetWindowRect(GetDlgItem(HWindow, IDC_FIND_ADVANCED), &r);
    AdvancedY = r.top - wr.top - captionH;

    GetWindowRect(GetDlgItem(HWindow, IDC_FIND_ADVANCED_TEXT), &r);
    AdvancedTextY = r.top - wr.top - captionH;
    AdvancedTextX = r.left - wr.left - windowMargin;

    //  GetWindowRect(GetDlgItem(HWindow, IDC_FIND_FOUND_FILES), &r);
    FindTextH = TBHeader->GetNeededHeight();
    FindTextY = ResultsY - FindTextH;
}

void Find_Dialog::SetTwoStatusParts(BOOL two, BOOL force)
{
    int margin = HMargin - 4;
    int parts[3] = {margin, -1, -1};
    RECT r;
    GetClientRect(HStatusBar, &r);

    int gripWidth = HMargin;
    if (!IsZoomed(HWindow))
        gripWidth = GetSystemMetrics(SM_CXVSCROLL);

    int progressWidth = 0;
    int progressHeight = 0;
    if (two)
    {
        progressWidth = 104; // 100 + ramecek
        if (HProgressBar == NULL)
        {
            HProgressBar = CreateWindowEx(0, PROGRESS_CLASS, NULL,
                                          WS_CHILD | PBS_SMOOTH,
                                          0, 0,
                                          progressWidth, r.bottom - 2,
                                          HStatusBar, (HMENU)0, HInstance, NULL);
            SendMessage(HProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
        }
    }
    else
    {
        if (HProgressBar != NULL)
        {
            DestroyWindow(HProgressBar);
            HProgressBar = NULL;
        }
    }

    parts[1] = r.right - progressWidth - gripWidth;

    if (HProgressBar != NULL)
    {
        parts[1] -= 10; // zvetsime odstup od progress bary
        SetWindowPos(HProgressBar, NULL,
                     r.right - progressWidth - gripWidth, 2, 0, 0,
                     SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
    }

    if (TwoParts != two || force)
    {
        TwoParts = two;
        SendMessage(HStatusBar, WM_SETREDRAW, FALSE, 0); // pokud je povolene prekreslovani, na konci statusbary nam zustava parazitni ramecek
        SendMessage(HStatusBar, SB_SETPARTS, 3, (LPARAM)parts);
        SendMessage(HStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS, (LPARAM) "");
        SendMessage(HStatusBar, SB_SETTEXT, 2 | SBT_NOBORDERS, (LPARAM) "");
        SendMessage(HStatusBar, WM_SETREDRAW, TRUE, 0);
        InvalidateRect(HStatusBar, NULL, TRUE);
    }
    else
        SendMessage(HStatusBar, SB_SETPARTS, 3, (LPARAM)parts);
}

void Find_Dialog::LayoutControls()
{
    RECT clientRect;

    if (CombosH == 0)
    {
        RECT r;
        GetWindowRect(GetDlgItem(HWindow, IDC_FIND_NAMED), &r);
        if (r.bottom - r.top != 0)
            CombosH = r.bottom - r.top;
    }

    GetClientRect(HWindow, &clientRect);
    clientRect.bottom -= StatusHeight;

    HDWP hdwp = HANDLES(BeginDeferWindowPos(14));
    if (hdwp != NULL)
    {
        // prostor mezi tlacitky
        int buttonMargin = ButtonH / 3;

        // umistim MenuBar
        hdwp = HANDLES(DeferWindowPos(hdwp, MenuBar->HWindow, NULL,
                                      0, -1, clientRect.right, MenuBarHeight,
                                      SWP_NOZORDER));

        // umistim Status Bar
        hdwp = HANDLES(DeferWindowPos(hdwp, HStatusBar, NULL,
                                      0, clientRect.bottom, clientRect.right, StatusHeight,
                                      SWP_NOZORDER));

        // umistim tlacitko Advanced
        hdwp = HANDLES(DeferWindowPos(hdwp, GetDlgItem(HWindow, IDC_FIND_ADVANCED), NULL,
                                      HMargin, AdvancedY, 0, 0, SWP_NOSIZE | SWP_NOZORDER));

        // umitstim a natahnu editlajnu Advanced
        hdwp = HANDLES(DeferWindowPos(hdwp, GetDlgItem(HWindow, IDC_FIND_ADVANCED_TEXT), NULL,
                                      AdvancedTextX, AdvancedTextY, clientRect.right - AdvancedTextX - HMargin, CombosH,
                                      SWP_NOZORDER));

        // umistim text Found files
        hdwp = HANDLES(DeferWindowPos(hdwp, TBHeader->HWindow /*GetDlgItem(HWindow, IDC_FIND_FOUND_FILES)*/, NULL,
                                      HMargin, FindTextY, clientRect.right - 2 * HMargin, FindTextH, SWP_NOZORDER));

        // umistim a natahnu list view
        hdwp = HANDLES(DeferWindowPos(hdwp, GetDlgItem(HWindow, IDC_FIND_RESULTS), NULL,
                                      HMargin, ResultsY, clientRect.right - 2 * HMargin,
                                      clientRect.bottom - ResultsY /*- VMargin*/, SWP_NOZORDER));

        // umistim a natahnu oddelovaci caru pod menu
        hdwp = HANDLES(DeferWindowPos(hdwp, GetDlgItem(HWindow, IDC_FIND_LINE1), NULL,
                                      0, MenuBarHeight - 1, clientRect.right, 2, SWP_NOZORDER));

        // natahnu combobox Named
        hdwp = HANDLES(DeferWindowPos(hdwp, GetDlgItem(HWindow, IDC_FIND_NAMED), NULL,
                                      0, 0, clientRect.right - CombosX - HMargin - ButtonW - buttonMargin, CombosH,
                                      SWP_NOMOVE | SWP_NOZORDER));

        // umistim tlacitko Find Now
        hdwp = HANDLES(DeferWindowPos(hdwp, GetDlgItem(HWindow, IDOK), NULL, //IDC_FIND_FINDNOW
                                      clientRect.right - HMargin - ButtonW, FindNowY, 0, 0,
                                      SWP_NOSIZE | SWP_NOZORDER));

        // natahnu combobox Look in
        hdwp = HANDLES(DeferWindowPos(hdwp, GetDlgItem(HWindow, IDC_FIND_LOOKIN), NULL,
                                      0, 0, clientRect.right - CombosX - HMargin - ButtonW - buttonMargin, CombosH,
                                      SWP_NOMOVE | SWP_NOZORDER));

        // umistim tlacitko Browse
        hdwp = HANDLES(DeferWindowPos(hdwp, GetDlgItem(HWindow, IDC_FIND_LOOKIN_BROWSE), NULL,
                                      clientRect.right - HMargin - ButtonW, BrowseY, 0, 0,
                                      SWP_NOSIZE | SWP_NOZORDER));

        // natahnu combobox Containing
        hdwp = HANDLES(DeferWindowPos(hdwp, GetDlgItem(HWindow, IDC_FIND_CONTAINING), NULL,
                                      0, 0, clientRect.right - CombosX - HMargin - RegExpButtonW - buttonMargin, CombosH,
                                      SWP_NOMOVE | SWP_NOZORDER));

        // umistim tlacitko Regular Expression Browse
        hdwp = HANDLES(DeferWindowPos(hdwp, GetDlgItem(HWindow, IDC_FIND_REGEXP_BROWSE), NULL,
                                      clientRect.right - HMargin - RegExpButtonW, RegExpButtonY, 0, 0,
                                      SWP_NOSIZE | SWP_NOZORDER));

        // natahnu oddelovaci caru u Search file content
        hdwp = HANDLES(DeferWindowPos(hdwp, GetDlgItem(HWindow, IDC_FIND_LINE2), NULL,
                                      0, 0, clientRect.right - Line2X - HMargin, 2,
                                      SWP_NOMOVE | SWP_NOZORDER));

        HANDLES(EndDeferWindowPos(hdwp));
    }
    SetTwoStatusParts(TwoParts);
}

void Find_Dialog::SetContentVisible(BOOL visible)
{
    if (Expanded != visible)
    {
        if (visible)
        {
            ResultsY += SpacerH;
            AdvancedY += SpacerH;
            AdvancedTextY += SpacerH;
            FindTextY += SpacerH;

            MinDlgH += SpacerH;
            LayoutControls();
            if (!IsIconic(HWindow) && !IsZoomed(HWindow))
            {
                RECT wr;
                GetWindowRect(HWindow, &wr);
                if (wr.bottom - wr.top < MinDlgH)
                {
                    SetWindowPos(HWindow, NULL, 0, 0, wr.right - wr.left, MinDlgH,
                                 SWP_NOMOVE | SWP_NOZORDER);
                }
            }
        }

        // pokud se polozky zjevuji, musim je povolit
        Expanded = visible;
        if (visible)
        {
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_CONTAINING_TEXT), TRUE);
            EnableControls();
        }

        ShowWindow(GetDlgItem(HWindow, IDC_FIND_CONTAINING_TEXT), visible);
        ShowWindow(GetDlgItem(HWindow, IDC_FIND_CONTAINING), visible);
        ShowWindow(GetDlgItem(HWindow, IDC_FIND_REGEXP_BROWSE), visible);
        ShowWindow(GetDlgItem(HWindow, IDC_FIND_HEX), visible);
        ShowWindow(GetDlgItem(HWindow, IDC_FIND_CASE), visible);
        ShowWindow(GetDlgItem(HWindow, IDC_FIND_WHOLE), visible);
        ShowWindow(GetDlgItem(HWindow, IDC_FIND_REGULAR), visible);

        if (!visible)
        {
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_CONTAINING_TEXT), FALSE);
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_CONTAINING), FALSE);
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_REGEXP_BROWSE), FALSE);
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_HEX), FALSE);
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_CASE), FALSE);
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_WHOLE), FALSE);
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_REGULAR), FALSE);
        }

        if (!visible)
        {
            ResultsY -= SpacerH;
            AdvancedY -= SpacerH;
            AdvancedTextY -= SpacerH;
            FindTextY -= SpacerH;
            MinDlgH -= SpacerH;
            LayoutControls();
        }
    }
}

void Find_Dialog::Validate(CTransferInfo& ti)
{
    CALL_STACK_MESSAGE1("Find_Dialog::Validate()");

    HWND hNamesWnd;
    HWND hLookInWnd;

    if (ti.GetControl(hNamesWnd, IDC_FIND_NAMED) &&
        ti.GetControl(hLookInWnd, IDC_FIND_LOOKIN))
    {
        // zaloha dat
        char bufNamed[NAMED_TEXT_LEN];
        char bufLookIn[LOOKIN_TEXT_LEN];
        strcpy(bufNamed, Data.NamedText);
        strcpy(bufLookIn, Data.LookInText);

        SendMessage(hNamesWnd, WM_GETTEXT, NAMED_TEXT_LEN, (LPARAM)Data.NamedText);
        CMaskGroup mask(Data.NamedText);
        int errorPos;
        if (!mask.PrepareMasks(errorPos))
        {
            SalMessageBox(HWindow, LoadStr(IDS_INCORRECTSYNTAX), LoadStr(IDS_ERRORTITLE),
                          MB_OK | MB_ICONEXCLAMATION);
            SetFocus(hNamesWnd); // aby korektne probehla message CB_SETEDITSEL
            SendMessage(hNamesWnd, CB_SETEDITSEL, 0, MAKELPARAM(errorPos, errorPos + 1));
            ti.ErrorOn(IDC_FIND_NAMED);
        }

        if (ti.IsGood())
        {
            SendMessage(hLookInWnd, WM_GETTEXT, LOOKIN_TEXT_LEN, (LPARAM)Data.LookInText);

            BuildSerchForData();
            if (SearchForData.Count == 0)
            {
                SalMessageBox(HWindow, LoadStr(IDS_FF_EMPTYSTRING), LoadStr(IDS_ERRORTITLE),
                              MB_OK | MB_ICONEXCLAMATION);
                ti.ErrorOn(IDC_FIND_LOOKIN);
            }
        }

        // obnovime data ze zalohy
        strcpy(Data.LookInText, bufLookIn);
        strcpy(Data.NamedText, bufNamed);
    }
}

void Find_Dialog::Transfer(CTransferInfo& ti)
{
    HistoryComboBox(HWindow, ti, IDC_FIND_NAMED, Data.NamedText, NAMED_TEXT_LEN,
                    FALSE, FIND_NAMED_HISTORY_SIZE, FindNamedHistory);
    HistoryComboBox(HWindow, ti, IDC_FIND_LOOKIN, Data.LookInText, LOOKIN_TEXT_LEN,
                    FALSE, FIND_LOOKIN_HISTORY_SIZE, FindLookInHistory);

    ti.CheckBox(IDC_FIND_INCLUDE_SUBDIR, Data.SubDirectories);
    HistoryComboBox(HWindow, ti, IDC_FIND_CONTAINING, Data.GrepText, GREP_TEXT_LEN,
                    !Data.RegularExpresions && Data.HexMode, FIND_GREP_HISTORY_SIZE,
                    FindGrepHistory);
    ti.CheckBox(IDC_FIND_HEX, Data.HexMode);
    ti.CheckBox(IDC_FIND_CASE, Data.CaseSensitive);
    ti.CheckBox(IDC_FIND_WHOLE, Data.WholeWords);
    ti.CheckBox(IDC_FIND_REGULAR, Data.RegularExpresions);
}

void Find_Dialog::UpdateAdvancedText()
{
    char buff[200];
    BOOL dirty;
    Data.Criteria.GetAdvancedDescription(buff, 200, dirty);
    SetDlgItemText(HWindow, IDC_FIND_ADVANCED_TEXT, buff);
    EnableWindow(GetDlgItem(HWindow, IDC_FIND_ADVANCED_TEXT), dirty);
}

void Find_Dialog::LoadControls(int index)
{
    CALL_STACK_MESSAGE2("Find_Dialog::LoadControls(0x%X)", index);
    Data = *FindOptions.Items_At(index);

    // pokud je nektera z editlajn prazdna, zachovame puvodni hodnotu
    if (Data.NamedText[0] == 0)
        GetDlgItemText(HWindow, IDC_FIND_NAMED, Data.NamedText, NAMED_TEXT_LEN);
    if (Data.LookInText[0] == 0)
        GetDlgItemText(HWindow, IDC_FIND_LOOKIN, Data.LookInText, LOOKIN_TEXT_LEN);
    if (Data.GrepText[0] == 0)
        GetDlgItemText(HWindow, IDC_FIND_CONTAINING, Data.GrepText, GREP_TEXT_LEN);

    TransferData(ttDataToWindow);

    // pokud je neco v grepu a dialog neni expanded, roztahnu ho
    if (Data.GrepText[0] != 0 && !Expanded)
    {
        CheckDlgButton(HWindow, IDC_FIND_GREP, TRUE);
        SetContentVisible(TRUE);
    }

    UpdateAdvancedText();
    EnableControls();
}

void Find_Dialog::BuildSerchForData()
{
    char named[NAMED_TEXT_LEN + NAMED_TEXT_LEN];
    char* begin;
    char* end;

    // Lide chteji v maskach zadavat pouze "jsem_pako", aby nasli soubory "*jsem_pako*".
    // Proto musime prohledat kazdou polozku z mask group a pokud neobsahuje nejaky
    // wildcard nebo znak '.', oblozime ho hvezdickama.
    char* iterator = named;
    begin = Data.NamedText;
    while (1)
    {
        end = begin;
        while (*end != 0)
        {
            if (*end == '|')
                break;
            if (*end == ';')
            {
                if (*(end + 1) != ';')
                    break;
                else
                    end++;
            }
            end++;
        }
        while (*begin != 0 && *begin <= ' ')
            begin++; // preskoceni mezer na zacatku
        char* tmpEnd = end;
        while (tmpEnd > begin && *(tmpEnd - 1) <= ' ')
            tmpEnd--; // preskoceni mezer na konci
        if (tmpEnd > begin)
        {
            // zjistime, jestli substring obsahuje wildcard '*', '?', '.'
            BOOL wildcard = FALSE;
            char* tmp = begin;
            while (tmp < tmpEnd)
            {
                if (*tmp == '*' || *tmp == '?' || *tmp == '.')
                {
                    wildcard = TRUE;
                    break;
                }
                tmp++;
            }

            if (!wildcard)
                *iterator++ = '*'; // zadny wildcard - oblozime zleva hvezdickou

            memcpy(iterator, begin, tmpEnd - begin);
            iterator += tmpEnd - begin;

            if (!wildcard)
                *iterator++ = '*'; // zadny wildcard - oblozime zprava hvezdickou
        }
        *iterator++ = *end;
        if (*end != 0)
            begin = end + 1;
        else
            break;
    }

    if (named[0] == 0)
        strcpy(named, "*"); // prazdny retezec nahradime znakem '*'

    SearchForData.DestroyMembers();

    char path[MAX_PATH];
    lstrcpy(path, Data.LookInText);
    begin = path;
    do
    {
        end = begin;
        while (*end != 0)
        {
            if (*end == ';')
            {
                if (*(end + 1) != ';')
                    break;
                else
                    memmove(end, end + 1, strlen(end + 1) + 1); // provedeme sesun (";;" -> ";")
            }
            end++;
        }
        char* tmp = end - 1;
        if (*end == ';')
        {
            *end = 0;
            end++;
        }
        // while (*end == ';') end++;   // je "always-false", protoze ";;" -> ";" a jde o normalni znak a ne oddelovac

        // odriznu mezery pred cestou
        while (*begin == ' ')
            begin++;
        // odriznu mezery za cestou
        if (tmp > begin)
        {
            while (tmp > begin && *tmp <= ' ')
                tmp--;
            *(tmp + 1) = 0; // bud uz tam '\0' je nebo ji tam dame
        }
        // odriznu prebytecne slashe+backslashe na konci cesty (necham tam max. jeden)
        if (tmp > begin)
        {
            while (tmp > begin && (*tmp == '/' || *tmp == '\\'))
                tmp--;
            if (*(tmp + 1) == '/' || *(tmp + 1) == '\\')
                tmp++;      // jeden tam nechame
            *(tmp + 1) = 0; // bud uz tam '\0' je nebo ji tam dame
        }

        if (*begin != 0)
        {
            Find_Data_Search* item = new Find_Data_Search(begin, named, Data.SubDirectories);
            if (item != NULL)
            {
                SearchForData.Add(item);
                if (!SearchForData.IsGood())
                {
                    SearchForData.ResetState();
                    delete item;
                    return;
                }
            }
        }
        if (*end != 0)
            begin = end;
    } while (*end != 0);
}

void Find_Dialog::StartSearch(WORD command)
{
    CALL_STACK_MESSAGE1("Find_Dialog::StartSearch()");
    if (FoundFilesListView == NULL || GrepThread != NULL)
        return;

    // pokud mame hledat duplikaty, doptame se na options
    CFindDuplicatesDialog findDupDlg(HWindow);
    if (command == CM_FIND_DUPLICATES)
    {
        if (findDupDlg.Execute() != IDOK)
            return;

        // radeji zkontrolujeme vystupni promenne
        if (!findDupDlg.SameName && !findDupDlg.SameSize)
        {
            TRACE_E("Invalid output from CFindDuplicatesDialog dialog.");
            return;
        }
    }

    TBHeader->SetFoundCount(0);
    TBHeader->SetErrorsInfosCount(0, 0);

    EnumFileNamesChangeSourceUID(FoundFilesListView->HWindow, &(FoundFilesListView->EnumFileNamesSourceUID));

    ListView_SetItemCount(FoundFilesListView->HWindow, 0);
    UpdateWindow(FoundFilesListView->HWindow);

    // pokud jsme drzeli nejake chyby ze stareho hledani, ted je uvolnime
    Log.Items_Clean();

    GrepData.FindDuplicates = FALSE;
    GrepData.FindDupFlags = 0;

    GrepData.Refine = 0; // no refine

    switch (command)
    {
    case IDOK:
    case CM_FIND_NOW:
    {
        FoundFilesListView->Items_DestroyMembers();
        break;
    }

    case CM_FIND_INTERSECT:
    {
        // pokud jde o refine, prevezmeme data do pole DataForRefine
        FoundFilesListView->TakeDataForRefine();
        GrepData.Refine = 1;
        break;
    }

    case CM_FIND_SUBTRACT:
    {
        // pokud jde o refine, prevezmeme data do pole DataForRefine
        FoundFilesListView->TakeDataForRefine();
        GrepData.Refine = 2;
        break;
    }

    case CM_FIND_APPEND:
    {
        break;
    }

    case CM_FIND_DUPLICATES:
    {
        GrepData.FindDuplicates = TRUE;
        GrepData.FindDupFlags = 0;
        if (findDupDlg.SameName)
            GrepData.FindDupFlags |= FIND_DUPLICATES_NAME;
        if (findDupDlg.SameSize)
            GrepData.FindDupFlags |= FIND_DUPLICATES_SIZE;
        if (findDupDlg.SameContent)
            GrepData.FindDupFlags |= FIND_DUPLICATES_SIZE | FIND_DUPLICATES_CONTENT;

        FoundFilesListView->Items_DestroyMembers();
        break;
    }
    }
    UpdateListViewItems();

    if (Data.GrepText[0] == 0)
        GrepData.Grep = FALSE;
    else
    {
        GrepData.EOL_CRLF = Configuration.EOL_CRLF;
        GrepData.EOL_CR = Configuration.EOL_CR;
        GrepData.EOL_LF = Configuration.EOL_LF;
        //    GrepData.EOL_NULL = Configuration.EOL_NULL;   // na to nemam regexp :(
        GrepData.Regular = Data.RegularExpresions;
        GrepData.WholeWords = Data.WholeWords;
        if (Data.RegularExpresions)
        {
            if (!GrepData.RegExp.Set(Data.GrepText, (WORD)(sfForward |
                                                           (Data.CaseSensitive ? sfCaseSensitive : 0))))
            {
                char buf[500];
                if (GrepData.RegExp.GetPattern() != NULL)
                    sprintf(buf, LoadStr(IDS_INVALIDREGEXP), GrepData.RegExp.GetPattern(), GrepData.RegExp.GetLastErrorText());
                else
                    strcpy(buf, GrepData.RegExp.GetLastErrorText());
                SalMessageBox(HWindow, buf, LoadStr(IDS_ERRORFINDINGFILE), MB_OK | MB_ICONEXCLAMATION);
                if (GrepData.Refine != 0)
                    FoundFilesListView->DestroyDataForRefine();
                return; // error
            }
            GrepData.Grep = TRUE;
        }
        else
        {
            if (Data.HexMode)
            {
                char hex[GREP_TEXT_LEN];
                int len;
                ConvertHexToString(Data.GrepText, hex, len);
                GrepData.SearchData.Set(hex, len, (WORD)(sfForward | (Data.CaseSensitive ? sfCaseSensitive : 0)));
            }
            else
                GrepData.SearchData.Set(Data.GrepText, (WORD)(sfForward |
                                                              (Data.CaseSensitive ? sfCaseSensitive : 0)));
            GrepData.Grep = GrepData.SearchData.IsGood();
        }
    }
    SetFocus(FoundFilesListView->HWindow);

    BuildSerchForData();
    GrepData.Data = &SearchForData;
    GrepData.StopSearch = FALSE;
    GrepData.HWindow = HWindow;

    // advanced search
    memmove(&GrepData.Criteria, &Data.Criteria, sizeof(Data.Criteria));

    GrepData.FoundFilesListView = FoundFilesListView;
    GrepData.FoundVisibleCount = 0;
    GrepData.FoundVisibleTick = GetTickCount();

    GrepData.SearchingText = &SearchingText;
    GrepData.SearchingText2 = &SearchingText2;

    DWORD threadId;
    GrepThread = HANDLES(CreateThread(NULL, 0, GrepThreadF, &GrepData, 0, &threadId));
    if (GrepThread == NULL)
    {
        TRACE_E("Unable to start GrepThread thread.");
        if (GrepData.Refine != 0)
            FoundFilesListView->DestroyDataForRefine();
        return;
    }

    if (OKButton != NULL) // shodime dropdown
    {
        DWORD flags = OKButton->GetFlags();
        flags &= ~BTF_DROPDOWN;
        OKButton->SetFlags(flags, FALSE);
    }
    SetDlgItemText(HWindow, IDOK, LoadStr(IDS_FF_STOP));

    SearchInProgress = TRUE;

    // rozjedeme timer pro aktualizaci Dirty textu
    SetTimer(HWindow, IDT_REPAINT, 100, NULL);

    // vynutime si prvni refresh status-bary
    SearchingText.Redraw_Set(TRUE);
    PostMessage(HWindow, WM_TIMER, IDT_REPAINT, 0);

    char buff[MAX_PATH + 100];
    _snprintf_s(buff, _TRUNCATE, NORMAL_FINDING_CAPTION, LoadStr(IDS_FF_NAME), LoadStr(IDS_FF_NAMED), SearchForData[0]->MasksGroup.GetMasksString());
    SetWindowText(HWindow, buff);

    EnableControls();
}

void Find_Dialog::StopSearch()
{
    CALL_STACK_MESSAGE1("Find_Dialog::StopSearch()");
    GrepData.StopSearch = TRUE;
    MSG msg;
    while (1)
    {
        BOOL oldCanClose = CanClose;
        CanClose = FALSE; // nenechame se zavrit, jsme uvnitr metody

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        { // message loopa pro message od grep-threadu
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        CanClose = oldCanClose;
        if (GrepThread == NULL)
            return; // DispatchMessage nas muze zavolat znovu a zavirani uz jsme provedli
        if (WaitForSingleObject(GrepThread, 100) != WAIT_TIMEOUT)
            break;
    }
    if (GrepThread != NULL)
        HANDLES(CloseHandle(GrepThread));
    GrepThread = NULL;

    SearchInProgress = FALSE;
    if (OKButton != NULL) // nahodime dropdown
    {
        DWORD flags = OKButton->GetFlags();
        flags |= BTF_DROPDOWN;
        OKButton->SetFlags(flags, FALSE);
    }
    SetDlgItemText(HWindow, IDOK, FindNowText);

    // stopneme timer pro aktualizaci textu
    KillTimer(HWindow, IDT_REPAINT);

    // pokud behem hledani doslo k zobrazeni druheho textu, je cas ho schovat
    if (TwoParts)
    {
        SearchingText2.String_Set("");
        SetTwoStatusParts(FALSE);
    }

    SearchingText.String_Set("");
    UpdateStatusBar = FALSE;
    if (!GrepData.SearchStopped)
    {
        DWORD items = FoundFilesListView->Items_Count();
        if (items == 0)
        {
            int msgID = GrepData.FindDuplicates ? IDS_FIND_NO_DUPS_FOUND : IDS_FIND_NO_FILES_FOUND;
            SendMessage(HStatusBar, SB_SETTEXT, 1 | SBT_NOBORDERS, (LPARAM)LoadStr(msgID));
        }
        else
            UpdateStatusBar = TRUE;

        if (Log.Items_Count_Error() > 0 && Configuration.ShowGrepErrors)
            OnShowLog();
    }
    else
    {
        SendMessage(HStatusBar, SB_SETTEXT, 1 | SBT_NOBORDERS, (LPARAM)LoadStr(IDS_STOPPED));
    }

    SetWindowText(HWindow, LoadStr(IDS_FF_NAME));
    if (GrepData.Refine != 0)
        FoundFilesListView->DestroyDataForRefine();
    UpdateListViewItems();
    EnableControls();
}

void Find_Dialog::EnableToolBar()
{
    if (FoundFilesListView == NULL)
        return;
    BOOL lvFocused = GetFocus() == FoundFilesListView->HWindow;
    BOOL selectedCount = ListView_GetSelectedCount(FoundFilesListView->HWindow);
    int focusedIndex = ListView_GetNextItem(FoundFilesListView->HWindow, -1, LVNI_FOCUSED);
    BOOL focusedIsFile = focusedIndex != -1 && !FoundFilesListView->Items_At(focusedIndex)->IsDir;

    TBHeader->EnableItem(CM_FIND_FOCUS, FALSE, lvFocused && focusedIndex != -1);
    TBHeader->EnableItem(CM_FIND_VIEW, FALSE, lvFocused && focusedIsFile);
    TBHeader->EnableItem(CM_FIND_EDIT, FALSE, lvFocused && focusedIsFile);
    TBHeader->EnableItem(CM_FIND_DELETE, FALSE, lvFocused && selectedCount > 0);
    TBHeader->EnableItem(CM_FIND_USERMENU, FALSE, lvFocused && selectedCount > 0);
    TBHeader->EnableItem(CM_FIND_PROPERTIES, FALSE, lvFocused && selectedCount > 0);
    TBHeader->EnableItem(CM_FIND_CLIPCUT, FALSE, lvFocused && selectedCount > 0);
    TBHeader->EnableItem(CM_FIND_CLIPCOPY, FALSE, lvFocused && selectedCount > 0);
    TBHeader->EnableItem(IDC_FIND_STOP, FALSE, SearchInProgress);
}

void Find_Dialog::EnableControls(BOOL nextIsButton)
{
    CALL_STACK_MESSAGE2("Find_Dialog::EnableButtons(%d)", nextIsButton);
    if (FoundFilesListView == NULL)
        return;

    EnableToolBar();

    HWND focus = GetFocus();
    if (SearchInProgress)
    {
        HWND hFocus = GetFocus();

        EnableWindow(GetDlgItem(HWindow, IDC_FIND_NAMED), FALSE);
        EnableWindow(GetDlgItem(HWindow, IDC_FIND_LOOKIN), FALSE);
        EnableWindow(GetDlgItem(HWindow, IDC_FIND_LOOKIN_BROWSE), FALSE);
        EnableWindow(GetDlgItem(HWindow, IDC_FIND_INCLUDE_SUBDIR), FALSE);
        EnableWindow(GetDlgItem(HWindow, IDC_FIND_INCLUDE_ARCHIVES), FALSE);
        EnableWindow(GetDlgItem(HWindow, IDC_FIND_GREP), FALSE);
        if (Expanded)
        {
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_CONTAINING), FALSE);
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_REGEXP_BROWSE), FALSE);
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_HEX), FALSE);
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_WHOLE), FALSE);
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_CASE), FALSE);
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_REGULAR), FALSE);
        }
        EnableWindow(GetDlgItem(HWindow, IDC_FIND_ADVANCED), FALSE);

        if (hFocus != NULL && !IsWindowEnabled(hFocus))
            SendMessage(HWindow, WM_NEXTDLGCTL, (WPARAM)FoundFilesListView->HWindow, TRUE);
    }
    else
    {
        HWND setFocus = NULL;

        BOOL enableHexMode = !Data.RegularExpresions;
        if (!enableHexMode && GetDlgItem(HWindow, IDC_FIND_HEX) == focus)
            setFocus = GetDlgItem(HWindow, IDC_FIND_CONTAINING);

        TBHeader->EnableItem(IDC_FIND_STOP, FALSE, FALSE);

        /*
    int foundItems = FoundFilesListView->Items_Count();
    int refineItems = FoundFilesListView->GetDataForRefineCount();
    BOOL refine = FALSE;
    EnableWindow(GetDlgItem(HWindow, IDC_FIND_INCLUDE_ARCHIVES), foundItems > 0);
    if (foundItems == 0 && refineItems == 0)
    {
      if (IsDlgButtonChecked(HWindow, IDC_FIND_INCLUDE_ARCHIVES) == BST_CHECKED)
        CheckDlgButton(HWindow, IDC_FIND_INCLUDE_ARCHIVES, BST_UNCHECKED);
    }
    else
      refine = IsDlgButtonChecked(HWindow, IDC_FIND_INCLUDE_ARCHIVES) == BST_CHECKED;
//    EnableWindow(GetDlgItem(HWindow, IDC_FIND_LOOKIN), !refine);
//    EnableWindow(GetDlgItem(HWindow, IDC_FIND_LOOKIN_BROWSE), !refine);
//    EnableWindow(GetDlgItem(HWindow, IDC_FIND_INCLUDE_SUBDIR), !refine);
    */

        EnableWindow(GetDlgItem(HWindow, IDC_FIND_NAMED), TRUE);
        EnableWindow(GetDlgItem(HWindow, IDC_FIND_LOOKIN), TRUE);
        EnableWindow(GetDlgItem(HWindow, IDC_FIND_LOOKIN_BROWSE), TRUE);
        EnableWindow(GetDlgItem(HWindow, IDC_FIND_INCLUDE_SUBDIR), TRUE);
        EnableWindow(GetDlgItem(HWindow, IDC_FIND_GREP), TRUE);
        if (Expanded)
        {
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_CONTAINING), TRUE);
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_REGEXP_BROWSE), TRUE);
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_HEX), enableHexMode);
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_WHOLE), TRUE);
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_CASE), TRUE);
            EnableWindow(GetDlgItem(HWindow, IDC_FIND_REGULAR), TRUE);
        }
        EnableWindow(GetDlgItem(HWindow, IDC_FIND_ADVANCED), TRUE);

        if (setFocus != NULL)
            SetFocus(setFocus);
    }

    int defID;
    if (focus == FoundFilesListView->HWindow &&
        ListView_GetItemCount(FoundFilesListView->HWindow) > 0)
    { // bez def-push buttonu
        defID = (int)SendMessage(HWindow, DM_GETDEFID, 0, 0);
        if (HIWORD(defID) == DC_HASDEFID)
            defID = LOWORD(defID);
        else
            defID = -1;
        SendMessage(HWindow, DM_SETDEFID, -1, 0);
        if (defID != -1)
            SendMessage(GetDlgItem(HWindow, defID), BM_SETSTYLE,
                        BS_PUSHBUTTON, MAKELPARAM(TRUE, 0));
    }
    else // vyber def-push buttonu
    {
        if (nextIsButton)
        {
            defID = (int)SendMessage(HWindow, DM_GETDEFID, 0, 0);
            if (HIWORD(defID) == DC_HASDEFID)
            {
                defID = LOWORD(defID);
                PostMessage(GetDlgItem(HWindow, defID), BM_SETSTYLE, BS_PUSHBUTTON,
                            MAKELPARAM(TRUE, 0));
            }
        }
        defID = IDOK;
        // nasleduici kod zpusoboval poblikavani Find Now tlacitka behem prohledavani
        // pokud na nej clovek postavil mysi Focus; odpojenim se nezda, ze bych neco porusil, uvidime...
        /*
    char className[30];
    WORD wl = LOWORD(GetWindowLongPtr(focus, GWL_STYLE));  // jen BS_...
    if (GetClassName(focus, className, 30) != 0 &&
        StrICmp(className, "BUTTON") == 0 &&
        (wl == BS_PUSHBUTTON || wl == BS_DEFPUSHBUTTON))
    {
      nextIsButton = TRUE;
      PostMessage(focus, BM_SETSTYLE, BS_DEFPUSHBUTTON, MAKELPARAM(TRUE, 0));
    }
*/
        SendMessage(HWindow, DM_SETDEFID, defID, 0);
        if (nextIsButton)
            PostMessage(GetDlgItem(HWindow, defID), BM_SETSTYLE, BS_PUSHBUTTON,
                        MAKELPARAM(TRUE, 0));
    }
}

void Find_Dialog::UpdateListViewItems()
{
    if (FoundFilesListView != NULL)
    {
        int count = FoundFilesListView->Items_Count();

        // reknu listview novy pocet polozek
        ListView_SetItemCountEx(FoundFilesListView->HWindow,
                                count,
                                LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
        // pokud jde o prvni pridana data, nultou polozku vyberu
        if (GrepData.FoundVisibleCount == 0 && count > 0)
            ListView_SetItemState(FoundFilesListView->HWindow, 0,
                                  LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED)

                // napisu pocet polozek nad listview
                TBHeader->SetFoundCount(count);
        TBHeader->SetErrorsInfosCount(Log.Items_Count_Error(), Log.Items_Count_Info());

        // pokud jsme minimalizovany, zobrazim pocet polozek do hlavicky
        if (IsIconic(HWindow))
        {
            char buf[MAX_PATH + 100];
            if (SearchInProgress)
            {
                _snprintf_s(buf, _TRUNCATE, MINIMIZED_FINDING_CAPTION, FoundFilesListView->Items_Count(),
                            LoadStr(IDS_FF_NAME), LoadStr(IDS_FF_NAMED), SearchForData[0]->MasksGroup.GetMasksString());
            }
            else
                lstrcpy(buf, LoadStr(IDS_FF_NAME));
            SetWindowText(HWindow, buf);
        }

        // slouzi pro hledaci thread - aby vedel, kdy nas ma priste upozornit
        GrepData.FoundVisibleCount = count;
        GrepData.FoundVisibleTick = GetTickCount();

        EnableToolBar();
    }
}

void Find_Dialog::OnFocusFile()
{
    CALL_STACK_MESSAGE1("Find_Dialog::FocusButton()");

    int index = ListView_GetNextItem(FoundFilesListView->HWindow, -1, LVNI_FOCUSED);
    if (index < 0)
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
    Find_FileList_Item* data = FoundFilesListView->Items_At(index);
    SendMessage(MainWindow->GetActivePanel()->HWindow, WM_USER_FOCUSFILE, (WPARAM)data->Name, (LPARAM)data->Path);
}

BOOL Find_Dialog::GetFocusedFile(char* buffer, int bufferLen, int* viewedIndex)
{
    int index = ListView_GetNextItem(FoundFilesListView->HWindow, -1, LVNI_FOCUSED);
    if (index < 0)
        return FALSE;

    if (viewedIndex != NULL)
        *viewedIndex = index;

    Find_FileList_Item* data = FoundFilesListView->Items_At(index);
    if (data->IsDir)
        return FALSE;
    char longName[MAX_PATH];
    int len = (int)strlen(data->Path);
    memmove(longName, data->Path, len);
    if (data->Path[len - 1] != '\\')
        longName[len++] = '\\';
    strcpy(longName + len, data->Name);

    lstrcpyn(buffer, longName, bufferLen);
    return TRUE;
}

void Find_Dialog::UpdateInternalViewerData()
{
    // kopie find textu do interniho viewru
    if (Configuration.CopyFindText)
    { // alt+F3 sem proste nechodi, takze zadny alternate viewer...
        CFindSetDialog oldGlobalFindDialog = GlobalFindDialog;

        GlobalFindDialog.Forward = TRUE;
        CTransferInfo dummyTI(HWindow, ttDataFromWindow);
        dummyTI.CheckBox(IDC_FIND_WHOLE, GlobalFindDialog.WholeWords);
        dummyTI.CheckBox(IDC_FIND_CASE, GlobalFindDialog.CaseSensitive);
        dummyTI.CheckBox(IDC_FIND_HEX, GlobalFindDialog.HexMode);
        dummyTI.CheckBox(IDC_FIND_REGULAR, GlobalFindDialog.Regular);
        dummyTI.EditLine(IDC_FIND_CONTAINING, GlobalFindDialog.Text, FIND_TEXT_LEN);

        HistoryComboBox(NULL, dummyTI, 0, GlobalFindDialog.Text,
                        (int)strlen(GlobalFindDialog.Text),
                        !GlobalFindDialog.Regular && GlobalFindDialog.HexMode,
                        VIEWER_HISTORY_SIZE,
                        ViewerHistory, TRUE);
        if (!dummyTI.IsGood()) // neco neni o.k. (hexmode)
            GlobalFindDialog = oldGlobalFindDialog;
    }
}

void Find_Dialog::OnViewFile(BOOL alternate)
{
    CALL_STACK_MESSAGE2("Find_Dialog::OnViewFile(%d)", alternate);
    char longName[MAX_PATH];
    int viewedIndex = 0;
    if (!GetFocusedFile(longName, MAX_PATH, &viewedIndex))
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
    UpdateInternalViewerData();
    COpenViewerData openData;
    openData.FileName = longName;
    openData.EnumFileNamesSourceUID = FoundFilesListView->EnumFileNamesSourceUID;
    openData.EnumFileNamesLastFileIndex = viewedIndex;
    SendMessage(MainWindow->GetActivePanel()->HWindow, WM_USER_VIEWFILE, (WPARAM)(&openData), (LPARAM)alternate);
}

void Find_Dialog::OnEditFile()
{
    CALL_STACK_MESSAGE1("Find_Dialog::OnEditFile()");
    char longName[MAX_PATH];
    if (!GetFocusedFile(longName, MAX_PATH, NULL))
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
    SendMessage(MainWindow->GetActivePanel()->HWindow, WM_USER_EDITFILE, (WPARAM)longName, 0);
}

void Find_Dialog::OnViewFileWith()
{
    CALL_STACK_MESSAGE1("Find_Dialog::OnViewFileWith()");
    char longName[MAX_PATH];
    int viewedIndex = 0;
    if (!GetFocusedFile(longName, MAX_PATH, &viewedIndex))
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
    UpdateInternalViewerData();
    POINT menuPoint;
    GetListViewContextMenuPos(FoundFilesListView->HWindow, &menuPoint);
    DWORD handlerID;
    // volani neni uplne v poradku, protoze ViewFileWith nema kriticke sekce pro praci s konfiguraci
    // predpokladem spravne funkce je, ze user dela jen jednu vec (nemeni konfiguraci a zaroven nedela
    // ve Find okne) - snad temer 100%
    MainWindow->GetActivePanel()->ViewFileWith(longName, FoundFilesListView->HWindow, &menuPoint, &handlerID, -1, -1);
    if (handlerID != 0xFFFFFFFF)
    {
        if (SalamanderBusy) // asi temer nerealne, ale nekdo mohl Salama zamestnat
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
        COpenViewerData openData;
        openData.FileName = longName;
        openData.EnumFileNamesSourceUID = FoundFilesListView->EnumFileNamesSourceUID;
        openData.EnumFileNamesLastFileIndex = viewedIndex;
        SendMessage(MainWindow->GetActivePanel()->HWindow, WM_USER_VIEWFILEWITH,
                    (WPARAM)(&openData), (LPARAM)handlerID);
    }
}

void Find_Dialog::OnEditFileWith()
{
    CALL_STACK_MESSAGE1("Find_Dialog::OnEditFileWith()");
    char longName[MAX_PATH];
    if (!GetFocusedFile(longName, MAX_PATH, NULL))
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
    POINT menuPoint;
    GetListViewContextMenuPos(FoundFilesListView->HWindow, &menuPoint);
    DWORD handlerID;
    // volani neni uplne v poradku, protoze EditFileWith nema kriticke sekce pro praci s konfiguraci
    // predpokladem spravne funkce je, ze user dela jen jednu vec (nemeni konfiguraci a zaroven nedela
    // ve Find okne) - snad temer 100%
    MainWindow->GetActivePanel()->EditFileWith(longName, FoundFilesListView->HWindow, &menuPoint, &handlerID);
    if (handlerID != 0xFFFFFFFF)
    {
        if (SalamanderBusy) // asi temer nerealne, ale nekdo mohl Salama zamestnat
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
        SendMessage(MainWindow->GetActivePanel()->HWindow, WM_USER_EDITFILEWITH,
                    (WPARAM)longName, (WPARAM)handlerID);
    }
}

void Find_Dialog::OnUserMenu()
{
    CALL_STACK_MESSAGE1("Find_Dialog::OnUserMenu()");
    DWORD selectedCount = ListView_GetSelectedCount(FoundFilesListView->HWindow);
    if (selectedCount < 1)
        return;

    UserMenuIconBkgndReader.BeginUserMenuIconsInUse();
    CMenuPopup menu;
    MainWindow->FillUserMenu(&menu, FALSE); // zatlucu customize
    POINT p;
    GetListViewContextMenuPos(FoundFilesListView->HWindow, &p);
    // dalsi kolo zamykani (BeginUserMenuIconsInUse+EndUserMenuIconsInUse) bude
    // v WM_USER_ENTERMENULOOP+WM_USER_LEAVEMENULOOP, ale to uz je vnorene, zadna rezie,
    // takze ignorujeme, nebudeme proti tomu nijak bojovat
    DWORD cmd = menu.Track(MENU_TRACK_RETURNCMD, p.x, p.y, HWindow, NULL);
    UserMenuIconBkgndReader.EndUserMenuIconsInUse();

    if (cmd != 0)
    {
        CUserMenuAdvancedData userMenuAdvancedData;

        char* list = userMenuAdvancedData.ListOfSelNames;
        char* listEnd = list + USRMNUARGS_MAXLEN - 1;
        int findItem = -1;
        DWORD i;
        for (i = 0; i < selectedCount; i++) // naplnime seznam vybranych jmen
        {
            findItem = ListView_GetNextItem(FoundFilesListView->HWindow, findItem, LVNI_SELECTED);
            if (findItem != -1)
            {
                if (list > userMenuAdvancedData.ListOfSelNames)
                {
                    if (list < listEnd)
                        *list++ = ' ';
                    else
                        break;
                }
                Find_FileList_Item* file = FoundFilesListView->Items_At(findItem);
                if (!AddToListOfNames(&list, listEnd, file->Name, (int)strlen(file->Name)))
                    break;
            }
        }
        if (i < selectedCount)
            userMenuAdvancedData.ListOfSelNames[0] = 0; // maly buffer pro seznam vybranych jmen
        else
            *list = 0;
        userMenuAdvancedData.ListOfSelNamesIsEmpty = FALSE; // to u Findu nehrozi (jinak se User Menu vubec neotevre)

        char* listFull = userMenuAdvancedData.ListOfSelFullNames;
        char* listFullEnd = listFull + USRMNUARGS_MAXLEN - 1;
        findItem = -1;
        for (i = 0; i < selectedCount; i++) // naplnime seznam vybranych jmen
        {
            findItem = ListView_GetNextItem(FoundFilesListView->HWindow, findItem, LVNI_SELECTED);
            if (findItem != -1)
            {
                if (listFull > userMenuAdvancedData.ListOfSelFullNames)
                {
                    if (listFull < listFullEnd)
                        *listFull++ = ' ';
                    else
                        break;
                }
                Find_FileList_Item* file = FoundFilesListView->Items_At(findItem);
                char fullName[MAX_PATH];
                lstrcpyn(fullName, file->Path, MAX_PATH);
                if (!SalPathAppend(fullName, file->Name, MAX_PATH) ||
                    !AddToListOfNames(&listFull, listFullEnd, fullName, (int)strlen(fullName)))
                    break;
            }
        }
        if (i < selectedCount)
            userMenuAdvancedData.ListOfSelFullNames[0] = 0; // maly buffer pro seznam vybranych plnych jmen
        else
            *listFull = 0;
        userMenuAdvancedData.ListOfSelFullNamesIsEmpty = FALSE; // to u Findu nehrozi (jinak se User Menu vubec neotevre)

        userMenuAdvancedData.FullPathLeft[0] = 0;
        userMenuAdvancedData.FullPathRight[0] = 0;
        userMenuAdvancedData.FullPathInactive = userMenuAdvancedData.FullPathLeft;

        int comp1 = -1;
        int comp2 = -1;
        if (selectedCount == 1)
            comp1 = ListView_GetNextItem(FoundFilesListView->HWindow, -1, LVNI_SELECTED);
        else
        {
            if (selectedCount == 2)
            {
                comp1 = ListView_GetNextItem(FoundFilesListView->HWindow, -1, LVNI_SELECTED);
                comp2 = ListView_GetNextItem(FoundFilesListView->HWindow, comp1, LVNI_SELECTED);
            }
        }
        userMenuAdvancedData.CompareNamesAreDirs = FALSE;
        userMenuAdvancedData.CompareNamesReversed = FALSE;
        if (comp1 != -1 && comp2 != -1 &&
            FoundFilesListView->Items_At(comp1)->IsDir != FoundFilesListView->Items_At(comp2)->IsDir)
        {
            comp1 = -1;
            comp2 = -1;
        }
        if (comp1 == -1)
            userMenuAdvancedData.CompareName1[0] = 0;
        else
        {
            Find_FileList_Item* file = FoundFilesListView->Items_At(comp1);
            userMenuAdvancedData.CompareNamesAreDirs = file->IsDir;
            lstrcpyn(userMenuAdvancedData.CompareName1, file->Path, MAX_PATH);
            if (!SalPathAppend(userMenuAdvancedData.CompareName1, file->Name, MAX_PATH))
                userMenuAdvancedData.CompareName1[0] = 0;
        }
        if (comp2 == -1)
            userMenuAdvancedData.CompareName2[0] = 0;
        else
        {
            Find_FileList_Item* file = FoundFilesListView->Items_At(comp2);
            userMenuAdvancedData.CompareNamesAreDirs = file->IsDir;
            lstrcpyn(userMenuAdvancedData.CompareName2, file->Path, MAX_PATH);
            if (!SalPathAppend(userMenuAdvancedData.CompareName2, file->Name, MAX_PATH))
                userMenuAdvancedData.CompareName2[0] = 0;
        }

        CUMDataFromFind data(FoundFilesListView->HWindow);
        MainWindow->UserMenu(HWindow, cmd - CM_USERMENU_MIN, GetNextItemFromFind, &data, &userMenuAdvancedData);
        SetFocus(FoundFilesListView->HWindow);
    }
}



BOOL Find_Dialog::IsMenuBarMessage(CONST MSG* lpMsg)
{
    CALL_STACK_MESSAGE_NONE
    if (MenuBar == NULL)
        return FALSE;
    return MenuBar->IsMenuBarMessage(lpMsg);
}

void Find_Dialog::InsertDrives(HWND hEdit, BOOL network)
{
    CALL_STACK_MESSAGE_NONE
    char drives[200];
    char* iterator = drives;
    char root[4] = " :\\";
    char drive = 'A';
    DWORD mask = GetLogicalDrives();
    int i = 1;
    while (i != 0)
    {
        if (mask & i) // disk je pristupny
        {
            root[0] = drive;
            DWORD driveType = GetDriveType(root);
            if (driveType == DRIVE_FIXED || network && driveType == DRIVE_REMOTE)
            {
                if (iterator > drives)
                {
                    *iterator++ = ';';
                }
                memmove(iterator, root, 3);
                iterator += 3;
            }
        }
        i <<= 1;
        drive++;
    }
    *iterator = '\0';

    SetWindowText(hEdit, drives);
    SendMessage(hEdit, EM_SETSEL, lstrlen(drives), lstrlen(drives));
}

BOOL Find_Dialog::CanCloseWindow()
{
    // chodily bug reporty s pady v Find_Dialog::StopSearch()
    // pravdepodobne dochazelo k destrukci okna jeste v metode
    // nasledujici test na promennou CanClose nam od verze 1.52 nejak vypadnul
    if (!CanClose)
        return FALSE;

    // pokud existuji okna CShellExecuteWnd, nabidneme preruseni zavirani nebo zaslani bug reportu + terminate
    char reason[BUG_REPORT_REASON_MAX]; // pricina problemu + seznam oken (multiline)
    strcpy(reason, "Some faulty shell extension has locked our find window.");
    if (EnumCShellExecuteWnd(HWindow, reason + (int)strlen(reason), BUG_REPORT_REASON_MAX - ((int)strlen(reason) + 1)) > 0)
    {
        // zeptame se, zda ma Salamander pokracovat nebo jestli ma vygenerovat bug report
        if (SalMessageBox(HWindow, LoadStr(IDS_SHELLEXTBREAK3), SALAMANDER_TEXT_VERSION,
                          MSGBOXEX_CONTINUEABORT | MB_ICONINFORMATION | MSGBOXEX_SETFOREGROUND) != IDABORT)
        {
            return FALSE; // mame pokracovat
        }

        // breakneme se
        strcpy(BugReportReasonBreak, reason);
        TaskList.FireEvent(TASKLIST_TODO_BREAK, GetCurrentProcessId());
        // zamrazime tento thread
        while (1)
            Sleep(1000);
    }
    return TRUE;
}

BOOL Find_Dialog::DoYouWantToStopSearching()
{
    int ret = IDYES;
    if (Configuration.CnfrmStopFind)
    {
        BOOL dontShow = !Configuration.CnfrmStopFind;

        MSGBOXEX_PARAMS params;
        memset(&params, 0, sizeof(params));
        params.HParent = HWindow;
        params.Flags = MSGBOXEX_YESNO | MSGBOXEX_ESCAPEENABLED | MSGBOXEX_ICONQUESTION | MSGBOXEX_SETFOREGROUND | MSGBOXEX_HINT;
        params.Caption = LoadStr(IDS_WANTTOSTOPTITLE);
        params.Text = LoadStr(IDS_WANTTOSTOP);
        params.CheckBoxText = LoadStr(IDS_DONTSHOWAGAINSS);
        params.CheckBoxValue = &dontShow;
        ret = SalMessageBoxEx(&params);
        Configuration.CnfrmStopFind = !dontShow;
    }
    return (ret == IDYES);
}

// z controlu vytahne text a hleda horkou klavesu;
// pokud ji najde, vrati jeji znak (UPCASE), jinak vrati 0
char GetControlHotKey(HWND hWnd, int resID)
{
    char buff[500];
    if (!GetDlgItemText(hWnd, resID, buff, 500))
        return 0;
    const char* p = buff;
    while (*p != 0)
    {
        if (*p == '&' && *(p + 1) != '&')
            return UpperCase[*(p + 1)];
        p++;
    }
    return 0;
}

BOOL Find_Dialog::ManageHiddenShortcuts(const MSG* msg)
{
    if (msg->message == WM_SYSKEYDOWN)
    {
        BOOL controlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        BOOL altPressed = (GetKeyState(VK_MENU) & 0x8000) != 0;
        BOOL shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        if (!controlPressed && altPressed && !shiftPressed)
        {
            // pokud je stisteno Alt+? a je zabalena Options cast, ma smysl zkoumat dal
            if (!IsDlgButtonChecked(HWindow, IDC_FIND_GREP))
            {
                // otukame horke klavesy sledovanych prvku
                int resID[] = {IDC_FIND_CONTAINING_TEXT, IDC_FIND_HEX, IDC_FIND_CASE,
                               IDC_FIND_WHOLE, IDC_FIND_REGULAR, -1}; // (terminatovat -1)
                int i;
                for (i = 0; resID[i] != -1; i++)
                {
                    char key = GetControlHotKey(HWindow, resID[i]);
                    if (key != 0 && (WPARAM)key == msg->wParam)
                    {
                        // rozbalime options cast
                        CheckDlgButton(HWindow, IDC_FIND_GREP, BST_CHECKED);
                        SendMessage(HWindow, WM_COMMAND, MAKEWPARAM(IDC_FIND_GREP, BN_CLICKED), 0);
                        return FALSE; // rozbaleno, o zbytek se postara IsDialogMessage po nasem navratu
                    }
                }
            }
        }
    }
    return FALSE; // neni to nase message
}

void Find_Dialog::SetFullRowSelect(BOOL fullRow)
{
    Configuration.FindFullRowSelect = fullRow;

    // dame vedet findum o zmene
    FindDialogQueue.BroadcastMessage(WM_USER_FINDFULLROWSEL, 0, 0);
}

INT_PTR Find_Dialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SLOW_CALL_STACK_MESSAGE4("Find_Dialog::DialogProc(0x%X, 0x%IX, 0x%IX)", uMsg, wParam, lParam);
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        FindDialogQueue.Add(new CWindowQueueItem(HWindow));

        GetDlgItemText(HWindow, IDOK, FindNowText, 100);

        UpdateAdvancedText();

        InstallWordBreakProc(GetDlgItem(HWindow, IDC_FIND_NAMED));      // instalujeme WordBreakProc do comboboxu
        InstallWordBreakProc(GetDlgItem(HWindow, IDC_FIND_LOOKIN));     // instalujeme WordBreakProc do comboboxu
        InstallWordBreakProc(GetDlgItem(HWindow, IDC_FIND_CONTAINING)); // instalujeme WordBreakProc do comboboxu

        CComboboxEdit* edit = new CComboboxEdit();
        if (edit != NULL)
        {
            HWND hCombo = GetDlgItem(HWindow, IDC_FIND_CONTAINING);
            edit->AttachToWindow(GetWindow(hCombo, GW_CHILD));
        }
        ChangeToArrowButton(HWindow, IDC_FIND_REGEXP_BROWSE);

        OKButton = new CButton(HWindow, IDOK, BTF_DROPDOWN);
        new CButton(HWindow, IDC_FIND_LOOKIN_BROWSE, BTF_RIGHTARROW);

        // nastavim checkbox pro viditelnost Content casti dialogu
        CheckDlgButton(HWindow, IDC_FIND_GREP, Configuration.SearchFileContent);

        // priradim oknu ikonku
        HICON findIcon = HANDLES(LoadIcon(ImageResDLL, MAKEINTRESOURCE(8)));
        if (findIcon == NULL)
            findIcon = HANDLES(LoadIcon(HInstance, MAKEINTRESOURCE(IDI_FIND)));
        SendMessage(HWindow, WM_SETICON, ICON_BIG, (LPARAM)findIcon);

        // konstrukce listview
        FoundFilesListView = new Find_FilesList_View(HWindow, IDC_FIND_RESULTS, *this);

        SetFullRowSelect(Configuration.FindFullRowSelect);

        TBHeader = new CFindTBHeader(HWindow, IDC_FIND_FOUND_FILES);

        // vytvorim status bar
        HStatusBar = CreateWindowEx(0,
                                    STATUSCLASSNAME,
                                    (LPCTSTR)NULL,
                                    SBARS_SIZEGRIP | WS_CHILD | CCS_BOTTOM | WS_VISIBLE,
                                    0, 0, 0, 0,
                                    HWindow,
                                    (HMENU)IDC_FIND_STATUS,
                                    HInstance,
                                    NULL);
        if (HStatusBar == NULL)
        {
            TRACE_E("Error creating StatusBar");
            DlgFailed = TRUE;
            PostMessage(HWindow, WM_COMMAND, IDCANCEL, 0);
            break;
        }

        SetTwoStatusParts(FALSE, TRUE);
        SendMessage(HStatusBar, SB_SETTEXT, 1 | SBT_NOBORDERS, (LPARAM)LoadStr(IDS_FIND_INIT_HINT));

        // priradim oknu menu
        MainMenu = new CMenuPopup;

        BuildFindMenu(MainMenu);
        MenuBar = new CMenuBar(MainMenu, HWindow);
        if (!MenuBar->CreateWnd(HWindow))
        {
            TRACE_E("Error creating Menu");
            DlgFailed = TRUE;
            PostMessage(HWindow, WM_COMMAND, IDCANCEL, 0);
            break;
        }
        ShowWindow(MenuBar->HWindow, SW_SHOW);

        // nactu paramatry pro layoutovani okna
        GetLayoutParams();

        WINDOWPLACEMENT* wp = &Configuration.FindDialogWindowPlacement;
        if (wp->length != 0)
        {
            RECT r = wp->rcNormalPosition;
            SetWindowPos(HWindow, NULL, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOMOVE | SWP_NOZORDER);
            if (wp->showCmd == SW_MAXIMIZE || wp->showCmd == SW_SHOWMAXIMIZED)
                ShowWindow(HWindow, SW_MAXIMIZE);
        }

        SetWindowPos(HWindow, Configuration.AlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                     0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        LayoutControls();
        FoundFilesListView->InitColumns();
        SetContentVisible(Configuration.SearchFileContent);

        // sejmu WS_TABSTOP z IDC_FIND_ADVANCED_TEXT
        DWORD style = (DWORD)GetWindowLongPtr(GetDlgItem(HWindow, IDC_FIND_ADVANCED_TEXT), GWL_STYLE);
        style &= ~WS_TABSTOP;
        SetWindowLongPtr(GetDlgItem(HWindow, IDC_FIND_ADVANCED_TEXT), GWL_STYLE, style);

        ListView_SetItemCount(FoundFilesListView->HWindow, 0);

        SetDlgItemText(HWindow, IDOK, FindNowText);
        TBHeader->SetFoundCount(0);

        SetWindowText(HWindow, LoadStr(IDS_FF_NAME));

        int i;
        for (i = 0; i < FindOptions.Items_Count(); i++)
            if (FindOptions.Items_At(i)->AutoLoad)
            {
                char buff[1024];
                sprintf(buff, LoadStr(IDS_FF_AUTOLOAD), FindOptions.Items_At(i)->ItemName);
                SendMessage(HStatusBar, SB_SETTEXT, 1 | SBT_NOBORDERS, (LPARAM)buff);
                break;
            }

        HWND hCombo = GetDlgItem(HWindow, IDC_FIND_LOOKIN);
        EditLine->AttachToWindow(GetWindow(hCombo, GW_CHILD));

        // zatim neumime, schovame volbu
        ShowWindow(GetDlgItem(HWindow, IDC_FIND_INCLUDE_ARCHIVES), FALSE);

        EnableControls();
        break;
    }

    case WM_TIMER:
    {
        if (wParam == IDT_REPAINT)
        {
            char buf[MAX_PATH + 50];
            if (SearchingText.Redraw_Get())
            {
                SearchingText.Redraw_Set(FALSE); // uz se prekresluje - zavola se Get; radsi 2x refresh nez vubec
                                               //          SearchingText.Get(buf, MAX_PATH + 50);
                SendMessage(HStatusBar, SB_SETTEXT, 1 | SBT_NOBORDERS | SBT_OWNERDRAW, 0);
            }
            if (SearchingText2.Redraw_Get())
            {
                if (!TwoParts)
                    SetTwoStatusParts(TRUE);
                SearchingText2.Redraw_Set(FALSE); // uz se prekresluje - zavola se Get; radsi 2x refresh nez vubec
                SearchingText2.String_Get(buf, MAX_PATH + 50);
                int pos = buf[0]; // misto retezce vytahneme primo hodnotu
                SendMessage(HProgressBar, PBM_SETPOS, pos, 0);
            }
            return 0;
        }
        break;
    }

    case WM_USER_FLASHICON:
    {
        if (GetForegroundWindow() == HWindow && TBHeader != NULL)
            TBHeader->StartFlashIcon();
        else
            FlashIconsOnActivation = TRUE;
        return 0;
    }

    case WM_USER_COLORCHANGEFIND:
    {
        OnColorsChange();
        return TRUE;
    }

    case WM_USER_FINDFULLROWSEL:
    {
        DWORD flags = ListView_GetExtendedListViewStyle(FoundFilesListView->HWindow);
        BOOL hasFullRow = (flags & LVS_EX_FULLROWSELECT) != 0;
        if (hasFullRow != Configuration.FindFullRowSelect)
        {
            if (Configuration.FindFullRowSelect)
                flags |= LVS_EX_FULLROWSELECT;
            else
                flags &= ~LVS_EX_FULLROWSELECT;
            ListView_SetExtendedListViewStyle(FoundFilesListView->HWindow, flags); // 4.71
        }
        return TRUE;
    }

    case WM_USER_CLEARHISTORY:
    {
        ClearComboboxListbox(GetDlgItem(HWindow, IDC_FIND_NAMED));
        ClearComboboxListbox(GetDlgItem(HWindow, IDC_FIND_LOOKIN));
        ClearComboboxListbox(GetDlgItem(HWindow, IDC_FIND_CONTAINING));
        return TRUE;
    }

    // slouzi pro test na zavreni okna z duvodu zavirani Salamander
    case WM_USER_QUERYCLOSEFIND:
    {
        BOOL query = TRUE;
        if (SearchInProgress)
        {
            if (lParam /* quiet */)
                StopSearch(); // nemame se na nic ptat, rozjete hledani kazdopadne zastavime
            else
            {
                if (!DoYouWantToStopSearching())
                    query = FALSE;
                else
                {
                    if (SearchInProgress) // zastavime search hned, kdyz si to user preje
                        StopSearch();
                }
            }
        }
        if (query)
            query = CanCloseWindow();
        if ( m_StateOfFindCloseQuery == StateOfFindCloseQuery::sentToFind )
        {
            m_StateOfFindCloseQuery = query ? StateOfFindCloseQuery::canClose : StateOfFindCloseQuery::cannotClose;
        }
        return TRUE;
    }

    // slouzi pro remote zavreni okna z duvodu zavirani Salamander
    case WM_USER_CLOSEFIND:
    {
        if (SearchInProgress)
            StopSearch();
        DestroyWindow(HWindow);
        return 0;
    }

    case WM_USER_INITMENUPOPUP:
    {
        // menu enablers
        if (FoundFilesListView != NULL && FoundFilesListView->HWindow != NULL)
        {
            CMenuPopup* popup = (CMenuPopup*)(CGUIMenuPopupAbstract*)wParam;
            WORD popupID = HIWORD(lParam);

            BOOL lvFocused = GetFocus() == FoundFilesListView->HWindow;
            DWORD totalCount = ListView_GetItemCount(FoundFilesListView->HWindow);
            BOOL selectedCount = ListView_GetSelectedCount(FoundFilesListView->HWindow);
            int focusedIndex = ListView_GetNextItem(FoundFilesListView->HWindow, -1, LVNI_FOCUSED);
            BOOL focusedIsFile = focusedIndex != -1 && !FoundFilesListView->Items_At(focusedIndex)->IsDir;

            switch (popupID)
            {
            case CML_FIND_FILES:
            {
                popup->EnableItem(CM_FIND_OPEN, FALSE, lvFocused && selectedCount > 0);
                popup->EnableItem(CM_FIND_OPENSEL, FALSE, lvFocused && selectedCount > 0);
                popup->EnableItem(CM_FIND_FOCUS, FALSE, lvFocused && focusedIndex != -1);
                popup->EnableItem(CM_FIND_HIDESEL, FALSE, lvFocused && selectedCount > 0);
                popup->EnableItem(CM_FIND_HIDE_DUP, FALSE, totalCount > 0);
                popup->EnableItem(CM_FIND_VIEW, FALSE, lvFocused && focusedIsFile);
                popup->EnableItem(CM_FIND_VIEW_WITH, FALSE, lvFocused && focusedIsFile);
                popup->EnableItem(CM_FIND_ALTVIEW, FALSE, lvFocused && focusedIsFile);
                popup->EnableItem(CM_FIND_EDIT, FALSE, lvFocused && focusedIsFile);
                popup->EnableItem(CM_FIND_EDIT_WITH, FALSE, lvFocused && focusedIsFile);
                popup->EnableItem(CM_FIND_DELETE, FALSE, lvFocused && selectedCount > 0);
                popup->EnableItem(CM_FIND_USERMENU, FALSE, lvFocused && selectedCount > 0);
                popup->EnableItem(CM_FIND_PROPERTIES, FALSE, lvFocused && selectedCount > 0);
                break;
            }

            case CML_FIND_FIND:
            {
                popup->EnableItem(CM_FIND_NOW, FALSE, !SearchInProgress);
                popup->EnableItem(CM_FIND_INTERSECT, FALSE, !SearchInProgress && totalCount > 0);
                popup->EnableItem(CM_FIND_SUBTRACT, FALSE, !SearchInProgress && totalCount > 0);
                popup->EnableItem(CM_FIND_APPEND, FALSE, !SearchInProgress && totalCount > 0);
                popup->EnableItem(CM_FIND_DUPLICATES, FALSE, !SearchInProgress);
                popup->EnableItem(CM_FIND_MESSAGES, FALSE, Log.Items_Count() > 0);
                break;
            }

            case CML_FIND_EDIT:
            {
                popup->EnableItem(CM_FIND_CLIPCUT, FALSE, lvFocused && selectedCount > 0);
                popup->EnableItem(CM_FIND_CLIPCOPY, FALSE, lvFocused && selectedCount > 0);
                popup->EnableItem(CM_FIND_CLIPCOPYFULLNAME, FALSE, lvFocused && selectedCount == 1);
                popup->EnableItem(CM_FIND_CLIPCOPYNAME, FALSE, lvFocused && selectedCount == 1);
                popup->EnableItem(CM_FIND_CLIPCOPYFULLPATH, FALSE, lvFocused && selectedCount == 1);
                popup->EnableItem(CM_FIND_CLIPCOPYUNCNAME, FALSE, lvFocused && selectedCount == 1);
                popup->EnableItem(CM_FIND_SELECTALL, FALSE, lvFocused && totalCount > 0);
                popup->EnableItem(CM_FIND_INVERTSEL, FALSE, lvFocused && totalCount > 0);
                break;
            }

            case CML_FIND_VIEW:
            {
                BOOL enabledNameSize = TRUE;
                BOOL enabledPathTime = TRUE;
                if (GrepData.FindDuplicates)
                {
                    enabledPathTime = FALSE; // v pripade duplikatu nema vyznam
                    // radit podle jmena a velikosti lze v pripade duplicatu pouze
                    // pokud bylo hledano podle stejneho jmena i velikosti
                    enabledNameSize = (GrepData.FindDupFlags & FIND_DUPLICATES_NAME) &&
                                      (GrepData.FindDupFlags & FIND_DUPLICATES_SIZE);
                }
                popup->EnableItem(CM_FIND_NAME, FALSE, enabledNameSize && totalCount > 0);
                popup->EnableItem(CM_FIND_PATH, FALSE, enabledPathTime && totalCount > 0);
                popup->EnableItem(CM_FIND_TIME, FALSE, enabledPathTime && totalCount > 0);
                popup->EnableItem(CM_FIND_SIZE, FALSE, enabledNameSize && totalCount > 0);
                break;
            }

            case CML_FIND_OPTIONS:
            {
                static int count = -1;
                if (count == -1)
                    count = popup->GetItemCount();

                popup->CheckItem(CM_FIND_SHOWERRORS, FALSE, Configuration.ShowGrepErrors);
                popup->CheckItem(CM_FIND_FULLROWSEL, FALSE, Configuration.FindFullRowSelect);
                // pokud je otevreny dialog manage, v jinem okne ho zakazu a take pridavani do pole
                popup->EnableItem(CM_FIND_ADD_CURRENT, FALSE, !FindManageInUse);
                popup->EnableItem(CM_FIND_MANAGE, FALSE, !FindManageInUse);
                popup->EnableItem(CM_FIND_IGNORE, FALSE, !FindIgnoreInUse);
                FindOptions.Menu_Set( *popup, !SearchInProgress, count);
                break;
            }
            }
        }
        break;
    }

    case WM_USER_BUTTONDROPDOWN:
    {
        if (SearchInProgress)
            return 0;

        HWND hCtrl = GetDlgItem(HWindow, (int)wParam);
        RECT r;
        GetWindowRect(hCtrl, &r);

        CGUIMenuPopupAbstract* popup = MainMenu->GetSubMenu(CML_FIND_FIND, FALSE);
        if (popup != NULL)
        {
            BOOL selectMenuItem = LOWORD(lParam);
            DWORD flags = 0;
            if (selectMenuItem)
            {
                popup->SetSelectedItemIndex(0);
                flags |= MENU_TRACK_SELECT;
            }
            popup->Track(flags, r.left, r.bottom, HWindow, &r);
        }
        break;
    }

    case WM_SIZE:
    {
        // pri restoru obnovim titulek okna
        if (SearchInProgress && (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED)) // restore
        {
            char buff[MAX_PATH + 100];
            _snprintf_s(buff, _TRUNCATE, NORMAL_FINDING_CAPTION, LoadStr(IDS_FF_NAME), LoadStr(IDS_FF_NAMED),
                        SearchForData[0]->MasksGroup.GetMasksString());
            SetWindowText(HWindow, buff);
        }

        //      if (FirstWMSize)
        //        FirstWMSize = FALSE;
        //      else
        LayoutControls();
        break;
    }

    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
        lpmmi->ptMinTrackSize.x = MinDlgW;
        lpmmi->ptMinTrackSize.y = MinDlgH;
        break;
    }

    case WM_HELP:
    {
        PostMessage(HWindow, WM_COMMAND, CM_HELP_CONTENTS, 0);
        return TRUE;
    }

    case WM_COMMAND:
    {
        if (FoundFilesListView != NULL && ListView_GetEditControl(FoundFilesListView->HWindow) != NULL)
            return 0; // list view nam behem editace posila nejake commandy
        if (LOWORD(wParam) >= CM_FIND_OPTIONS_FIRST && LOWORD(wParam) <= CM_FIND_OPTIONS_LAST)
        {
            LoadControls(LOWORD(wParam) - CM_FIND_OPTIONS_FIRST);
            return TRUE;
        }

        if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_FIND_GREP)
        {
            Configuration.SearchFileContent = IsDlgButtonChecked(HWindow, IDC_FIND_GREP);
            SetContentVisible(Configuration.SearchFileContent);
            if (!Configuration.SearchFileContent)
            {
                // sejmu priadny obsah schovanych prvku
                SetDlgItemText(HWindow, IDC_FIND_CONTAINING, "");
                CheckDlgButton(HWindow, IDC_FIND_HEX, FALSE);
                CheckDlgButton(HWindow, IDC_FIND_CASE, FALSE);
                CheckDlgButton(HWindow, IDC_FIND_WHOLE, FALSE);
                CheckDlgButton(HWindow, IDC_FIND_REGULAR, FALSE);
                Data.HexMode = FALSE;
                Data.RegularExpresions = FALSE;
            }
            return TRUE;
        }

        switch (LOWORD(wParam))
        {
        case CM_FIND_INTERSECT:
        case CM_FIND_SUBTRACT:
        case CM_FIND_APPEND:
        {
            DWORD totalCount = ListView_GetItemCount(FoundFilesListView->HWindow);
            if (!SearchInProgress && totalCount > 0)
            {
                if (ValidateData() && TransferData(ttDataFromWindow))
                    StartSearch(LOWORD(wParam));
            }
            return 0;
        }

        case CM_FIND_NOW:
        case CM_FIND_DUPLICATES:
        {
            if (!SearchInProgress)
            {
                if (ValidateData() && TransferData(ttDataFromWindow))
                    StartSearch(LOWORD(wParam));
            }
            return 0;
        }

        case IDOK:
        {
            if (SearchInProgress) // jde o Stop?
            {
                if (Configuration.MinBeepWhenDone && GetForegroundWindow() != HWindow)
                    MessageBeep(0);
                StopSearch();
                return TRUE;
            }
            else // ne, jde o start
            {
                if (!ValidateData() || !TransferData(ttDataFromWindow))
                    return TRUE;
                StartSearch(LOWORD(wParam));
                return TRUE;
            }
        }

        case IDCANCEL:
        {
            if (!CanCloseWindow())
                return TRUE;
            if (SearchInProgress)
            {
                if (!DoYouWantToStopSearching())
                    return TRUE;

                if (SearchInProgress)
                    StopSearch();

                if (ProcessingEscape)
                    return TRUE;
                else
                    break;
            }
            else
            {
                if (ProcessingEscape && Configuration.CnfrmCloseFind)
                {
                    BOOL dontShow = !Configuration.CnfrmCloseFind;

                    MSGBOXEX_PARAMS params;
                    memset(&params, 0, sizeof(params));
                    params.HParent = HWindow;
                    params.Flags = MSGBOXEX_YESNO | MSGBOXEX_ESCAPEENABLED | MSGBOXEX_ICONQUESTION | MSGBOXEX_SILENT | MSGBOXEX_HINT;
                    params.Caption = LoadStr(IDS_WANTTOSTOPTITLE);
                    params.Text = LoadStr(IDS_WANTTOCLOSEFIND);
                    params.CheckBoxText = LoadStr(IDS_DONTSHOWAGAINCF);
                    params.CheckBoxValue = &dontShow;
                    int ret = SalMessageBoxEx(&params);
                    Configuration.CnfrmCloseFind = !dontShow;
                    if (ret != IDYES)
                        return 0;
                }
            }
            break;
        }

        case IDC_FIND_STOP:
        {
            if (SearchInProgress)
            {
                if (Configuration.MinBeepWhenDone && GetForegroundWindow() != HWindow)
                    MessageBeep(0);
                StopSearch();
                return TRUE;
            }
            break;
        }

        case IDC_FIND_HEX:
        {
            if (HIWORD(wParam) == BN_CLICKED)
            {
                Data.HexMode = (IsDlgButtonChecked(HWindow, IDC_FIND_HEX) != BST_UNCHECKED);
                if (Data.HexMode)
                    CheckDlgButton(HWindow, IDC_FIND_CASE, BST_CHECKED);
                return TRUE;
            }
            break;
        }

        case IDC_FIND_REGEXP_BROWSE:
        {
            const CExecuteItem* item = TrackExecuteMenu(HWindow, IDC_FIND_REGEXP_BROWSE,
                                                        IDC_FIND_CONTAINING, TRUE,
                                                        RegularExpressionItems);
            if (item != NULL)
            {
                BOOL regular = (IsDlgButtonChecked(HWindow, IDC_FIND_REGULAR) == BST_CHECKED);
                if (item->Keyword == EXECUTE_HELP)
                {
                    // otevreme help se strankou venovanou regular expressions
                    OpenHtmlHelp(NULL, HWindow, HHCDisplayContext, IDH_REGEXP, FALSE);
                }
                if (item->Keyword != EXECUTE_HELP && !regular)
                {
                    // user zvolil nejaky vyraz -> zaskrtneme checkbox pro hledani regularu
                    CheckDlgButton(HWindow, IDC_FIND_REGULAR, BST_CHECKED);
                    PostMessage(HWindow, WM_COMMAND, MAKELPARAM(IDC_FIND_REGULAR, BN_CLICKED), 0);
                }
            }
            return 0;
        }

        case IDC_FIND_REGULAR:
        {
            if (HIWORD(wParam) == BN_CLICKED)
            {
                Data.RegularExpresions = (IsDlgButtonChecked(HWindow, IDC_FIND_REGULAR) != BST_UNCHECKED);
                if (Data.RegularExpresions)
                {
                    Data.HexMode = FALSE;
                    CheckDlgButton(HWindow, IDC_FIND_HEX, FALSE);
                }
                EnableControls();
                return TRUE;
            }
            break;
        }

            /*
        case IDC_FIND_INCLUDE_ARCHIVES:
        {
          if (HIWORD(wParam) == BN_CLICKED)
          {
            EnableControls();
            return TRUE;
          }
          break;
        }
*/

        case IDC_FIND_CONTAINING:
        {
            if (!Data.RegularExpresions && Data.HexMode && HIWORD(wParam) == CBN_EDITUPDATE)
            {
                DoHexValidation((HWND)lParam, GREP_TEXT_LEN);
                return TRUE;
            }
            break;
        }

        case IDC_FIND_ADVANCED:
        {
            CFilterCriteriaDialog dlg(HWindow, &Data.Criteria, TRUE);
            if (dlg.Execute() == IDOK)
                UpdateAdvancedText();
            return TRUE;
        }

        case CM_FIND_ADD_CURRENT:
        {
            Find_Options_Item* item = new Find_Options_Item();
            if (item != NULL)
            {
                TransferData(ttDataFromWindow);
                *item = Data;
                item->BuildItemName();
                if (!FindOptions.Items_Add(item))
                    delete item;
            }
            else
                TRACE_E(LOW_MEMORY);

            return TRUE;
        }

        case CM_FIND_MANAGE:
        {
            if (FindManageInUse)
                return 0;
            FindManageInUse = TRUE;
            TransferData(ttDataFromWindow);
            CFindManageDialog dlg(HWindow, &Data);
            if (dlg.IsGood())
                dlg.Execute();
            FindManageInUse = FALSE;
            return 0;
        }

        case CM_FIND_IGNORE:
        {
            if (FindIgnoreInUse)
                return 0;
            FindIgnoreInUse = TRUE;
            TransferData(ttDataFromWindow);
            CFindIgnoreDialog dlg(HWindow, &FindIgnore);
            if (dlg.IsGood())
                dlg.Execute();
            FindIgnoreInUse = FALSE;
            return 0;
        }

        case IDC_FIND_LOOKIN_BROWSE:
        {
            RECT r;
            GetWindowRect(GetDlgItem(HWindow, IDC_FIND_LOOKIN_BROWSE), &r);
            POINT p;
            p.x = r.right;
            p.y = r.top;

            CMenuPopup menu;
            MENU_ITEM_INFO mii;
            mii.Mask = MENU_MASK_TYPE | MENU_MASK_STRING | MENU_MASK_ID;
            mii.Type = MENU_TYPE_STRING;

            /* slouzi pro skript export_mnu.py, ktery generuje salmenu.mnu pro Translator
   udrzovat synchronizovane s volanim InsertItem() dole...
MENU_TEMPLATE_ITEM FindLookInBrowseMenu[] = 
{
  {MNTT_PB, 0
  {MNTT_IT, IDS_FF_BROWSE
  {MNTT_IT, IDS_FF_LOCALDRIVES
  {MNTT_IT, IDS_FF_ALLDRIVES
  {MNTT_PE, 0
};
*/
            int ids[] = {IDS_FF_BROWSE, -1, IDS_FF_LOCALDRIVES, IDS_FF_ALLDRIVES, 0};
            int i;
            for (i = 0; ids[i] != 0; i++)
            {
                if (ids[i] == -1)
                    mii.Type = MENU_TYPE_SEPARATOR;
                else
                {
                    mii.Type = MENU_TYPE_STRING;
                    mii.String = LoadStr(ids[i]);
                    mii.ID = i + 1;
                }
                menu.InsertItem(-1, TRUE, &mii);
            }

            DWORD cmd = menu.Track(MENU_TRACK_VERTICAL | MENU_TRACK_RETURNCMD, p.x, p.y, HWindow, &r);

            if (cmd != 0)
            {
                if (cmd == 1)
                {
                    // Browse...
                    char path[MAX_PATH + 200];
                    char buff[1024];
                    DWORD start, end;
                    EditLine->GetSel(&start, &end);
                    SendMessage(EditLine->HWindow, WM_GETTEXT, (WPARAM)1024, (LPARAM)buff);
                    path[0] = 0;
                    if (start < end)
                        lstrcpyn(path, buff + start, end - start + 1);
                    if (GetTargetDirectory(HWindow, HWindow, LoadStr(IDS_CHANGE_DIRECTORY), LoadStr(IDS_BROWSECHANGEDIRTEXT), path, FALSE, path))
                    {
                        char* s = path;
                        while (*s != 0) // zdvojime znak ';' (escape sekvence pro ";" == ";;")
                        {
                            if (*s == ';')
                            {
                                memmove(s + 1, s, strlen(s) + 1);
                                s++;
                            }
                            s++;
                        }

                        int leftIndex = -1;  // posledni znak, za ktery uz bude vlozeny text
                        int rightIndex = -1; // prvni znak za vlozenym textem
                        if (start > 0)
                            leftIndex = start - 1;
                        if (end < (DWORD)lstrlen(buff))
                            rightIndex = end;
                        if (leftIndex != -1)
                        {
                            s = buff + leftIndex;
                            while (s >= buff && *s == ';')
                                s--;
                            if ((((buff + leftIndex) - s) & 1) == 0)
                            {
                                memmove(path + 2, path, lstrlen(path) + 1);
                                path[0] = ';';
                                path[1] = ' ';
                            }
                        }
                        if (rightIndex != -1 && (buff[rightIndex] != ';' || buff[rightIndex + 1] == ';'))
                            lstrcat(path, "; ");

                        EditLine->ReplaceText(path);
                    }
                    return TRUE;
                }
                if (cmd == 3 || cmd == 4)
                    InsertDrives(EditLine->HWindow, cmd == 4); // local drives (3) || all drives (4)
            }
            return 0;
        }

        case CM_FIND_NAME:
        {
            FoundFilesListView->SortItems(0);
            return TRUE;
        }

        case CM_FIND_PATH:
        {
            FoundFilesListView->SortItems(1);
            return TRUE;
        }

        case CM_FIND_TIME:
        {
            FoundFilesListView->SortItems(3);
            return TRUE;
        }

        case CM_FIND_SIZE:
        {
            FoundFilesListView->SortItems(2);
            return TRUE;
        }

        case CM_FIND_OPEN:
        {
            OnOpen(TRUE);
            return TRUE;
        }

        case CM_FIND_OPENSEL:
        {
            OnOpen(FALSE);
            return TRUE;
        }

        case CM_FIND_FOCUS:
        {
            OnFocusFile();
            return TRUE;
        }

        case CM_FIND_VIEW:
        {
            OnViewFile(FALSE);
            return TRUE;
        }

        case CM_FIND_VIEW_WITH:
        {
            OnViewFileWith();
            return TRUE;
        }

        case CM_FIND_ALTVIEW:
        {
            OnViewFile(TRUE);
            return TRUE;
        }

        case CM_FIND_EDIT:
        {
            OnEditFile();
            return TRUE;
        }

        case CM_FIND_EDIT_WITH:
        {
            OnEditFileWith();
            return TRUE;
        }

        case CM_FIND_USERMENU:
        {
            OnUserMenu();
            return TRUE;
        }

        case CM_FIND_PROPERTIES:
        {
            OnProperties();
            return TRUE;
        }

        case CM_FIND_HIDESEL:
        {
            OnHideSelection();
            return TRUE;
        }

        case CM_FIND_HIDE_DUP:
        {
            OnHideDuplicateNames();
            return TRUE;
        }

        case CM_FIND_DELETE:
        {
            OnDelete((GetKeyState(VK_SHIFT) & 0x8000) == 0);
            return TRUE;
        }

        case CM_FIND_CLIPCUT:
        {
            OnCutOrCopy(TRUE);
            return TRUE;
        }

        case CM_FIND_CLIPCOPY:
        {
            OnCutOrCopy(FALSE);
            return TRUE;
        }

        case CM_FIND_CLIPCOPYFULLNAME:
        {
            Clipboard_CopyTo( Clipboard_CopyMode::name_full );
            return TRUE;
        }

        case CM_FIND_CLIPCOPYNAME:
        {
            Clipboard_CopyTo( Clipboard_CopyMode::name );
            return TRUE;
        }

        case CM_FIND_CLIPCOPYFULLPATH:
        {
            Clipboard_CopyTo( Clipboard_CopyMode::path_full );
            return TRUE;
        }

        case CM_FIND_CLIPCOPYUNCNAME:
        {
            Clipboard_CopyTo( Clipboard_CopyMode::name_unc );
            return TRUE;
        }

        case CM_FIND_SELECTALL:
        {
            OnSelectAll();
            return TRUE;
        }

        case CM_FIND_INVERTSEL:
        {
            OnInvertSelection();
            return TRUE;
        }

        case CM_FIND_SHOWERRORS:
        {
            Configuration.ShowGrepErrors = !Configuration.ShowGrepErrors;
            return TRUE;
        }

        case CM_FIND_FULLROWSEL:
        {
            SetFullRowSelect(!Configuration.FindFullRowSelect);
            return TRUE;
        }

        case CM_FIND_MESSAGES:
        {
            if (TBHeader != NULL)
                TBHeader->StopFlashIcon();
            OnShowLog();
            return TRUE;
        }

        case CM_HELP_CONTENTS:
        case CM_HELP_INDEX:
        case CM_HELP_SEARCH:
        {
            CHtmlHelpCommand command;
            DWORD_PTR dwData = 0;
            switch (LOWORD(wParam))
            {
            case CM_HELP_INDEX:
            {
                command = HHCDisplayIndex;
                break;
            }

            case CM_HELP_SEARCH:
            {
                command = HHCDisplaySearch;
                break;
            }

            case CM_HELP_CONTENTS:
            {
                OpenHtmlHelp(NULL, HWindow, HHCDisplayTOC, 0, TRUE); // nechceme dva messageboxy za sebou
                command = HHCDisplayContext;
                dwData = IDD_FIND;
                break;
            }
            }

            OpenHtmlHelp(NULL, HWindow, command, dwData, FALSE);

            return 0;
        }
        }
        break;
    }

    case WM_INITMENUPOPUP:
    case WM_DRAWITEM:
    case WM_MEASUREITEM:
    case WM_MENUCHAR:
    {
        if (wParam == IDC_FIND_STATUS)
        {
            DRAWITEMSTRUCT* di = (DRAWITEMSTRUCT*)lParam;
            int prevBkMode = SetBkMode(di->hDC, TRANSPARENT);
            char buff[MAX_PATH + 50];
            SearchingText.String_Get(buff, MAX_PATH + 50);
            DrawText(di->hDC, buff, (int)strlen(buff), &di->rcItem, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_PATH_ELLIPSIS);
            SetBkMode(di->hDC, prevBkMode);
            return TRUE;
        }

        if (ContextMenu != NULL)
        {
            IContextMenu3* contextMenu3 = NULL;
            LRESULT lResult = 0;
            if (uMsg == WM_MENUCHAR)
            {
                if (SUCCEEDED(ContextMenu->QueryInterface(IID_IContextMenu3, (void**)&contextMenu3)))
                {
                    contextMenu3->HandleMenuMsg2(uMsg, wParam, lParam, &lResult);
                    contextMenu3->Release();
                    return (BOOL)lResult;
                }
            }
            if (ContextMenu->HandleMenuMsg(uMsg, wParam, lParam) == NOERROR)
            {
                if (uMsg == WM_INITMENUPOPUP) // zajistime vraceni spravne navratove hodnoty
                    return 0;
                else
                    return TRUE;
            }
        }
        break;
    }

    case WM_SYSCOMMAND:
    {
        if (SkipCharacter) // zamezime pipani pri Alt+Enter
        {
            SkipCharacter = FALSE;
            return TRUE; // podle MSDN bychom meli vratit 0, ale to pipa, tak nevim
        }
        break;
    }

    case WM_NOTIFY:
    {
        if (wParam == IDC_FIND_RESULTS)
        {
            switch (((LPNMHDR)lParam)->code)
            {
            case NM_DBLCLK:
            {
                if (((LPNMITEMACTIVATE)lParam)->iItem >= 0) // double-click mimo polozku nic neotvira
                    OnOpen(TRUE);
                break;
            }

            case NM_RCLICK:
            {
                int clickedIndex = ((LPNMITEMACTIVATE)lParam)->iItem;
                if (clickedIndex >= 0) // right-click mimo polozku menu neukaze
                {
                    BOOL controlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                    BOOL altPressed = (GetKeyState(VK_MENU) & 0x8000) != 0;
                    BOOL shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

                    // pri kliknuti mimo selectionu, pokud user drzi Shift (na Alt+Ctrl nezalezi) nebo
                    // drzi jen Alt, dojde ke zmene oznaceni na kliknutou polozku jeste pred vybalenim menu
                    HWND hListView = FoundFilesListView->HWindow;
                    if ((shiftPressed || altPressed && !controlPressed) &&
                        (ListView_GetItemState(hListView, clickedIndex, LVIS_SELECTED) & LVIS_SELECTED) == 0)
                    {
                        ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED | LVIS_FOCUSED); // -1: all items
                        ListView_SetItemState(hListView, clickedIndex, LVIS_SELECTED | LVIS_FOCUSED, 0x000F);
                    }

                    DWORD pos = GetMessagePos();
                    OnContextMenu(GET_X_LPARAM(pos), GET_Y_LPARAM(pos));
                }
                break;
            }

            case NM_CUSTOMDRAW:
            {
                LPNMLVCUSTOMDRAW cd = (LPNMLVCUSTOMDRAW)lParam;

                if (cd->nmcd.dwDrawStage == CDDS_PREPAINT)
                {
                    SetWindowLongPtr(HWindow, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
                    return TRUE;
                }

                if (cd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
                {
                    // pozadame si o zaslani notifikace CDDS_ITEMPREPAINT | CDDS_SUBITEM
                    SetWindowLongPtr(HWindow, DWLP_MSGRESULT, CDRF_NOTIFYSUBITEMDRAW);
                    return TRUE;
                }

                if (cd->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM))
                {
                    Find_FileList_Item* item = FoundFilesListView->Items_At((int)cd->nmcd.dwItemSpec);

                    // sloupec Path bychom si radi malovali sami (s vypustkou pro cesty)
                    if (cd->iSubItem == 1)
                    {
                        HDC hDC = cd->nmcd.hdc;

                        // pokud jeste neexistuje cache DC, pokusime se ho vytvorit
                        if (CacheBitmap == NULL)
                        {
                            CacheBitmap = new CBitmap();
                            if (CacheBitmap != NULL)
                                CacheBitmap->CreateBmp(hDC, 1, 1);
                        }
                        if (CacheBitmap == NULL)
                            break; // out of memory; kresleni nechame na listview; padame

                        RECT r; // obdelnik kolem sub item
                        ListView_GetSubItemRect(FoundFilesListView->HWindow, cd->nmcd.dwItemSpec, cd->iSubItem, LVIR_BOUNDS, &r);
                        RECT r2; // obdelnik o stejnem rozmetu jako r, ale posunuty do nuly
                        r2.left = 0;
                        r2.top = 0;
                        r2.right = r.right - r.left;
                        r2.bottom = r.bottom - r.top;

                        // nafoukneme cache bitmap
                        if (CacheBitmap->NeedEnlarge(r2.right, r2.bottom))
                            CacheBitmap->Enlarge(r2.right, r2.bottom);

                        // podmazeme pozadi default barvou
                        int bkColor = (GrepData.FindDuplicates && item->Different == 1) ? COLOR_3DFACE : COLOR_WINDOW;
                        int textColor = COLOR_WINDOWTEXT;

                        if (Configuration.FindFullRowSelect)
                        {
                            if (ListView_GetItemState(FoundFilesListView->HWindow, cd->nmcd.dwItemSpec, LVIS_SELECTED) & LVIS_SELECTED)
                            {
                                if (GetFocus() == FoundFilesListView->HWindow)
                                {
                                    bkColor = COLOR_HIGHLIGHT;
                                    textColor = COLOR_HIGHLIGHTTEXT;
                                }
                                else
                                {
                                    if (GetSysColor(COLOR_3DFACE) != GetSysColor(COLOR_WINDOW))
                                        bkColor = COLOR_3DFACE;
                                    else
                                    {
                                        // pro high contrast barevna schemata
                                        bkColor = COLOR_HIGHLIGHT;
                                        textColor = COLOR_HIGHLIGHTTEXT;
                                    }
                                }
                            }
                        }

                        SetBkColor(CacheBitmap->HMemDC, GetSysColor(bkColor));
                        ExtTextOut(CacheBitmap->HMemDC, 0, 0, ETO_OPAQUE, &r2, "", 0, NULL);
                        SetBkMode(CacheBitmap->HMemDC, TRANSPARENT);

                        // vykreslime text s vypustkou
                        r2.left += 5;
                        r2.right -= 5;
                        Find_FileList_Item* item2 = FoundFilesListView->Items_At((int)cd->nmcd.dwItemSpec);
                        SelectObject(CacheBitmap->HMemDC, (HFONT)SendMessage(FoundFilesListView->HWindow, WM_GETFONT, 0, 0));
                        int oldTextColor = SetTextColor(CacheBitmap->HMemDC, GetSysColor(textColor));

                        // DT_PATH_ELLIPSIS nefunguje na nekterych retezcich, dochazi pak vytisteni oclipovaneho textu
                        // PathCompactPath() sice potrebuje kopii do lokalniho bufferu, ale neclipuje texty
                        char buff[2 * MAX_PATH];
                        strncpy_s(buff, _countof(buff), item2->Path, _TRUNCATE);
                        PathCompactPath(CacheBitmap->HMemDC, buff, r2.right - r2.left);
                        DrawText(CacheBitmap->HMemDC, buff, -1, &r2,
                                 DT_VCENTER | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE);
                        //                DrawText(CacheBitmap->HMemDC, item2->Path, -1, &r2,
                        //                         DT_VCENTER | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_PATH_ELLIPSIS);
                        SetTextColor(CacheBitmap->HMemDC, oldTextColor);

                        // cache preneseme do listview
                        BitBlt(hDC, r.left, r.top, r.right - r.left, r.bottom - r.top,
                               CacheBitmap->HMemDC, 0, 0, SRCCOPY);

                        // zakazeme default kresleni
                        SetWindowLongPtr(HWindow, DWLP_MSGRESULT, CDRF_SKIPDEFAULT);
                        return TRUE;
                    }

                    if (GrepData.FindDuplicates && item->Different == 1)
                    {
                        cd->clrTextBk = GetSysColor(COLOR_3DFACE);
                        SetWindowLongPtr(HWindow, DWLP_MSGRESULT, CDRF_NEWFONT);
                        return TRUE;
                    }
                    break;
                }

                break;
            }

            case LVN_ODFINDITEM:
            {
                // pomuzeme listview s quick search
                NMLVFINDITEM* pFindInfo = (NMLVFINDITEM*)lParam;
                int iStart = pFindInfo->iStart;
                LVFINDINFO* fi = &pFindInfo->lvfi;
                int ret = -1; // not found

                if (fi->flags & LVFI_STRING || fi->flags & LVFI_PARTIAL)
                {
                    //              BOOL partial = fi->flags & LVFI_PARTIAL != 0;
                    // podle dokumentace by melo chodit LVFI_PARTIAL a LVFI_STRING,
                    // ale chodi pouze LVFI_STRING. Nejakej manik si na to stezoval
                    // na newsech, ale zadna odpoved. Takze to tady forcnu.
                    BOOL partial = TRUE;
                    int i;
                    for (i = iStart; i < FoundFilesListView->Items_Count(); i++)
                    {
                        const Find_FileList_Item* item = FoundFilesListView->Items_At(i);
                        if (partial)
                        {
                            if (StrNICmp(item->Name, fi->psz, (int)strlen(fi->psz)) == 0)
                            {
                                ret = i;
                                break;
                            }
                        }
                        else
                        {
                            if (StrICmp(item->Name, fi->psz) == 0)
                            {
                                ret = i;
                                break;
                            }
                        }
                    }
                    if (ret == -1 && fi->flags & LVFI_WRAP)
                    {
                        for (i = 0; i < iStart; i++)
                        {
                            const Find_FileList_Item* item = FoundFilesListView->Items_At(i);
                            if (partial)
                            {
                                if (StrNICmp(item->Name, fi->psz, (int)strlen(fi->psz)) == 0)
                                {
                                    ret = i;
                                    break;
                                }
                            }
                            else
                            {
                                if (StrICmp(item->Name, fi->psz) == 0)
                                {
                                    ret = i;
                                    break;
                                }
                            }
                        }
                    }
                }

                SetWindowLongPtr(HWindow, DWLP_MSGRESULT, ret);
                return TRUE;
            }

            case LVN_COLUMNCLICK:
            {
                int subItem = ((NM_LISTVIEW*)lParam)->iSubItem;
                if (subItem >= 0 && subItem < 5)
                    FoundFilesListView->SortItems(subItem);
                break;
            }

            case LVN_GETDISPINFO:
            {
                LV_DISPINFO* info = (LV_DISPINFO*)lParam;
                Find_FileList_Item* item = FoundFilesListView->Items_At(info->item.iItem);
                if (info->item.mask & LVIF_IMAGE)
                    info->item.iImage = item->IsDir ? 0 : 1;
                if (info->item.mask & LVIF_TEXT)
                    info->item.pszText = item->Text_Get( info->item.iSubItem, FoundFilesDataTextBuffer, FileNameFormat );
                break;
            }

            case LVN_ITEMCHANGED:
            {
                EnableToolBar();
                if (!IsSearchInProgress())
                    UpdateStatusBar = TRUE; // pri Idle bude nastaven text
                break;
            }

            case LVN_BEGINDRAG:
            case LVN_BEGINRDRAG:
            {
                OnDrag(((LPNMHDR)lParam)->code == LVN_BEGINRDRAG);
                return 0;
            }

            case LVN_KEYDOWN:
            {
                NMLVKEYDOWN* kd = (NMLVKEYDOWN*)lParam;
                BOOL controlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                BOOL altPressed = (GetKeyState(VK_MENU) & 0x8000) != 0;
                BOOL shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                DWORD cmd = 0;
                switch (kd->wVKey)
                {
                case VK_F3:
                {
                    if (!controlPressed && !altPressed && !shiftPressed)
                        cmd = CM_FIND_VIEW;
                    if (!controlPressed && altPressed && !shiftPressed)
                        cmd = CM_FIND_ALTVIEW;
                    if (controlPressed && !altPressed && shiftPressed)
                        cmd = CM_FIND_VIEW_WITH;
                    if (controlPressed && !altPressed && !shiftPressed)
                        cmd = CM_FIND_NAME;
                    break;
                }

                case VK_F4:
                {
                    if (!controlPressed && !altPressed && !shiftPressed)
                        cmd = CM_FIND_EDIT;
                    if (controlPressed && !altPressed && shiftPressed)
                        cmd = CM_FIND_EDIT_WITH;
                    if (controlPressed && !altPressed && !shiftPressed)
                        cmd = CM_FIND_PATH;
                    break;
                }

                case VK_F5:
                {
                    if (controlPressed && !altPressed && !shiftPressed)
                        cmd = CM_FIND_TIME;
                    break;
                }

                case VK_F6:
                {
                    if (controlPressed && !altPressed && !shiftPressed)
                        cmd = CM_FIND_SIZE;
                    break;
                }

                case VK_DELETE:
                {
                    if (!controlPressed && !altPressed && !shiftPressed ||
                        !controlPressed && !altPressed && shiftPressed)
                        cmd = CM_FIND_DELETE;
                    break;
                }

                case VK_F8:
                {
                    if (!controlPressed && !altPressed && !shiftPressed)
                        cmd = CM_FIND_DELETE;
                    break;
                }

                case VK_F9:
                {
                    if (!controlPressed && !altPressed && !shiftPressed)
                        cmd = CM_FIND_USERMENU;
                    break;
                }

                case VK_RETURN:
                {
                    if (!controlPressed && !altPressed)
                        OnOpen(!shiftPressed);
                    if (!controlPressed && altPressed && !shiftPressed)
                    {
                        cmd = CM_FIND_PROPERTIES;
                        SkipCharacter = TRUE;
                    }
                    break;
                }

                case VK_SPACE:
                {
                    if (!controlPressed && !altPressed && !shiftPressed)
                        cmd = CM_FIND_FOCUS;
                    break;
                }

                case VK_INSERT:
                {
                    if (!controlPressed && altPressed && !shiftPressed)
                        cmd = CM_FIND_CLIPCOPYFULLNAME;
                    if (!controlPressed && altPressed && shiftPressed)
                        cmd = CM_FIND_CLIPCOPYNAME;
                    if (controlPressed && altPressed && !shiftPressed)
                        cmd = CM_FIND_CLIPCOPYFULLPATH;
                    if (controlPressed && !altPressed && shiftPressed)
                        cmd = CM_FIND_CLIPCOPYUNCNAME;
                    break;
                }

                case 'A':
                {
                    if (controlPressed && !altPressed && !shiftPressed)
                        cmd = CM_FIND_SELECTALL;
                    break;
                }

                case 'C':
                {
                    if (controlPressed && !altPressed && !shiftPressed)
                        cmd = CM_FIND_CLIPCOPY;
                    break;
                }

                case 'X':
                {
                    if (controlPressed && !altPressed && !shiftPressed)
                        cmd = CM_FIND_CLIPCUT;
                    break;
                }

                case 'H':
                {
                    if (controlPressed && !altPressed && !shiftPressed)
                        cmd = CM_FIND_HIDESEL;
                    if (controlPressed && !altPressed && shiftPressed)
                        cmd = CM_FIND_HIDE_DUP;
                    break;
                }
                }
                if (cmd != 0)
                    PostMessage(HWindow, WM_COMMAND, cmd, 0);
                return 0;
            }
            }
        }
        break;
    }

    case WM_USER_ADDFILE:
    {
        UpdateListViewItems();
        return 0;
    }

    case WM_USER_ADDLOG:
    {
    //Add new log.
    //
    //Notice:
    //Request is running in the find thread.
        const auto*     item = (Find_Log_Item_VM*)wParam;

        Log.Items_Add( *item );

        return 0;
    }

    case WM_USER_BUTTONS:
    {
        EnableControls(wParam != NULL);
        if (wParam != NULL)
            PostMessage((HWND)wParam, BM_SETSTYLE, BS_DEFPUSHBUTTON, MAKELPARAM(TRUE, 0));
        return 0;
    }

    case WM_USER_CFGCHANGED:
    {
        TBHeader->SetFont();
        return 0;
    }

    case WM_ACTIVATEAPP:
    {
        if (wParam == FALSE) // pri deaktivaci uteceme z adresaru zobrazenych v panelech,
        {                    // aby sly mazat, odpojovat atd. z jinych softu
            if (CanChangeDirectory())
                SetCurrentDirectoryToSystem();
        }
        else
        {
            SuppressToolTipOnCurrentMousePos(); // potlaceni nechteneho tooltipu pri prepnuti do okna
        }
        break;
    }

    case WM_ACTIVATE:
    {
        if (wParam != WA_INACTIVE)
        {
            if (FlashIconsOnActivation)
            {
                if (TBHeader != NULL)
                    TBHeader->StartFlashIcon();
                FlashIconsOnActivation = FALSE;
            }
        }
        break;
    }

    case WM_DESTROY:
    {
        if (SearchInProgress)
            StopSearch();

        if (!DlgFailed)
        {
            // ulozim sirku sloupce Name
            Configuration.FindColNameWidth = ListView_GetColumnWidth(FoundFilesListView->HWindow, 0);
            // ulozim umisteni okna
            Configuration.FindDialogWindowPlacement.length = sizeof(WINDOWPLACEMENT);
            GetWindowPlacement(HWindow, &Configuration.FindDialogWindowPlacement);
        }
        if (FoundFilesListView != NULL)
        {
            // uvolnime handle, jinak by ho ListView vzalo s sebou do pekel
            ListView_SetImageList(FoundFilesListView->HWindow, NULL, LVSIL_SMALL);
        }
        if (MenuBar != NULL)
        {
            DestroyWindow(MenuBar->HWindow);
            delete MenuBar;
            MenuBar = NULL;
        }
        if (MainMenu != NULL)
        {
            delete MainMenu;
            MainMenu = NULL;
        }
        if (TBHeader != NULL)
        {
            DestroyWindow(TBHeader->HWindow);
            TBHeader = NULL;
        }
        if (EditLine->HWindow == NULL)
        {
            delete EditLine;
            EditLine = NULL;
        }

        FindDialogQueue.Remove(HWindow);

        // pokud uzivatel nakopiruje vysledek hledani do schranky (Ctrl+C), prepne se do hlavniho okna
        // a da prikaz Paste Shortcut (Ctrl+S) a behem vytvareni zastupcu zavre Find okno, musime pockat
        // na dobehnuti Paste prikazu v hlavnim okne; jinak dochazelo k padu
        //
        // pokud provede Paste Shortcut do okna Explorera (pripadne jinam), nedozvime se to a pad porad hrozi
        //
        // pokud po Ctrl+C zavre okno Findu a teprve potom da Paste, zavolame v ramci UninitializeOle()
        // funkci OleFlushClipboard(), ktera data odpouta od tohoto vlakna a zadny problem nenastane
        // teoreticky bychom OleFlushClipboard() mohli volat po kazdem Ctrl+C primo v tomto vlakne,
        // ale nejsme si jisty, zda by neco neprestalo fungovat (nevim jak je renderovani dat dokonale),
        // navic OleFlushClipboard() na 2000 souborech dokaze vterinu trvat
        // takze volime tento hack,
        while (PasteLinkIsRunning > 0)
        {
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (PasteLinkIsRunning > 0)
                Sleep(50); // jde o aktivni cekani, trosku thread pribrzdime
        }

        UninitializeOle();

        if (ZeroOnDestroy != NULL)
            *ZeroOnDestroy = NULL;
        PostQuitMessage(0);
        break;
    }

    case WM_SYSCOLORCHANGE:
    {
        ListView_SetBkColor(GetDlgItem(HWindow, IDC_FIND_RESULTS), GetSysColor(COLOR_WINDOW));
        break;
    }
    }

    return CCommonDialog::DialogProc(uMsg, wParam, lParam);
}

