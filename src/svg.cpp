// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"

#include "svg.h"

#define NANOSVG_IMPLEMENTATION
#include "nanosvg\nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvg\nanosvgrast.h"

CSVGSprite SVGArrowRight;
CSVGSprite SVGArrowRightSmall;
CSVGSprite SVGArrowMore;
CSVGSprite SVGArrowLess;
CSVGSprite SVGArrowDropDown_Buttons;
CSVGSprite SVGFilter_Active;
CSVGSprite SVGFilter_Inactive;
CSVGSprite SVGHistory_Active;
CSVGSprite SVGHistory_Inactive;
CSVGSprite SVGLoading_Active[40];       //40 frames (has to divide 1000 and 360 values)
CSVGSprite SVGLoading_Inactive[40];     //40 frames (has to divide 1000 and 360 values)
CSVGSprite SVGSecurity_Locked_Active;
CSVGSprite SVGSecurity_Locked_Inactive;
CSVGSprite SVGSecurity_Unlocked_Active;
CSVGSprite SVGSecurity_Unlocked_Inactive;
CSVGSprite SVGZoom_In_Active;
CSVGSprite SVGZoom_In_Inactive;
CSVGSprite SVGZoom_Out_Active;
CSVGSprite SVGZoom_Out_Inactive;

// alternativa: http://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
// (asi by se nasla i pro kratsi verze)
//
// nasledujici reseni ma vyhodu, ze pro konstanty bude napocitano v ramci precompileru
// LOG2_k(n) returns floor(log2(n)) and is valid for values 0 <= n < 1 << k
#define LOG2_2(n) ((n)&0x2 ? 1 : 0)
#define LOG2_4(n) ((n)&0xC ? 2 + LOG2_2((n) >> 2) : LOG2_2(n))
#define LOG2_8(n) ((n)&0xF0 ? 4 + LOG2_4((n) >> 4) : LOG2_4(n))
#define LOG2_16(n) ((n)&0xFF00 ? 8 + LOG2_8((n) >> 8) : LOG2_8(n))
#define LOG2_32(n) ((n)&0xFFFF0000 ? 16 + LOG2_16((n) >> 16) : LOG2_16(n))
#define LOG2_64(n) ((n)&0xFFFFFFFF00000000 ? 32 + LOG2_32((n) >> 32) : LOG2_32(n))

//__popcnt16, __popcnt, __popcnt64
//https://msdn.microsoft.com/en-us/library/bb385231(v=vs.100).aspx

DWORD GetSVGSysColor_App(int colorId)
{
    return ( 0xFF000000 | GetCOLORREF(CurrentColors[colorId]) );
}
DWORD GetSVGSysColor_Sys(int index)
{
    //DWORD color = GetSysColor(index);
    //DWORD ret = 0xFF000000;
    //ret |= GetBValue(color) << 16;
    //ret |= GetGValue(color) << 8;
    //ret |= GetRValue(color);
    //return ret;

    return ( 0xFF000000 | ( 0x00FFFFFF & GetSysColor(index) ) );
}

//*****************************************************************************
//
// RenderSVGImage
//

char* ReadSVGFile(const char* fileName)
{
    char* buff = NULL;
    HANDLE hFile = HANDLES_Q(CreateFile(fileName, GENERIC_READ,
                                        FILE_SHARE_READ, NULL,
                                        OPEN_EXISTING,
                                        FILE_FLAG_SEQUENTIAL_SCAN,
                                        NULL));
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD size = GetFileSize(hFile, NULL);
        if (size != INVALID_FILE_SIZE)
        {
            buff = (char*)malloc(size + 1);
            DWORD read;
            if (ReadFile(hFile, buff, size, &read, NULL) && read == size)
            {
                buff[size] = 0;
            }
            else
            {
                TRACE_E("ReadSVGFile(): ReadFile() failed on " << fileName);
                free(buff);
                buff = NULL;
            }
        }
        else
        {
            TRACE_E("ReadSVGFile(): GetFileSize() failed on " << fileName);
        }
        HANDLES(CloseHandle(hFile));
    }
    else
    {
        TRACE_I("ReadSVGFile(): cannot open SVG file " << fileName);
    }
    return buff;
}

// render icons for which we have SVG representation
void RenderSVGImage(NSVGrasterizer* rast, HDC hDC, int x, int y, const char* svgName, int iconSize, COLORREF bkColor, BOOL enabled)
{
    char svgFile[2 * MAX_PATH];
    GetModuleFileName(NULL, svgFile, _countof(svgFile));
    char* s = strrchr(svgFile, '\\');
    if (s != NULL)
        sprintf(s + 1, "toolbars\\%s.svg", svgName);
    char* svg = ReadSVGFile(svgFile);
    if (svg != NULL)
    {
        HDC hMemDC = HANDLES(CreateCompatibleDC(NULL));
        BITMAPINFOHEADER bmhdr;
        memset(&bmhdr, 0, sizeof(bmhdr));
        bmhdr.biSize = sizeof(bmhdr);
        bmhdr.biWidth = iconSize;
        bmhdr.biHeight = -iconSize;
        if (bmhdr.biHeight == 0)
            bmhdr.biHeight = -1;
        bmhdr.biPlanes = 1;
        bmhdr.biBitCount = 32;
        bmhdr.biCompression = BI_RGB;
        void* lpMemBits = NULL;
        HBITMAP hMemBmp = HANDLES(CreateDIBSection(hMemDC, (CONST BITMAPINFO*)&bmhdr, DIB_RGB_COLORS, &lpMemBits, NULL, 0));
        SelectObject(hMemDC, hMemBmp);

        RECT r;
        r.left = x;
        r.top = y;
        r.right = x + iconSize;
        r.bottom = y + iconSize;
        SetBkColor(hDC, bkColor);
        ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &r, "", 0, NULL);

        float sysDPIScale = (float)GetScaleForSystemDPI();
        NSVGimage* image = nsvgParse(svg, "px", sysDPIScale);

        if (!enabled)
        {
            DWORD disabledColor = GetSVGSysColor_Sys(COLOR_BTNSHADOW); // JRYFIXME - prvotni nastrel, kde budeme brat disabled barvu?
            NSVGshape* shape = image->shapes;
            while (shape != NULL)
            {
                if ((shape->fill.color & 0x00FFFFFF) != 0x00FFFFFF)
                    shape->fill.color = disabledColor;
                shape = shape->next;
            }
        }

        float scale = sysDPIScale / 100;
        nsvgRasterize(rast, image, 0, 0, scale, (BYTE*)lpMemBits, iconSize, iconSize, iconSize * 4);
        nsvgDelete(image);

        BLENDFUNCTION bf;
        bf.BlendOp = AC_SRC_OVER;
        bf.BlendFlags = 0;
        bf.SourceConstantAlpha = 0xff; // want to use per-pixel alpha values
        bf.AlphaFormat = AC_SRC_ALPHA;
        AlphaBlend(hDC, x, y, iconSize, iconSize, hMemDC, 0, 0, iconSize, iconSize, bf);

        HANDLES(DeleteObject(hMemBmp));
        HANDLES(DeleteDC(hMemDC));

        free(svg);
    }
}

//*****************************************************************************
//
// CSVGSprite
//

CSVGSprite::CSVGSprite()
{
    for (int i = 0; i < SVGSTATE_COUNT; i++)
        HBitmaps[i] = NULL;
    Clean();
}

CSVGSprite::~CSVGSprite()
{
    Clean();
}

void CSVGSprite::Clean()
{
    for (int i = 0; i < SVGSTATE_COUNT; i++)
    {
        if (HBitmaps[i] != NULL)
        {
            HANDLES(DeleteObject(HBitmaps[i]));
            HBitmaps[i] = NULL;
        }
    }
    Width = -1;
    Height = -1;
}

std::string CSVGSprite::LoadSVGResource(int resID)
{
    std::string     svg;

    HRSRC hRsrc = FindResource(HInstance, MAKEINTRESOURCE(resID), RT_RCDATA);
    if ( hRsrc != NULL )
    {
        char* rawSVG = (char*)LoadResource(HInstance, hRsrc);
        if (rawSVG != NULL)
        {
            DWORD size = SizeofResource(HInstance, hRsrc);
            if (size > 0)
            {
                svg.append( rawSVG, size );

                ////NSVGimage* image = NULL;
                ////NSVGrasterizer* rast = NULL;
                //
                //char* terminatedSVG = (char*)malloc(size + 1);
                //memcpy(terminatedSVG, rawSVG, size);
                //terminatedSVG[size] = 0;
                //ret = terminatedSVG;
            }
            else
            {
                TRACE_E("LoadSVGResource() Invalid resource data! resID=" << resID);
            }
        }
        else
        {
            TRACE_E("LoadSVGResource() Cannot load resource! resID=" << resID);
        }
    }
    else
    {
        TRACE_E("LoadSVGResource() Resource not found! resID=" << resID);
    }
    return svg;
}

void CSVGSprite::GetScaleAndSize(const NSVGimage* image, const SIZE* sz, float* scale, int* width, int* height)
{
    if (sz->cx != -1 || sz->cy != -1)
    {
        float scaleX, scaleY;
        if (sz->cx != -1)
            scaleX = sz->cx / image->width;
        if (sz->cy != -1)
            scaleY = sz->cy / image->height;
        if (sz->cx == -1)
        {
            *scale = scaleY;
            *height = sz->cy;
            *width = (int)(image->width * *scale);
        }
        else
        {
            if (sz->cy == -1)
            {
                *scale = scaleX;
                *width = sz->cx;
                *height = (int)(image->height * *scale);
            }
            else
            {
                *scale = min(scaleX, scaleY);
                *width = (int)(image->width * *scale);
                *height = (int)(image->height * *scale);
            }
        }
    }
    else
    {
        *scale = (float)GetScaleForSystemDPI() / 100;
        *width = (int)(image->width * *scale);
        *height = (int)(image->height * *scale);
    }
}
/*
HBITMAP
CSVGSprite::LoadSVGToBitmap(int resID, SIZE *sz)
{
  if (sz == NULL)
    TRACE_C("LoadSVGToBitmap(): invalid parameters!");

  HBITMAP hMemBmp = NULL;

  char *terminatedSVG = LoadSVGResource(resID);
  if (terminatedSVG != NULL)
  {
    NSVGimage *image = NULL;
    image = nsvgParse(terminatedSVG, "px", (float)GetSystemDPI());
    free(terminatedSVG);

    float scale;
    int w, h;
    GetScaleAndSize(image, sz, &scale, &w, &h);

    NSVGrasterizer *rast = NULL;
    rast = nsvgCreateRasterizer();

    HDC hMemDC = HANDLES(CreateCompatibleDC(NULL));
    BITMAPINFOHEADER bmhdr;
    memset(&bmhdr, 0, sizeof(bmhdr));
    bmhdr.biSize = sizeof(bmhdr);
    bmhdr.biWidth = w;
    bmhdr.biHeight = -h;
    if (bmhdr.biHeight == 0) bmhdr.biHeight = -1;
    bmhdr.biPlanes = 1;
    bmhdr.biBitCount = 32;
    bmhdr.biCompression = BI_RGB;
    void *lpMemBits = NULL;
    hMemBmp = HANDLES(CreateDIBSection(hMemDC, (CONST BITMAPINFO *)&bmhdr, DIB_RGB_COLORS, &lpMemBits, NULL, 0));
    HANDLES(DeleteDC(hMemDC));

    nsvgRasterize(rast, image, 0, 0, scale, (BYTE*)lpMemBits, w, h, w * 4);

    sz->cx = w;
    sz->cy = h;

    nsvgDeleteRasterizer(rast);
    nsvgDelete(image);
  }
  return hMemBmp;
}
*/
void CSVGSprite::CreateDIB(int width, int height, HBITMAP* hMemBmp, void** lpMemBits)
{
    HDC hMemDC = HANDLES(CreateCompatibleDC(NULL));
    BITMAPINFOHEADER bmhdr;
    memset(&bmhdr, 0, sizeof(bmhdr));
    bmhdr.biSize = sizeof(bmhdr);
    bmhdr.biWidth = width;
    bmhdr.biHeight = -height;
    if (bmhdr.biHeight == 0)
        bmhdr.biHeight = -1;
    bmhdr.biPlanes = 1;
    bmhdr.biBitCount = 32;
    bmhdr.biCompression = BI_RGB;
    *hMemBmp = HANDLES(CreateDIBSection(hMemDC, (CONST BITMAPINFO*)&bmhdr, DIB_RGB_COLORS, lpMemBits, NULL, 0));
    HANDLES(DeleteDC(hMemDC));
}

void CSVGSprite::ColorizeSVG(NSVGimage* image, DWORD state, int colorId )
{
//Use original color?
    if (state == SVGSTATE_ORIGINAL)
        return;

//Set color.
    DWORD color = 0;
    BOOL defineSystemColor = TRUE;

    if ( colorId >= 0 )
    {
    //Use user defined color.
        const auto color_full = CurrentColors[colorId];
        const auto flags = GetFValue( color_full );
        
        if ( flags & SCF_NOCOLOR )
        {
        //Don't change the color -> use original icon.
            return;
        }
        else if ( ( flags & SCF_DEFAULT ) == 0 )
        {
        //Don't use default colors -> use custom color.
            color = GetSVGSysColor_App(colorId);

            defineSystemColor = FALSE;
        }
    }

    //Was user's color defined?
    if ( defineSystemColor == TRUE )
    {
    //No -> use default system colors.
        int sysIndex;

        switch (state)
        {
        case SVGSTATE_ENABLED_OR_NORMAL:
            sysIndex = COLOR_BTNTEXT;
            break;

        case SVGSTATE_DISABLED_OR_FOCUSED:
            sysIndex = COLOR_BTNSHADOW;
            break;

        default:
            sysIndex = COLOR_BTNTEXT;
            TRACE_E("CSVGSprite::ColorizeSVG() unknown state=" << state);
        }

        color = GetSVGSysColor_Sys(sysIndex);
    }

//Set shape color.
    NSVGshape*  pShape = image->shapes;

    while ( pShape != NULL )
    {
    //Set shape color.
    //
    //Notice:
    //As we are using fill the shape must always be a path.
        pShape->fill.color = color;

    //Go to next shape.
        pShape = pShape->next;
    }
}

BOOL CSVGSprite::Load(int resID, int width, int height, DWORD states, const int pColorIds[SVGSTATE_COUNT], std::string replaceParameter )
{
    if (states == 0 || states >= (1 << SVGSTATE_COUNT))
    {
        TRACE_E("CSVGSprite::Load() wrong states combination: " << states);
        states |= SVGSTATE_ORIGINAL;
    }
    Clean();

    auto    svg = LoadSVGResource(resID);

    if ( svg.size() > 0 )
    {
    //Replace "{1}" text before parsing.
        if ( replaceParameter.size() > 0 )
        {
            auto    position = svg.find( "{1}", 0 );

            if ( position != std::string::npos )
            {
                svg.replace( position, sizeof("{1}")-1, replaceParameter );
            }
        }

    //Parse SVG.
        NSVGimage*  pImage = nsvgParse( svg.data(), "px", (float)GetSystemDPI() );

    //Get SVG image size in screen DPI.
        float   image_scale;
        SIZE    image_size = { width, height };

        GetScaleAndSize( pImage, &image_size, &image_scale, &Width, &Height );

    //Raster SVG in different state colors.
        NSVGrasterizer*     pRasterizer = nsvgCreateRasterizer();

        for ( int i = 0; i < SVGSTATE_COUNT; i++ )
        {
        //Is state bit flag defined?
            DWORD state = 1 << i;

            if ( !( states & state ) )
            {
            //No -> skip it.
                continue;
            }

        //Define shape color.
            ColorizeSVG(pImage, state, ( pColorIds != NULL ) ? pColorIds[i] : -1 );

        //Create DIB.
            void*   lpMemBits;

            CreateDIB( Width, Height, &HBitmaps[i], &lpMemBits );

        //Draw the image.
            nsvgRasterize( pRasterizer, pImage, 0, 0, image_scale, (BYTE*)lpMemBits, Width, Height, Width * 4 );
        }

    //Free resources.
        nsvgDeleteRasterizer( pRasterizer );
        nsvgDelete( pImage );
    }
    return TRUE;
}

void CSVGSprite::GetSize(SIZE* s)
{
    s->cx = Width;
    s->cy = Height;
}

int CSVGSprite::GetWidth()
{
    return Width;
}

int CSVGSprite::GetHeight()
{
    return Height;
}

void CSVGSprite::AlphaBlend(HDC hDC, int x, int y, int width, int height, DWORD state)
{
//Get icon index.
    int     index_icon = 0;

    switch ( state )
    {
    default:
    case SVGSTATE_ORIGINAL: index_icon = 0; break;
    case SVGSTATE_ENABLED_OR_NORMAL: index_icon = 1; break;
    case SVGSTATE_DISABLED_OR_FOCUSED:  index_icon = 2; break;
    }

//Get bitmap.
    HDC         hMemTmpDC = HANDLES(CreateCompatibleDC(hDC));
    HBITMAP     hOldBitmap = (HBITMAP)SelectObject(hMemTmpDC, HBitmaps[index_icon]);

//Draw icon.
    if (width == -1)
        width = Width;
    if (height == -1)
        height = Height;

    BLENDFUNCTION bf;
    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 0xff; // want to use per-pixel alpha values
    bf.AlphaFormat = AC_SRC_ALPHA;

    ::AlphaBlend(hDC, x, y, width, height, hMemTmpDC, 0, 0, Width, Height, bf);

    SelectObject( hMemTmpDC, hOldBitmap );

//Free resources.
    HANDLES( DeleteDC(hMemTmpDC) );
}
