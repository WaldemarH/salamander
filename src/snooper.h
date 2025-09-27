// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

class CPanelWindow;

extern HANDLE RefreshFinishedEvent;
extern int SnooperSuspended;

void AddDirectory(CPanelWindow* win, const char* path, BOOL registerDevNotification);                           // novy adresar pro cmuchala
void ChangeDirectory(CPanelWindow* win, const char* newPath, BOOL registerDevNotification);                     // zmena zadaneho adresare
void DetachDirectory(CPanelWindow* win, BOOL waitForHandleClosure = FALSE, BOOL closeDevNotifification = TRUE); // uz neni treba cmuchat

BOOL InitializeThread();
void TerminateThread();

void BeginSuspendMode(BOOL debugDoNotTestCaller = FALSE);
void EndSuspendMode(BOOL debugDoNotTestCaller = FALSE);

typedef TDirectArray<CPanelWindow*> CWindowArray; // (CPanelWindow *)
typedef TDirectArray<HANDLE> CObjectArray;        // (HANDLE)

extern CWindowArray WindowArray; // shodne indexovana pole
extern CObjectArray ObjectArray; // k ObjectHandlu patri MainWindow
