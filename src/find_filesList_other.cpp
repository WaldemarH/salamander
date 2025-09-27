// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_filesList.h"
#include "find_dialog.h"
#include "find.old.h"
#include "cfgdlg.h"


//TODO -> cleanup


BOOL Find_FilesList_View::TakeDataForRefine()
{
    m_Items_ForRefine.DestroyMembers();
    int i;
    for (i = 0; i < m_Items.Count; i++)
    {
        Find_FileList_Item* refineData = m_Items[i];
        m_Items_ForRefine.Add(refineData);
        if (!m_Items_ForRefine.IsGood())
        {
            m_Items_ForRefine.ResetState();
            m_Items_ForRefine.DetachMembers();
            return FALSE;
        }
    }
    m_Items.DetachMembers();
    return TRUE;
}

void Find_FilesList_View::DestroyDataForRefine()
{
    m_Items_ForRefine.DestroyMembers();
}

int Find_FilesList_View::GetDataForRefineCount()
{
    return m_Items_ForRefine.Count;
}

Find_FileList_Item* Find_FilesList_View::GetDataForRefine(int index)
{
    Find_FileList_Item* ptr;
    ptr = m_Items_ForRefine[index];
    return ptr;
}

DWORD Find_FilesList_View::GetSelectedListSize()
{
    // tato metoda je volana pouze z hlavniho threadu
    DWORD size = 0;
    int index = -1;
    do
    {
        index = ListView_GetNextItem(HWindow, index, LVIS_SELECTED);
        if (index != -1)
        {
            Find_FileList_Item* ptr = m_Items[index];
            int pathLen = lstrlen(ptr->Path);
            if (ptr->Path[pathLen - 1] != '\\')
                pathLen++; // pokud path neobsahuje zpetne lomitko, vyhradime pro nej prostor
            int nameLen = lstrlen(ptr->Name);
            size += pathLen + nameLen + 1; // vyhradime prostor na terminator
        }
    } while (index != -1);
    if (size == 0)
        size = 2;
    else
        size++;

    return size;
}

BOOL Find_FilesList_View::GetSelectedList(char* list, DWORD maxSize)
{
    DWORD size = 0;
    int index = -1;
    do
    {
        index = ListView_GetNextItem(HWindow, index, LVIS_SELECTED);
        if (index != -1)
        {
            Find_FileList_Item* ptr = m_Items[index];
            int pathLen = lstrlen(ptr->Path);
            if (ptr->Path[pathLen - 1] != '\\')
                size++; // pokud path neobsahuje zpetne lomitko, vyhradime pro nej prostor
            size += pathLen;
            if (size > maxSize)
            {
                TRACE_E("Buffer is too short");
                return FALSE;
            }
            memmove(list, ptr->Path, pathLen);
            list += pathLen;
            if (ptr->Path[pathLen - 1] != '\\')
                *list++ = '\\';
            int nameLen = lstrlen(ptr->Name);
            size += nameLen + 1; // vyhradime prostor na terminator
            if (size > maxSize)
            {
                TRACE_E("Buffer is too short");
                return FALSE;
            }
            memmove(list, ptr->Name, nameLen + 1);
            list += nameLen + 1;
        }
    } while (index != -1);

    if (size == 0)
    {
        if (size + 2 > maxSize)
        {
            TRACE_E("Buffer is too short");
            return FALSE;
        }
        *list++ = '\0';
        *list++ = '\0';
    }
    else
    {
        if (size + 1 > maxSize)
        {
            TRACE_E("Buffer is too short");
            return FALSE;
        }
        *list++ = '\0';
    }
    return TRUE;
}

void Find_FilesList_View::CheckAndRemoveSelectedItems(BOOL forceRemove, int lastFocusedIndex, const Find_FileList_Item* lastFocusedItem)
{
    int removedItems = 0;

    int totalCount = ListView_GetItemCount(HWindow);
    int i;
    for (i = totalCount - 1; i >= 0; i--)
    {
        if (ListView_GetItemState(HWindow, i, LVIS_SELECTED) & LVIS_SELECTED)
        {
            Find_FileList_Item* ptr = m_Items[i];
            BOOL remove = forceRemove;
            if (!forceRemove)
            {
                char fullPath[MAX_PATH];
                int pathLen = lstrlen(ptr->Path);
                memmove(fullPath, ptr->Path, pathLen + 1);
                if (ptr->Path[pathLen - 1] != '\\')
                {
                    fullPath[pathLen] = '\\';
                    fullPath[pathLen + 1] = '\0';
                }
                lstrcat(fullPath, ptr->Name);
                remove = (SalGetFileAttributes(fullPath) == -1);
            }
            if (remove)
            {
                Items_Delete(i);
                removedItems++;
            }
        }
    }
    if (removedItems > 0)
    {
        // reknu listview novy pocet polozek
        totalCount = totalCount - removedItems;
        ListView_SetItemCount(HWindow, totalCount);
        if (totalCount > 0)
        {
            // zahodim select vsech polozek
            ListView_SetItemState(HWindow, -1, 0, LVIS_SELECTED);

            // pokusime se dohledat, zda drive vybrana polozka jeste existuje a vybrat ji
            int selectIndex = -1;
            if (lastFocusedIndex != -1)
            {
                for (i = 0; i < totalCount; i++)
                {
                    Find_FileList_Item* ptr = m_Items[i];
                    if (lastFocusedItem != NULL &&
                        lastFocusedItem->Name != NULL && strcmp(ptr->Name, lastFocusedItem->Name) == 0 &&
                        lastFocusedItem->Path != NULL && strcmp(ptr->Path, lastFocusedItem->Path) == 0)
                    {
                        selectIndex = i;
                        break;
                    }
                }
                if (selectIndex == -1)
                    selectIndex = min(lastFocusedIndex, totalCount - 1); // pokud jsme ji nenasli, nechame stat kurzor na sve pozici, ale maximalne do poctu polozek
            }
            if (selectIndex == -1) // zachrana -- nulta polozka
                selectIndex = 0;
            ListView_SetItemState(HWindow, selectIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
            ListView_EnsureVisible(HWindow, selectIndex, FALSE);
        }
        else
            m_FindDialog.UpdateStatusBar = TRUE;
        m_FindDialog.UpdateListViewItems();
    }
}

BOOL Find_FilesList_View::IsGood()
{
    BOOL isGood;
    HANDLES(EnterCriticalSection(&m_CriticalSection));
    isGood = m_Items.IsGood();
    HANDLES(LeaveCriticalSection(&m_CriticalSection));
    return isGood;
}

void Find_FilesList_View::ResetState()
{
    HANDLES(EnterCriticalSection(&m_CriticalSection));
    m_Items.ResetState();
    HANDLES(LeaveCriticalSection(&m_CriticalSection));
}

void Find_FilesList_View::StoreItemsState()
{
    int count = Items_Count();
    int i;
    for (i = 0; i < count; i++)
    {
        DWORD state = ListView_GetItemState(HWindow, i, LVIS_FOCUSED | LVIS_SELECTED);
        m_Items[i]->Selected = (state & LVIS_SELECTED) != 0 ? 1 : 0;
        m_Items[i]->Focused = (state & LVIS_FOCUSED) != 0 ? 1 : 0;
    }
}

void Find_FilesList_View::RestoreItemsState()
{
    int count = Items_Count();
    int i;
    for (i = 0; i < count; i++)
    {
        DWORD state = 0;
        if (m_Items[i]->Selected)
            state |= LVIS_SELECTED;
        if (m_Items[i]->Focused)
            state |= LVIS_FOCUSED;
        ListView_SetItemState(HWindow, i, state, LVIS_FOCUSED | LVIS_SELECTED);
    }
}

void Find_FilesList_View::SortItems(int sortBy)
{
    if (sortBy == 5)
        return; // podle atributu neumime radit

    BOOL enabledNameSize = TRUE;
    BOOL enabledPathTime = TRUE;
    if (m_FindDialog.GrepData.FindDuplicates)
    {
        enabledPathTime = FALSE; // v pripade duplikatu nema vyznam
        // radit podle jmena a velikosti lze v pripade duplicatu pouze
        // pokud bylo hledano podle stejneho jmena i velikosti
        enabledNameSize = (m_FindDialog.GrepData.FindDupFlags & FIND_DUPLICATES_NAME) &&
                          (m_FindDialog.GrepData.FindDupFlags & FIND_DUPLICATES_SIZE);
    }

    if (!enabledNameSize && (sortBy == 0 || sortBy == 2))
        return;
    if (!enabledPathTime && (sortBy == 1 || sortBy == 3 || sortBy == 4))
        return;

    HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    HANDLES(EnterCriticalSection(&m_CriticalSection));

    //   EnumFileNamesChangeSourceUID(HWindow, &EnumFileNamesSourceUID);  // zakomentovano, nevim proc to tu je: Petr

    // pokud mame nejake polozky v datech a nejsou v listview, preneseme je
    m_FindDialog.UpdateListViewItems();

    if (m_Items.Count > 0)
    {
        // ulozime stav slected a focused polozek
        StoreItemsState();

        // seradim pole podle pozadovaneho kriteria
        QuickSort(0, m_Items.Count - 1, sortBy);
        if (m_FindDialog.GrepData.FindDuplicates)
        {
            QuickSortDuplicates(0, m_Items.Count - 1, sortBy == 0);
            SetDifferentByGroup();
        }
        else
        {
            QuickSort(0, m_Items.Count - 1, sortBy);
        }

        // obnovime stavy polozek
        RestoreItemsState();

        int focusIndex = ListView_GetNextItem(HWindow, -1, LVNI_FOCUSED);
        if (focusIndex != -1)
            ListView_EnsureVisible(HWindow, focusIndex, FALSE);
        ListView_RedrawItems(HWindow, 0, m_Items.Count - 1);
        UpdateWindow(HWindow);
    }

    HANDLES(LeaveCriticalSection(&m_CriticalSection));
    SetCursor(hCursor);
}

void Find_FilesList_View::SetDifferentByGroup()
{
    Find_FileList_Item* lastData = NULL;
    int different = 0;
    if (m_Items.Count > 0)
    {
        lastData = m_Items.At(0);
        lastData->Different = different;
    }
    int i;
    for (i = 1; i < m_Items.Count; i++)
    {
        Find_FileList_Item* data = m_Items.At(i);
        if (data->Group == lastData->Group)
        {
            data->Different = different;
        }
        else
        {
            different++;
            if (different > 1)
                different = 0;
            lastData = data;
            lastData->Different = different;
        }
    }
}

void Find_FilesList_View::QuickSort(int left, int right, int sortBy)
{

LABEL_QuickSort2:

    int i = left, j = right;
    Find_FileList_Item* pivot = m_Items[(i + j) / 2];

    do
    {
        while (CompareFunc(m_Items[i], pivot, sortBy) < 0 && i < right)
            i++;
        while (CompareFunc(pivot, m_Items[j], sortBy) < 0 && j > left)
            j--;

        if (i <= j)
        {
            Find_FileList_Item* swap = m_Items[i];
            m_Items[i] = m_Items[j];
            m_Items[j] = swap;
            i++;
            j--;
        }
    } while (i <= j);

    // nasledujici "hezky" kod jsme nahradili kodem podstatne setricim stack (max. log(N) zanoreni rekurze)
    //  if (left < j) QuickSort(left, j, sortBy);
    //  if (i < right) QuickSort(i, right, sortBy);

    if (left < j)
    {
        if (i < right)
        {
            if (j - left < right - i) // je potreba seradit obe "poloviny", tedy do rekurze posleme tu mensi, tu druhou zpracujeme pres "goto"
            {
                QuickSort(left, j, sortBy);
                left = i;
                goto LABEL_QuickSort2;
            }
            else
            {
                QuickSort(i, right, sortBy);
                right = j;
                goto LABEL_QuickSort2;
            }
        }
        else
        {
            right = j;
            goto LABEL_QuickSort2;
        }
    }
    else
    {
        if (i < right)
        {
            left = i;
            goto LABEL_QuickSort2;
        }
    }
}

int Find_FilesList_View::CompareFunc(Find_FileList_Item* f1, Find_FileList_Item* f2, int sortBy)
{
    int res;
    int next = sortBy;
    do
    {
        if (f1->IsDir == f2->IsDir) // jde o polozky ze stejne skupiny (adresare/soubory)?
        {
            switch (next)
            {
            case 0:
            {
                res = RegSetStrICmp(f1->Name, f2->Name);
                break;
            }

            case 1:
            {
                res = RegSetStrICmp(f1->Path, f2->Path);
                break;
                break;
            }

            case 2:
            {
                if (f1->Size < f2->Size)
                    res = -1;
                else
                {
                    if (f1->Size == f2->Size)
                        res = 0;
                    else
                        res = 1;
                }
                break;
            }

            default:
            {
                res = CompareFileTime(&f1->LastWrite, &f2->LastWrite);
                break;
            }
            }
        }
        else
            res = f1->IsDir ? -1 : 1;

        if (next == sortBy)
        {
            if (sortBy != 0)
                next = 0;
            else
                next = 1;
        }
        else if (next + 1 != sortBy)
            next++;
        else
            next += 2;
    } while (res == 0 && next <= 3);

    return res;
}

// quick sort pro rezim duplikatu; vola specialni compare
void Find_FilesList_View::QuickSortDuplicates(int left, int right, BOOL byName)
{

LABEL_QuickSortDuplicates:

    int i = left, j = right;
    Find_FileList_Item* pivot = m_Items[(i + j) / 2];

    do
    {
        while (CompareDuplicatesFunc(m_Items[i], pivot, byName) < 0 && i < right)
            i++;
        while (CompareDuplicatesFunc(pivot, m_Items[j], byName) < 0 && j > left)
            j--;

        if (i <= j)
        {
            Find_FileList_Item* swap = m_Items[i];
            m_Items[i] = m_Items[j];
            m_Items[j] = swap;
            i++;
            j--;
        }
    } while (i <= j);

    // nasledujici "hezky" kod jsme nahradili kodem podstatne setricim stack (max. log(N) zanoreni rekurze)
    //  if (left < j) QuickSortDuplicates(left, j, byName);
    //  if (i < right) QuickSortDuplicates(i, right, byName);

    if (left < j)
    {
        if (i < right)
        {
            if (j - left < right - i) // je potreba seradit obe "poloviny", tedy do rekurze posleme tu mensi, tu druhou zpracujeme pres "goto"
            {
                QuickSortDuplicates(left, j, byName);
                left = i;
                goto LABEL_QuickSortDuplicates;
            }
            else
            {
                QuickSortDuplicates(i, right, byName);
                right = j;
                goto LABEL_QuickSortDuplicates;
            }
        }
        else
        {
            right = j;
            goto LABEL_QuickSortDuplicates;
        }
    }
    else
    {
        if (i < right)
        {
            left = i;
            goto LABEL_QuickSortDuplicates;
        }
    }
}

// compare pro rezim zobrazenych duplikatu; pokud je 'byName', radi se primarne podle jmena, jinak podle velikost
int Find_FilesList_View::CompareDuplicatesFunc(Find_FileList_Item* f1, Find_FileList_Item* f2, BOOL byName)
{
    int res;
    if (byName)
    {
        // by name
        res = RegSetStrICmp(f1->Name, f2->Name);
        if (res == 0)
        {
            // by size
            if (f1->Size < f2->Size)
                res = -1;
            else
            {
                if (f1->Size == f2->Size)
                {
                    // by group
                    if (f1->Group < f2->Group)
                        res = -1;
                    else
                    {
                        if (f1->Group == f2->Group)
                            res = 0;
                        else
                            res = 1;
                    }
                }
                else
                    res = 1;
            }
        }
    }
    else
    {
        // by size
        if (f1->Size < f2->Size)
            res = -1;
        else
        {
            if (f1->Size == f2->Size)
            {
                // by name
                res = RegSetStrICmp(f1->Name, f2->Name);
                if (res == 0)
                {
                    // by group
                    if (f1->Group < f2->Group)
                        res = -1;
                    else
                    {
                        if (f1->Group == f2->Group)
                            res = 0;
                        else
                            res = 1;
                    }
                }
            }
            else
                res = 1;
        }
    }
    if (res == 0)
        res = RegSetStrICmp(f1->Path, f2->Path);
    return res;
}

BOOL Find_FilesList_View::InitColumns()
{
    CALL_STACK_MESSAGE1("Find_FilesList_View::InitColumns()");
    LV_COLUMN lvc;
    int header[] = {IDS_FOUNDFILESCOLUMN1, IDS_FOUNDFILESCOLUMN2,
                    IDS_FOUNDFILESCOLUMN3, IDS_FOUNDFILESCOLUMN4,
                    IDS_FOUNDFILESCOLUMN5, IDS_FOUNDFILESCOLUMN6,
                    -1};

    lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;
    int i;
    for (i = 0; header[i] != -1; i++) // vytvorim sloupce
    {
        if (i == 2)
            lvc.fmt = LVCFMT_RIGHT;
        lvc.pszText = LoadStr(header[i]);
        lvc.iSubItem = i;
        if (ListView_InsertColumn(HWindow, i, &lvc) == -1)
            return FALSE;
    }

    RECT r;
    GetClientRect(HWindow, &r);
    DWORD cx = r.right - r.left - 1;
    ListView_SetColumnWidth(HWindow, 5, ListView_GetStringWidth(HWindow, "ARH") + 20);

    char format1[200];
    char format2[200];
    SYSTEMTIME st;
    ZeroMemory(&st, sizeof(st));
    st.wYear = 2000; // nejdelsi mozna hodnota
    st.wMonth = 12;  // nejdelsi mozna hodnota
    st.wDay = 30;    // nejdelsi mozna hodnota
    st.wHour = 10;   // dopoledne (nevime, zda bude kratsi zapis dopoledne nebo odpoledne, zkusime oba)
    st.wMinute = 59; // nejdelsi mozna hodnota
    st.wSecond = 59; // nejdelsi mozna hodnota
    if (GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, format1, 200) == 0)
        sprintf(format1, "%u:%02u:%02u", st.wHour, st.wMinute, st.wSecond);
    st.wHour = 20; // odpoledne
    if (GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, format2, 200) == 0)
        sprintf(format2, "%u:%02u:%02u", st.wHour, st.wMinute, st.wSecond);

    int maxWidth = ListView_GetStringWidth(HWindow, format1);
    int w = ListView_GetStringWidth(HWindow, format2);
    if (w > maxWidth)
        maxWidth = w;
    ListView_SetColumnWidth(HWindow, 4, maxWidth + 20);

    maxWidth = 0;
    if (GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, format1, 200) == 0)
        sprintf(format1, "%u.%u.%u", st.wDay, st.wMonth, st.wYear);
    else
    {
        // overime, ze kratky format datumu neobsahuje alpha znaky
        const char* p = format1;
        while (*p != 0 && !IsAlpha[*p])
            p++;
        if (IsAlpha[*p])
        {
            // obsahuje alpha znaky -- musime dohledat nejdelsi zapis mesice a dne
            int maxMonth = 0;
            int sats[] = {1, 5, 4, 1, 6, 3, 1, 5, 2, 7, 4, 2};
            int mo;
            for (mo = 0; mo < 12; mo++) // projdeme vsechny mesice pocinaje lednem, den v tydnu bude shodny, aby se neprojevila jeho sire, wDay bude jednociferny ze stejneho duvodu
            {
                st.wDay = sats[mo];
                st.wMonth = 1 + mo;
                if (GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, format1, 200) != 0)
                {
                    w = ListView_GetStringWidth(HWindow, format1);
                    if (w > maxWidth)
                    {
                        maxWidth = w;
                        maxMonth = st.wMonth;
                    }
                }
            }
            if (maxWidth > 0)
            {
                st.wMonth = maxMonth;
                for (st.wDay = 21; st.wDay < 28; st.wDay++) // vsechny mozne dny v tydnu (nemusi zacinat od pondeli)
                {
                    if (GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, format1, 200) != 0)
                    {
                        w = ListView_GetStringWidth(HWindow, format1);
                        if (w > maxWidth)
                        {
                            maxWidth = w;
                        }
                    }
                }
            }
        }
    }

    ListView_SetColumnWidth(HWindow, 3, (maxWidth > 0 ? maxWidth : ListView_GetStringWidth(HWindow, format1)) + 20);
    ListView_SetColumnWidth(HWindow, 2, ListView_GetStringWidth(HWindow, "000 000 000 000") + 20); // do 1TB se vejdeme
    int width;
    if (Configuration.FindColNameWidth != -1)
        width = Configuration.FindColNameWidth;
    else
        width = 20 + ListView_GetStringWidth(HWindow, "XXXXXXXX.XXX") + 20;
    ListView_SetColumnWidth(HWindow, 0, width);
    cx -= ListView_GetColumnWidth(HWindow, 0) + ListView_GetColumnWidth(HWindow, 2) +
          ListView_GetColumnWidth(HWindow, 3) + ListView_GetColumnWidth(HWindow, 4) +
          ListView_GetColumnWidth(HWindow, 5) + GetSystemMetrics(SM_CXHSCROLL) - 1;
    ListView_SetColumnWidth(HWindow, 1, cx);
    ListView_SetImageList(HWindow, HFindSymbolsImageList, LVSIL_SMALL);

    return TRUE;
}
