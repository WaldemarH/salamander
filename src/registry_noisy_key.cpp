// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "registry.h"
#include "cfgdlg.h"


bool Registry::Noisy_Key_Create( HWND hWnd_parent, const HKEY hKey, const String_TChar_View& name, HKEY& hKey_created )
{
//Execute request.
    LSTATUS     status_system = ERROR_SUCCESS;

    auto        status = Silent_Key_Create( hKey, name, hKey_created, &status_system );

    if ( status == false )
    {
    //Request failed -> make some noise.
        //Get error description.
        const auto      text_description = System::Error_GetErrorDescription( status_system );

        //Show message box.
        if ( HLanguage == NULL )
        {
            MessageBox( hWnd_parent, text_description.Text_Get(), TEXT( "Error saving configuration" ), MB_OK | MB_ICONEXCLAMATION );
        }
        else
        {
            SalMessageBox( hWnd_parent, text_description.Text_Get(), LoadStr( IDS_ERRORSAVECONFIG ), MB_OK | MB_ICONEXCLAMATION );
        }
    }

    return status;
}

bool Registry::Noisy_Key_Open( HWND hWnd_parent, const HKEY hKey, const String_TChar_View& name, HKEY& hKey_opened )
{
//Execute request.
    LSTATUS     status_system = ERROR_SUCCESS;

    auto        status = Silent_Key_Open( hKey, name, hKey_opened, &status_system );

    if ( status == false )
    {
    //Request failed -> make some noise.
        //Get error description.
        const auto      text_description = System::Error_GetErrorDescription( status_system );

        //Show message box.
        if ( HLanguage == NULL )
        {
            MessageBox( hWnd_parent, text_description.Text_Get(), TEXT( "Error loading configuration" ), MB_OK | MB_ICONEXCLAMATION );
        }
        else
        {
            SalMessageBox( hWnd_parent, text_description.Text_Get(), LoadStr( IDS_ERRORLOADCONFIG ), MB_OK | MB_ICONEXCLAMATION );
        }
    }

    return status;
}




// ****************************************************************************

BOOL LoadRGB(HKEY hKey, const char* name, COLORREF& color)
{
    char buf[50];
    DWORD returnedType;
    // for backward compatibility (up to reg:\HKEY_CURRENT_USER\Software\Altap\Altap Salamander 2.53 beta 1 (DB 33) included) we can load both
    // the string representation and the more efficient binary

    if (GetValue2(hKey, name, REG_SZ, REG_DWORD, &returnedType, buf, 50))
    {
        if (returnedType == REG_SZ)
        {
            BOOL end;
            char *s, *st;
            BYTE c[3];
            c[0] = c[1] = c[2] = 0;
            s = st = buf;
            int i;
            for (i = 0; i < 3; i++)
            {
                while (*s != 0 && *s != ',')
                    s++;
                end = (*s == 0);
                *s = 0;
                c[i] = (BYTE)(DWORD)atoi(st);
                if (end)
                    break;
                st = ++s;
            }
            color = RGB(c[0], c[1], c[2]);
        }
        else
        {
            color = (*(DWORD*)buf) & 0x00ffffff;
        }
        return TRUE;
    }
    return FALSE;
}

// ****************************************************************************

BOOL SaveRGB(HKEY hKey, const char* name, COLORREF color)
{
    //  char buf[50];
    //  sprintf(buf, "%d, %d, %d", GetRValue(color), GetGValue(color), GetBValue(color));
    //  return SetValue(hKey, name, REG_SZ, buf, strlen(buf) + 1);
    DWORD clr = color & 0x00ffffff; // zahodime "alpha" kanal
    return SetValue(hKey, name, REG_DWORD, &clr, 4);
}

// ****************************************************************************

BOOL LoadRGBF(HKEY hKey, const char* name, SALCOLOR& color)
{
    char buf[50];
    DWORD returnedType;
    // pro zpetnou kompatibilitu (do reg:\HKEY_CURRENT_USER\Software\Altap\Altap Salamander 2.53 beta 1 (DB 33) vcetne) umime nacitat jak
    // reprezentaci ve stringu, tak efektivnejsi binarni
    if (GetValue2(hKey, name, REG_SZ, REG_DWORD, &returnedType, buf, 50))
    {
        if (returnedType == REG_SZ)
        {
            BOOL end;
            char *s, *st;
            BYTE c[4];
            c[0] = c[1] = c[2] = c[3] = 0;
            s = st = buf;
            int i;
            for (i = 0; i < 4; i++)
            {
                while (*s != 0 && *s != ',')
                    s++;
                end = (*s == 0);
                *s = 0;
                c[i] = (BYTE)(DWORD)atoi(st);
                if (end)
                    break;
                st = ++s;
            }
            color = RGBF(c[0], c[1], c[2], c[3]);
        }
        else
        {
            color = *(DWORD*)buf;
        }
        return TRUE;
    }
    return FALSE;
}

// ****************************************************************************

BOOL SaveRGBF(HKEY hKey, const char* name, SALCOLOR color)
{
    //  char buf[50];
    //  sprintf(buf, "%d, %d, %d, %d", GetRValue(color), GetGValue(color), GetBValue(color), GetFValue(color));
    //  return SetValue(hKey, name, REG_SZ, buf, strlen(buf) + 1);
    return SetValue(hKey, name, REG_DWORD, &color, 4);
}

// ****************************************************************************

BOOL LoadLogFont(HKEY hKey, const char* name, LOGFONT* logFont)
{
    char buf[200];
    if (logFont != NULL && GetValue(hKey, name, REG_SZ, buf, 200))
    {
        logFont->lfHeight = -10;
        logFont->lfWidth = 0;
        logFont->lfEscapement = 0;
        logFont->lfOrientation = 0;
        logFont->lfWeight = FW_NORMAL;
        logFont->lfItalic = 0;
        logFont->lfUnderline = 0;
        logFont->lfStrikeOut = 0;
        logFont->lfCharSet = UserCharset;
        logFont->lfOutPrecision = OUT_DEFAULT_PRECIS;
        logFont->lfClipPrecision = CLIP_DEFAULT_PRECIS;
        logFont->lfQuality = DEFAULT_QUALITY;
        logFont->lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
        strcpy(logFont->lfFaceName, "MS Shell Dlg 2");

        char *s, *st;
        BOOL end;
        st = buf;
        s = buf;
        int i;
        for (i = 0; i < 9; i++)
        {
            while (*s != 0 && *s != ',')
                s++;
            end = (*s == 0);
            *s = 0;

            switch (i)
            {
            case 0: // lfFaceName
            {
                int l = (int)(s - st);
                if (l >= LF_FACESIZE)
                    l = LF_FACESIZE - 1;
                if (l > 0)
                {
                    memmove(logFont->lfFaceName, st, l);
                    logFont->lfFaceName[l] = 0;
                }
                break;
            }
            case 1:
                logFont->lfHeight = atoi(st);
                break;
            case 2:
                logFont->lfWeight = atoi(st);
                break;
            case 3:
                logFont->lfItalic = (BYTE)atoi(st);
                break;
            case 4:
                logFont->lfCharSet = (BYTE)atoi(st);
                break;
            case 5:
                logFont->lfOutPrecision = (BYTE)atoi(st);
                break;
            case 6:
                logFont->lfClipPrecision = (BYTE)atoi(st);
                break;
            case 7:
                logFont->lfQuality = (BYTE)atoi(st);
                break;
            case 8:
                logFont->lfPitchAndFamily = (BYTE)atoi(st);
                break;
            }

            if (end)
                break;
            s++;
            st = s;
        }
        return TRUE;
    }
    else
        return FALSE;
}

// ****************************************************************************

BOOL SaveLogFont(HKEY hKey, const char* name, LOGFONT* logFont)
{
    char buf[200];
    if (logFont != NULL)
    {
        char* s = buf;
        int i;
        for (i = 0; i < 9; i++)
        {
            switch (i)
            {
            case 0:
                strcpy(s, logFont->lfFaceName);
                break;
            case 1:
                itoa(logFont->lfHeight, s, 10);
                break;
            case 2:
                itoa(logFont->lfWeight, s, 10);
                break;
            case 3:
                itoa(logFont->lfItalic, s, 10);
                break;
            case 4:
                itoa(logFont->lfCharSet, s, 10);
                break;
            case 5:
                itoa(logFont->lfOutPrecision, s, 10);
                break;
            case 6:
                itoa(logFont->lfClipPrecision, s, 10);
                break;
            case 7:
                itoa(logFont->lfQuality, s, 10);
                break;
            case 8:
                itoa(logFont->lfPitchAndFamily, s, 10);
                break;
            }
            if (i < 8)
            {
                s += strlen(s);
                *s++ = ',';
                *s = 0;
            }
        }
        return SetValue(hKey, name, REG_SZ, buf, (int)(strlen(buf) + 1));
    }
    else
        return FALSE;
}

// ****************************************************************************

BOOL LoadHistory(HKEY hKey, const char* name, char* history[], int maxCount)
{
    HKEY historyKey;
    int i;
    for (i = 0; i < maxCount; i++)
        if (history[i] != NULL)
        {
            free(history[i]);
            history[i] = NULL;
        }
    if (OpenKey(hKey, name, historyKey))
    {
        char buf[10];
        for (i = 0; i < maxCount; i++)
        {
            itoa(i + 1, buf, 10);
            DWORD bufferSize;
            if (GetSize(historyKey, buf, REG_SZ, bufferSize))
            {
                history[i] = (char*)malloc(bufferSize);
                if (history[i] == NULL)
                {
                    TRACE_E(LOW_MEMORY);
                    break;
                }
                if (!GetValue(historyKey, buf, REG_SZ, history[i], bufferSize))
                    break;
            }
        }
        CloseKey(historyKey);
    }
    return TRUE;
}

// ****************************************************************************

BOOL SaveHistory(HKEY hKey, const char* name, char* history[], int maxCount, BOOL onlyClear)
{
    HKEY historyKey;
    if (CreateKey(hKey, name, historyKey))
    {
        ClearKey(historyKey);

        if (!onlyClear) // pokud se nema jen vycistit klic, ulozime hodnoty z historie
        {
            char buf[10];
            int i;
            for (i = 0; i < maxCount; i++)
            {
                if (history[i] != NULL)
                {
                    itoa(i + 1, buf, 10);
                    SetValue(historyKey, buf, REG_SZ, history[i], (int)strlen(history[i]) + 1);
                }
                else
                    break;
            }
        }
        CloseKey(historyKey);
    }
    return TRUE;
}

// ****************************************************************************

BOOL LoadViewers(HKEY hKey, const char* name, CViewerMasks* viewerMasks)
{
    HKEY viewersKey;
    if (OpenKey(hKey, name, viewersKey))
    {
        HKEY subKey;
        char buf[30];
        strcpy(buf, "1");
        char masks[MAX_PATH];
        char command[MAX_PATH];
        char arguments[MAX_PATH];
        char initDir[MAX_PATH];
        int type;
        int i = 1;
        viewerMasks->DestroyMembers();

        while (OpenKey(viewersKey, buf, subKey))
        {
            if (GetValue(subKey, VIEWERS_MASKS_REG, REG_SZ, masks, MAX_PATH) &&
                strchr(masks, '|') == NULL &&
                GetValue(subKey, VIEWERS_TYPE_REG, REG_DWORD, &type, sizeof(DWORD)))
            {
                if (!GetValue(subKey, VIEWERS_COMMAND_REG, REG_SZ, command, MAX_PATH))
                    *command = 0;
                if (!GetValue(subKey, VIEWERS_ARGUMENTS_REG, REG_SZ, arguments, MAX_PATH))
                    *arguments = 0;
                if (!GetValue(subKey, VIEWERS_INITDIR_REG, REG_SZ, initDir, MAX_PATH))
                    *initDir = 0;

                if (Configuration.ConfigVersion < 44) // prevod pripon na lowercase
                {
                    char masksAux[MAX_PATH];
                    lstrcpyn(masksAux, masks, MAX_PATH);
                    StrICpy(masks, masksAux);
                }
                CViewerMasksItem* item = new CViewerMasksItem(masks, command, arguments,
                                                              initDir, type, Configuration.ConfigVersion < 6);
                if (item != NULL && item->IsGood())
                {
                    viewerMasks->Add(item);
                    if (!viewerMasks->IsGood())
                    {
                        delete item;
                        viewerMasks->ResetState();
                        break;
                    }
                }
                else
                {
                    if (item != NULL)
                        delete item;
                    TRACE_E(LOW_MEMORY);
                    break;
                }
            }
            else
                break;
            itoa(++i, buf, 10);
            CloseKey(subKey);
        }
        CloseKey(viewersKey);
    }
    return TRUE;
}

// ****************************************************************************

BOOL SaveViewers(HKEY hKey, const char* name, CViewerMasks* viewerMasks)
{
    HKEY viewersKey;
    if (CreateKey(hKey, name, viewersKey))
    {
        ClearKey(viewersKey);
        HKEY subKey;
        char buf[30];
        int i;
        for (i = 0; i < viewerMasks->Count; i++)
        {
            itoa(i + 1, buf, 10);
            if (CreateKey(viewersKey, buf, subKey))
            {
                SetValue(subKey, VIEWERS_MASKS_REG, REG_SZ, viewerMasks->At(i)->Masks->GetMasksString(), -1);
                if (viewerMasks->At(i)->Command[0] != 0)
                    SetValue(subKey, VIEWERS_COMMAND_REG, REG_SZ, viewerMasks->At(i)->Command, -1);
                if (viewerMasks->At(i)->Arguments[0] != 0)
                    SetValue(subKey, VIEWERS_ARGUMENTS_REG, REG_SZ, viewerMasks->At(i)->Arguments, -1);
                if (viewerMasks->At(i)->InitDir[0] != 0)
                    SetValue(subKey, VIEWERS_INITDIR_REG, REG_SZ, viewerMasks->At(i)->InitDir, -1);
                SetValue(subKey, VIEWERS_TYPE_REG, REG_DWORD,
                         &viewerMasks->At(i)->ViewerType, sizeof(DWORD));
                CloseKey(subKey);
            }
            else
                break;
        }
        CloseKey(viewersKey);
    }
    return TRUE;
}

// ****************************************************************************

BOOL LoadEditors(HKEY hKey, const char* name, CEditorMasks* editorMasks)
{
    HKEY editorKey;
    if (OpenKey(hKey, name, editorKey))
    {
        HKEY subKey;
        char buf[30];
        strcpy(buf, "1");
        char masks[MAX_PATH];
        char command[MAX_PATH];
        char arguments[MAX_PATH];
        char initDir[MAX_PATH];
        int i = 1;
        editorMasks->DestroyMembers();

        while (OpenKey(editorKey, buf, subKey))
        {
            if (GetValue(subKey, EDITORS_MASKS_REG, REG_SZ, masks, MAX_PATH))
            {
                if (!GetValue(subKey, EDITORS_COMMAND_REG, REG_SZ, command, MAX_PATH))
                    *command = 0;
                if (!GetValue(subKey, EDITORS_ARGUMENTS_REG, REG_SZ, arguments, MAX_PATH))
                    *arguments = 0;
                if (!GetValue(subKey, EDITORS_INITDIR_REG, REG_SZ, initDir, MAX_PATH))
                    *initDir = 0;

                if (Configuration.ConfigVersion < 44) // prevod pripon na lowercase
                {
                    char masksAux[MAX_PATH];
                    lstrcpyn(masksAux, masks, MAX_PATH);
                    StrICpy(masks, masksAux);
                }
                CEditorMasksItem* item = new CEditorMasksItem(masks, command, arguments, initDir);
                if (item != NULL && item->IsGood())
                {
                    editorMasks->Add(item);
                    if (!editorMasks->IsGood())
                    {
                        delete item;
                        editorMasks->ResetState();
                        break;
                    }
                }
                else
                {
                    if (item != NULL)
                        delete item;
                    TRACE_E(LOW_MEMORY);
                    break;
                }
            }
            else
                break;
            itoa(++i, buf, 10);
            CloseKey(subKey);
        }
        CloseKey(editorKey);
    }
    return TRUE;
}

// ****************************************************************************

BOOL SaveEditors(HKEY hKey, const char* name, CEditorMasks* editorMasks)
{
    HKEY editorKey;
    if (CreateKey(hKey, name, editorKey))
    {
        ClearKey(editorKey);
        HKEY subKey;
        char buf[30];
        int i;
        for (i = 0; i < editorMasks->Count; i++)
        {
            itoa(i + 1, buf, 10);
            if (CreateKey(editorKey, buf, subKey))
            {
                SetValue(subKey, EDITORS_MASKS_REG, REG_SZ, editorMasks->At(i)->Masks->GetMasksString(), -1);
                SetValue(subKey, EDITORS_COMMAND_REG, REG_SZ, editorMasks->At(i)->Command, -1);
                SetValue(subKey, EDITORS_ARGUMENTS_REG, REG_SZ, editorMasks->At(i)->Arguments, -1);
                SetValue(subKey, EDITORS_INITDIR_REG, REG_SZ, editorMasks->At(i)->InitDir, -1);
                CloseKey(subKey);
            }
            else
                break;
        }
        CloseKey(editorKey);
    }
    return TRUE;
}

