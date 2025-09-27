// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_ignore.h"


FindIgnore::FindIgnore() : m_Items( 5, 5 )
{
//Initialize variables.
    Items_Reset();
}

BOOL FindIgnore::Load( FindIgnore* source )
{
//Free any old resources.
    m_Items.DestroyMembers();

//Insert items to list.
    BOOL    status = TRUE;

    for ( int i = 0; i < source->m_Items.Count; ++i )
    {
    //Get item.
        auto*   item = source->Items_At( i );

    //Append item.
        status = Items_Add( item->Enabled, item->Path );

        if ( !status )
        {
            break;
        }
    }

    return status;
}



BOOL FindIgnore::Prepare(FindIgnore* source)
{
    Items.DestroyMembers();
    int i;
    for (i = 0; i < source->Items.Count; i++)
    {
        Find_Ignore_Item* item = source->At(i);
        if (item->Enabled) // zajimaji nas pouze enabled polozky
        {
            const char* path = item->Path;
            while (*path == ' ')
                path++;
            CFindIgnoreItemType type = fiitRelative;
            if (path[0] == '\\' && path[1] != '\\')
                type = fiitRooted;
            else if ((path[0] == '\\' && path[1] == '\\') ||
                     LowerCase[path[0]] >= 'a' && LowerCase[path[0]] <= 'z' && path[1] == ':')
                type = fiitFull;

            char buff[3 * MAX_PATH];
            if (strlen(path) >= 2 * MAX_PATH)
            {
                TRACE_E("FindIgnore::Prepare() Path too long!");
                return FALSE;
            }
            if (type == fiitFull)
            {
                strcpy(buff, path);
            }
            else
            {
                if (path[0] == '\\')
                    strcpy(buff, path);
                else
                {
                    buff[0] = '\\';
                    strcpy(buff + 1, path);
                }
            }
            if (buff[strlen(buff) - 1] != '\\')
                strcat(buff, "\\");
            if (!Add(TRUE, buff))
                return FALSE;
            item = Items[Items.Count - 1];
            item->Type = type;
            item->Len = (int)strlen(buff);
        }
    }
    return TRUE;
}


BOOL FindIgnore::Contains(const char* path, int startPathLen)
{
    // plna cesta
    int i;
    for (i = 0; i < Items.Count; i++)
    {
        // startPathLen - delka cesty zadane ve Find okne (root hledani), ignoruji se jen jeho
        //                podcesty, viz https://forum.altap.cz/viewtopic.php?f=7&t=7434
        Find_Ignore_Item* item = Items[i];
        switch (item->Type)
        {
        case fiitFull:
        {
            if (item->Len > startPathLen && StrNICmp(path, item->Path, item->Len) == 0)
                return TRUE;
            break;
        }

        case fiitRooted:
        {
            const char* noRoot = SkipRoot(path);
            if ((noRoot - path) + item->Len > startPathLen && StrNICmp(noRoot, item->Path, item->Len) == 0)
                return TRUE;
            break;
        }

        case fiitRelative:
        {
            const char* m = path;
            while (m != NULL)
            {
                m = StrIStr(m, item->Path);
                if (m != NULL) // nalezeno
                {
                    if ((m - path) + item->Len > startPathLen) // je to podcesta = ignorovat
                        return TRUE;
                    m++; // jdeme hledat dalsi vyskyt, treba uz bude v podceste
                }
            }
            break;
        }
        }
    }
    return FALSE;
}


BOOL FindIgnore::AddUnique(BOOL enabled, const char* path)
{
    int len = (int)strlen(path);
    if (len < 1)
        return FALSE;
    if (path[len - 1] == '\\') // budeme porovnavat bez koncovych lomitek
        len--;
    int i;
    for (i = 0; i < Items.Count; i++)
    {
        Find_Ignore_Item* item = Items[i];
        int itemLen = (int)strlen(item->Path);
        if (itemLen < 1)
            continue;
        if (item->Path[itemLen - 1] == '\\') // budeme porovnavat bez koncovych lomitek
            itemLen--;
        if (len != itemLen)
            continue;
        if (StrNICmp(path, item->Path, len) == 0)
        {
            item->Enabled = TRUE; // v kazdem pripade polozku povolime
            return TRUE;
        }
    }
    // nenasli jsme -- pridame
    return Add(enabled, path);
}
