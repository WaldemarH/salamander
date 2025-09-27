// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//Includes

//Class
    class Registry
    {
    //Defines
        #define REGISTRY_LIMIT_NAME_KEY         255
        #define REGISTRY_LIMIT_NAME_VALUE       16383

    //Noisy
        public: static bool Noisy_Key_Create( HWND hWnd_parent, const HKEY hKey, const String_TChar_View& name, HKEY& hKey_created );               //CreateKeyAux
        public: static bool Noisy_Key_Open( HWND hWnd_parent, const HKEY hKey, const String_TChar_View& name, HKEY& hKey_opened );                  //OpenKeyAux

        public: static bool Noisy_Size_Get( HWND hWnd_parent, HKEY hKey, const String_TChar_View& name, DWORD& size_inBytes );                      //GetSizeAux

    //Silent
        public: static void Silent_Key_Close( const HKEY hKey );                                                                                    //CloseKeyAux
        public: static bool Silent_Key_Create( const HKEY hKey, const String_TChar_View& name, HKEY& hKey_created, LSTATUS* status = nullptr );     //CreateKeyAux
        public: static bool Silent_Key_Delete( const HKEY hKey, const String_TChar_View& name, LSTATUS* status = nullptr );                         //DeleteKeyAux
        public: static bool Silent_Key_Delete_Branch( const HKEY hKey );                                                                            //ClearKeyAux
        public: static bool Silent_Key_Open( const HKEY hKey, const String_TChar_View& name, HKEY& hKey_opened, LSTATUS* status = nullptr );        //OpenKeyAux

        public: static bool Silent_Size_Get( HKEY hKey, const String_TChar_View& name, DWORD& size_inBytes, LSTATUS* status = nullptr );            //GetSizeAux

        public: static bool Silent_Value_Delete( HKEY hKey, const String_TChar_View& name );                                                        //DeleteValueAux

        public: static bool Silent_Value_Get( HKEY hKey, const String_TChar_View& name, BYTE* buffer, DWORD buffer_size );
        public: static bool Silent_Value_Get( HKEY hKey, const String_TChar_View& name, String_TChar& text );                                       //Reallocates text if needed.  GeValueAux, replaces SalRegQueryValue and SalRegQueryValueEx
        public: static bool Silent_Value_Get( HKEY hKey, const String_TChar_View& name, DWORD& value );                                             //GetValueAux, replaces SalRegQueryValue and SalRegQueryValueEx
        public: static bool Silent_Value_Get( HKEY hKey, const String_TChar_View& name, QWORD& value );                                             //GetValueAux, replaces SalRegQueryValue and SalRegQueryValueEx
        public: static bool Silent_Value_Set( HKEY hKey, const String_TChar_View& name, const String_TChar_View& text );                            //SetValueAux
        public: static bool Silent_Value_Set( HKEY hKey, const String_TChar_View& name, const DWORD value );                                        //SetValueAux
        public: static bool Silent_Value_Set( HKEY hKey, const String_TChar_View& name, const QWORD value );                                        //SetValueAux
    };

//Forward to registry thread -> [W: TODO: clean]

//// WARNING: almost all functions in this section display a LOAD / SAVE
//// configuration error message, which makes them unsuitable for normal Registry access,
//// see the functions at the beginning of regwork.h for solutions: OpenKeyAux, CreateKeyAux, etc.
BOOL ClearKey(HKEY key);
BOOL SetValue(HKEY hKey, const char* name, DWORD type,const void* data, DWORD dataSize);
BOOL OpenKey(HKEY hKey, const char* name, HKEY& openedKey);
void CloseKey(HKEY key);
BOOL DeleteKey(HKEY hKey, const char* name);
BOOL DeleteValue(HKEY hKey, const char* name);
BOOL GetValue(HKEY hKey, const char* name, DWORD type, void* buffer, DWORD bufferSize);
BOOL GetValue2(HKEY hKey, const char* name, DWORD type1, DWORD type2, DWORD* returnedType, void* buffer, DWORD bufferSize);
BOOL GetSize(HKEY hKey, const char* name, DWORD type, DWORD& bufferSize);
BOOL CreateKey(HKEY hKey, const char* name, HKEY& createdKey);
//// pro dataSize = -1 si funkce napocita delku stringu pres strlen
BOOL LoadRGB(HKEY hKey, const char* name, COLORREF& color);
BOOL SaveRGB(HKEY hKey, const char* name, COLORREF color);
BOOL LoadRGBF(HKEY hKey, const char* name, SALCOLOR& color);
BOOL SaveRGBF(HKEY hKey, const char* name, SALCOLOR color);
BOOL LoadLogFont(HKEY hKey, const char* name, LOGFONT* logFont);
BOOL SaveLogFont(HKEY hKey, const char* name, LOGFONT* logFont);
BOOL LoadHistory(HKEY hKey, const char* name, char* history[], int maxCount);
BOOL SaveHistory(HKEY hKey, const char* name, char* history[], int maxCount, BOOL onlyClear = FALSE);
BOOL LoadViewers(HKEY hKey, const char* name, CViewerMasks* viewerMasks);
BOOL SaveViewers(HKEY hKey, const char* name, CViewerMasks* viewerMasks);
BOOL LoadEditors(HKEY hKey, const char* name, CEditorMasks* editorMasks);
BOOL SaveEditors(HKEY hKey, const char* name, CEditorMasks* editorMasks);

