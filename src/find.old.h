// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once


#define WM_USER_ADDLOG WM_APP + 210     // prida polozku do logu [Find_Log_Item *item, 0]
#define WM_USER_ADDFILE WM_APP + 211    // [0, 0]
#define WM_USER_SETREADING WM_APP + 212 // [0, 0] - zadost o prekresleni status-bary ("Reading:")
#define WM_USER_BUTTONS WM_APP + 213    // zavolej si EnableButtons() [HWND hButton]
#define WM_USER_FLASHICON WM_APP + 214  // po aktivaci findu mame zablikat stavovou ikonkou

extern BOOL IsNotAlpha[256];

extern BOOL GetNextItemFromFind(int index, char* path, char* name, void* param);

// delka mapovaneho view souboru; musi byt vetsi nez delka radky pro regexp + EOL +
// AllocationGranularity
#define VOF_VIEW_SIZE 0x2800400 // 40 MB (vic radsi ne, nemusel by byt k dispozici virt. prostor) + 1 KB (rezerva pro rozumnou text. radku)

// historie pro combobox Named
#define FIND_NAMED_HISTORY_SIZE 30 // pocet pamatovanych stringu
extern char* FindNamedHistory[FIND_NAMED_HISTORY_SIZE];

// historie pro combobox LookIn
#define FIND_LOOKIN_HISTORY_SIZE 30 // pocet pamatovanych stringu
extern char* FindLookInHistory[FIND_LOOKIN_HISTORY_SIZE];

// historie pro combobox Containing
#define FIND_GREP_HISTORY_SIZE 30 // pocet pamatovanych stringu
extern char* FindGrepHistory[FIND_GREP_HISTORY_SIZE];

extern BOOL FindManageInUse; // je otevreny Manage dialog?
extern BOOL FindIgnoreInUse; // je otevreny Ignore dialog?

BOOL InitializeFind();
void ReleaseFind();

// promazne vsechny historie Findu; pokud je 'dataOnly' == TRUE, nebudou se
// promazavat combboxy otevrenych oken
void ClearFindHistory(BOOL dataOnly);

DWORD WINAPI GrepThreadF(void* ptr); // telo grep-threadu

extern HACCEL FindDialogAccelTable;

class Find_FilesList_View;
class Find_Dialog;
class CMenuPopup;
class CMenuBar;


//*********************************************************************************
//
// FindIgnore
//

//*********************************************************************************
//
// CFindAdvancedDialog
//
/*
class CFindAdvancedDialog: public CCommonDialog
{
  public:
    BOOL             SetDateAndTime;
    Find_Options_Item *Data;

  public:
    CFindAdvancedDialog(Find_Options_Item *data);

    int Execute();
    virtual void Validate(CTransferInfo &ti);
    virtual void Transfer(CTransferInfo &ti);
    void EnableControls();   // zajistuje disable/enable operace
    void LoadTime();

  protected:
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};
*/
//*********************************************************************************
//
// CFindManageDialog
//

class CEditListBox;
class Find_Options;
class Find_Options_Item;

class CFindManageDialog : public CCommonDialog
{
protected:
    CEditListBox* EditLB;
    Find_Options* FO;
    const Find_Options_Item* CurrenOptionsItem;

public:
    CFindManageDialog(HWND hParent, const Find_Options_Item* currenOptionsItem);
    ~CFindManageDialog();

    virtual void Transfer(CTransferInfo& ti);
    void LoadControls();

    BOOL IsGood() { return FO != NULL; }

protected:
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

//*********************************************************************************
//
// CFindIgnoreDialog
//

class CFindIgnoreDialog : public CCommonDialog
{
protected:
    CEditListBox* EditLB;
    FindIgnore* IgnoreList; // nase pracovni kopie dat
    FindIgnore* GlobalIgnoreList;
    BOOL DisableNotification;
    HICON HChecked;
    HICON HUnchecked;

public:
    CFindIgnoreDialog(HWND hParent, FindIgnore* globalIgnoreList);
    ~CFindIgnoreDialog();

    virtual void Transfer(CTransferInfo& ti);
    virtual void Validate(CTransferInfo& ti);

    BOOL IsGood() { return IgnoreList != NULL; }

protected:
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void FillList();
};

//****************************************************************************
//
// CFindDuplicatesDialog
//

class CFindDuplicatesDialog : public CCommonDialog
{
public:
    // nastaveni si budeme pamatovat po dobu behu Salamandera
    static BOOL SameName;
    static BOOL SameSize;
    static BOOL SameContent;

public:
    CFindDuplicatesDialog(HWND hParent);

    virtual void Validate(CTransferInfo& ti);
    virtual void Transfer(CTransferInfo& ti);

protected:
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void EnableControls(); // zajistuje disable/enable operace
};


//*********************************************************************************
//
// Find_Log_Dialog
//

#define MD5_DIGEST_SIZE 16

struct CMD5Digest
{
    BYTE Digest[MD5_DIGEST_SIZE];
};




//****************************************************************************
//
// CFindTBHeader
//

class CToolBar;

class CFindTBHeader : public CWindow
{
protected:
    CToolBar* ToolBar;
    CToolBar* LogToolBar;
    HWND HNotifyWindow; // kam posilam comandy
    char Text[200];
    int FoundCount;
    int ErrorsCount;
    int InfosCount;
    HICON HWarningIcon;
    HICON HInfoIcon;
    HICON HEmptyIcon;
    BOOL WarningDisplayed;
    BOOL InfoDisplayed;
    int FlashIconCounter;
    BOOL StopFlash;

public:
    CFindTBHeader(HWND hDlg, int ctrlID);

    void SetNotifyWindow(HWND hWnd) { HNotifyWindow = hWnd; }

    int GetNeededHeight();

    BOOL EnableItem(DWORD position, BOOL byPosition, BOOL enabled);

    void SetFoundCount(int foundCount);
    void SetErrorsInfosCount(int errorsCount, int infosCount);

    void OnColorsChange();

    BOOL CreateLogToolbar(BOOL errors, BOOL infos);

    void StartFlashIcon();
    void StopFlashIcon();

    void SetFont();

protected:
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

//*********************************************************************************
//
// Find_Dialog
//

class CFindDialogQueue : public CWindowQueue
{
public:
    CFindDialogQueue(const char* queueName) : CWindowQueue(queueName) {}

    void AddToArray(TDirectArray<HWND>& arr);
};

//*********************************************************************************
//
// externs
//

BOOL OpenFindDialog(HWND hCenterAgainst, const char* initPath);

extern FindIgnore FindIgnore;
extern CFindDialogQueue FindDialogQueue; // seznam vsech Find dialogu
