// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

//Includes
    #include "find_log.h"

//Class
    class Find_Log_Dialog : public CCommonDialog
    {
    //Constructo/Destructor
        public: Find_Log_Dialog( HWND hParent, Find_Log* log );

        protected: HWND         HListView = NULL;
        protected: Find_Log*    Log = nullptr;

    //Window
        protected: virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;


    //TODO
        public: virtual void Transfer(CTransferInfo& ti);
        protected: void OnFocusFile();
        protected: void OnIgnore();
        protected: void EnableControls();
        protected: const Find_Log_Item* GetSelectedItem();
    };
