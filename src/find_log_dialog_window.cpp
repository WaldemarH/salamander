// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "find_log_dialog.h"


INT_PTR Find_Log_Dialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CALL_STACK_MESSAGE4( "Find_Log_Dialog::DialogProc(0x%X, 0x%IX, 0x%IX)", uMsg, wParam, lParam );

//Handle message.
    switch ( uMsg )
    {
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_FINDLOG_FOCUS:
        {
            OnFocusFile();
            return 0;
        }
        case IDC_FINDLOG_IGNORE:
        {
            OnIgnore();
            return 0;
        }
        }
        break;
    }
    case WM_NOTIFY:
    {
        if (wParam == IDC_FINDLOG_LIST)
        {
            switch (((LPNMHDR)lParam)->code)
            {
            case LVN_ITEMCHANGED:
            {
                EnableControls();
                return 0;
            }
            }
        }
        break;
    }
    }

//Let base class handle the message.
    return CCommonDialog::DialogProc( uMsg, wParam, lParam );
}
