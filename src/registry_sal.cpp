// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "registry.h"


//[W: following functions are here just for plugins and will be renamed in time.. or completely changed something something.. we'll see]
LONG SalRegQueryValue( HKEY hKey, LPCSTR lpSubKey, LPSTR lpData, PLONG lpcbData )
{
    DWORD dataBufSize = lpData == NULL || lpcbData == NULL ? 0 : *lpcbData;
    LONG ret = RegQueryValue(hKey, lpSubKey, lpData, lpcbData);
    if (lpcbData != NULL &&
        (ret == ERROR_MORE_DATA || lpData == NULL && ret == ERROR_SUCCESS))
    {
        (*lpcbData)++; // rekneme si radsi o pripadny null-terminator navic
    }
    if (ret == ERROR_SUCCESS && lpData != NULL)
    {
        if (*lpcbData < 1 || ((char*)lpData)[*lpcbData - 1] != 0)
        {
            if ((DWORD)*lpcbData < dataBufSize) // lezou sem jen hodnoty typu REG_SZ a REG_EXPAND_SZ, takze jeden null-terminator staci
            {
                ((char*)lpData)[*lpcbData] = 0;
                (*lpcbData)++;
            }
            else // nedostatek mista pro null-terminator v bufferu
            {
                (*lpcbData)++; // rekneme si o potrebny null-terminator
                return ERROR_MORE_DATA;
            }
        }
    }
    return ret;
}

LONG SalRegQueryValueEx( HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData )
{
    DWORD dataBufSize = lpData == NULL ? 0 : *lpcbData;
    DWORD type = REG_NONE;
    LONG ret = RegQueryValueEx(hKey, lpValueName, lpReserved, &type, lpData, lpcbData);

    if (lpType != NULL)
        *lpType = type;

    if (
        ( type == REG_SZ )
        ||
        ( type == REG_MULTI_SZ )
        ||
        ( type == REG_EXPAND_SZ )
    )
    {
        if (
            ( hKey != HKEY_PERFORMANCE_DATA )
            &&
            ( lpcbData != NULL )
            &&
            (
                ( ret == ERROR_MORE_DATA )
                ||
                (
                    ( lpData == NULL )
                    &&
                    ( ret == ERROR_SUCCESS )
                )
            )
        )
        {
            (*lpcbData) += type == REG_MULTI_SZ ? 2 : 1; // we'd rather talk about any extra null-terminator(s).
            return ret;
        }
        if (ret == ERROR_SUCCESS && lpData != NULL)
        {
            if (
                ( *lpcbData < 1 )
                ||
                ( ((char*)lpData)[*lpcbData - 1] != 0 )
            )
            {
                if (*lpcbData < dataBufSize)
                {
                    ((char*)lpData)[*lpcbData] = 0;
                    (*lpcbData)++;
                }
                else // lack of space for null-terminator in buffer
                {
                    (*lpcbData) += type == REG_MULTI_SZ ? 2 : 1; // let's talk about the necessary null-terminator(s)
                    return ERROR_MORE_DATA;
                }
            }
            if (
                ( type == REG_MULTI_SZ )
                &&
                ( *lpcbData < 2 || ((char*)lpData)[*lpcbData - 2] != 0 )
            )
            {
                if (*lpcbData < dataBufSize)
                {
                    ((char*)lpData)[*lpcbData] = 0;
                    (*lpcbData)++;
                }
                else // nedostatek mista pro druhy null-terminator v bufferu
                {
                    (*lpcbData)++; // rekneme si o potrebny null-terminator
                    return ERROR_MORE_DATA;
                }
            }
        }
    }
    return ret;
}
