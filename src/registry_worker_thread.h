// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

//[W: TODO: everything will be moved to registry.h]

//functions for convenient work with the Registry + no voice messages about LOAD and SAVE configuration in case of errors
BOOL GetValueAux(HWND parent, HKEY hKey, const char* name, DWORD type, void* buffer,DWORD bufferSize, BOOL quiet = TRUE);
BOOL SetValueAux(HWND parent, HKEY hKey, const char* name, DWORD type,const void* data, DWORD dataSize, BOOL quiet = TRUE);

enum CRegistryWorkType
{
    rwtNone,
    rwtStopWorker, // servisni prace: ukonceni threadu
    rwtClearKey,
    rwtCreateKey,
    rwtOpenKey,
    rwtCloseKey,
    rwtDeleteKey,
    rwtGetValue,
    rwtGetValue2,
    rwtSetValue,
    rwtDeleteValue,
    rwtGetSize,
};

class CRegistryWorkerThread
{
protected:
    class CInUseHandler
    {
    protected:
        CRegistryWorkerThread*  T = NULL;

    public:
        ~CInUseHandler()
        {
            if (T != NULL)
                T->InUse = FALSE;
        };

        BOOL CanUseThread(CRegistryWorkerThread* t)
        {
        //Are we running in a registry thread?
            if (
                ( t->Thread == NULL )
                ||
                ( t->OwnerTID != GetCurrentThreadId() )
            )
            {
            //No, the thread can't be used/use direct calls.
                return FALSE;
            }

        // Work in a worker only affects the thread that started it.
            BOOL ret = !t->InUse;
       
            if ( ret ) // the job can be run in a registry worker thread
            {
            //Thread can be used.
                T = t; // in the destructor, T->InUse = FALSE is set
                t->InUse = TRUE;
            }
            //else
            //{
            //// recursive call (thanks to message-loop and message distribution team) = we reject work in thread
            //}

            return ret;
        };
        void ResetT()
        {
            T = NULL;
        };
    };

    HANDLE Thread;           // thread registry-workera
    DWORD OwnerTID;          // TID of the thread that started the worker thread (no one else can terminate it)
    BOOL InUse;              // TRUE = already doing some work, next work will be started without a thread (resolves recursion, use from multiple threads is rejected, see OwnerTID)
    int StopWorkerSkipCount; // how many StopThread() calls in thread OwnerTID to ignore (number of recursive StartThread() calls)

    HANDLE WorkReady; // signaled: thread has data ready to process (main thread is waiting for completion + executing message-loop)
    HANDLE WorkDone;  // signaled: thread has finished working (main thread can continue)

    CRegistryWorkType WorkType;
    BOOL LastWorkSuccess;
    HKEY Key;
    const char* Name;
    HKEY OpenedKey;
    DWORD ValueType;
    DWORD ValueType2;
    DWORD* ReturnedValueType;
    void* Buffer;
    DWORD BufferSize;
    const void* Data;
    DWORD DataSize;

public:
    CRegistryWorkerThread();
    ~CRegistryWorkerThread();

    // start threadu registry-workera, vraci uspech
    BOOL StartThread();

    // ukonceni threadu registry-workera
    void StopThread();

    // vycisti klic 'key' od vsech podklicu a hodnot, vraci uspech
    BOOL ClearKey(HKEY key);

    // vytvori nebo otevre existujici podklic 'name' klice 'key', vraci 'createdKey' a uspech;
    // ziskany klic ('createdKey') je nutne zavrit volanim CloseKey
    BOOL CreateKey(HKEY key, const char* name, HKEY& createdKey);

    // otevre existujici podklic 'name' klice 'key', vraci 'openedKey' a uspech
    // ziskany klic ('openedKey') je nutne zavrit volanim CloseKey
    BOOL OpenKey(HKEY key, const char* name, HKEY& openedKey);

    // zavre klic otevreny pres OpenKey nebo CreateKey
    void CloseKey(HKEY key);

    // smaze podklic 'name' klice 'key', vraci uspech
    BOOL DeleteKey(HKEY key, const char* name);

    // nacte hodnotu 'name'+'type'+'buffer'+'bufferSize' z klice 'key', vraci uspech
    BOOL GetValue(HKEY key, const char* name, DWORD type, void* buffer, DWORD bufferSize);

    // nacte hodnotu 'name'+'type1 || type2' do 'returnedType'+'buffer'+'bufferSize' z klice 'key', vraci uspech
    BOOL GetValue2(HKEY hKey, const char* name, DWORD type1, DWORD type2, DWORD* returnedType, void* buffer, DWORD bufferSize);

    // ulozi hodnotu 'name'+'type'+'data'+'dataSize' do klice 'key', pro retezce je mozne
    // zadat 'dataSize' == -1 -> vypocet delky retezce pomoci funkce strlen,
    // vraci uspech
    BOOL SetValue(HKEY key, const char* name, DWORD type, const void* data, DWORD dataSize);

    // smaze hodnotu 'name' klice 'key', vraci uspech
    BOOL DeleteValue(HKEY key, const char* name);

    // vytahne do 'bufferSize' protrebnou velikost pro hodnotu 'name'+'type' z klice 'key', vraci uspech
    BOOL GetSize(HKEY key, const char* name, DWORD type, DWORD& bufferSize);

protected:
    // ceka na dokonceni prace + provadi message-loopu
    void WaitForWorkDoneWithMessageLoop();

    // telo threadu - zde se odehrava vsechna prace
    unsigned Body();

    static DWORD WINAPI ThreadBody(void* param); // pomocna funkce pro telo threadu
    static unsigned ThreadBodyFEH(void* param);  // pomocna funkce pro telo threadu
};

extern CRegistryWorkerThread RegistryWorkerThread;
