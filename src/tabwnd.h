// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//
// ****************************************************************************

class CTabWindow : public CWindow
{
public:
    CPanelWindow* PanelWindow;

    //  protected:
    //    TDirectArray<CTabItem> TabItems;

public:
    CTabWindow(CPanelWindow* filesWindow);
    ~CTabWindow();

    void DestroyWindow();
    int GetNeededHeight();

protected:
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};
