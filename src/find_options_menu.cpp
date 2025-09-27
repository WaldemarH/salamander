// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_options.h"
#include "menu.h"


void Find_Options::Menu_Set( CMenuPopup& popup, BOOL enabled, int originalCount )
{
    int count = popup.GetItemCount();
    if (count > originalCount)
    {
        // sestrelim drive nabouchane polozky
        popup.RemoveItemsRange(originalCount, count - 1);
    }

    if ( m_Items.Count > 0)
    {
        MENU_ITEM_INFO mii;

        // pokud mame co pridavat, pridam separator
        mii.Mask = MENU_MASK_TYPE;
        mii.Type = MENU_TYPE_SEPARATOR;
        popup.InsertItem(-1, TRUE, &mii);

        // a pripojim zobrazovanou cast polozek
        int maxCount = CM_FIND_OPTIONS_LAST - CM_FIND_OPTIONS_FIRST;
        int i;
        for (i = 0; i < min( m_Items.Count, maxCount); i++)
        {
            mii.Mask = MENU_MASK_TYPE | MENU_MASK_STATE | MENU_MASK_STRING | MENU_MASK_ID;
            mii.Type = MENU_TYPE_STRING;
            mii.State = enabled ? 0 : MENU_STATE_GRAYED;
            if ( m_Items[i]->AutoLoad)
                mii.State |= MENU_STATE_DEFAULT;
            mii.ID = CM_FIND_OPTIONS_FIRST + i;
            mii.String = m_Items[i]->ItemName;
            popup.InsertItem(-1, TRUE, &mii);
        }
    }
}
