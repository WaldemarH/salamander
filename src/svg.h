// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

struct NSVGrasterizer;
struct NSVGimage;
void RenderSVGImage(NSVGrasterizer* rast, HDC hDC, int x, int y, const char* svgName, int iconSize, COLORREF bkColor, BOOL enabled);

// vraci SysColor ve formatu pro SVG knihovnu (BGR misto Win32 RGB)
DWORD GetSVGSysColor_Sys(int index);

//*****************************************************************************
//
// CSVGSprite
//

#define SVGSTATE_ORIGINAL 0x0001 // the original SVG form is unchanged
#define SVGSTATE_ENABLED_OR_NORMAL 0x0002  // SVG colored in the 'enabled text' or 'normal' color
#define SVGSTATE_DISABLED_OR_FOCUSED 0x0004 // SVG colored in 'disabled text' or 'focused' color
#define SVGSTATE_COUNT 3

// Objekt slouzi k vykresleni SVG prostrednictvi cachovaci bitmapy.
// Primarne drzi barevnou verzi obrazku vyrenderovanou podle barev ve zdrojovem SVG.
// Dale dokaze drzet barevne verze bitmapy (odtud Sprite v nazvu - vnitrne pouziva vetsi bitmapu s vice obrazky),
// napriklad "disabled", "active", "selected".
class CSVGSprite
{
public:
    CSVGSprite();
    ~CSVGSprite();

    // zahodi bitmapu, inicializuje promenne na vychozi stav
    void Clean();

    // 'states' je kombinace bitu z rodiny SVGSTATE_*
    BOOL Load(int resID, int width, int height, DWORD states, const int pColorIds[SVGSTATE_COUNT] = NULL, std::string replaceParameter = "" );

    void GetSize(SIZE* s);
    int GetWidth();
    int GetHeight();

    // 'hDC' je cilove DC, kam se ma bitmapa vykreslit
    // 'x' a 'y' jsou cilove souradnice v 'hDC'
    // 'width' a 'height' je cilovy rozmer; pokud jsou -1, pouzije se rozmer 'Width'/'Height'
    void AlphaBlend(HDC hDC, int x, int y, int width, int height, DWORD state);

protected:
    // nacte resource do pameti, naalokuje buffer o bajt delsi a terminuje resource nulou
    // pri uspechu vraci ukazatel na alokovanou pamet (je treba uvolnit), pri chybe vraci NULL
    std::string LoadSVGResource(int resID);

    // Vstupni 'sz' urcuje velikost v bodech, do ktere se ma SVG po prevodu na bitmapu vepsat.
    // Pokud je jeden rozmer -1, neni urcen a dopocita se na zaklade zachovani pomeru stran.
    // Pokud nejsou urceny oba rozmery, prevezmou se ze zdrojovych dat.
    // Na vystupu se vrati velikost vystupni bitmapy v bodech.
    void GetScaleAndSize(const NSVGimage* image, const SIZE* sz, float* scale, int* width, int* height);

    // vytvori DIB o velikosti 'width' a 'height', vraci jeho handle a ukazatel na data
    void CreateDIB(int width, int height, HBITMAP* hMemBmp, void** lpMemBits);

    // natonuje SVG 'image' do barvy urcene stavem 'state'
    void ColorizeSVG(NSVGimage* image, DWORD state, int colorId = -1);

protected:
    int Width; // rozmer jednoho obrazku v bodech
    int Height;
    HBITMAP HBitmaps[SVGSTATE_COUNT];
};

//*****************************************************************************
//
// global variables
//

//extern HBITMAP HArrowRight;         // bitmapa vytvorena z SVG, pouzivame pro tlacitka jako sipku vpravo
//extern SIZE ArrowRightSize;         // rozmery v bodech
//HBITMAP HArrowRight = NULL;
//SIZE ArrowRightSize = { 0 };

extern CSVGSprite SVGArrowRight;
extern CSVGSprite SVGArrowRightSmall;
extern CSVGSprite SVGArrowMore;
extern CSVGSprite SVGArrowLess;
extern CSVGSprite SVGArrowDropDown_Buttons;
extern CSVGSprite SVGFilter_Active;
extern CSVGSprite SVGFilter_Inactive;
extern CSVGSprite SVGHistory_Active;
extern CSVGSprite SVGHistory_Inactive;
extern CSVGSprite SVGLoading_Active[40];
extern CSVGSprite SVGLoading_Inactive[40];
extern CSVGSprite SVGSecurity_Locked_Active;
extern CSVGSprite SVGSecurity_Locked_Inactive;
extern CSVGSprite SVGSecurity_Unlocked_Active;
extern CSVGSprite SVGSecurity_Unlocked_Inactive;
extern CSVGSprite SVGZoom_In_Active;
extern CSVGSprite SVGZoom_In_Inactive;
extern CSVGSprite SVGZoom_Out_Active;
extern CSVGSprite SVGZoom_Out_Inactive;
