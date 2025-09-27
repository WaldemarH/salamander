// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_filesList_item.h"


TCHAR* Find_FileList_Item::Text_Get( const int index_column, TCHAR* text, int fileNameFormat )
{
//Return column text.
    switch ( index_column )
    {
    case 0:
    {
        AlterFileName(text, Name, -1, fileNameFormat, 0, IsDir);
        return text;
    }
    case 1:
    {
        return Path;
    }
    case 2:
    {
        if (IsDir)
            CopyMemory(text, DirColumnStr, DirColumnStrLen + 1);
        else
            NumberToStr(text, Size);
        break;
    }
    case 3:
    {
        SYSTEMTIME st;
        FILETIME ft;

        if (
            FileTimeToLocalFileTime(&LastWrite, &ft)
            &&
            FileTimeToSystemTime(&ft, &st)
        )
        {
            if (GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, text, 50) == 0)
            {
                sprintf(text, "%u.%u.%u", st.wDay, st.wMonth, st.wYear);
            }
        }
        else
        {
            strcpy(text, LoadStr(IDS_INVALID_DATEORTIME));
        }
        break;
    }
    case 4:
    {
        SYSTEMTIME st;
        FILETIME ft;
        if (
            FileTimeToLocalFileTime(&LastWrite, &ft)
            &&
            FileTimeToSystemTime(&ft, &st)
        )
        {
            if (GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, text, 50) == 0)
            {
                sprintf(text, "%u:%02u:%02u", st.wHour, st.wMinute, st.wSecond);
            }
        }
        else
            strcpy(text, LoadStr(IDS_INVALID_DATEORTIME));
        break;
    }
    default:
    {
        GetAttrsString( text, Attr );
        break;
    }
    }

    return text;
}

BOOL Find_FileList_Item::Text_Set( const TCHAR* path, const TCHAR* name, const CQuadWord& size, DWORD attr, const FILETIME* lastWrite, BOOL isDir )
{
    //CALL_STACK_MESSAGE5("Find_FileList_Item::Set(%s, %s, %g, 0x%X, )", path, name, size.GetDouble(), attr);
    CALL_STACK_MESSAGE_NONE

//Allocate memory.
    const auto  size_name = strlen( name );
    const auto  size_path = strlen( path );

    Name = (char*)malloc( size_name + 1 );
    Path = (char*)malloc( size_path + 1 );

    if (
        ( Name == NULL )
        ||
        ( Path == NULL )
    )
    {
        return FALSE;
    }

//Copy text.
    memmove( Name, name, size_name + 1 );
    memmove( Path, path, size_path + 1 );

    Size = size;

//Set flags.
    Attr = attr;
    LastWrite = *lastWrite;
    IsDir = isDir ? 1 : 0;

    return TRUE;
}
