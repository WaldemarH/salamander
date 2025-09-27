#pragma once

//Includes
#include "common/array.h"

//Class
class Menu_Queue
{
    private: struct State
    {
        public: enum Value
        {
            test        = -1,

            no          = 0,
            yes         = 1
        };
    };

//Constructor/Destructor
    public: Menu_Queue();
    public: ~Menu_Queue();

//Menus
    public: bool Menu_Add( HWND hWindow );
    public: void Menu_Remove( HWND hWindow );

    public: void Menus_CloseAll_Blocking();         //Can deadloop (be careful).
    public: void Menus_CloseAll_NonBlocking();

    public: void Menus_PostToAll( UINT uMsg, WPARAM wParam, LPARAM lParam );
    public: void Menus_SendToAll( UINT uMsg, WPARAM wParam, LPARAM lParam );

    private: TDirectArray<HWND>     m_Menus;
    private: CRITICAL_SECTION       m_Menus_CriticalSection;
};
extern Menu_Queue MenuQueue;

