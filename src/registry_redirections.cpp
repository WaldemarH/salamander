// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"
#include "registry.h"

//Forward to registry thread -> [W: TODO]
BOOL CreateKey(HKEY hKey, const char* name, HKEY& createdKey)
{
    return RegistryWorkerThread.CreateKey(hKey, name, createdKey);
}
BOOL SetValue(HKEY hKey, const char* name, DWORD type, const void* data, DWORD dataSize)
{
    return RegistryWorkerThread.SetValue(hKey, name, type, data, dataSize);
}
BOOL OpenKey(HKEY hKey, const char* name, HKEY& openedKey)
{
    return RegistryWorkerThread.OpenKey(hKey, name, openedKey);
}
void CloseKey(HKEY hKey)
{
    RegistryWorkerThread.CloseKey(hKey);
}
BOOL DeleteKey(HKEY hKey, const char* name)
{
    return RegistryWorkerThread.DeleteKey(hKey, name);
}
BOOL DeleteValue(HKEY hKey, const char* name)
{
    return RegistryWorkerThread.DeleteValue(hKey, name);
}
BOOL GetValue(HKEY hKey, const char* name, DWORD type, void* buffer, DWORD bufferSize)
{
    return RegistryWorkerThread.GetValue(hKey, name, type, buffer, bufferSize);
}
BOOL GetValue2(HKEY hKey, const char* name, DWORD type1, DWORD type2, DWORD* returnedType, void* buffer, DWORD bufferSize)
{
    return RegistryWorkerThread.GetValue2(hKey, name, type1, type2, returnedType, buffer, bufferSize);
}
BOOL GetSize(HKEY hKey, const char* name, DWORD type, DWORD& bufferSize)
{
    return RegistryWorkerThread.GetSize(hKey, name, type, bufferSize);
}
BOOL ClearKey(HKEY key)
{
    return RegistryWorkerThread.ClearKey(key);
}

