// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"

#include "toolbar.h"
#include "cfgdlg.h"
#include "mainwnd.h"
#include "menu.h"
#include "menu_queue.h"
#include "stswnd.h"
#include "plugins.h"
#include "fileswnd.h"
#include "shellib.h"
#include "svg.h"

//
// ****************************************************************************
// CStatusWindow
//

CStatusWindow::CStatusWindow(CPanelWindow* pPanelWindow, int border, CObjectOrigin origin) : CWindow(origin), HotTrackItems(10, 5)
{
    CALL_STACK_MESSAGE_NONE
    Text = NULL;
    AlpDX = NULL;
    Allocated = 0;
    PathLen = -1;
    TextLen = 0;
    Border = border;
    DiskSpaceAvailable = NULL;
    Hidden = FALSE;
    History = FALSE;
    ShowThrobber = FALSE;
    DelayedThrobber = FALSE;
    DelayedThrobberShowTime = 0;
    Throbber = FALSE;
    ThrobberTooltip = NULL;
    ThrobberID = -1;
    Security = sisNone;
    SecurityTooltip = NULL;
    HiddenFilesCount = 0;
    HiddenDirsCount = 0;
    Left = TRUE; // dummy
    ToolBar = NULL;
    ToolBarWidth = 0;
    EllipsedChars = -1;
    EllipsedWidth = -1;
    NeedToInvalidate = FALSE;
    Width = 0;
    Height = 0;
    MouseCaptured = FALSE;
    PanelWindow = pPanelWindow;
    LButtonDown = FALSE;
    MButtonDown = FALSE;
    RButtonDown = FALSE;
    SubTexts = NULL;
    SubTextsCount = 0;
    IDropTargetPtr = NULL;
}

CStatusWindow::~CStatusWindow()
{
    CALL_STACK_MESSAGE1("CStatusWindow::~CStatusWindow()");
    if (SubTexts != NULL)
        free(SubTexts);
    if (Text != NULL)
        free(Text);
    if (AlpDX != NULL)
        free(AlpDX);
    if (DiskSpaceAvailable != NULL)
        free(DiskSpaceAvailable);
    if (ThrobberTooltip != NULL)
        free(ThrobberTooltip);
    if (SecurityTooltip != NULL)
        free(SecurityTooltip);
    if (ToolBar != NULL)
    {
        if (ToolBar->HWindow != NULL)
            ToolBar->DetachWindow();
        delete ToolBar;
    }
}

BOOL CStatusWindow::SetSubTexts(DWORD* subTexts, DWORD subTextsCount)
{
    CALL_STACK_MESSAGE2("CStatusWindow::SetSubTexts(, %u)", subTextsCount);
    m_Hot_Item = nullptr;
    m_Hot_Item_Last = nullptr;
    if (SubTexts != NULL)
    {
        SubTextsCount = 0;
        free(SubTexts);
    }

    if (subTexts == NULL || subTextsCount == 0)
        return TRUE;

    SubTexts = (DWORD*)malloc(subTextsCount * sizeof(DWORD));
    if (SubTexts == NULL)
    {
        TRACE_E(LOW_MEMORY);
        return FALSE;
    }
    memmove(SubTexts, subTexts, subTextsCount * sizeof(DWORD));
    SubTextsCount = subTextsCount;

    // nechame sestavit pole pro sledovani kurzoru
    BuildHotTrackItems();

    return TRUE;
}

BOOL CStatusWindow::SetText(const char* txt, int pathLen)
{
    CALL_STACK_MESSAGE3("CStatusWindow::SetText(%s, %d)", txt, pathLen);
    if (Text != NULL && strcmp(Text, txt) == 0)
    {
        PathLen = pathLen;
        return TRUE;
    }
    HotTrackItemsMeasured = FALSE;
    m_Hot_Item = nullptr;
    m_Hot_Item_Last = nullptr;

    int l = (int)strlen(txt) + 1;
    if (Allocated < l)
    {
        char* newText = (char*)realloc(Text, l);
        int* newAlpDX = (int*)realloc(AlpDX, l * sizeof(int));
        if (newText == NULL || newAlpDX == NULL)
        {
            TRACE_E(LOW_MEMORY);
            return FALSE;
        }
        Text = newText;
        AlpDX = newAlpDX;
        Allocated = l;
    }
    memmove(Text, txt, l);
    PathLen = pathLen;
    TextLen = l - 1;

    if (SubTexts != NULL)
    {
        SubTextsCount = 0;
        free(SubTexts);
        SubTexts = NULL;
    }

    // nechame sestavit pole pro sledovani kurzoru
    BuildHotTrackItems();

    if (MouseCaptured)
        WindowProc(WM_MOUSELEAVE, 0, 0);

    if (HWindow != NULL)
        InvalidateRect(HWindow, NULL, FALSE);
    return TRUE;
}

void CStatusWindow::BuildHotTrackItems()
{
    CALL_STACK_MESSAGE1("CStatusWindow::BuildHotTrackItems()");

    HDC dc = HANDLES(GetDC(HWindow));
    HFONT oldFont = (HFONT)SelectObject(dc, EnvFont);

    m_Hot_Item = nullptr;
    m_Hot_Item_Last = nullptr;
    if (Border == blTop)
    {
        //Fill HotTrackItems.
        CHotTrackItem item;
        HotTrackItems.DestroyMembers();

        if (Text != NULL)
        {
            // zde nam to padalo v SS2.0:execution address = 0x7800D9B0
            // doslo k zavolani strlen v pripade, ze Text byl jeste NULL
            int pathLen = (PathLen != -1) ? PathLen : (int)strlen(Text);

            // we get the positions of all characters
            SIZE s;
            GetTextExtentExPoint(dc, Text, TextLen, 0, NULL, AlpDX, &s);

            if (PanelWindow->Is(ptDisk) || PanelWindow->Is(ptZIPArchive))
            {
                int chars;
                if (
                    ( Text[0] == '\\' )
                    &&
                    ( Text[1] == '\\' )
                    &&
                    (
                        ( Text[2] != '.' )
                        ||
                        ( Text[3] != '\\' )
                        ||
                        ( Text[4] == 0 )
                        ||
                        ( Text[5] != ':' )
                    )
                    &&
                    Plugins.GetFirstNethoodPluginFSName()
                )
                {
                    chars = 2;
                }
                else
                {
                    char rootPath[MAX_PATH];
                    GetRootPath(rootPath, Text);
                    chars = (int)strlen(rootPath);

                    // u UNC upicnu posledni zpetne lomitko
                    BOOL isDotDriveFormat =
                        ( Text[0] == '\\' )
                        &&
                        ( Text[1] == '\\' )
                        &&
                        ( Text[2] == '.' )
                        &&
                        ( Text[3] == '\\' )
                        &&
                        ( Text[4] != 0 )
                        &&
                        ( Text[5] == ':' );

                    if (
                        ( chars > pathLen )
                        ||
                        (
                            ( !isDotDriveFormat )
                            &&
                            ( chars > 3 )
                        )
                    )
                    {
                        chars--;
                    }
                }

                BOOL exit;
                do
                {
                    item.Offset = 0;
                    item.PixelsOffset = 0;
                    item.Chars = chars;
                    item.Pixels = chars != 0 ? (WORD)AlpDX[chars - 1] : 0;
                    HotTrackItems.Add(item);

                    if (Text[chars] == '\\')
                        chars++;

                    exit = TRUE;
                    while (chars < pathLen)
                    {
                        exit = FALSE;
                        if (Text[chars] == '\\')
                            break;
                        chars++;
                    }
                } while (!exit);
            }
            else
            {
                if (PanelWindow->Is(ptPluginFS))
                {
                    int chars = 0;
                    while (1)
                    {
                        int lastChars = chars;
                        if (!PanelWindow->GetPluginFS()->GetNextDirectoryLineHotPath(Text, pathLen, chars))
                        {
                            chars = pathLen;
                        }
                        if (chars == lastChars)
                            chars++; // to by byla nekonecna smycka, radeji osetrime...
                        if (chars > pathLen)
                            chars = pathLen;

                        item.Offset = 0;
                        item.PixelsOffset = 0;
                        item.Chars = chars;
                        item.Pixels = chars != 0 ? (WORD)AlpDX[chars - 1] : 0;
                        HotTrackItems.Add(item);

                        if (chars == pathLen)
                            break;
                    }
                }
            }
            HotTrackItemsMeasured = TRUE;
        }
    }
    if (Border == blBottom)
    {
        // naplnim HotTrackItems
        CHotTrackItem item;
        HotTrackItems.DestroyMembers();
        if (Text != NULL)
        {
            // ziskame pozice vsech znaku
            SIZE s;
            GetTextExtentExPoint(dc, Text, TextLen, 0, NULL, AlpDX, &s);

            DWORD len = TextLen;
            SIZE sOffset;
            SIZE sSub;
            DWORD i;
            for (i = 0; i < (DWORD)SubTextsCount; i++)
            {
                WORD charOffset = LOWORD(SubTexts[i]);
                WORD charLen = HIWORD(SubTexts[i]);
                if (charOffset + charLen > (WORD)len)
                {
                    TRACE_E("charOffset + charLen >= len");
                    continue;
                }
                GetTextExtentPoint32(dc, Text, charOffset, &sOffset);
                GetTextExtentPoint32(dc, Text + charOffset, charLen, &sSub);
                item.PixelsOffset = (WORD)sOffset.cx;
                item.Pixels = (WORD)sSub.cx;
                item.Offset = charOffset;
                item.Chars = charLen;
                HotTrackItems.Add(item);
                HotTrackItemsMeasured = TRUE;
            }
        }
    }
    SelectObject(dc, oldFont);
    HANDLES(ReleaseDC(HWindow, dc));
}

void CStatusWindow::DestroyWindow()
{
    CALL_STACK_MESSAGE1("CStatusWindow::DestroyWindow()");
    if (ToolBar != NULL)
    {
        if (ToolBar->HWindow != NULL)
            ToggleToolBar();
        delete ToolBar;
        ToolBar = NULL;
    }
    if (Throbber || DelayedThrobber)
        SetThrobber(FALSE, 0, TRUE); // potrebujeme sestrelit timer

    ::DestroyWindow(HWindow);
}

void CStatusWindow::SetHidden(int hiddenFilesCount, int hiddenDirsCount)
{
    CALL_STACK_MESSAGE_NONE
    BOOL hidden = hiddenFilesCount != 0 || hiddenDirsCount != 0;
    HiddenFilesCount = hiddenFilesCount;
    HiddenDirsCount = hiddenDirsCount;
    if (Hidden != hidden)
    {
        Hidden = hidden;
        if (HWindow != NULL)
        {
            NeedToInvalidate = TRUE;
            InvalidateIfNeeded();
        }
    }
}

void CStatusWindow::SetHistory(BOOL history)
{
    CALL_STACK_MESSAGE_NONE
    if (History != history)
    {
        History = history;
        if (HWindow != NULL)
        {
            NeedToInvalidate = TRUE;
            InvalidateIfNeeded();
        }
    }
}

void CStatusWindow::SetThrobber(BOOL show, int delay, BOOL calledFromDestroyWindow)
{
    CALL_STACK_MESSAGE_NONE
    if (!calledFromDestroyWindow)
        ShowThrobber = show;
    if (show)
    {
        if (DelayedThrobber) // ceka se na zobrazeni
        {
            if (HWindow == NULL)
                TRACE_E("Unexpected situation in CStatusWindow::SetThrobber(): DelayedThrobber is TRUE but HWindow is NULL");
            if (Throbber)
                TRACE_E("Unexpected situation in CStatusWindow::SetThrobber(): DelayedThrobber and Throbber are both TRUE");
            KillTimer(HWindow, IDT_DELAYEDTHROBBER);
            if (Throbber /* jen korekce nekonzistentniho stavu */ ||
                delay <= 0 || !SetTimer(HWindow, IDT_DELAYEDTHROBBER, delay, NULL))
            {
                DelayedThrobber = FALSE; // ma se zobrazit hned nebo doslo k chybe pri nastavovani timeru, takze ho zobrazime hned
                DelayedThrobberShowTime = 0;
            }
            else
            {
                DelayedThrobberShowTime = GetTickCount() + delay;
                if (DelayedThrobberShowTime == 0)
                    DelayedThrobberShowTime++; // 0 je neplatna hodnota
            }
        }
        else
        {
            if (!Throbber && delay > 0)
            {
                if (HWindow != NULL && SetTimer(HWindow, IDT_DELAYEDTHROBBER, delay, NULL))
                    DelayedThrobber = TRUE; // neni zobrazen + ma se zobrazit se zpozdenim + okno je videt (pokud neni, napocita se jen DelayedThrobberShowTime)
                DelayedThrobberShowTime = GetTickCount() + delay;
                if (DelayedThrobberShowTime == 0)
                    DelayedThrobberShowTime++; // 0 je neplatna hodnota
            }
        }
    }
    else
    {
        if (DelayedThrobber) // ceka se na zobrazeni, ale throbber se ma schovat, konec cekani
        {
            if (HWindow == NULL)
                TRACE_E("Unexpected situation 2 in CStatusWindow::SetThrobber(): DelayedThrobber is TRUE but HWindow is NULL");
            KillTimer(HWindow, IDT_DELAYEDTHROBBER);
            DelayedThrobber = FALSE;
        }
        if (!calledFromDestroyWindow)
            DelayedThrobberShowTime = 0;
    }
    if (HWindow == NULL && Throbber)
    {
        Throbber = FALSE;
        TRACE_E("Unexpected situation in CStatusWindow::SetThrobber(): Throbber is TRUE but HWindow is NULL");
    }
    if (HWindow != NULL && !DelayedThrobber && Throbber != show)
    {
        Throbber = show;

        if (Throbber)
        {
        //Start throbber animation.
            ThrobberFrameIndex = 0;

        //Start drawing timer.
            const int   nFrames = sizeof( SVGLoading_Active )/sizeof( SVGLoading_Active[0] );
            const int   refreshRate = 1000 / nFrames;

            SetTimer(HWindow, IDT_THROBBER, refreshRate, NULL);
        }
        else
        {
            KillTimer(HWindow, IDT_THROBBER);
        }

        if (StopStatusbarRepaint == 0)
        {
            NeedToInvalidate = TRUE;
            InvalidateIfNeeded();
        }
        else
        {
            PostStatusbarRepaint = TRUE;
        }
    }
}

void CStatusWindow::SetThrobberTooltip(const char* throbberTooltip)
{
    if (ThrobberTooltip != NULL)
    {
        free(ThrobberTooltip);
        ThrobberTooltip = NULL;
    }
    if (throbberTooltip != NULL)
        ThrobberTooltip = DupStr(throbberTooltip);
}

void CStatusWindow::SetSecurity(CSecurityIconState iconState)
{
    CALL_STACK_MESSAGE_NONE
    if (Security != iconState)
    {
        Security = iconState;
        if (HWindow != NULL)
        {
            NeedToInvalidate = TRUE;
            InvalidateIfNeeded();
        }
    }
}

void CStatusWindow::SetSecurityTooltip(const char* tooltip)
{
    if (SecurityTooltip != NULL)
    {
        free(SecurityTooltip);
        SecurityTooltip = NULL;
    }
    if (tooltip != NULL)
        SecurityTooltip = DupStr(tooltip);
}

int CStatusWindow::ChangeThrobberID()
{
    static int NewID = 0; // id throbberu musi byt unikatni (tzn. jediny counter pro oba panely)
    ThrobberID = NewID++;
    if (ThrobberID == -1)
        ThrobberID = NewID++;
    return ThrobberID;
}

void CStatusWindow::HideThrobberAndSecurityIcon()
{
//#ifdef _DEBUG
//    SetThrobber(TRUE, 0);
//    SetThrobberTooltip("NEKI NEKI");
//    SetSecurity(sisSecured);
//    SetSecurityTooltip("NEKI DRUGO");
//#else
    SetThrobber(FALSE);
    SetThrobberTooltip(NULL);
    SetSecurity(sisNone);
    SetSecurityTooltip(NULL);
//#endif
}

void CStatusWindow::InvalidateIfNeeded()
{
    CALL_STACK_MESSAGE_NONE
    if (NeedToInvalidate)
    {
        NeedToInvalidate = FALSE;
        if (HWindow != NULL)
            InvalidateRect(HWindow, NULL, TRUE);
    }
}

int CStatusWindow::GetNeededHeight()
{
    CALL_STACK_MESSAGE_NONE
    int height = 2 + EnvFontCharHeight + 2;
    if (Border & blTop)
    {
        height += 2 + 2;
        //    int needed = ToolBar->GetNeededHeight();
        int needed = 3 + 16 + 3;
        if (height < needed)
            height = needed;
    }
    if (Border & blBottom)
        height++;
    return height;
}

void CStatusWindow::SetSize(const CQuadWord& size)
{
    CALL_STACK_MESSAGE_NONE
    if (DiskSpaceAvailable == NULL)
    {
        DiskSpaceAvailable = (char*)malloc(30);
        DiskSpaceAvailable[0] = 0;
    }
    if (DiskSpaceAvailable != NULL)
    {
        if (size == CQuadWord(-1, -1))
            DiskSpaceAvailable[0] = 0;
        else
        {
            char buf[100];
            PrintDiskSize(buf, size, 0);
            if (strcmp(buf, DiskSpaceAvailable) == 0)
                return;
            else
                strcpy(DiskSpaceAvailable, buf);
        }
    }
    if (HWindow != NULL)
        InvalidateRect(HWindow, NULL, FALSE);
}

void CStatusWindow::SetLeftPanel(BOOL left)
{
    CALL_STACK_MESSAGE_NONE
    Left = left;
    if (ToolBar != NULL)
    {
        ToolBar->SetType(Left ? mtbtLeft : mtbtRight);
        ToolBar->Load(Left ? Configuration.LeftToolBar : Configuration.RightToolBar);
    }
}

BOOL CStatusWindow::ToggleToolBar()
{
    CALL_STACK_MESSAGE1("CStatusWindow::ToggleToolBar()");
    if (ToolBar == NULL)
        return FALSE;
    if (ToolBar->HWindow != NULL)
    {
        ::DestroyWindow(ToolBar->HWindow);
        return TRUE;
    }
    else
    {
        if (!ToolBar->CreateWnd(HWindow))
            return FALSE;
        ToolBar->SetImageList(HGrayToolBarImageList);
        ToolBar->SetHotImageList(HHotToolBarImageList);
        ToolBar->SetStyle(TLB_STYLE_IMAGE | TLB_STYLE_ADJUSTABLE);
        ToolBar->Load(Left ? Configuration.LeftToolBar : Configuration.RightToolBar);
        SendMessage(ToolBar->HWindow, TB_SETPARENT, (WPARAM)MainWindow->HWindow, 0);
        ShowWindow(ToolBar->HWindow, SW_SHOW);
        return TRUE;
    }
    return TRUE;
}

BOOL CStatusWindow::SetDriveIcon(HICON hIcon)
{
    CALL_STACK_MESSAGE_NONE
    if (ToolBar != NULL && ToolBar->HWindow != NULL)
        ToolBar->ReplaceImage(Left ? CM_LCHANGEDRIVE : CM_RCHANGEDRIVE, FALSE, hIcon, TRUE, TRUE);
    return TRUE;
}

void CStatusWindow::SetDrivePressed(BOOL pressed)
{
    CALL_STACK_MESSAGE_NONE
    if (ToolBar != NULL && ToolBar->HWindow != NULL)
    {
        TLBI_ITEM_INFO2 tii;
        tii.Mask = TLBI_MASK_STATE;
        tii.State = pressed ? TLBI_STATE_PRESSED : 0;
        ToolBar->SetItemInfo2(Left ? CM_LCHANGEDRIVE : CM_RCHANGEDRIVE, FALSE, &tii);
        UpdateWindow(ToolBar->HWindow);
    }
}

void CStatusWindow::LayoutWindow()
{
    CALL_STACK_MESSAGE_NONE
    SendMessage(HWindow, WM_SIZE, 0, 0);
    InvalidateRect(HWindow, NULL, TRUE);
    UpdateWindow(HWindow);
}

void CStatusWindow::GetHotText(char* buffer, int bufSize)
{
    CALL_STACK_MESSAGE_NONE

    if (
        ( m_Hot_Item != nullptr )
        &&
        ( Text != NULL )
    )
    {
        lstrcpyn( buffer, Text + m_Hot_Item->Offset, min( m_Hot_Item->Chars + 1, bufSize ) );

        //For Directory Line with plugin FS it is still necessary to allow the plugin to make the last path modifications (adding ']' to VMS paths for FTP)
        if (
            ( Border & blTop )
            &&
            PanelWindow->Is(ptPluginFS)
            &&
            PanelWindow->GetPluginFS()->NotEmpty()
        )
        {
            PanelWindow->GetPluginFS()->CompleteDirectoryLineHotPath(buffer, bufSize);
        }
    }
    else
        buffer[0] = 0;
}

BOOL CStatusWindow::FindHotTrackItem(int xPos, int& index)
{
    CALL_STACK_MESSAGE_NONE
    int i;
    for (i = 0; i < HotTrackItems.Count; i++)
    {
        CHotTrackItem* item = &HotTrackItems[i];

        // pokud je za root slozkou vypustka, musime o ni posunout xPos
        if (i == 1 && EllipsedWidth != -1)
            xPos += EllipsedWidth - TextEllipsisWidthEnv;

        if (xPos >= item->PixelsOffset && xPos < item->PixelsOffset + item->Pixels)
        {
            index = i;
            return TRUE;
        }
    }
    return FALSE;
}

void CStatusWindow::FlashText(BOOL hotTrackOnly)
{
    CALL_STACK_MESSAGE_NONE
    Repaint(TRUE, hotTrackOnly);
    Sleep(100);
    Repaint(FALSE, hotTrackOnly);
}

void PaintSymbol(HDC hDC, HDC hMemDC, HBITMAP hBitmap, int xOffset, int width, int height, const RECT* rect, BOOL hot, BOOL activeCaption)
{
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
    COLORREF textColor;
    if (hot)
    {
        if (Configuration.ShowPanelCaption)
            textColor = GetCOLORREF(CurrentColors[activeCaption ? HOT_ACTIVE : HOT_INACTIVE]);
        else
            textColor = GetCOLORREF(CurrentColors[HOT_PANEL]);
    }
    else
    {
        if (Configuration.ShowPanelCaption)
            textColor = GetCOLORREF(CurrentColors[activeCaption ? ACTIVE_CAPTION_FG : INACTIVE_CAPTION_FG]);
        else
            textColor = GetSysColor(COLOR_BTNTEXT);
    }
    COLORREF bkColor;
    if (Configuration.ShowPanelCaption)
        bkColor = GetCOLORREF(CurrentColors[activeCaption ? ACTIVE_CAPTION_BK : INACTIVE_CAPTION_BK]);
    else
        bkColor = GetSysColor(COLOR_BTNFACE);

    int oldTextColor = SetTextColor(hDC, textColor);
    int oldBkColor = SetBkColor(hDC, bkColor);

    //Position the icon to the left side of the rect i.e. extra space is right padding.
    //int x = (rect->left + rect->right) / 2 - width / 2;
    int x = rect->left;
    int y = (rect->top + rect->bottom) / 2 - height / 2;

    BitBlt(hDC, x, y, width, height, hMemDC, xOffset, 0, SRCCOPY);
    SetBkColor(hDC, oldBkColor);
    SetTextColor(hDC, oldTextColor);
    SelectObject(hMemDC, hOldBitmap);
}

void CStatusWindow::Paint(HDC hdc, BOOL highlightText, BOOL highlightHotTrackOnly)
{
    CALL_STACK_MESSAGE3( "CStatusWindow::Paint(, %d, %d)", highlightText, highlightHotTrackOnly );

//Initialize temporary/working image (fill background with default color).
//
//Notice:
//At the end we'll copy data from it to the window.
    HDC     dc_tmp = ItemBitmap.HMemDC;
    RECT    rect_dc_tmp = { .left = 0, .top = 0, .right = Width, .bottom = Height };

    FillRect( dc_tmp, &rect_dc_tmp, HDialogBrush );

//Get window working surfuce.
    RECT&   rect_window = rect_dc_tmp;     //Reuse memory as FillRect doesn't need it anymore.
    
    GetClientRect( HWindow, &rect_window );

//???
    if (Border & blBottom)
        rect_window.bottom--;

//Are we drawing active caption (header)?
    BOOL    isActiveCaption = ( PanelWindow == MainWindow->GetActivePanel() ) && MainWindow->CaptionIsActive;

// instead of the pro toolbar, please
    BOOL    isDirectoryLine = (Border & blTop) != 0;

    if (isDirectoryLine)
        rect_window.left += ToolBarWidth + 1;

    if (isDirectoryLine && Configuration.ShowPanelCaption)
    {
        // a frame around the text
        RECT textR = rect_window;
        textR.top += 2;
        textR.bottom -= 2;
        DrawEdge( dc_tmp, &textR, BDR_SUNKENOUTER, BF_RECT );

        // fill in the area under the text (active/inactive)
        textR.left++;
        textR.top++;
        textR.right--;
        textR.bottom--;
        FillRect( dc_tmp, &textR, isActiveCaption ? HActiveCaptionBrush : HInactiveCaptionBrush );
    }

//Draw text.
    EllipsedChars = -1;
    EllipsedWidth = -1;

    if ( Text != NULL )
    {
    //Set fill mode (don't change background in anyway).
        SetBkMode( dc_tmp, TRANSPARENT );

    //Store whole useful working surface.
        m_Rect_Whole = rect_window;

    //Reset working surfaces.
    //
    //Why?
    //In case there will not be enough space (i.e. width) to draw all the surfaces we'll draw "empty width" surfaces.
        //Define working surface.
        RECT    rect_working = { .left = 0, .top = rect_window.top + 3, .right = 0, .bottom = rect_window.bottom - 3 };

        //Reset surfaces.
        m_Rect_Security = rect_working;
        m_Rect_Text = rect_working;
        m_Rect_Filtered = rect_working;
        m_Rect_Throbber = rect_working;
        m_Rect_Size = rect_working;
        m_Rect_History = rect_working;
        m_Rect_Zoom = rect_working;

        //Define real working surface.
        rect_working.left = rect_window.left + 2;
        rect_working.right = rect_window.right - 5,

    //???
        WholeTextVisible = FALSE;

    //Set font to be used in the temporary/working image.
        HFONT   hFont_old = (HFONT)SelectObject( dc_tmp, EnvFont );

    //Detect which items (text/history/size/zoom) should be drawn.
    //
    //Notice:
    //If there will be not enough space (i.e. width) specific elements will be dropped (will not be drawn).
    //
    //Notice 2:
    //For every item the right padding will be defined i.e. the icon/text will be placed to the left side of the allocated space.
        SIZE    size_text_diskSpaceAvailable;
        LONG    width_working = rect_working.right - rect_working.left;

        if ( isDirectoryLine )
        {
            #define SET_ITEM_POSITION( item_rect, width, padding_right )                                                \
            {                                                                                                           \
            /* Is there enough space/width left to draw icon/text? */                                                   \
                const LONG      width_item = width;                                                                     \
                const LONG      width_used = width_item + padding_right;                                                \
                                                                                                                        \
                if ( width_working < width_used )                                                                       \
                {                                                                                                       \
                /* No -> stop detecting items to be drawn. */                                                           \
                    goto Label_skipMeasuring;                                                                           \
                }                                                                                                       \
                                                                                                                        \
            /* Allocate space for the item. */                                                                          \
            /* */                                                                                                       \
            /* Notice: */                                                                                               \
            /* Padding is not included. */                                                                              \
                const auto      position_right = rect_working.right - padding_right;                                    \
                                                                                                                        \
                item_rect.right = position_right;                                                                       \
                item_rect.left = position_right - width_item;                                                           \
                                                                                                                        \
            /* Update working space. */                                                                                 \
                rect_working.right -= width_used;                                                                       \
                width_working -= width_used;                                                                            \
            }                                                                                                           \
            (void)0

        //Draw 'zoom' (maximize/restore panel) icon.
            if ( Configuration.ShowPanelZoom )
            {
            //Get width of the icon.
                const LONG      width_icon = SVGZoom_In_Active.GetWidth();

            //Set item position.
                SET_ITEM_POSITION( m_Rect_Zoom, width_icon, 2 );
            }

        //Draw 'free disk space still available' text.
            if ( DiskSpaceAvailable != NULL )
            {
            //Get 'space available' text width.
                GetTextExtentPoint32( dc_tmp, DiskSpaceAvailable, (int)strlen( DiskSpaceAvailable ), &size_text_diskSpaceAvailable );

            //Set item position.
                SET_ITEM_POSITION( m_Rect_Size, size_text_diskSpaceAvailable.cx, 5 );
            }

        //Draw 'directory history dropdown' icon.
            if ( History )
            {
            //Get width of the icon.
                const LONG      width_icon = SVGHistory_Active.GetWidth();

            //Set item position.
                SET_ITEM_POSITION( m_Rect_History, width_icon, 2 );
            }

        //Draw 'filter' (are there hidden ???) icon.
            if ( Hidden )
            {
            //Get width of the icon.
                const LONG      width_icon = SVGFilter_Active.GetWidth();

            //Set item position.
                SET_ITEM_POSITION( m_Rect_Filtered, width_icon, 7 );
            }

        //Draw 'throbber' (???) icon.
            if ( Throbber )
            {
            //Get width of the icon.
                const LONG      width_icon = SVGLoading_Active[0].GetWidth();

            //Set item position.
                SET_ITEM_POSITION( m_Rect_Throbber, width_icon, 7 );
            }

        //Draw 'security' (???) icon.
            if (
                ( Security != sisNone )
                &&
                ( ( Border & blTop ) != 0 )       //[W: ???]
            )
            {
            //Get width of the icon.
                //const LONG      width_icon = ( ( Security == sisSecured ) ? SVGSecurity_Locked_Active : SVGSecurity_Unlocked_Active ).GetWidth();
                const LONG      width_icon = SVGSecurity_Unlocked_Active.GetWidth();

            //Set item position.
                SET_ITEM_POSITION( m_Rect_Security, width_icon, 7 );
            }
        }
    Label_skipMeasuring:

    //Draw caption text.
        //Initialize text parameters.
        struct Text_Truncate
        {
            public: enum Value
            {
                end,            //truncate end of text
                folderText      //leave path root and truncate folder text (e.g. C:\...der1\folder2\folder3)
            };
        };

        Text_Truncate::Value    truncateType = Text_Truncate::end;
        int                     visibleChars = 0;

        //Is there enough space to draw at least ellipsis character?
        if ( width_working > TextEllipsisWidthEnv )
        {
        //Yes -> check how much of the text can it be drawn.
            //Initialize variables.
            visibleChars = TextLen;

            //Is there enough space to draw whole text?
            if ( width_working >= AlpDX[TextLen - 1] )
            {
            //Yes.
                WholeTextVisible = TRUE;
            }
            else
            {
            //No -> are we drawing upper directory or lower info line?
                if (
                    isDirectoryLine
                    &&
                    ( HotTrackItems.Count > 1 )
                    &&
                    ( ( HotTrackItems[0].Pixels + TextEllipsisWidthEnv ) <= width_working )
                )
                {
                //The upper directory line -> abbreviate the root component of the path.
                    EllipsedChars = 0;
                    EllipsedWidth = 0;

                    int     width_text = AlpDX[TextLen - 1];
                    int     iter = HotTrackItems[0].Chars;

                    while (
                        ( width_text > ( width_working - TextEllipsisWidthEnv ) )
                        &&
                        ( iter < TextLen )
                    )
                    {
                        int charWidth = AlpDX[iter] - AlpDX[iter - 1];
                        width_text -= charWidth;
                        iter++;

                        EllipsedChars++;
                        EllipsedWidth += charWidth;
                    }
                    visibleChars = TextLen - iter;

                //Set truncation type.
                    truncateType = Text_Truncate::folderText;
                }
                else
                {
                //The lower info line -> find how many characters can be drawn (including ellipsis character at the end)
                    while (
                        ( visibleChars > 0 )
                        &&
                        ( width_working < ( AlpDX[visibleChars - 1] + TextEllipsisWidthEnv ) )
                    )
                    {
                    //Not enough space available -> remove another character.
                        visibleChars--;
                    }
                }
            }

        //Get width of the (will be) drawn text.
            LONG    width_used = 0;

            if ( TextLen > 1 )
            {
            //Get 
                width_used = AlpDX[TextLen - 1];

            //Account the used width in case ellipsis will be drawn?
                if ( EllipsedWidth != -1 )
                {
                    width_used = width_used - EllipsedWidth + TextEllipsisWidthEnv;
                }
            }

        //Allocate space for the text.
            m_Rect_Text.left = rect_working.left;
            m_Rect_Text.right = rect_working.left + width_used;

        //Let text use all the remaining space until the icons.
            if ( m_Rect_Text.right > rect_working.right )
            {
                m_Rect_Text.right = rect_working.right;
            }
        }

    //Draw text.
        //Get vertical text position.
        int     myYOffset = 0;

        if ( isDirectoryLine && !Configuration.ShowPanelCaption )
        {
        //???
            myYOffset = 1;
        }

        const int   position_text_y = ( rect_working.top + rect_working.bottom - EnvFontCharHeight + myYOffset ) / 2;

        //Draw text.
        if ( m_Rect_Text.right > m_Rect_Text.left )
        {
            // let's determine in advance which part of the text should be rendered prominently, because of cleartype we have to cut out the normal text from the drawing
            CHotTrackItem*  hot_item = nullptr;
            BOOL            showFlashText = ( highlightText && highlightHotTrackOnly && ( m_Hot_Item_Last != nullptr ) );

            if ( m_Hot_Item != nullptr )
                hot_item = m_Hot_Item;

            if (showFlashText)
                hot_item = m_Hot_Item_Last;

            if (isDirectoryLine && Configuration.ShowPanelCaption)
            {
                if (isActiveCaption)
                    SetTextColor( dc_tmp, GetCOLORREF( CurrentColors[ACTIVE_CAPTION_FG] ) );
                else
                    SetTextColor( dc_tmp, GetCOLORREF( CurrentColors[INACTIVE_CAPTION_FG] ) );

                if (highlightText && !highlightHotTrackOnly)
                    SetTextColor( dc_tmp, GetCOLORREF( CurrentColors[isActiveCaption ? HOT_ACTIVE : HOT_INACTIVE] ) );
            }
            else
            {
                SetTextColor( dc_tmp, GetSysColor( COLOR_BTNTEXT ) );

                if (highlightText && !highlightHotTrackOnly)
                    SetTextColor(dc_tmp, GetSysColor(COLOR_HIGHLIGHTTEXT));
            }

        //
            int firstClipChar = 2 * TextLen;
            int lastClipChar = 2 * TextLen;
            if (hot_item != nullptr)
            {
                firstClipChar = hot_item->Offset;
                lastClipChar = hot_item->Offset + hot_item->Chars;
            }

            // let's draw the first part of the text (up to the hot_item, if the text starts with a hot_item, we won't draw anything)
            if (firstClipChar != 0)
            {
                if (truncateType == Text_Truncate::end)
                {
                // without shortening or cut end
                    ExtTextOut(dc_tmp, m_Rect_Text.left, position_text_y, 0, NULL, Text, min(visibleChars, firstClipChar), NULL);

                    if (visibleChars < min(TextLen, firstClipChar)) // pokud byl ustrizen konec -> pripojime "..."
                    {
                        int offset = (visibleChars > 0) ? AlpDX[visibleChars - 1] : 0;
                        ExtTextOut(dc_tmp, m_Rect_Text.left + offset, position_text_y, 0, NULL, "...", 3, NULL);
                    }
                }
                else
                {
                // rooted cast for the root folder
                    // root cast
                    int rootChars = HotTrackItems[0].Chars;
                    ExtTextOut(dc_tmp, m_Rect_Text.left, position_text_y, 0, NULL, Text, rootChars, NULL);
                    // "..."
                    ExtTextOut(dc_tmp, m_Rect_Text.left + AlpDX[rootChars - 1], position_text_y, 0, NULL, "...", 3, NULL);
                    // zbytek
                    ExtTextOut(dc_tmp, m_Rect_Text.left + AlpDX[rootChars - 1] + TextEllipsisWidthEnv, position_text_y, 0, NULL, Text + TextLen - visibleChars, visibleChars, NULL);
                }
            }

            // let's render the second part of the text (after the hot_item dal) -- shortening at the end
            if (hot_item != nullptr && ( truncateType == Text_Truncate::end ) && lastClipChar <= visibleChars)
            {
                // without shortening or cut end
                int visibleChars2 = visibleChars - lastClipChar;
                ExtTextOut(dc_tmp, m_Rect_Text.left + AlpDX[lastClipChar - 1], position_text_y, 0, NULL, Text + lastClipChar, visibleChars2, NULL);

                if (visibleChars < TextLen) // pokud byl ustrizen konec -> pripojime "..."
                {
                    int offset = (visibleChars > 0) ? AlpDX[visibleChars - 1] : 0;
                    ExtTextOut(dc_tmp, m_Rect_Text.left + offset, position_text_y, 0, NULL, "...", 3, NULL);
                }
            }
            // let's draw the second part of the text (behind the hot_item dal) -- avoid shortening
            // only paths in the directory line are shortened this way (condition !truncateEnd applies)
            if (hot_item != nullptr && ( truncateType == Text_Truncate::folderText ) && lastClipChar <= TextLen)
            {
            // rooted cast for the root folder
                int rootChars = HotTrackItems[0].Chars;
                int firstChar = hot_item->Chars;

                if (lastClipChar <= rootChars)
                {
                    ExtTextOut(dc_tmp, m_Rect_Text.left + AlpDX[rootChars - 1], position_text_y, 0, NULL, "...", 3, NULL); // "..."
                    firstChar += EllipsedChars;                                                           // posuneme se pres vypustene znaky
                }
                else
                {
                    if (firstChar < rootChars + EllipsedChars) // je potreba preskocit pripadne zpetne lomitko, ktere by lezlo do vypustky
                        firstChar = rootChars + EllipsedChars;
                }
                ExtTextOut(dc_tmp, m_Rect_Text.left + AlpDX[firstChar - 1] - EllipsedWidth + TextEllipsisWidthEnv, position_text_y, 0, NULL, Text + firstChar, TextLen - firstChar, NULL);
            }

            // show hot track item
            if (hot_item != nullptr)
            {
                COLORREF oldColor;
                if (isDirectoryLine && Configuration.ShowPanelCaption)
                {
                    oldColor = SetTextColor(dc_tmp, GetCOLORREF(CurrentColors[isActiveCaption ? HOT_ACTIVE : HOT_INACTIVE]));
                }
                else
                {
                    oldColor = SetTextColor(dc_tmp, GetCOLORREF(CurrentColors[HOT_PANEL]));

                    if (showFlashText)
                        SetTextColor(dc_tmp, GetSysColor(COLOR_HIGHLIGHTTEXT));
                }
                HFONT hOldFont = NULL;
                if (
                    ( Configuration.SingleClick != 0 )
                    &&
                    ( m_Hot_Item != nullptr )
                )
                {
                    hOldFont = (HFONT)SelectObject(dc_tmp, EnvFontUL);
                }

                if ( truncateType == Text_Truncate::end )
                {
                // bez zkraceni nebo ustrizen konec
                    int showChars = hot_item->Chars;

                    if (hot_item->Offset + showChars > visibleChars)
                    {
                        showChars = visibleChars - hot_item->Offset;
                        int offset = (visibleChars > 0) ? AlpDX[visibleChars - 1] : 0;
                        ExtTextOut(dc_tmp, m_Rect_Text.left + offset, position_text_y, 0, NULL, "...", 3, NULL);
                    }
                    if (showChars > 0)
                    {
                        ExtTextOut(dc_tmp, m_Rect_Text.left + hot_item->PixelsOffset, position_text_y, 0, NULL, Text + hot_item->Offset, showChars, NULL);
                    }
                }
                else
                {
                // uriznuta cast za root slozkou
                    int showChars = hot_item->Chars;

                    int rootChars = HotTrackItems[0].Chars;
                    ExtTextOut(dc_tmp, m_Rect_Text.left, position_text_y, 0, NULL, Text, rootChars, NULL);
                    if (showChars > rootChars)
                    {
                        // "..."
                        ExtTextOut(dc_tmp, m_Rect_Text.left + AlpDX[rootChars - 1], position_text_y, 0, NULL, "...", 3, NULL);
                        if (showChars - rootChars - EllipsedChars > 0)
                        {
                            // zbytek
                            ExtTextOut(dc_tmp, m_Rect_Text.left + AlpDX[rootChars - 1] + TextEllipsisWidthEnv, position_text_y, 0, NULL, Text + rootChars + EllipsedChars, showChars - rootChars - EllipsedChars, NULL);
                        }
                    }
                }

                if (hOldFont != NULL)
                    SelectObject(dc_tmp, hOldFont);

                SetTextColor(dc_tmp, oldColor);
            }
        }

    //Draw icons, ...
        HDC     hMemDC = HANDLES( CreateCompatibleDC( NULL ) );

        //Draw zoom (maximize/restore panel) icon.
        if ( m_Rect_Zoom.left < m_Rect_Zoom.right )
        {
        //Is panel zoomed?
            const BOOL    zoomed = MainWindow->IsPanelZoomed( ( MainWindow->LeftPanel == PanelWindow ) ? TRUE : FALSE );

        //Select icon.
            auto& icon =
                ( zoomed == 0 ) ?
                    ( ( isActiveCaption ) ? SVGZoom_In_Active : SVGZoom_In_Inactive )
                    :
                    ( ( isActiveCaption ) ? SVGZoom_Out_Active : SVGZoom_Out_Inactive );

        //Draw icon.
            icon.AlphaBlend(
                dc_tmp,
                m_Rect_Zoom.left,
                m_Rect_Zoom.top + ( m_Rect_Zoom.bottom - m_Rect_Zoom.top - icon.GetHeight() ) / 2,      //Vertical center
                -1, -1,
                m_Hot_Highlight_Zoom ? SVGSTATE_DISABLED_OR_FOCUSED : SVGSTATE_ENABLED_OR_NORMAL
            );
        }

        //Draw 'free disk space still available' text.
        if ( m_Rect_Size.left < m_Rect_Size.right )
        {
        //Set text color.
            if ( m_Hot_Highlight_Size )
            {
                if (isDirectoryLine && Configuration.ShowPanelCaption)
                    SetTextColor(dc_tmp, GetCOLORREF(CurrentColors[isActiveCaption ? HOT_ACTIVE : HOT_INACTIVE]));
                else
                    SetTextColor(dc_tmp, GetCOLORREF(CurrentColors[HOT_PANEL]));
            }
            else
            {
                if (isDirectoryLine && Configuration.ShowPanelCaption)
                    SetTextColor(dc_tmp, GetCOLORREF(CurrentColors[isActiveCaption ? ACTIVE_CAPTION_FG : INACTIVE_CAPTION_FG]));
                else
                    SetTextColor(dc_tmp, GetSysColor(COLOR_BTNTEXT));
            }

        //Switch font.
            HFONT hOldFont = NULL;

            if (
                ( Configuration.SingleClick != 0 )
                &&
                ( m_Hot_Highlight_Size )
            )
            {
                hOldFont = (HFONT)SelectObject(dc_tmp, EnvFontUL);
            }

        //Draw text.
            ExtTextOut( dc_tmp, m_Rect_Size.left, position_text_y, 0, NULL, DiskSpaceAvailable, (UINT)strlen(DiskSpaceAvailable), NULL );

        //Switch back to original font.
            if ( hOldFont != NULL )
            {
                SelectObject(dc_tmp, hOldFont);
            }
        }

        //Draw 'directory history dropdown' icon.
        if ( m_Rect_History.left < m_Rect_History.right )
        {
        //Select icon.
            auto& icon = ( isActiveCaption ) ? SVGHistory_Active : SVGHistory_Inactive;

        //Draw icon.
            icon.AlphaBlend(
                dc_tmp,
                m_Rect_History.left,
                m_Rect_History.top + ( m_Rect_History.bottom - m_Rect_History.top - icon.GetHeight() ) / 2,      //Vertical center
                -1, -1,
                m_Hot_Highlight_History ? SVGSTATE_DISABLED_OR_FOCUSED : SVGSTATE_ENABLED_OR_NORMAL
            );
        }

        //Draw 'filter' (are there hidden ???) icon.
        if ( m_Rect_Filtered.left < m_Rect_Filtered.right )
        {
        //Select icon.
            auto& icon = ( isActiveCaption ) ? SVGFilter_Active : SVGFilter_Inactive;

        //Draw icon.
            icon.AlphaBlend(
                dc_tmp,
                m_Rect_Filtered.left,
                m_Rect_Filtered.top + ( m_Rect_Filtered.bottom - m_Rect_Filtered.top - icon.GetHeight() ) / 2,         //Vertical center
                -1, -1,
                m_Hot_Highlight_Filtered ? SVGSTATE_DISABLED_OR_FOCUSED : SVGSTATE_ENABLED_OR_NORMAL
            );
        }

        //Draw 'throbber' (???) icon.
        if ( m_Rect_Throbber.left < m_Rect_Throbber.right )
        {
        //Select icon.
            auto& icon = ( ( isActiveCaption ) ? SVGLoading_Active : SVGLoading_Inactive )[ThrobberFrameIndex];

        //Draw icon.
            icon.AlphaBlend(
                dc_tmp,
                m_Rect_Throbber.left,
                m_Rect_Throbber.top + ( m_Rect_Throbber.bottom - m_Rect_Throbber.top - icon.GetHeight() ) / 2,         //Vertical center
                -1, -1,
                m_Hot_Highlight_Throbber ? SVGSTATE_DISABLED_OR_FOCUSED : SVGSTATE_ENABLED_OR_NORMAL
            );
        }

        //Draw 'security' (???) icon.
        if ( m_Rect_Security.left < m_Rect_Security.right )
        {
        //Select icon.
        //
        //Notice:
        //This will not be drawn if Security = sisNone.
            auto& icon =
                ( Security == sisSecured ) ?
                    ( ( isActiveCaption ) ? SVGSecurity_Locked_Active : SVGSecurity_Locked_Inactive )
                    :
                    ( ( isActiveCaption ) ? SVGSecurity_Unlocked_Active : SVGSecurity_Unlocked_Inactive );

        //Draw icon.
            icon.AlphaBlend(
                dc_tmp,
                m_Rect_Security.left,
                m_Rect_Security.top + ( m_Rect_Security.bottom - m_Rect_Security.top - icon.GetHeight() ) / 2,         //Vertical center
                -1, -1,
                m_Hot_Highlight_Security ? SVGSTATE_DISABLED_OR_FOCUSED : SVGSTATE_ENABLED_OR_NORMAL
            );
        }

        //Free resources.
        HANDLES( DeleteDC( hMemDC ) );

    //Set back the default temporary/working image font.
        SelectObject( dc_tmp, hFont_old );
    }

//Copy image to window.
    //???
    int delta = 0;

    if ( Border & blBottom )
        delta = 1;

    //Copy data.
    BitBlt( hdc, delta + ToolBarWidth, 0, Width - ToolBarWidth - 2 * delta, Height - delta, dc_tmp, ToolBarWidth, 0, SRCCOPY );
}

void CStatusWindow::Repaint(BOOL flashText, BOOL hotTrackOnly)
{
    CALL_STACK_MESSAGE_NONE
    if (HWindow == NULL)
        return;
    HDC hdc = HANDLES(GetDC(HWindow));
    Paint(hdc, flashText, hotTrackOnly);
    HANDLES(ReleaseDC(HWindow, hdc));
}

void CStatusWindow::InvalidateAndUpdate(BOOL update)
{
    CALL_STACK_MESSAGE_NONE
    if (HWindow == NULL)
        return;
    InvalidateRect(HWindow, NULL, FALSE);
    if (update)
        UpdateWindow(HWindow);
}

class CTextDropTarget : public IDropTarget
{
private:
    long RefCount;                    // zivotnost objektu
    IDataObject* DataObject;          // IDataObject, ktery vstoupil do dragu
    IDataObject* ForbiddenDataObject; // IDataObject, ktery nebereme (jsme jeho zdrojem)
    BOOL UseUnicode;                  // je v DataObject unicode text? (jinak zkusime ANSI text)
    CPanelWindow* PanelWindow;        // panel, ke kteremu jsme asociovani
    char Buffer[2 * MAX_PATH];

public:
    CTextDropTarget(CPanelWindow* filesWindow)
    {
        RefCount = 1;
        DataObject = NULL;
        ForbiddenDataObject = NULL;
        UseUnicode = TRUE;
        PanelWindow = filesWindow;
    }

    virtual ~CTextDropTarget()
    {
        if (RefCount != 0)
            TRACE_E("Preliminary destruction of this object.");
    }

    void SetForbiddenDataObject(IDataObject* forbiddenDataObject)
    {
        ForbiddenDataObject = forbiddenDataObject;
    }

    // vrati adresar (musi byt prave jeden)
    BOOL GetDirFromDataObject(IDataObject* pDataObject, char* path)
    {
        FORMATETC formatEtc;
        formatEtc.cfFormat = RegisterClipboardFormat(SALCF_FAKE_REALPATH);
        formatEtc.ptd = NULL;
        formatEtc.dwAspect = DVASPECT_CONTENT;
        formatEtc.lindex = -1;
        formatEtc.tymed = TYMED_HGLOBAL;

        STGMEDIUM stgMedium;
        stgMedium.tymed = TYMED_HGLOBAL;
        stgMedium.hGlobal = NULL;
        stgMedium.pUnkForRelease = NULL;

        if (pDataObject->GetData(&formatEtc, &stgMedium) == S_OK)
        {
            path[0] = 0;
            if (stgMedium.tymed == TYMED_HGLOBAL && stgMedium.hGlobal != NULL)
            {
                char* data = (char*)HANDLES(GlobalLock(stgMedium.hGlobal));
                if (data != NULL)
                {
                    if (data[0] == 'D')
                        lstrcpyn(path, data + 1, MAX_PATH);
                    HANDLES(GlobalUnlock(stgMedium.hGlobal));
                }
            }
            ReleaseStgMedium(&stgMedium);
            return path[0] != 0;
        }

        formatEtc.cfFormat = CF_HDROP;
        formatEtc.ptd = NULL;
        formatEtc.dwAspect = DVASPECT_CONTENT;
        formatEtc.lindex = -1;
        formatEtc.tymed = TYMED_HGLOBAL;

        stgMedium.tymed = TYMED_HGLOBAL;
        stgMedium.hGlobal = NULL;
        stgMedium.pUnkForRelease = NULL;

        BOOL ret = FALSE;
        if (pDataObject->GetData(&formatEtc, &stgMedium) == S_OK)
        {
            if (stgMedium.tymed == TYMED_HGLOBAL && stgMedium.hGlobal != NULL)
            {
                DROPFILES* data = (DROPFILES*)HANDLES(GlobalLock(stgMedium.hGlobal));
                if (data != NULL)
                {
                    if (data->fWide)
                    {
                        const wchar_t* fileW = (wchar_t*)(((char*)data) + data->pFiles);
                        int l = lstrlenW(fileW);
                        if (*(fileW + l + 1) == 0)
                        {
                            WideCharToMultiByte(CP_ACP, 0, fileW, l + 1, path, l + 1, NULL, NULL);
                            path[l] = 0;
                            ret = TRUE;
                        }
                    }
                    else
                    {
                        const char* fileA = ((char*)data) + data->pFiles;
                        int l = (int)strlen(fileA);
                        if (*(fileA + l + 1) == 0)
                        {
                            strcpy(path, fileA);
                            ret = TRUE;
                        }
                    }

                    HANDLES(GlobalUnlock(stgMedium.hGlobal));
                }
            }
            ReleaseStgMedium(&stgMedium);
        }
        if (ret)
        {
            DWORD attrs = SalGetFileAttributes(path);
            if (attrs == 0xFFFFFFFF)
                ret = FALSE;
            else if (!(attrs & FILE_ATTRIBUTE_DIRECTORY)) // nejedna se o adresar
                ret = FALSE;
        }
        return ret;
    }

    STDMETHOD(QueryInterface)
    (REFIID refiid, void FAR* FAR* ppv)
    {
        if (refiid == IID_IUnknown || refiid == IID_IDropTarget)
        {
            *ppv = this;
            AddRef();
            return NOERROR;
        }
        else
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }
    }

    STDMETHOD_(ULONG, AddRef)
    (void) { return ++RefCount; }
    STDMETHOD_(ULONG, Release)
    (void)
    {
        if (--RefCount == 0)
        {
            delete this;
            return 0; // nesmime sahnout do objektu, uz neexistuje
        }
        return RefCount;
    }

    STDMETHOD(DragEnter)
    (IDataObject* pDataObject, DWORD grfKeyState,
     POINTL pt, DWORD* pdwEffect)
    {
        if (ImageDragging)
            ImageDragEnter(pt.x, pt.y);
        if (DataObject != NULL)
            DataObject->Release();
        DataObject = pDataObject;
        DataObject->AddRef();

        // pokud je nas panel zaroven zdrojem, zakazu paste
        if (DataObject == ForbiddenDataObject)
        {
            *pdwEffect = DROPEFFECT_NONE;
            return S_OK;
        }

        // zjistime jestli je na clipboardu text
        FORMATETC formatEtc;
        ZeroMemory(&formatEtc, sizeof(formatEtc));
        formatEtc.cfFormat = CF_UNICODETEXT;
        formatEtc.dwAspect = DVASPECT_CONTENT;
        formatEtc.lindex = -1;
        formatEtc.tymed = TYMED_HGLOBAL;
        UseUnicode = TRUE;
        HRESULT textRes;
        if ((textRes = pDataObject->QueryGetData(&formatEtc)) != S_OK)
        {
            formatEtc.cfFormat = CF_TEXT;
            UseUnicode = FALSE;
            textRes = pDataObject->QueryGetData(&formatEtc);
        }
        if (textRes == S_OK)
        {
            *pdwEffect = DROPEFFECT_COPY;
            return S_OK;
        }
        char dummy[MAX_PATH];
        if (GetDirFromDataObject(DataObject, dummy))
        {
            *pdwEffect = DROPEFFECT_COPY;
            return S_OK;
        }

        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }

    STDMETHOD(DragOver)
    (DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
    {
        if (ImageDragging)
            ImageDragMove(pt.x, pt.y);
        if (DataObject != NULL)
        {
            // pokud je nas panel zaroven zdrojem, zakazu paste
            if (DataObject == ForbiddenDataObject)
            {
                *pdwEffect = DROPEFFECT_NONE;
                return S_OK;
            }
            // zjistime jestli je na clipboardu text
            FORMATETC formatEtc;
            ZeroMemory(&formatEtc, sizeof(formatEtc));
            formatEtc.cfFormat = UseUnicode ? CF_UNICODETEXT : CF_TEXT;
            formatEtc.dwAspect = DVASPECT_CONTENT;
            formatEtc.lindex = -1;
            formatEtc.tymed = TYMED_HGLOBAL;
            if (DataObject->QueryGetData(&formatEtc) == S_OK)
            {
                *pdwEffect = DROPEFFECT_COPY;
                return S_OK;
            }
            char dummy[MAX_PATH];
            if (GetDirFromDataObject(DataObject, dummy))
            {
                *pdwEffect = DROPEFFECT_COPY;
                return S_OK;
            }
        }
        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }

    STDMETHOD(DragLeave)
    ()
    {
        if (ImageDragging)
            ImageDragLeave();
        if (DataObject != NULL)
        {
            DataObject->Release();
            DataObject = NULL;
        }
        return S_OK;
    }

    STDMETHOD(Drop)
    (IDataObject* pDataObject, DWORD grfKeyState, POINTL pt,
     DWORD* pdwEffect)
    {
        if (ImageDragging)
            ImageDragLeave();
        // pokusim se vytahnout z DataObjectu text
        FORMATETC formatEtc;
        ZeroMemory(&formatEtc, sizeof(formatEtc));
        formatEtc.cfFormat = UseUnicode ? CF_UNICODETEXT : CF_TEXT;
        formatEtc.dwAspect = DVASPECT_CONTENT;
        formatEtc.lindex = -1;
        formatEtc.tymed = TYMED_HGLOBAL;

        STGMEDIUM stgMedium;
        ZeroMemory(&stgMedium, sizeof(stgMedium));
        stgMedium.tymed = TYMED_HGLOBAL;

        if (pDataObject->GetData(&formatEtc, &stgMedium) == S_OK)
        {
            char* path = (char*)HANDLES(GlobalLock(stgMedium.hGlobal));
            if (path != NULL)
            {
                if (UseUnicode)
                    path = ConvertAllocU2A((const WCHAR*)path, -1);
                if (path != NULL)
                {
                    // zmenim cestu
                    lstrcpyn(Buffer, path, _countof(Buffer));

                    if (!IsPluginFSPath(Buffer))
                    {
                        int l = (int)strlen(Buffer);
                        if ((l != 2 || Buffer[0] != '\\' || Buffer[1] != '\\') && // nejde o cestu "\\\\" (Nethood root)
                            l > 0 && Buffer[l - 1] == '\\')
                            Buffer[--l] = 0;             // '\\' na konci neni vitan
                        if (l == 2 && Buffer[0] != '\\') // za neUNC root cestou musi byt '\\'
                        {
                            Buffer[l++] = '\\';
                            Buffer[l] = 0;
                        }
                        if (l == 6 && Buffer[0] == '\\' && Buffer[1] == '\\' && Buffer[2] == '.' && Buffer[3] == '\\' &&
                            Buffer[4] != 0 && Buffer[5] == ':') // za "\\.\C:\" root cestou musi byt '\\'
                        {
                            Buffer[l++] = '\\';
                            Buffer[l] = 0;
                        }
                    }

                    PostMessage(PanelWindow->HWindow, WM_USER_CHANGEDIR, TRUE, (LPARAM)Buffer);
                    if (UseUnicode)
                        free(path);
                }
                HANDLES(GlobalUnlock(stgMedium.hGlobal));
            }
        }
        else
        {
            char path[MAX_PATH];
            if (GetDirFromDataObject(pDataObject, path))
            {
                // zmenim cestu
                strcpy(Buffer, path);

                if (!IsPluginFSPath(Buffer))
                {
                    int l = (int)strlen(Buffer);
                    if (l > 0 && Buffer[l - 1] == '\\')
                        Buffer[--l] = 0;             // '\\' na konci neni vitan
                    if (l == 2 && Buffer[0] != '\\') // za neUNC root cestou musi byt '\\'
                    {
                        Buffer[l++] = '\\';
                        Buffer[l] = 0;
                    }
                }

                PostMessage(PanelWindow->HWindow, WM_USER_CHANGEDIR, FALSE, (LPARAM)Buffer);
            }
        }

        if (DataObject != NULL)
        {
            DataObject->Release();
            DataObject = NULL;
        }
        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }
};

void CStatusWindow::RegisterDragDrop()
{
    CALL_STACK_MESSAGE1("CStatusWindow::RegisterDragDrop()");
    CTextDropTarget* dropTarget = new CTextDropTarget(PanelWindow);
    if (dropTarget != NULL)
    {
        if (HANDLES(RegisterDragDrop(HWindow, dropTarget)) != S_OK)
        {
            TRACE_E("RegisterDragDrop error.");
        }
        else
            IDropTargetPtr = dropTarget;
        dropTarget->Release(); // RegisterDragDrop volala AddRef()
    }
}

void CStatusWindow::RevokeDragDrop()
{
    CALL_STACK_MESSAGE_NONE
    HANDLES(RevokeDragDrop(HWindow));
}

#define BUTTON_OFFSET 0

LRESULT CStatusWindow::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SLOW_CALL_STACK_MESSAGE4("CStatusWindow::WindowProc(0x%X, 0x%IX, 0x%IX)", uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_CREATE:
    {
    //Reset hot item.
        m_Hot_Item = nullptr;
        m_Hot_Item_Last = nullptr;
        m_Hot_Highlight_Filtered = false;
        m_Hot_Highlight_History = false;
        m_Hot_Highlight_Security = false;
        m_Hot_Highlight_Size = false;
        m_Hot_Highlight_Throbber = false;
        m_Hot_Highlight_Zoom = false;

        MouseCaptured = FALSE;

        if (Border & blTop)
        {
            ToolBar = new CMainToolBar(MainWindow->HWindow, Left ? mtbtLeft : mtbtRight);
            if (ToolBar == NULL)
            {
                TRACE_E(LOW_MEMORY);
                return -1;
            }
            ToggleToolBar();
            RegisterDragDrop();
        }

        if (ShowThrobber)
        {
            int ti = DelayedThrobberShowTime == 0 /* neplatna hodnota */ ? 0 : DelayedThrobberShowTime - GetTickCount();
            if (ti < 0)
                ti = 0;
            DelayedThrobberShowTime = 0; // tuto hodnotu uz nepotrebujeme, pripadne si ji musi nastavit SetThrobber znovu, priradime neplatnou hodnotu
            SetThrobber(TRUE, ti);       // mame take zapnout throbber
        }

        return 0;
    }

    case WM_DESTROY:
    {
        if (Throbber || DelayedThrobber)
            SetThrobber(FALSE, 0, TRUE); // potrebujeme sestrelit timer
        if (Border & blTop)
            RevokeDragDrop();
        if (ToolBar != NULL)
        {
            ToolBar->DetachWindow();
            delete ToolBar;
            ToolBar = NULL;
        }
        return 0;
    }

    case WM_SIZE:
    {
        RECT r;
        GetClientRect(HWindow, &r);

        if (ToolBar != NULL && ToolBar->HWindow != NULL)
        {
            ToolBarWidth = ToolBar->GetNeededWidth();
            SetWindowPos(ToolBar->HWindow, 0, 0, 0, ToolBarWidth, r.bottom, SWP_NOACTIVATE | SWP_NOZORDER);
        }
        if (Width != r.right || Height != r.bottom)
        {
            Width = r.right;
            Height = r.bottom;
            ItemBitmap.Enlarge(Width, Height); // alokace bitmapy v ItemBitmap.HMemDC
        }

        break;
    }

    case WM_USER_TTGETTEXT:
    {
        DWORD id = (DWORD)wParam; // FIXME_X64 - overit pretypovani na (DWORD)
        char* text = (char*)lParam;
        switch (id)
        {
        case 0:
        {
            break;
        }

        case 2:
        {
            lstrcpy(text, LoadStr(IDS_PANELFILTER));
            break;
        }

        case 3:
        {
            text[0] = 0;
            ExpandPluralFilesDirs(text, 200, HiddenFilesCount, HiddenDirsCount, epfdmHidden, FALSE);
            //          lstrcat(text, " ");
            //          CQuadWord qwHidden(HiddenFilesCount + HiddenDirsCount, 0);
            //          ExpandPluralString(text + strlen(text), 100, LoadStr(IDS_PLURAL_SWITCH_HIDDEN),
            //                             1, &qwHidden);
            break;
        }

        case 4:
        {
            char* str;
            if (Border == blTop && WholeTextVisible)
                str = LoadStr(IDS_TRIM_DRAG_PATH);
            else if (Border == blBottom && WholeTextVisible)
                str = LoadStr(IDS_COPY_DRAG_TEXT);
            else
                str = Text;
            if (str == NULL)
                text[0] = 0;
            else
                lstrcpy(text, str);
            break;
        }

        case 5:
        {
            lstrcpy(text, LoadStr(IDS_DIRHISTORY));
            break;
        }

        case 6:
        {
            lstrcpy(text, LoadStr(IDS_FREESPACE));
            break;
        }

        case 7:
        {
            lstrcpy(text, LoadStr(IDS_ZOOMPANEL));
            break;
        }

        case 8:
        {
            lstrcpyn(text, ThrobberTooltip != NULL ? ThrobberTooltip : "", TOOLTIP_TEXT_MAX);
            break;
        }

        case 9:
        {
            lstrcpyn(text, SecurityTooltip != NULL ? SecurityTooltip : "", TOOLTIP_TEXT_MAX);
            break;
        }

        default:
            TRACE_E("Unknown ID:" << id);
            break;
        }
        return 0;
    }

    case WM_MOUSEMOVE:
    {
    //Is mouse inside icon,... position?
        const short     xPos = LOWORD( lParam );
        const short     yPos = HIWORD( lParam );

        auto      isMouseInside_filtered = m_Rect_Filtered.IsMouseInside( xPos, yPos );
        auto      isMouseInside_history = ( History ) ? m_Rect_History.IsMouseInside( xPos, yPos ) : false;
        auto      isMouseInside_security = m_Rect_Security.IsMouseInside( xPos, yPos );
        auto      isMouseInside_size = m_Rect_Size.IsMouseInside( xPos, yPos );
        auto      isMouseInside_text = m_Rect_Text.IsMouseInside( xPos, yPos );
        auto      isMouseInside_throbber = m_Rect_Throbber.IsMouseInside( xPos, yPos );
        auto      isMouseInside_zoom = m_Rect_Zoom.IsMouseInside( xPos, yPos );

    //Set tooltip.
        //Get tooltip id.
        DWORD    id_toolTip = 0;

        if ( isMouseInside_filtered ) id_toolTip = 3;
        else if ( isMouseInside_history ) id_toolTip = 5;
        else if ( isMouseInside_security ) id_toolTip = 9;
        else if ( isMouseInside_size ) id_toolTip = 6;
        else if ( isMouseInside_text ) id_toolTip = 4;
        else if ( isMouseInside_throbber ) id_toolTip = 8;
        else if ( isMouseInside_zoom ) id_toolTip = 7;

        if (wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON))
            id_toolTip = 0;

        //Set it.
        SetCurrentToolTip( HWindow, id_toolTip );

    // I'll fix the tearing of the text and the start of the drag&drop
        if (
            MouseCaptured
            &&
            ( LButtonDown || MButtonDown || RButtonDown )
            &&
            ( HotTrackItems.Count > 0 )
        )
        {
            int x = abs(LButtonDownPoint.x - (short)LOWORD(lParam));
            int y = abs(LButtonDownPoint.y - (short)HIWORD(lParam));
            if (x > GetSystemMetrics(SM_CXDRAG) || y > GetSystemMetrics(SM_CYDRAG))
            {
                int index;
                if (FindHotTrackItem(LButtonDownPoint.x - m_Rect_Text.left, index))
                {
                    char buffer[MAX_PATH];
                    int hotChars = HotTrackItems[index].Chars;
                    if (hotChars + 1 > MAX_PATH)
                        hotChars = MAX_PATH - 1;
                    lstrcpyn(buffer, Text + HotTrackItems[index].Offset, hotChars + 1);

                    // u Directory Line s pluginovym FS je jeste potreba umoznit pluginu posledni upravy cesty (pridani ']' u VMS cest u FTP)
                    if ((Border & blTop) && PanelWindow->Is(ptPluginFS) && PanelWindow->GetPluginFS()->NotEmpty())
                    {
                        PanelWindow->GetPluginFS()->CompleteDirectoryLineHotPath(buffer, MAX_PATH);
                        PanelWindow->GetPluginFS()->GetPluginInterfaceForFS()->ConvertPathToExternal(PanelWindow->GetPluginFS()->GetPluginFSName(),
                                                                                                     PanelWindow->GetPluginFS()->GetPluginFSNameIndex(),
                                                                                                     strchr(buffer, ':') + 1);
                        hotChars = (int)strlen(buffer);
                    }

                    WindowProc(WM_MOUSELEAVE, 0, 0);
                    MouseCaptured = FALSE;
                    LButtonDown = FALSE;
                    MButtonDown = FALSE;
                    RButtonDown = FALSE;

                    HGLOBAL h = NOHANDLES(GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, hotChars + 1));

                    if (h != NULL)
                    {
                        char* s = (char*)HANDLES(GlobalLock(h));
                        if (s != NULL)
                        {
                            memcpy(s, buffer, hotChars + 1);
                            HANDLES(GlobalUnlock(h));
                        }

                        CImpIDropSource* dropSource = new CImpIDropSource(FALSE);
                        IDataObject* dataObject = new CTextDataObject(h);
                        if (IDropTargetPtr != NULL)
                            ((CTextDropTarget*)IDropTargetPtr)->SetForbiddenDataObject(dataObject);
                        if (dataObject != NULL && dropSource != NULL)
                        {
                            DWORD dwEffect;

                            HIMAGELIST hDragIL = NULL;
                            int dxHotspot, dyHotspot;
                            int imgWidth, imgHeight;
                            hDragIL = CreateDragImage(buffer, dxHotspot, dyHotspot, imgWidth, imgHeight);
                            ImageList_BeginDrag(hDragIL, 0, dxHotspot, dyHotspot);
                            ImageDragBegin(imgWidth, imgHeight, dxHotspot, dyHotspot);

                            DoDragDrop(dataObject, dropSource, DROPEFFECT_COPY, &dwEffect);

                            ImageDragEnd();
                            ImageList_EndDrag();
                            ImageList_Destroy(hDragIL);

                            isMouseInside_filtered = false;
                            isMouseInside_history = false;
                            isMouseInside_security = false;
                            isMouseInside_size = false;
                            isMouseInside_text = false;
                            isMouseInside_throbber = false;
                            isMouseInside_zoom = false;
                        }
                        if (IDropTargetPtr != NULL)
                            ((CTextDropTarget*)IDropTargetPtr)->SetForbiddenDataObject(NULL);
                        if (dataObject != NULL)
                            dataObject->Release();
                        if (dropSource != NULL)
                            dropSource->Release();
                    }
                }
            }
        }

        BOOL repaint = FALSE;

    //Set capture mode.
        if ( MouseCaptured )
        {
        //Already in capture mode -> has mouse left the status area?
            //Get current mouse position.
            POINT   point_current;

            GetCursorPos( &point_current );

            //Has mouse left the status area?
            if (
                (
                    ( isMouseInside_filtered == false )
                    &&
                    ( isMouseInside_history == false )
                    &&
                    ( isMouseInside_security == false )
                    &&
                    ( isMouseInside_size == false )
                    &&
                    ( isMouseInside_text == false )
                   //&&
                   //( isMouseInside_throbber == false )    Is it correct that throbber is not part of this and any of the code bellow? Why?
                    &&
                    ( isMouseInside_zoom == false )
                )
                || 
                ( WindowFromPoint( point_current ) != HWindow )
            )
            {
            //Yes -> signal the mouse has left.
                WindowProc( WM_MOUSELEAVE, 0, 0 );

                MouseCaptured = FALSE;
                repaint = TRUE;
            }
        }
        else if (
            isMouseInside_filtered
            ||
            isMouseInside_history
            ||
            isMouseInside_security
            ||
            isMouseInside_size
            ||
            isMouseInside_text
            ||
            isMouseInside_throbber
            ||
            isMouseInside_zoom
        )
        {
        //Not in capture mode -> start it now.
            TRACKMOUSEEVENT     tme;

            tme.cbSize = sizeof( tme );
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = HWindow;
            TrackMouseEvent( &tme );

            MouseCaptured = TRUE;
            repaint = TRUE;
        }

    //Update highlight state/item.
        CHotTrackItem*      newHot_Item = nullptr;
        bool                newHot_highlight_filtered = false;
        bool                newHot_highlight_history = false;
        bool                newHot_highlight_security = false;
        bool                newHot_highlight_size = false;
        bool                newHot_highlight_throbber = false;
        bool                newHot_highlight_zoom = false;

        if ( MouseCaptured )
        {
            if ( isMouseInside_text )
            {
                int index;
                if (FindHotTrackItem(xPos - m_Rect_Text.left, index))
                {
                //Highlight text from start to word.
                    newHot_Item = &HotTrackItems[index];

                //Set single click cursor.
                    if (Configuration.SingleClick)
                        SetHandCursor();
                }
                else
                {
                //Set single click cursor.
                    if (Configuration.SingleClick)
                        SetCursor(LoadCursor(NULL, IDC_ARROW));
                }
            }
            else if (isMouseInside_history)
            {
            //Highlight history.
                newHot_highlight_history = true;

            //Set single click cursor.
                if (Configuration.SingleClick)
                    SetHandCursor();
            }
            else if (isMouseInside_zoom)
            {
            //Highlight zoom.
                newHot_highlight_zoom = true;

            //Set single click cursor.
                if (Configuration.SingleClick)
                    SetHandCursor();
            }
            else if (isMouseInside_filtered)
            {
            //Highlight hidden.
                newHot_highlight_filtered = true;

            //Set single click cursor.
                if (Configuration.SingleClick)
                    SetHandCursor();
            }
            else if (isMouseInside_size)
            {
                // drive-info funguje jen pokud nejde o FS, ktery drive-info nepodporuje
                if (
                    PanelWindow->Is(ptDisk)
                    ||
                    PanelWindow->Is(ptZIPArchive)
                    ||
                    (
                        PanelWindow->Is(ptPluginFS)
                        &&
                        PanelWindow->GetPluginFS()->NotEmpty()
                        &&
                        PanelWindow->GetPluginFS()->IsServiceSupported( FS_SERVICE_SHOWINFO )
                    )
                )
                {
                //Highlight size.
                    newHot_highlight_size = true;

                //Set single click cursor.
                    if (Configuration.SingleClick)
                        SetHandCursor();
                }
            }
            else if (isMouseInside_security)
            {
                if (
                    PanelWindow->Is( ptPluginFS )
                    &&
                    PanelWindow->GetPluginFS()->NotEmpty()
                    &&
                    PanelWindow->GetPluginFS()->IsServiceSupported( FS_SERVICE_SHOWSECURITYINFO )
                )
                {
                //Highlight security.
                    newHot_highlight_security = true;

                //Set single click cursor.
                    if (Configuration.SingleClick)
                        SetHandCursor();
                }
            }
            else if (isMouseInside_throbber)
            {
            //Highlight throbber.
                newHot_highlight_throbber = true;

            //Set single click cursor.
                if (Configuration.SingleClick)
                    SetHandCursor();
            }
        }
        if (
            ( repaint )
            ||
            ( newHot_Item != m_Hot_Item )
            ||
            ( newHot_highlight_history != m_Hot_Highlight_History )
            ||
            ( newHot_highlight_filtered != m_Hot_Highlight_Filtered )
            ||
            ( newHot_highlight_security != m_Hot_Highlight_Security )
            ||
            ( newHot_highlight_size != m_Hot_Highlight_Size )
            ||
            ( newHot_highlight_throbber != m_Hot_Highlight_Throbber )
            ||
            ( newHot_highlight_zoom != m_Hot_Highlight_Zoom )
        )
        {
        //Update selected item.
            m_Hot_Item = newHot_Item;

            if (m_Hot_Item != nullptr)
                m_Hot_Item_Last = m_Hot_Item;

        //Set currently set highlight flag.
            m_Hot_Highlight_Filtered = newHot_highlight_filtered;
            m_Hot_Highlight_History = newHot_highlight_history;
            m_Hot_Highlight_Security = newHot_highlight_security;
            m_Hot_Highlight_Size = newHot_highlight_size;
            m_Hot_Highlight_Throbber = newHot_highlight_throbber;
            m_Hot_Highlight_Zoom = newHot_highlight_zoom;

        //Repaint.
            Repaint();
        }
        break;
    }
    case WM_MOUSEWHEEL:
    {
    //Change mouse position relative to client area.
        POINT   point = { .x = LOWORD( lParam ), .y = HIWORD( lParam ) };

        ScreenToClient( HWindow, &point );

    //Is mouse inside useful surface?
        const auto      isMouseInside_whole = m_Rect_Whole.IsMouseInside( point );

        if ( isMouseInside_whole == false )
        {
        //No -> ignore message.
            break;
        }

    //Is history menu on current panel opened?
        if ( PanelWindow->History_IsMenuShown() == false )
        {
        //No -> open it.
            //Close any other opened menu.
            //
            //Notice:
            //We must not block the messaging loop as currently opened menu wouldn't close in blocking situation.
            MenuQueue.Menus_CloseAll_NonBlocking();

            //Send open history message.
            //
            //Notice:
            //We have to wait for other messages to close, before opening current one.
            PostMessage( HWindow, WM_USER_OPEN_HISTORY, 0, 0 );
        }
        break;
    }
    case WM_USER_OPEN_HISTORY:
    {
    //Open history menu.
        if ( PanelWindow->History_IsMenuShown() == false )
        {
            PanelWindow->OpenDirHistory();
        }
        break;
    }

    case WM_SETCURSOR:
    {
        if ( MouseCaptured )
            return TRUE;
        break;
    }

    case WM_MOUSELEAVE:
    case WM_CANCELMODE:
    {
    //Stop showing tooltip.
        SetCurrentToolTip( NULL, 0 );

    //Clear mouse capture.
        if ( MouseCaptured )
        {
        //Release mouse capture.
            if (GetCapture() == HWindow)
                ReleaseCapture();

            MouseCaptured = FALSE;

        //Reset button states.
            LButtonDown = FALSE;
            MButtonDown = FALSE;
            RButtonDown = FALSE;

        //Set single click cursor.
            if (Configuration.SingleClick)
                SetCursor(LoadCursor(NULL, IDC_ARROW));
        }

    //Clear item highlight.
        if (
            ( m_Hot_Item != nullptr )
            ||
            m_Hot_Highlight_Size
            ||
            m_Hot_Highlight_History
            ||
            m_Hot_Highlight_Zoom
            ||
            m_Hot_Highlight_Filtered
            ||
            m_Hot_Highlight_Security
        )
        {
        //Reset hightlighted item.
            m_Hot_Item = nullptr;

            m_Hot_Highlight_History = false;
            m_Hot_Highlight_Size = false;
            m_Hot_Highlight_Zoom = false;
            m_Hot_Highlight_Filtered = false;
            m_Hot_Highlight_Security = false;

        //Repaint.
            Repaint();
        }
        break;
    }

    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    {
    //Cancel QuickSearch and QuickEdit.
        MainWindow->CancelPanelsUI();

    //Stop showing tooltip.
        SetCurrentToolTip( NULL, 0 );

    //[W: why is this executed before other stuff?]
    //Switch which panel is active.
        if (
            ( m_Hot_Highlight_History )
            &&
            ( MainWindow->GetActivePanel() != PanelWindow )
        )
        {
            MainWindow->ChangePanel();
        }

    //Capture mouse
        if ( MouseCaptured )
        {
        //Capture mouse.
            SetCapture( HWindow );

        //Store button state.
            LButtonDownPoint.x = LOWORD(lParam);
            LButtonDownPoint.y = HIWORD(lParam);

            LButtonDown = ( uMsg == WM_LBUTTONDOWN );
            RButtonDown = ( uMsg == WM_RBUTTONDOWN );

        //Was left button click?
            if (
                ( uMsg == WM_LBUTTONDOWN )
                ||
                ( uMsg == WM_LBUTTONDBLCLK )
            )
            {
            //What was clicked?
                if ( m_Hot_Highlight_Filtered )
                {
                //Show filter menu.
                    PanelWindow->OpenStopFilterMenu();
                }
                else if ( m_Hot_Highlight_History )
                {
                //Show history directory list.
                    PanelWindow->OpenDirHistory();
                }
            }

        //Show change directory dialog.
            if (
                ( uMsg == WM_LBUTTONDBLCLK )
                &&
                ( m_Hot_Item != nullptr )
                &&
                ( Border & blTop )
            )
            {
                SendMessage( MainWindow->HWindow, WM_COMMAND, MAKEWPARAM( CM_ACTIVE_CHANGEDIR, 0 ), 0 );
            }
        }
        else
        {
        //
            switch ( uMsg )
            {
            case WM_LBUTTONDOWN:
            case WM_LBUTTONDBLCLK:
            {
            //[W: why is this executed before other stuff?]
            //Switch which panel is active.
                if ( MainWindow->GetActivePanel() != PanelWindow )
                {
                    MainWindow->ChangePanel();
                }

            //Show change directory dialog.
                if (
                    ( uMsg == WM_LBUTTONDBLCLK )
                    &&
                    ( Border & blTop )
                )
                {
                    SendMessage( MainWindow->HWindow, WM_COMMAND, MAKEWPARAM( CM_ACTIVE_CHANGEDIR, 0 ), 0 );
                }
                break;
            }
            }
        }
        break;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    {
    //Stop showing tooltip.
        SetCurrentToolTip( NULL, 0 );

    //
        if (
            MouseCaptured
            &&
            ( uMsg == WM_LBUTTONUP )
            &&
            (
                ( m_Hot_Item != nullptr )
                ||
                m_Hot_Highlight_Size
                ||
                m_Hot_Highlight_Zoom
                ||
                m_Hot_Highlight_Security
            )
        )
        {
        //Release capture.
            if ( GetCapture() == HWindow )
            {
                ReleaseCapture();
            }

        //Was a drag & drop movement?
            int x = abs( LButtonDownPoint.x - (short)LOWORD(lParam) );
            int y = abs( LButtonDownPoint.y - (short)HIWORD(lParam) );

            if (
                ( x <= GetSystemMetrics( SM_CXDRAG ) )
                &&
                ( y <= GetSystemMetrics( SM_CYDRAG ) )
            )
            {
            //No -> execute command.
                if ( m_Hot_Item != nullptr )
                {
                //Text is highlighted -> ...
                    if (Border & blTop)
                    {
                        if (MainWindow->GetActivePanel() != PanelWindow)
                            MainWindow->ChangePanel();

                        CHotTrackItem* lastItem = NULL;
                        if (HotTrackItems.Count > 0)
                            lastItem = &HotTrackItems[HotTrackItems.Count - 1];
                        //if (m_Hot_Item->Chars != (int)TextLen) // tato podminka selhala pokud byl pripojen filtr
                        if (m_Hot_Item != lastItem)
                        {
                            // zkraceni cesty
                            char path[MAX_PATH];
                            strncpy(path, Text, m_Hot_Item->Chars);
                            path[m_Hot_Item->Chars] = 0;

                            if (PanelWindow->Is(ptPluginFS) && PanelWindow->GetPluginFS()->NotEmpty())
                                PanelWindow->GetPluginFS()->CompleteDirectoryLineHotPath(path, MAX_PATH);

                            PanelWindow->ChangeDir(path, -1, NULL, 2 /* jako back/forward in history*/, NULL, FALSE);
                        }
                        else
                        {
                            // klik na posledni komponentu -- change dir
                            SendMessage(MainWindow->HWindow, WM_COMMAND, MAKEWPARAM(CM_ACTIVE_CHANGEDIR, 0), 0);
                        }
                    }
                    if (Border & blBottom)
                    {
                        if (CopyTextToClipboard(Text + m_Hot_Item->Offset, m_Hot_Item->Chars))
                            FlashText(TRUE);
                    }
                }
                else if ( m_Hot_Highlight_Size )
                {
                //Show 'drive information' dialog.
                    PanelWindow->DriveInfo();
                }
                else if ( m_Hot_Highlight_Zoom )
                {
                //Zoom in/out panel.
                    SendMessage( MainWindow->HWindow, WM_COMMAND, MainWindow->LeftPanel == PanelWindow ? CM_LEFTZOOMPANEL : CM_RIGHTZOOMPANEL, 0 );

                //Redraw main window (and its children).
                    UpdateWindow( MainWindow->HWindow );
                }
                else if ( m_Hot_Highlight_Security )
                {
                    if (PanelWindow->Is(ptPluginFS) && PanelWindow->GetPluginFS()->NotEmpty())
                        PanelWindow->GetPluginFS()->ShowSecurityInfo(PanelWindow->HWindow);
                }
            }
        }

    //Clear button state.
        LButtonDown = FALSE;
        MButtonDown = FALSE;
        RButtonDown = FALSE;

    //???
        if (!MainWindow->HelpMode && GetActiveWindow() == NULL)
            SetForegroundWindow( MainWindow->HWindow );
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HANDLES(BeginPaint(HWindow, &ps));
        Paint(ps.hdc);
        HANDLES(EndPaint(HWindow, &ps));
        return 0;
    }

    case WM_ERASEBKGND:
    {
        HotTrackItemsMeasured = FALSE; // mohlo dojit ke zmene fontu - nechame znovu premerit
        RECT r;
        GetClientRect(HWindow, &r);
        if (Width != r.right || Height != r.bottom)
        {
            Width = r.right;
            Height = r.bottom;
            ItemBitmap.Enlarge(Width, Height); // alokace bitmapy v ItemBitmap.HMemDC
        }
        HDC dc = (HDC)wParam;
        if (Border != blNone)
        {
            HPEN oldPen = (HPEN)SelectObject(dc, BtnShadowPen);
            if (Border & blBottom)
            {
                MoveToEx(dc, r.left, r.top, NULL);
                LineTo(dc, r.left, r.bottom);
                SelectObject(dc, BtnHilightPen);
                MoveToEx(dc, r.right - 1, r.top, NULL);
                LineTo(dc, r.right - 1, r.bottom);
                MoveToEx(dc, r.left, r.bottom - 1, NULL);
                LineTo(dc, r.right, r.bottom - 1);
                r.bottom--;
            }
            SelectObject(dc, oldPen);
        }

        // j.r. vsechny statusbary tahat pres jednu cache CBitmap

        return TRUE;
    }

    case WM_TIMER:
    {
        if (wParam == IDT_THROBBER)
        {
            if (StopStatusbarRepaint == 0)
            {
            //Go to next frame.
                const int   nFrames = sizeof( SVGLoading_Active )/sizeof( SVGLoading_Active[0] );

                ThrobberFrameIndex++;

                if (ThrobberFrameIndex >= nFrames)
                {
                //Index is at the last frame -> reset it.
                    ThrobberFrameIndex = 0;
                }

                NeedToInvalidate = TRUE;
                InvalidateIfNeeded();
            }
            else
                PostStatusbarRepaint = TRUE;
        }
        if (wParam == IDT_DELAYEDTHROBBER)
        {
            if (DelayedThrobber)
                SetThrobber(TRUE);
            else
            {
                KillTimer(HWindow, IDT_DELAYEDTHROBBER);
                TRACE_E("CStatusWindow::WindowProc(): Unexpected timer: IDT_DELAYEDTHROBBER");
            }
        }
        break;
    }
    }

    return CWindow::WindowProc(uMsg, wParam, lParam);
}

HIMAGELIST
CStatusWindow::CreateDragImage(const char* text, int& dxHotspot, int& dyHotspot, int& imgWidth, int& imgHeight)
{
    CALL_STACK_MESSAGE6("CStatusWindow::CreateDragImage(%s, %d, %d, %d, %d)",
                        text, dxHotspot, dyHotspot, imgWidth, imgHeight);
    int textLen = lstrlen(text);
    HDC hDC = ItemBitmap.HMemDC;
    HFONT hOldFont = (HFONT)SelectObject(hDC, Font);
    SIZE sz;
    GetTextExtentPoint32(hDC, text, textLen, &sz);
    ItemBitmap.Enlarge(sz.cx, sz.cy); // alokace bitmapy v ItemBitmap.HMemDC
    // podmazu pozadi
    RECT r;
    r.left = 0;
    r.top = 0;
    r.right = sz.cx;
    r.bottom = sz.cy;
    FillRect(hDC, &r, HNormalBkBrush);
    int oldBkMode = SetBkMode(hDC, TRANSPARENT);
    int oldTextColor = SetTextColor(hDC, GetCOLORREF(CurrentColors[ITEM_FG_NORMAL]));
    DrawText(hDC, text, textLen, &r, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
    SetTextColor(hDC, oldTextColor);
    SetBkMode(hDC, oldBkMode);
    SelectObject(hDC, hOldFont);

    dxHotspot = -15;
    dyHotspot = 0;

    imgWidth = sz.cx;
    imgHeight = sz.cy;
    HIMAGELIST himl = ImageList_Create(sz.cx, sz.cy, ILC_COLORDDB | ILC_MASK, 1, 0);
    SelectObject(ItemBitmap.HMemDC, ItemBitmap.HOldBmp); // na chvilku pustime bitmapu z HMemDC
    ImageList_AddMasked(himl, ItemBitmap.HBmp, GetCOLORREF(CurrentColors[ITEM_BK_NORMAL]));
    SelectObject(ItemBitmap.HMemDC, ItemBitmap.HBmp); // zase ji selectneme
    return himl;
}

BOOL CStatusWindow::GetRect(RECT* r)
{
    CALL_STACK_MESSAGE_NONE
    if (HWindow == NULL)
        return FALSE;
    GetWindowRect(HWindow, r);
    return TRUE;
}
BOOL CStatusWindow::GetRect_TextFrame(RECT* r)
{
    CALL_STACK_MESSAGE_NONE
    if (HWindow == NULL)
        return FALSE;
    GetWindowRect(HWindow, r);
    //  r->left += TextRect.left - 2;
    r->top += 2;
    r->bottom -= 2;
    return TRUE;
}
BOOL CStatusWindow::GetRect_FilterFrame(RECT* r)
{
    CALL_STACK_MESSAGE_NONE
    if (HWindow == NULL)
        return FALSE;

    if (!Hidden)
        return GetRect_TextFrame(r);

    *r = m_Rect_Filtered;
    MapWindowPoints(HWindow, NULL, (POINT*)r, 2);
    return TRUE;
}

void CStatusWindow::OnColorsChanged()
{
    if (ToolBar != NULL)
        ToolBar->OnColorsChanged();
}

void CStatusWindow::SetFont()
{
    // mohlo dojit ke zmene velikosti fontu
    InvalidateRect(HWindow, NULL, TRUE);
    BuildHotTrackItems();
}
