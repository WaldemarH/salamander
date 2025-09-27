// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

//Includes
    #include "find_data_grep.h"
    #include "find_data_search.h"
    #include "find_log.h"
    #include "find_options.h"
    #include "find_string_search.h"

//Class
    class CComboboxEdit;
    class CButton;
    class CFindTBHeader;
    class CMenuBar;
    class Find_FilesList_View;

    class Find_Dialog : public CCommonDialog
    {
        friend class Find_FilesList_View;

        protected: struct Clipboard_CopyMode
        {
            public: enum Value
            {
                name,
                name_full,
                name_unc,
                path_full,
            };
        };
        public: struct StateOfFindCloseQuery
        {
            public: enum Value
            {
                notUsed,

                canClose,       //Can dialog be closed?
                cannotClose,    //Dialog can not be closed.
                sentToFind,     //Request send to find (waiting for the answer) and/or user didn't respond to "Stop searching?".
            };
        };

    //Constructor/Destructor
        Find_Dialog( HWND hCenterAgainst, const char* initPath );
        ~Find_Dialog();

    //Clipboard
        protected: void Clipboard_CopyTo( const Clipboard_CopyMode::Value mode );


    //-----------------------------------------------------------------------------------------------------
    //OLD

    protected:
        // data potrebna pro layoutovani dialogu
        BOOL FirstWMSize = TRUE;
        int VMargin = 0; // prostor vlevo a vpravo mezi rameckem dialogu a controly
        int HMargin = 0; // prostor dole mezi tlacitky a status barou
        int ButtonW = 0; // rozmery tlacitka
        int ButtonH = 0;
        int RegExpButtonW = 0; // rozmery tlacitka RegExpBrowse
        int RegExpButtonY = 0; // umisteni tlacitka RegExpBrowse
        int MenuBarHeight = 0; // vyska menu bary
        int StatusHeight = 0;  // vyska status bary
        int ResultsY = 0;      // umisteni seznamu vysledku
        int AdvancedY = 0;     // umisteni tlacitka Advanced
        int AdvancedTextY = 0; // umisteni textu za tlacitkem Advanced
        int AdvancedTextX = 0; // umisteni textu za tlacitkem Advanced
        int FindTextY = 0;     // Umisteni headru nad vysledkama
        int FindTextH = 0;     // vyska headru
        int CombosX = 0;       // umisteni comboboxu
        int CombosH = 0;       // vyska comboboxu
        int BrowseY = 0;       // umisteni tlacitka Browse
        int Line2X = 0;        // umisteni oddelovaci cary u Search file content
        int FindNowY = 0;      // umisteni tlacitka Find now
        int SpacerH = 0;       // vyska o kterou budeme zkracovat/prodluzovat dialog

        BOOL Expanded = TRUE; // dialog je roztazeny - jsou zobrazeny polozky SearchFileContent

        int MinDlgW = 0; // minimalni rozmery Find dialogu
        int MinDlgH = 0;

        int FileNameFormat; // jak upravit filename po nacteni z disku, prebirame z globalni konfigurace, kvuli problemum se synchronizaci
        BOOL SkipCharacter = FALSE; // zabrani pipnuti pri Alt+Enter ve findu

        // dalsi data
        BOOL DlgFailed = FALSE;
        CMenuPopup* MainMenu = NULL;
        CMenuBar* MenuBar = NULL;
        HWND HStatusBar = NULL;
        HWND HProgressBar = NULL; // child okno status bary, zobrazene pro nektere operace ve zvlastnim policku
        BOOL TwoParts = FALSE;     // ma status bar dva texty?
                                 //    CFindAdvancedDialog FindAdvanced;
        Find_FilesList_View* FoundFilesListView = NULL;
        char FoundFilesDataTextBuffer[MAX_PATH] = {0}; // pro ziskavani textu z Find_FileList_Item::GetText
        CFindTBHeader* TBHeader = NULL;
        BOOL SearchInProgress = FALSE;
        BOOL CanClose = TRUE;; // je mozne zavrit okno (nejsme v metode tohoto objektu)
        HANDLE GrepThread = NULL;
        Find_Data_Grep GrepData;
        Find_String_Search SearchingText;
        Find_String_Search SearchingText2;
        CComboboxEdit* EditLine;
        BOOL UpdateStatusBar = FALSE;
        IContextMenu2* ContextMenu = NULL;
        Find_Dialog** ZeroOnDestroy = NULL; // hodnota, ukazatele bude pri destrukci nulovana
        CButton* OKButton = NULL;

        BOOL OleInitialized = FALSE;

        TIndirectArray<Find_Data_Search> SearchForData; // seznam adresaru a masek, ktere budou prohledany

        // jedna polozka, nad kterou pracuji Find dialog a Advanced dialog
        Find_Options_Item Data;

        BOOL ProcessingEscape = FALSE; // message loop prave maka na ESCAPE -- pokud bude nagenerovano
                               // IDCANCEL, zobrazime konfirmaci

        Find_Log Log; // uloziste chyb a informaci

        CBitmap* CacheBitmap = NULL; // pro vykreslovani cesty

        BOOL FlashIconsOnActivation = FALSE; // az nas aktivuji, nechame zablikat stavove ikonky

        char FindNowText[100] = { 0 };

    public:
        StateOfFindCloseQuery::Value m_StateOfFindCloseQuery = StateOfFindCloseQuery::notUsed;        //The main thread checks in the Find thread whether it can close the window, unsynchronized, only used during shutdown, enough...

    public:


        virtual void Validate(CTransferInfo& ti);
        virtual void Transfer(CTransferInfo& ti);

        BOOL IsGood() { return EditLine != NULL; }

        void SetZeroOnDestroy(Find_Dialog** zeroOnDestroy) { ZeroOnDestroy = zeroOnDestroy; }

        BOOL GetFocusedFile(char* buffer, int bufferLen, int* viewedIndex /* muze byt NULL */);
        const char* GetName(int index);
        const char* GetPath(int index);
        void UpdateInternalViewerData();

        BOOL IsSearchInProgress() { return SearchInProgress; }

        void OnEnterIdle();

        // If the message is translated, the return value is TRUE.
        BOOL IsMenuBarMessage(CONST MSG* lpMsg);

        // pro vybrane polozky alokuje prislusna data
        HGLOBAL CreateHDrop();
        HGLOBAL CreateShellIdList();

        // hlavni okno vola tuto metodu vsem Find dialogum - doslo ke zmene barev
        void OnColorsChange();

        void SetProcessingEscape(BOOL value) { ProcessingEscape = value; }

        // umoznuje zpracovat Alt+C a dalsi horke klavesy, ktere patri prave
        // schovanym controlum: pokud je dialog zabalen a jedna se o horkou
        // klavesu skrytych controlu, rozbali ho
        // vzdy vraci FALSE
        BOOL ManageHiddenShortcuts(const MSG* msg);

    protected:
        void GetLayoutParams();
        void LayoutControls(); // rozlozi prvky do plochy okna

        void SetTwoStatusParts(BOOL two, BOOL force = FALSE); // nastavi jednu nebo dve casti pro status bar; rozmery nastavuje podle delky status bar

        void SetContentVisible(BOOL visible);
        void UpdateAdvancedText();

        void LoadControls(int index); // z pole Items vytahne prislusnou polozku a nacpe ji do dialogu

        void StartSearch(WORD command);
        void StopSearch();

        void BuildSerchForData(); // naplni seznam SearchForData

        void EnableControls(BOOL nextIsButton = FALSE);
        void EnableToolBar();

        void InsertDrives(HWND hEdit, BOOL network); // naleje do hEdit seznam fixed disku (pripadne i network disku)

        void UpdateListViewItems();

        void OnContextMenu(int x, int y);
        void OnFocusFile();
        void OnViewFile(BOOL alternate);
        void OnEditFile();
        void OnViewFileWith();
        void OnEditFileWith();
        void OnHideSelection();
        void OnHideDuplicateNames();
        void OnDelete(BOOL toRecycle);
        void OnSelectAll();
        void OnInvertSelection();
        void OnShowLog();
        void OnOpen(BOOL onlyFocused);
        void UpdateStatusText(); // pokud nejsme v search modu, informuje o poctu a velikosti vybranych polozek

        // OLE clipboard operations

        // pro vybrane polozky vytvori context menu a vola ContextMenuInvoke pro specifikovany lpVerb
        // vraci TRUE, pokud byla zavolana Invoke, jinak pokud neco selze vraci FALSE
        BOOL InvokeContextMenu(const char* lpVerb);

        void OnCutOrCopy(BOOL cut);
        void OnDrag(BOOL rightMouseButton);

        void OnProperties();

        void OnUserMenu();


        virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

        // obehne vybrane polozky v listview a pokusi se najit spolecny nadadresar
        // pokud ho najde, nakopci hodo bufferu a vrati TRUE
        // pokud ho nenajde nebo je buffer maly, vrati FALSE
        BOOL GetCommonPrefixPath(char* buffer, int bufferMax, int& commonPrefixChars);

        BOOL InitializeOle();
        void UninitializeOle();

        BOOL CanCloseWindow();

        BOOL DoYouWantToStopSearching();

        void SetFullRowSelect(BOOL fullRow);
    };
