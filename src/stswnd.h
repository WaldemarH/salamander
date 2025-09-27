// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//
// ****************************************************************************

class CMainToolBar;

enum CBorderLines
{
    blNone = 0x00,
    blTop = 0x01,
    blBottom = 0x02
};

enum CSecurityIconState
{
    sisNone = 0x00,      // icon not displayed
    sisUnsecured = 0x01, // unlocked padlock icon displayed
    sisSecured = 0x02    // locked padlock icon displayed
};

/*
enum
{
  otStatusWindow = otLastWinLibObject
};
*/

//
// CHotTrackItem
//
// polozka obsahuje index prvniho znaku, pocet znaku, offset prvniho znaku v bodech
// a jejich delku v bodech k zobrazene ceste se vytvori seznam techto polozek a drzi
// se v poli
//
// pro cestu "\\john\c\winnt
//
// se vytvori tyto polozky:
//
// (0, 9,  0, delka prvnich deviti znaku)   = \\john\c\
// (0, 14, 0, delka 14 znaku)              = \\john\c\winnt
//
// pro "DIR: 12"
//
// (0, 3, 0, delka tri znaku DIR)
// (5, 2, bodovy offset "12", delka dvou znaku "12")

struct CHotTrackItem
{
    WORD Offset;       // offset prvniho znaku ve znacich
    WORD Chars;        // pocet znaku
    WORD PixelsOffset; // offset prvniho znaku v bodech
    WORD Pixels;       // jejich delka v bodech
};

class CStatusWindow : public CWindow        //Status window is shown on top (history directory list) and at the bottom as detail information.
{
    protected: struct RectEx : public RECT
    {
    //Functions
        public: RectEx& operator=( const RECT& from )
        {
        //Copy operator.
            left = from.left;
            right = from.right;
            top = from.top;
            bottom = from.bottom;

            return *this;
        };

        public: inline POINT Center() const
        {
        //Get the center of the RECT.
            return { .x = left + ( right - left )/2, .y = bottom + ( top - bottom )/2 };
        }

        public: inline bool IsMouseInside( const LONG xPosition, const LONG yPosition ) const
        {
        //Is mouse position inside the rect?
            return (
                ( xPosition >= left )
                &&
                ( xPosition < right )
                &&
                ( yPosition >= top )
                &&
                ( yPosition < bottom )
            );
        };
        public: inline bool IsMouseInside( const POINT position ) const
        {
        //Forward call.
            return IsMouseInside( position.x, position.y );
        };
    };

public:
    CMainToolBar* ToolBar;
    CPanelWindow* PanelWindow;

protected:
    TDirectArray<CHotTrackItem> HotTrackItems;
    BOOL HotTrackItemsMeasured;

    int Border; // dividing line up/down
    char* Text;
    int TextLen; // the number of characters on the 'Text' pointer without the terminator
    char* DiskSpaceAvailable;
    int PathLen;          // -1 (the path is all Text), otherwise the length of the path in Text (the rest is a filter)
    BOOL History;         // display sip between text and size? //zobrazovat sipku mezi textem a size?
    BOOL Hidden;          // show filter symbol?
    int HiddenFilesCount; // how many files are filtered out
    int HiddenDirsCount;  // to address
    BOOL WholeTextVisible;

    BOOL ShowThrobber;             // TRUE pokud se ma zobrazovat 'progress' throbber za textem/hidden filtrem (nezalezi na existenci okna)
    BOOL DelayedThrobber;          // TRUE pokud uz bezi timer pro zobrazeni throbbera
    DWORD DelayedThrobberShowTime; // kolik bude GetTickCount() v okamziku, kdy se ma zobrazit zpozdeny throbber (0 = nezobrazujeme se zpozdenim)
    BOOL Throbber;                 // zobrazovat 'progress' throbber za textem/hidden filtrem? (TRUE jen pokud existuje okno)
    int ThrobberFrameIndex = 0;     // index aktualniho policka animace
    char* ThrobberTooltip;         // pokud je NULL, nebude zobrazen
    int ThrobberID;                // identifikacni cislo throbbera (-1 = neplatne)

    CSecurityIconState Security;
    char* SecurityTooltip; // pokud je NULL, nebude zobrazen

    int Allocated;
    int* AlpDX; // array of lengths (from zero to the Xth character in the string)
    BOOL Left;

    int ToolBarWidth; // aktualni sirka toolbary

    int EllipsedChars; // pocet vypustenych znaku za rootem; jinak -1
    int EllipsedWidth; // delka vypusteho retezce za rootem; jinak -1

    bool m_Hot_Highlight_Filtered = false;            //Set when the item is highlighted.
    bool m_Hot_Highlight_History = false;
    bool m_Hot_Highlight_Security = false;
    bool m_Hot_Highlight_Size = false;
    bool m_Hot_Highlight_Throbber = false;
    bool m_Hot_Highlight_Zoom = false;
    CHotTrackItem* m_Hot_Item = nullptr;            // selected item
    CHotTrackItem* m_Hot_Item_Last = nullptr;       // last posted position

    RectEx m_Rect_Filtered;                           //Location of the icons/text.
    RectEx m_Rect_History;
    RectEx m_Rect_Security;
    RectEx m_Rect_Size;
    RectEx m_Rect_Text;
    RectEx m_Rect_Throbber;
    RectEx m_Rect_Zoom;
    RectEx m_Rect_Whole;

    int MaxTextRight;
    BOOL MouseCaptured;
    BOOL RButtonDown;
    BOOL LButtonDown;
    BOOL MButtonDown;
    POINT LButtonDownPoint; // kde user stisknul LButton

    int Height;
    int Width; // rozmery

    BOOL NeedToInvalidate; // pro SetAutomatic() - nastala zmena, musime prekreslovat?

    DWORD* SubTexts;     // pole DWORDU: LOWORD pozice, HIWORD delka
    DWORD SubTextsCount; // pocet polozek v poli SubTexts

    IDropTarget* IDropTargetPtr;

public:
    CStatusWindow(CPanelWindow* pPanelWindow, int border, CObjectOrigin origin = ooAllocated);
    ~CStatusWindow();

    BOOL SetSubTexts(DWORD* subTexts, DWORD subTextsCount);
    // nastavuje text 'text' do status-line, 'pathLen' urcuje delku cesty (zbytek je filter),
    // pokud se 'pathLen' nepouziva (cesta je kompletni 'text') je rovno -1
    BOOL SetText(const char* text, int pathLen = -1);

    // sestavi pole HotTrackItems: pro disky a rchivatory na zaklade zpetnych lomitek
    // a pro FS se dopta pluginu
    void BuildHotTrackItems();

    void GetHotText(char* buffer, int bufSize);

    void DestroyWindow();

    int GetToolBarWidth() { return ToolBarWidth; }

    int GetNeededHeight();
    void SetSize(const CQuadWord& size);
    void SetHidden(int hiddenFiles, int hiddenDirs);
    void SetHistory(BOOL history);
    void SetThrobber(BOOL show, int delay = 0, BOOL calledFromDestroyWindow = FALSE); // volat pouze z hlavniho (GUI) threadu, stejne jako ostatni metody objektu
    // nastavi text, ktery se bude zobrazovat jako tooltip po najeti mysi na throbber, objekt si udela kopii
    // pokud je NULL, nebude tooltip zobrazen
    void SetThrobberTooltip(const char* throbberTooltip);
    int ChangeThrobberID(); // zmeni ThrobberID a vrati jeho novou hodnotu
    BOOL IsThrobberVisible(int throbberID) { return ShowThrobber && ThrobberID == throbberID; }
    void HideThrobberAndSecurityIcon();

    void SetSecurity(CSecurityIconState iconState);
    void SetSecurityTooltip(const char* tooltip);

    void InvalidateIfNeeded();

    void LayoutWindow();
    void Paint(HDC hdc, BOOL highlightText = FALSE, BOOL highlightHotTrackOnly = FALSE);
    void Repaint(BOOL flashText = FALSE, BOOL hotTrackOnly = FALSE);
    void InvalidateAndUpdate(BOOL update); // lze o volat i pro HWindow == NULL
    void FlashText(BOOL hotTrackOnly = FALSE);

    BOOL FindHotTrackItem(int xPos, int& index);

    void SetLeftPanel(BOOL left);
    BOOL ToggleToolBar();

    BOOL IsLeft() { return Left; }

    BOOL SetDriveIcon(HICON hIcon);     // ikona se okopiruje do imagelistu - destrukci musi zajistit volajici kod
    void SetDrivePressed(BOOL pressed); // zamackne drive ikonku

    BOOL GetRect(RECT* r);              // returns window's rect in screen coordinates
    BOOL GetRect_TextFrame(RECT* r);    // returns a rectangle around the text in screen coordinates
    BOOL GetRect_FilterFrame(RECT* r);  // returns a rectangle around the filter symbol in screen coordinates

    // mohlo dojit ke zmene barevne hloubky obrazovky; je treba prebuildit CacheBitmap
    void OnColorsChanged();

    void SetFont();

protected:
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void RegisterDragDrop();
    void RevokeDragDrop();

    // vytvori imagelist s jednim prvkem, ktery bude pouzit pro zobrazovani prubehu tazeni
    // po ukonceni tazeni je treba tento imagelist uvolnit
    // vstupem je bod, ke kteremu se napocitaji offsety dxHotspot a dyHotspot
    HIMAGELIST CreateDragImage(const char* text, int& dxHotspot, int& dyHotspot, int& imgWidth, int& imgHeight);
};
