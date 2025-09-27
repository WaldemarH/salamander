// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

//Includes
    #include "find_filesList_item.h"

//Class
    class Find_Dialog;

    class Find_FilesList_View : public CWindow
    {
    //Constructor/Destructor
        public: Find_FilesList_View( HWND dlg, int ctrlID, Find_Dialog& findDialog );
        public: ~Find_FilesList_View();

        protected: CRITICAL_SECTION                         m_CriticalSection;
        protected: Find_Dialog&                             m_FindDialog;

    //Items
        public: int Items_Add( Find_FileList_Item* item );
        public: Find_FileList_Item* Items_At( int index );
        public: int Items_Count();
        public: void Items_Delete( int index );
        public: void Items_DestroyMembers();

        protected: TIndirectArray<Find_FileList_Item>       m_Items;
        protected: TIndirectArray<Find_FileList_Item>       m_Items_ForRefine;

    //Window
        protected: virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam ) override;



    //[TODO: unorganized... get the meaning of the members]
    protected:

    public:
        int EnumFileNamesSourceUID;             // Resource UID for enumeration of names in viewers
    
    public:
    
    
        BOOL InitColumns();
    
        void StoreItemsState();
        void RestoreItemsState();
    
        int CompareFunc(Find_FileList_Item* f1, Find_FileList_Item* f2, int sortBy);
        void QuickSort(int left, int right, int sortBy);
        void SortItems(int sortBy);
    
        void QuickSortDuplicates(int left, int right, BOOL byName);
        int CompareDuplicatesFunc(Find_FileList_Item* f1, Find_FileList_Item* f2, BOOL byName);
        void SetDifferentByGroup(); // nastavi bit Different podle Group tak, aby se bit Different stridal na rozhrani skupin
    
        // interface pro Data
        BOOL IsGood();
        void ResetState();
    
        // ze seznamu Data pretaha potrebne casti do seznamu DataForRefine
        // smi byt volano pouze pokud nebezi hledaci thread
        BOOL TakeDataForRefine();
        void DestroyDataForRefine();
        int GetDataForRefineCount();
        Find_FileList_Item* GetDataForRefine(int index);
    
        DWORD GetSelectedListSize();                     // vrati pocet bajtu, ktery bude potreba pro ulozeni vsech vybranych
                                                         // polozek ve tvaru "c:\\bla\\bla.txt\\0c:\\bla\\bla2.txt\0\0"
                                                         // pokud neni vybrana zadna polozka, vraci 2 (dva terminatory)
        BOOL GetSelectedList(char* list, DWORD maxSize); // naplni list seznamem dle GetSelectedListSize
                                                         // neprekroci maxSize
    
        // probehne vsechny selected soubory a adresare a pokud uz neexistuji, vyradi je
        // je-li nastavena promenna 'forceRemove', budou vyrazeny vybrane polozky bez kontroly
        // 'lastFocusedIndex' rika index, ktery byl 'focused' nez doslo k "mazani"
        // 'lastFocusedItem' ukazuje na kopii dat 'focused' polozky, abychom se ji mohli pokusit dohledat polde nazvu a cesty
        void CheckAndRemoveSelectedItems(BOOL forceRemove, int lastFocusedIndex, const Find_FileList_Item* lastFocusedItem);
    };
