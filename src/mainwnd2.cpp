// SPDX-FileCopyrightText: 2023 Open Salamander Authors
// SPDX-License-Identifier: GPL-2.0-or-later

#include "precomp.h"

#include <shlwapi.h>
#undef PathIsPrefix // jinak kolize s CSalamanderGeneral::PathIsPrefix

#include "toolbar.h"
#include "stswnd.h"
#include "plugins.h"
#include "fileswnd.h"
#include "mainwnd.h"
#include "cfgdlg.h"
#include "usermenu.h"
#include "viewer.h"
#include "zip.h"
#include "pack.h"
#include "find.old.h"
#include "dialogs.h"
#include "logo.h"
#include "tasklist.h"
#include "pwdmngr.h"
#include "find_options.h"


//
// ConfigVersion - cislo verze nactene konfigurace
//
// 0 = zadna konfigurace nebyla nalezena - pouziji se default hodnoty
// 1 = v1.52 a starsi
// 2 = 1.6b1
// 3 = 1.6b1.x
// 4 = 1.6b3.x
// 5 = 1.6b3.x          kvuli spravne konverzi konfigurace pakovacu mezi nasima verzema
// 6 = 1.6b4.x
// 7 = 1.6b5.x          kvuli spravne konverzi podporovanych funkci pluginu (viz CPlugins::Load)
// 8 = 1.6b5.x          radeji prehlednout ;-)
// 9 = 1.6b5.x          kvuli prechodu ze jmena exe na promennou u custom packers
// 10 = 1.6b6           kvuli prejmenovani "XXX (Internal)" na "XXX (Plug-in)" v Pack a Unpack dialozich
//                      a kvuli nastaveni ANSI verze "list of files" souboru i pro (un)packery ACE32 a PKZIP25
// 11 = 1.6b7           pribyl plugin CheckVer - zajistime jeho automatickou do-instalaci
// 12 = 2.0             auto-vypnuti salopen.exe + pribyl plugin PEViewer - zajistime jeho automatickou do-instalaci
// 13 = 2.5b1           dopsana chybejici konverze konfigurace u custom-packeru - promitnuti zmeny u LHA
// 14 = 2.5b1           Nove Advanced Options ve Find dialogu. Prechod na CFilterCriteria. Konverze inverzni masky u filtru.
// 15 = 2.5b2           novejsi verze, at se naloadi pluginy (upgradnou zaznamy v registry)
// 16 = 2.5b2           pridano barveni Encrypted souboru a adresaru (pridava se pri loadu konfigu + je v defaultnim konfigu)
// 17 = 2.5b2           pridana maska *.xml do nastaveni interniho vieweru - "force text mode"
// 18 = 2.5b3           zatim jen kvuli prenosu konfigurace pluginu z verze 2.5b2
// 19 = 2.5b4           zatim jen kvuli prenosu konfigurace pluginu z verze 2.5b3
// 20 = 2.5b5           zatim jen kvuli prenosu konfigurace pluginu z verze 2.5b4
// 21 = 2.5b6           zatim jen kvuli prenosu konfigurace pluginu z verze 2.5b5(a)
// 22 = 2.5b6           filtry v panelech -- sjednoceni na jednu historii
// 23 = 2.5b6           novy pohled v panelu (Tiles)
// 24 = 2.5b7           zatim jen kvuli prenosu konfigurace pluginu z verze 2.5b6
// 25 = 2.5b7           plugins: show in plugin bar -> prenos promenne do CPluginData
// 26 = 2.5b8           zatim jen kvuli prenosu konfigurace pluginu z verze 2.5b7
// 27 = 2.5b9           zatim jen kvuli prenosu konfigurace pluginu z verze 2.5b8
// 28 = 2.5b9           nove barevne schema dle stareho DOS Navigator -> konverze 'scheme'
// 29 = 2.5b10          zatim jen kvuli prenosu konfigurace pluginu z verze 2.5b9
// 30 = 2.5b11          zatim jen kvuli prenosu konfigurace pluginu z verze 2.5b10
// 31 = 2.5b11          zavedli jsme Floppy sekci v konfiguraci Drives a potrebujeme pro Removable forcnout cteni ikon
// 32 = 2.5b11          Find: "Local Settings\\Temporary Internet Files" je implicitne prohledavane
// 33 = 2.5b12          zatim jen kvuli prenosu konfigurace pluginu z verze 2.5b11
// 34 = 2.5b12          uprava externiho packeru/unpackeru PKZIP25 (externi Win32 verze)
// 35 = 2.5RC1          jen kvuli prenosu konfigurace pluginu z verze 2.5b12 (jen interni, pustili jsme misto ni RC1)
// 36 = 2.5RC2          jen kvuli prenosu konfigurace pluginu z verze 2.5RC1
// 37 = 2.5RC3          jen kvuli prenosu konfigurace pluginu z verze 2.5RC2
// 38 = 2.5RC3          prejmenovani Servant Salamander na Altap Salamander
// 39 = 2.5             jen kvuli prenosu konfigurace pluginu z verze 2.5RC3
// 40 = 2.51            jen kvuli prenosu konfigurace pluginu z verze 2.5
// 41 = 2.51            verze konfigu obsahujici seznam zakazanych icon overlay handleru (viz CONFIG_DISABLEDCUSTICOVRLS_REG)
// 42 = 2.52b1          jen kvuli prenosu konfigurace pluginu z verze 2.51
// 43 = 2.52b2          jen kvuli prenosu konfigurace pluginu z verze 2.52 beta 1
// 44 = 2.52b2          zmena pripon vieweru, editoru a archivatoru na lowercase (uppercase pripony jsou uz ve woknech prezitek)
// 45 = 2.52b2          zavedeni password manageru, vynuceny load FTP klienta, aby se prihlasil k pouzivani Password Manageru, viz SetPluginUsesPasswordManager
// 46 = 2.52 (DB30)     jen kvuli prenosu konfigurace pluginu z verze 2.52 beta 2
// 47 = 2.52 (IB31)     podpora pro Sal/Env promenne jako je $(SalDir) nebo $[USERPROFILE] v hot paths; potrebujeme escapovat stare hot paths
// 48 = 2.52            jen kvuli prenosu konfigurace pluginu z verze 2.52 (DB30)
// 49 = 2.53b1 (DB33)   jen kvuli prenosu konfigurace pluginu z verze 2.52
// 50 = 2.53b1 (DB36)   jen kvuli prenosu konfigurace pluginu z verze 2.53b1 (DB33)
// 51 = 2.53b1 (PB38)   jen kvuli prenosu konfigurace pluginu z verze 2.53b1 (DB36)
// 52 = 2.53b1 (DB39)   jen kvuli prenosu konfigurace pluginu z verze 2.53b1 (PB38)
// 53 = 2.53b1 (DB41)   jen kvuli prenosu konfigurace pluginu z verze 2.53b1 (DB39)
// 54 = 2.53b1 (PB44)   jen kvuli prenosu konfigurace pluginu z verze 2.53b1 (DB41)
// 55 = 2.53b1 (DB46)   jen kvuli prenosu konfigurace pluginu z verze 2.53b1 (PB44)
// 56 = 2.53b1          jen kvuli prenosu konfigurace pluginu z verze 2.53b1 (DB46)
// 57 = 2.53 (DB52)     jen kvuli prenosu konfigurace pluginu z verze 2.53b1
// 58 = 2.53b2 (IB55)   jen kvuli prenosu konfigurace pluginu z verze 2.53 (DB52)
// 59 = 2.53b2          jen kvuli prenosu konfigurace pluginu z verze 2.53b2 (IB55)
// 60 = 2.53 (DB60)     jen kvuli prenosu konfigurace pluginu z verze 2.53b2
// 61 = 2.53            jen kvuli prenosu konfigurace pluginu z verze 2.53 (DB60)
// 62 = 2.54b1 (DB66)   jen kvuli prenosu konfigurace pluginu z verze 2.53
// 63 = 2.54            jen kvuli prenosu konfigurace pluginu z verze 2.54b1 (DB66)
// 64 = 2.55b1 (DB72)   jen kvuli prenosu konfigurace pluginu z verze 2.54
// 65 = 2.55b1 (DB72)   externi archivatory: identifikace podle UID misto podle Title (preklada se dle jazykove verze, tedy nelze pouzit pro identifikaci) - pri prepnuti jazyku se ztracelo nastaveni cest k externim archiverum
// 66 = 3.00b1 (PB75)   jen kvuli prenosu konfigurace pluginu z verze 2.55b1 (DB72)
// 67 = 3.00b1 (DB76)   jen kvuli prenosu konfigurace pluginu z verze 3.00b1 (PB75)
// 68 = 3.00b1 (PB79)   jen kvuli prenosu konfigurace pluginu z verze 3.00b1 (DB76)
// 69 = 3.00b1 (DB80)   jen kvuli prenosu konfigurace pluginu z verze 3.00b1 (PB79)
// 70 = 3.00b1 (DB83)   jen kvuli prenosu konfigurace pluginu z verze 3.00b1 (DB80)
// 71 = 3.00b1 (PB87)   jen kvuli prenosu konfigurace pluginu z verze 3.00b1 (DB83)
// 72 = 3.00b1 (DB88)   jen kvuli prenosu konfigurace pluginu z verze 3.00b1 (PB87)
// 73 = 3.00b1          jen kvuli prenosu konfigurace pluginu z verze 3.00b1 (DB88)
// 74 = 3.00b2 (DB94)   jen kvuli prenosu konfigurace pluginu z verze 3.00b1
// 75 = 3.00b2          jen kvuli prenosu konfigurace pluginu z verze 3.00b2 (DB94)
// 76 = 3.00b3 (DB100)  jen kvuli prenosu konfigurace pluginu z verze 3.00b2
// 77 = 3.00b3 (PB103)  jen kvuli prenosu konfigurace pluginu z verze 3.00b3 (DB100)
// 78 = 3.00b3 (DB105)  jen kvuli prenosu konfigurace pluginu z verze 3.00b3 (PB103)
// 79 = 3.00b3          jen kvuli prenosu konfigurace pluginu z verze 3.00b3 (DB105)
// 80 = 3.00b4 (DB111)  jen kvuli prenosu konfigurace pluginu z verze 3.00b3
// 81 = 3.00b4 (DB111)  RAR 5.0 potrebuje novy switch na command line kvuli kodovani file list souboru
// 82 = 3.00b4          jen kvuli prenosu konfigurace pluginu z verze 3.00b4 (DB111)
// 83 = 3.00b5 (DB117)  jen kvuli prenosu konfigurace pluginu z verze 3.00b4
// 84 = 3.0             jen kvuli prenosu konfigurace pluginu z verze 3.00b5 (DB117)
// 85 = 3.10b1 (DB123)  jen kvuli prenosu konfigurace pluginu z verze 3.0
// 86 = 3.01            jen kvuli prenosu konfigurace pluginu z verze 3.10b1 (DB123)
// 87 = 3.10b1 (DB129)  jen kvuli prenosu konfigurace pluginu z verze 3.01
// 88 = 3.02            jen kvuli prenosu konfigurace pluginu z verze 3.10b1 (DB129)
// 89 = 3.10b1 (DB135)  jen kvuli prenosu konfigurace pluginu z verze 3.02
// 90 = 3.03            jen kvuli prenosu konfigurace pluginu z verze 3.10b1 (DB135)
// 91 = 3.10b1 (DB141)  jen kvuli prenosu konfigurace pluginu z verze 3.03
// 92 = 3.04            jen kvuli prenosu konfigurace pluginu z verze 3.10b1 (DB141)
// 93 = 3.10b1 (DB147)  jen kvuli prenosu konfigurace pluginu z verze 3.04
// 94 = 3.05            jen kvuli prenosu konfigurace pluginu z verze 3.10b1 (DB147)
// 95 = 3.10b1 (DB153)  jen kvuli prenosu konfigurace pluginu z verze 3.05
// 96 = 3.06            jen kvuli prenosu konfigurace pluginu z verze 3.10b1 (DB153)
// 97 = 3.10b1 (DB159)  jen kvuli prenosu konfigurace pluginu z verze 3.06
// 98 = 3.10b1 (DB162)  jen kvuli prenosu konfigurace pluginu z verze 3.10b1 (DB159)
// 99 = 3.07            jen kvuli prenosu konfigurace pluginu z verze 3.10b1 (DB162)
// 100 = 4.00b1 (DB168) jen kvuli prenosu konfigurace pluginu z verze 3.07
// 101 = 3.08           jen kvuli prenosu konfigurace pluginu z verze 4.00b1 (DB168) - omylem maji 3.08 a DB171 stejne cislo verze 101, snad neva, priste si dam vetsi pozor
// 101 = 4.00b1 (DB171) jen kvuli prenosu konfigurace pluginu z verze 4.00b1 (DB168), ktera je posledni z VC2008, dalsi verze jsou z VC2019
// 102 = 4.00b1 (DB177) jen kvuli prenosu konfigurace pluginu z verze 4.00b1 (DB171)
// 103 = 4.00           jen kvuli prenosu konfigurace pluginu z verze 4.00b1 (DB177)
// 104 = 5.00           jen kvuli prenosu konfigurace pluginu z verze 4.00, first Open Salamander
//
// Pri zvetseni verze konfigurace je potreba pridat jednicku k THIS_CONFIG_VERSION
//
// Pri prechodu na novou verzi programu je treba THIS_CONFIG_VERSION zvysit o 1,
// aby se provedla autoinstalace novych plug-inu a nulovani pocitadla posledni
// verze plugins.ver.
//

const DWORD THIS_CONFIG_VERSION = 104;

// Koreny konfiguraci jednotlivych verzi programu Open Salamander.
// Koren soucasne (nejmladsi) konfigurace je na indexu 0.
// Potom nasleduji dalsi koreny smerem ke starsim verzim programu.
// Na poslednim indexu lezi NULL, ktery slouzi jako terminator pri praci s polem.
// Pri zalozeni nove verze konfigurace (ktera ma byt v registry oddelena od predesle)
// staci vlozit radek s cestou na index 0.

// !!! Zaroven je treba udrzovat odpovidajici radky v SalamanderConfigurationVersions
const TCHAR* SalamanderConfigurationRoots[SALCFG_ROOTS_COUNT + 1] =
    {
        TEXT( "Software\\Open Salamander\\5.0" ),
        TEXT( "Software\\Altap\\Altap Salamander 4.0" ),
        TEXT( "Software\\Altap\\Altap Salamander 4.0 beta 1 (DB177)" ),
        TEXT( "Software\\Altap\\Altap Salamander 4.0 beta 1 (DB171)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.08" ),
        TEXT( "Software\\Altap\\Altap Salamander 4.0 beta 1 (DB168)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.07" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.1 beta 1 (DB162)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.1 beta 1 (DB159)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.06" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.1 beta 1 (DB153)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.05" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.1 beta 1 (DB147)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.04" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.1 beta 1 (DB141)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.03" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.1 beta 1 (DB135)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.02" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.1 beta 1 (DB129)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.01" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.1 beta 1 (DB123)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 5 (DB117)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 4" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 4 (DB111)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 3" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 3 (DB105)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 3 (PB103)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 3 (DB100)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 2" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 2 (DB94)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 1" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 1 (DB88)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 1 (PB87)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 1 (DB83)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 1 (DB80)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 1 (PB79)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 1 (DB76)" ),
        TEXT( "Software\\Altap\\Altap Salamander 3.0 beta 1 (PB75)" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.55 beta 1 (DB 72)" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.54" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.54 beta 1 (DB 66)" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.53" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.53 (DB 60)" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.53 beta 2" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.53 beta 2 (IB 55)" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.53 (DB 52)" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.53 beta 1" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.53 beta 1 (DB 46)" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.53 beta 1 (PB 44)" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.53 beta 1 (DB 41)" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.53 beta 1 (DB 39)" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.53 beta 1 (PB 38)" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.53 beta 1 (DB 36)" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.53 beta 1 (DB 33)" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.52" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.52 (DB 30)" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.52 beta 2" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.52 beta 1" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.51" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.5" ),
        TEXT( "Software\\Altap\\Altap Salamander 2.5 RC3" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.5 RC3" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.5 RC2" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.5 RC1" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.5 beta 12" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.5 beta 11" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.5 beta 10" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.5 beta 9" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.5 beta 8" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.5 beta 7" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.5 beta 6" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.5 beta 5" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.5 beta 4" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.5 beta 3" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.5 beta 2" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.5 beta 1" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.1 beta 1" ),
        TEXT( "Software\\Altap\\Servant Salamander 2.0" ),
        TEXT( "Software\\Altap\\Servant Salamander 1.6 beta 7" ),
        TEXT( "Software\\Altap\\Servant Salamander 1.6 beta 6" ),
        TEXT( "Software\\Altap\\Servant Salamander" ), // 1.6 beta 1 az 1.6 beta 5
        TEXT( "Software\\Salamander" )                 // nejstarsi verze (1.52 a starsi)
    };
const TCHAR* SalamanderConfigurationVersions[SALCFG_ROOTS_COUNT] =
    {
        TEXT( "5.0" ),
        TEXT( "4.0" ),
        TEXT( "4.0 beta 1 (DB177)" ),
        TEXT( "4.0 beta 1 (DB171)" ),
        TEXT( "3.08" ),
        TEXT( "4.0 beta 1 (DB168)" ),
        TEXT( "3.07" ),
        TEXT( "3.1 beta 1 (DB162)" ),
        TEXT( "3.1 beta 1 (DB159)" ),
        TEXT( "3.06" ),
        TEXT( "3.1 beta 1 (DB153)" ),
        TEXT( "3.05" ),
        TEXT( "3.1 beta 1 (DB147)" ),
        TEXT( "3.04" ),
        TEXT( "3.1 beta 1 (DB141)" ),
        TEXT( "3.03" ),
        TEXT( "3.1 beta 1 (DB135)" ),
        TEXT( "3.02" ),
        TEXT( "3.1 beta 1 (DB129)" ),
        TEXT( "3.01" ),
        TEXT( "3.1 beta 1 (DB123)" ),
        TEXT( "3.0" ),
        TEXT( "3.0 beta 5 (DB117)" ),
        TEXT( "3.0 beta 4" ),
        TEXT( "3.0 beta 4 (DB111)" ),
        TEXT( "3.0 beta 3" ),
        TEXT( "3.0 beta 3 (DB105)" ),
        TEXT( "3.0 beta 3 (PB103)" ),
        TEXT( "3.0 beta 3 (DB100)" ),
        TEXT( "3.0 beta 2" ),
        TEXT( "3.0 beta 2 (DB94)" ),
        TEXT( "3.0 beta 1" ),
        TEXT( "3.0 beta 1 (DB88)" ),
        TEXT( "3.0 beta 1 (PB87)" ),
        TEXT( "3.0 beta 1 (DB83)" ),
        TEXT( "3.0 beta 1 (DB80)" ),
        TEXT( "3.0 beta 1 (PB79)" ),
        TEXT( "3.0 beta 1 (DB76)" ),
        TEXT( "3.0 beta 1 (PB75)" ),
        TEXT( "2.55 beta 1 (DB72)" ),
        TEXT( "2.54" ),
        TEXT( "2.54 beta 1 (DB66)" ),
        TEXT( "2.53" ),
        TEXT( "2.53 (DB60)" ),
        TEXT( "2.53 beta 2" ),
        TEXT( "2.53 beta 2 (IB55)" ),
        TEXT( "2.53 (DB52)" ),
        TEXT( "2.53 beta 1" ),
        TEXT( "2.53 beta 1 (DB46)" ),
        TEXT( "2.53 beta 1 (PB44)" ),
        TEXT( "2.53 beta 1 (DB41)" ),
        TEXT( "2.53 beta 1 (DB39)" ),
        TEXT( "2.53 beta 1 (PB38)" ),
        TEXT( "2.53 beta 1 (DB36)" ),
        TEXT( "2.53 beta 1 (DB33)" ),
        TEXT( "2.52" ),
        TEXT( "2.52 (DB30)" ),
        TEXT( "2.52 beta 2" ),
        TEXT( "2.52 beta 1" ),
        TEXT( "2.51" ),
        TEXT( "2.5" ),
        TEXT( "2.5 RC3" ),
        TEXT( "2.5 RC3" ),
        TEXT( "2.5 RC2" ),
        TEXT( "2.5 RC1" ),
        TEXT( "2.5 beta 12" ),
        TEXT( "2.5 beta 11" ),
        TEXT( "2.5 beta 10" ),
        TEXT( "2.5 beta 9" ),
        TEXT( "2.5 beta 8" ),
        TEXT( "2.5 beta 7" ),
        TEXT( "2.5 beta 6" ),
        TEXT( "2.5 beta 5" ),
        TEXT( "2.5 beta 4" ),
        TEXT( "2.5 beta 3" ),
        TEXT( "2.5 beta 2" ),
        TEXT( "2.5 beta 1" ),
        TEXT( "2.1 beta 1" ),
        TEXT( "2.0" ),
        TEXT( "1.6 beta 7" ),
        TEXT( "1.6 beta 6" ),
        TEXT( "1.6 beta 1-5" ),
        TEXT( "1.52" )
    };

const TCHAR* SALAMANDER_ROOT_REG = NULL; // bude nastavena v salamdr1.cpp

const TCHAR* SALAMANDER_SAVE_IN_PROGRESS = TEXT( "Save In Progress" ); // hodnota existuje jen behem ukladani konfigurace (pro detekci preruseni ukladani konfigu -> poskozene konfigurace)
BOOL IsSetSALAMANDER_SAVE_IN_PROGRESS = FALSE;                // TRUE = v registry je vytvorena hodnota SALAMANDER_SAVE_IN_PROGRESS (detekce preruseni ukladani konfigurace)

const TCHAR* SALAMANDER_COPY_IS_OK = TEXT( "Copy Is OK" ); // jen backup klic: hodnota existuje, jen kdyz se klic podari kompletne nakopirovat

const TCHAR* SALAMANDER_AUTO_IMPORT_CONFIG = TEXT( "AutoImportConfig" ); // hodnota existuje jen pri UPGRADE (instalak prevali starou verzi novou verzi + do klice nove verze ulozi tuto hodnotu nasmerovanou na klic konfigurace stare verze, odkud by se mela importovat konfigurace do nove verze)

const TCHAR* FINDDIALOG_WINDOW_REG = TEXT( "Find Dialog Window" );
const TCHAR* SALAMANDER_WINDOW_REG = TEXT( "Window" );
const TCHAR* WINDOW_LEFT_REG = TEXT( "Left" );
const TCHAR* WINDOW_RIGHT_REG = TEXT( "Right" );
const TCHAR* WINDOW_TOP_REG = TEXT( "Top" );
const TCHAR* WINDOW_BOTTOM_REG = TEXT( "Bottom" );
const TCHAR* WINDOW_SPLIT_REG = TEXT( "Split Position" );
const TCHAR* WINDOW_BEFOREZOOMSPLIT_REG = TEXT( "Before Zoom Split Position" );
const TCHAR* WINDOW_SHOW_REG = TEXT( "Show" );
const TCHAR* FINDDIALOG_NAMEWIDTH_REG = TEXT( "Name Width" );

const TCHAR* SALAMANDER_LEFTP_REG = TEXT( "Left Panel" );
const TCHAR* SALAMANDER_RIGHTP_REG = TEXT( "Right Panel" );
const TCHAR* PANEL_PATH_REG = TEXT( "Path" );
const TCHAR* PANEL_VIEW_REG = TEXT( "View Type" );
const TCHAR* PANEL_SORT_REG = TEXT( "Sort Type" );
const TCHAR* PANEL_REVERSE_REG = TEXT( "Reverse Sort" );
const TCHAR* PANEL_DIRLINE_REG = TEXT( "Directory Line" );
const TCHAR* PANEL_STATUS_REG = TEXT( "Status Line" );
const TCHAR* PANEL_HEADER_REG = TEXT( "Header Line" );
const TCHAR* PANEL_FILTER_ENABLE = TEXT( "Enable Filter" );
const TCHAR* PANEL_FILTER_INVERSE = TEXT( "Inverse Filter" );
const TCHAR* PANEL_FILTERHISTORY_REG = TEXT( "Filter History" );
const TCHAR* PANEL_FILTER = TEXT( "Filter" );

const TCHAR* SALAMANDER_DEFDIRS_REG = TEXT( "Default Directories" );

const TCHAR* SALAMANDER_CONFIG_REG = TEXT( "Configuration" );
const TCHAR* CONFIG_SKILLLEVEL_REG = TEXT( "Skill Level" );
const TCHAR* CONFIG_FILENAMEFORMAT_REG = TEXT( "File Name Format" );
const TCHAR* CONFIG_SIZEFORMAT_REG = TEXT( "Size Format" );
const TCHAR* CONFIG_SELECTION_REG = TEXT( "Select/Deselect Directories" );
const TCHAR* CONFIG_LONGNAMES_REG = TEXT( "Use Long File Names" );
const TCHAR* CONFIG_RECYCLEBIN_REG = TEXT( "Use Recycle Bin" );
const TCHAR* CONFIG_RECYCLEMASKS_REG = TEXT( "Use Recycle Bin For" );
const TCHAR* CONFIG_SAVEONEXIT_REG = TEXT( "Save Configuration On Exit" );
const TCHAR* CONFIG_SHOWGREPERRORS_REG = TEXT( "Show Errors In Find Files" );
const TCHAR* CONFIG_FINDFULLROW_REG = TEXT( "Show Full Row In Find Files" );
const TCHAR* CONFIG_MINBEEPWHENDONE_REG = TEXT( "Use Speeker Beep" );
const TCHAR* CONFIG_INTRN_VIEWER_REG = TEXT( "Internal Viewer" );
const TCHAR* CONFIG_VIEWER_REG = TEXT( "External Viewer" );
const TCHAR* CONFIG_EDITOR_REG = TEXT( "External Editor" );
const TCHAR* CONFIG_CMDLINE_REG = TEXT( "Command Line" );
const TCHAR* CONFIG_CMDLFOCUS_REG = TEXT( "Command Line Focused" );
const TCHAR* CONFIG_CLOSESHELL_REG = TEXT( "Close Shell Window" );
const TCHAR* CONFIG_USECUSTOMPANELFONT_REG = TEXT( "Use Custom Panel Font" );
const TCHAR* CONFIG_PANELFONT_REG = TEXT( "Panel Font" );
const TCHAR* CONFIG_NAMEDHISTORY_REG = TEXT( "Named History" );
const TCHAR* CONFIG_LOOKINHISTORY_REG = TEXT( "Look In History" );
const TCHAR* CONFIG_GREPHISTORY_REG = TEXT( "Grep History" );
const TCHAR* CONFIG_VIEWERHISTORY_REG = TEXT( "Viewer History" );
const TCHAR* CONFIG_COMMANDHISTORY_REG = TEXT( "Command History" );
const TCHAR* CONFIG_SELECTHISTORY_REG = TEXT( "Select History" );
const TCHAR* CONFIG_COPYHISTORY_REG = TEXT( "Copy History" );
const TCHAR* CONFIG_CHANGEDIRHISTORY_REG = TEXT( "ChangeDir History" );
const TCHAR* CONFIG_FILELISTHISTORY_REG = TEXT( "File List History" );
const TCHAR* CONFIG_CREATEDIRHISTORY_REG = TEXT( "Create Directory History" );
const TCHAR* CONFIG_QUICKRENAMEHISTORY_REG = TEXT( "Quick Rename History" );
const TCHAR* CONFIG_EDITNEWHISTORY_REG = TEXT( "Edit New History" );
const TCHAR* CONFIG_CONVERTHISTORY_REG = TEXT( "Convert History" );
const TCHAR* CONFIG_FILTERHISTORY_REG = TEXT( "Filter History" );
const TCHAR* CONFIG_WORKDIRSHISTORY_REG = TEXT( "Working Directories" );
const TCHAR* CONFIG_FILELISTNAME_REG = TEXT( "Make File List Name" );
const TCHAR* CONFIG_FILELISTAPPEND_REG = TEXT( "Make File List Append" );
const TCHAR* CONFIG_FILELISTDESTINATION_REG = TEXT( "Make File List Destination" );
const TCHAR* CONFIG_COPYFINDTEXT_REG = TEXT( "Copy Find Text" );
const TCHAR* CONFIG_CLEARREADONLY_REG = TEXT( "Clear Readonly Attribute" );
const TCHAR* CONFIG_PRIMARYCONTEXTMENU_REG = TEXT( "Primary Context Menu" );
const TCHAR* CONFIG_NOTHIDDENSYSTEM_REG = TEXT( "Hide Hidden and System Files and Directories" );
const TCHAR* CONFIG_RIGHT_FOCUS_REG = TEXT( "Right Panel Focused" );
const TCHAR* CONFIG_SHOWCHDBUTTON_REG = TEXT( "Show Change Drive Button" );
const TCHAR* CONFIG_ALWAYSONTOP_REG = TEXT( "Always On Top" );
//const TCHAR *CONFIG_FASTDIRMOVE_REG = TEXT( "Fast Directory Move" );
const TCHAR* CONFIG_SORTUSESLOCALE_REG = TEXT( "Sort Uses Locale" );
const TCHAR* CONFIG_SORTDETECTNUMBERS_REG = TEXT( "Sort Detects Numbers" );
const TCHAR* CONFIG_SORTNEWERONTOP_REG = TEXT( "Sort Newer On Top" );
const TCHAR* CONFIG_SORTDIRSBYNAME_REG = TEXT( "Sort Dirs By Name" );
const TCHAR* CONFIG_SORTDIRSBYEXT_REG = TEXT( "Sort Dirs By Ext" );
const TCHAR* CONFIG_SAVEHISTORY_REG = TEXT( "Save History" );
const TCHAR* CONFIG_SAVEWORKDIRS_REG = TEXT( "Save Working Dirs" );
const TCHAR* CONFIG_ENABLECMDLINEHISTORY_REG = TEXT( "Enable CmdLine History" );
const TCHAR* CONFIG_SAVECMDLINEHISTORY_REG = TEXT( "Save CmdLine History" );
//const TCHAR *CONFIG_LANTASTICCHECK_REG = TEXT( "Lantastic Check" );
const TCHAR* CONFIG_USESALOPEN_REG = TEXT( "Use salopen.exe" );
const TCHAR* CONFIG_NETWAREFASTDIRMOVE_REG = TEXT( "Netware Fast Dir Move" );
const TCHAR* CONFIG_ASYNCCOPYALG_REG = TEXT( "Async Copy Alg On Network" );
const TCHAR* CONFIG_RELOAD_ENV_VARS_REG = TEXT( "Reload Environment Variables" );
const TCHAR* CONFIG_QUICKRENAME_SELALL_REG = TEXT( "Quick Rename Select All" );
const TCHAR* CONFIG_EDITNEW_SELALL_REG = TEXT( "Edit New File Select All" );
const TCHAR* CONFIG_SHIFTFORHOTPATHS_REG = TEXT( "Use Shift For GoTo HotPath" );
const TCHAR* CONFIG_ONLYONEINSTANCE_REG = TEXT( "Only One Instance" );
const TCHAR* CONFIG_STATUSAREA_REG = TEXT( "Status Area" );
const TCHAR* CONFIG_SINGLECLICK_REG = TEXT( "Single Click" );
//const TCHAR *CONFIG_SHOWTIPOFTHEDAY_REG = TEXT( "Show tip of the Day" );
//const TCHAR *CONFIG_LASTTIPOFTHEDAY_REG = TEXT( "Last tip of the Day" );
const TCHAR* CONFIG_TOPTOOLBAR_REG = TEXT( "Top ToolBar" );
const TCHAR* CONFIG_MIDDLETOOLBAR_REG = TEXT( "Middle ToolBar" );
const TCHAR* CONFIG_LEFTTOOLBAR_REG = TEXT( "Left ToolBar" );
const TCHAR* CONFIG_RIGHTTOOLBAR_REG = TEXT( "Right ToolBar" );
const TCHAR* CONFIG_TOPTOOLBARVISIBLE_REG = TEXT( "Show Top ToolBar" );
const TCHAR* CONFIG_PLGTOOLBARVISIBLE_REG = TEXT( "Show Plugins Bar" );
const TCHAR* CONFIG_MIDDLETOOLBARVISIBLE_REG = TEXT( "Show Middle ToolBar" );
const TCHAR* CONFIG_USERMENUTOOLBARVISIBLE_REG = TEXT( "Show User Menu ToolBar" );
const TCHAR* CONFIG_HOTPATHSBARVISIBLE_REG = TEXT( "Hot Paths Bar" );
const TCHAR* CONFIG_DRIVEBARVISIBLE_REG = TEXT( "Show Drive Bar" );
const TCHAR* CONFIG_DRIVEBAR2VISIBLE_REG = TEXT( "Show Drive Bar2" );
const TCHAR* CONFIG_BOTTOMTOOLBARVISIBLE_REG = TEXT( "Show Bottom ToolBar" );
const TCHAR* CONFIG_EXPLORERLOOK_REG = TEXT( "Explorer Look" );
const TCHAR* CONFIG_FULLROWSELECT_REG = TEXT( "Full Row Select" );
const TCHAR* CONFIG_FULLROWHIGHLIGHT_REG = TEXT( "Full Row Highlight" );
const TCHAR* CONFIG_USEICONTINCTURE_REG = TEXT( "Use Icon Tincture" );
const TCHAR* CONFIG_SHOWPANELCAPTION_REG = TEXT( "Show Panel Caption" );
const TCHAR* CONFIG_SHOWPANELZOOM_REG = TEXT( "Show Panel Zoom" );
const TCHAR* CONFIG_INFOLINECONTENT_REG = TEXT( "Information Line Content" );
const TCHAR* CONFIG_IFPATHISINACCESSIBLEGOTOISMYDOCS_REG = TEXT( "If Path Is Inaccessible Go To My Docs" );
const TCHAR* CONFIG_IFPATHISINACCESSIBLEGOTO_REG = TEXT( "If Path Is Inaccessible Go To" );
const TCHAR* CONFIG_HOTPATH_AUTOCONFIG = TEXT( "Auto Configurate Hot Paths" );
const TCHAR* CONFIG_LASTUSEDSPEEDLIM_REG = TEXT( "Speed Limit" );
const TCHAR* CONFIG_QUICKSEARCHENTER_REG = TEXT( "Quick Search Enter Alt" );
const TCHAR* CONFIG_CHD_SHOWMYDOC = TEXT( "Change Drive Show My Documents" );
const TCHAR* CONFIG_CHD_SHOWANOTHER = TEXT( "Change Drive Show Another" );
const TCHAR* CONFIG_CHD_SHOWCLOUDSTOR = TEXT( "Change Drive Show Cloud Storages" );
const TCHAR* CONFIG_CHD_SHOWNET = TEXT( "Change Drive Network" );
const TCHAR* CONFIG_CURRRENTTIPINDEX = TEXT( "Current Tip Index" );
const TCHAR* CONFIG_SEARCHFILECONTENT = TEXT( "Search File Content" );
const TCHAR* CONFIG_FINDOPTIONS_REG = TEXT( "Find Options" );
const TCHAR* CONFIG_FINDIGNORE_REG = TEXT( "Find Ignore" );
#ifdef _WIN64
const TCHAR* CONFIG_LASTPLUGINVER = TEXT( "Plugins.ver Version (x64)" );
const TCHAR* CONFIG_LASTPLUGINVER_OP = TEXT( "Plugins.ver Version (x86)" );
#else  // _WIN64
const TCHAR* CONFIG_LASTPLUGINVER = TEXT( "Plugins.ver Version (x86)" );
const TCHAR* CONFIG_LASTPLUGINVER_OP = TEXT( "Plugins.ver Version (x64)" );
#endif // _WIN64
const TCHAR* CONFIG_LANGUAGE_REG = TEXT( "Language" );
const TCHAR* CONFIG_SHOWSPLASHSCREEN_REG = TEXT( "Show Splash Screen" );
const TCHAR* CONFIG_CONVERSIONTABLE_REG = TEXT( "Conversion Table" );
const TCHAR* CONFIG_TITLEBARSHOWPATH_REG = TEXT( "Title bar show path" );
const TCHAR* CONFIG_TITLEBARMODE_REG = TEXT( "Title bar mode" );
const TCHAR* CONFIG_TITLEBARPREFIX_REG = TEXT( "Title bar prefix" );
const TCHAR* CONFIG_TITLEBARPREFIXTEXT_REG = TEXT( "Title bar prefix text" );
const TCHAR* CONFIG_MAINWINDOWICONINDEX_REG = TEXT( "Main window icon index" );
const TCHAR* CONFIG_CLICKQUICKRENAME_REG = TEXT( "Click to Quick Rename" );
const TCHAR* CONFIG_VISIBLEDRIVES_REG = TEXT( "Visible Drives" );
const TCHAR* CONFIG_SEPARATEDDRIVES_REG = TEXT( "Separated Drives" );

const TCHAR* CONFIG_COMPAREBYTIME_REG = TEXT( "Compare By Time" );
const TCHAR* CONFIG_COMPAREBYSIZE_REG = TEXT( "Compare By Size" );
const TCHAR* CONFIG_COMPAREBYCONTENT_REG = TEXT( "Compare By Content" );
const TCHAR* CONFIG_COMPAREBYATTR_REG = TEXT( "Compare By Attr" );
const TCHAR* CONFIG_COMPAREBYSUBDIRS_REG = TEXT( "Compare By Subdirs" );
const TCHAR* CONFIG_COMPAREBYSUBDIRSATTR_REG = TEXT( "Compare By Subdirs Attr" );
const TCHAR* CONFIG_COMPAREONEPANELDIRS_REG = TEXT( "Compare One Panel Dirs" );
const TCHAR* CONFIG_COMPAREMOREOPTIONS_REG = TEXT( "Compare More Options" );
const TCHAR* CONFIG_COMPAREIGNOREFILES_REG = TEXT( "Compare Ignore Files" );
const TCHAR* CONFIG_COMPAREIGNOREDIRS_REG = TEXT( "Compare Ignore Dirs" );
const TCHAR* CONFIG_CONFIGTIGNOREFILESMASKS_REG = TEXT( "Compare Ignore Files Masks" );
const TCHAR* CONFIG_CONFIGTIGNOREDIRSMASKS_REG = TEXT( "Compare Ignore Dirs Masks" );
const TCHAR* CONFIG_THUMBNAILSIZE_REG = TEXT( "Thumbnail Size" );
const TCHAR* CONFIG_ALTLANGFORPLUGINS_REG = TEXT( "Alternate Language for Plugins" );
const TCHAR* CONFIG_USEALTLANGFORPLUGINS_REG = TEXT( "Use Alternate Language for Plugins" );
const TCHAR* CONFIG_LANGUAGECHANGED_REG = TEXT( "Language Changed" );
const TCHAR* CONFIG_ENABLECUSTICOVRLS_REG = TEXT( "Enable Custom Icon Overlays" );
const TCHAR* CONFIG_DISABLEDCUSTICOVRLS_REG = TEXT( "Disabled Custom Icon Overlays" );
const TCHAR* CONFIG_COPYMOVEOPTIONS_REG = TEXT( "Copy Move Options" );
const TCHAR* CONFIG_KEEPPLUGINSSORTED_REG = TEXT( "Keep Plugins Sorted" );
const TCHAR* CONFIG_SHOWSLGINCOMPLETE_REG = TEXT( "Show Translation Is Incomplete" );

const TCHAR* CONFIG_EDITNEWFILE_USEDEFAULT_REG = TEXT( "Edit New File Use Default" );
const TCHAR* CONFIG_EDITNEWFILE_DEFAULT_REG = TEXT( "Edit New File Default" );

//const TCHAR *CONFIG_SPACESELCALCSPACE = TEXT( "Space Selecting" );
const TCHAR* CONFIG_USETIMERESOLUTION = TEXT( "Use Time Resolution" );
const TCHAR* CONFIG_TIMERESOLUTION = TEXT( "Time Resolution" );
const TCHAR* CONFIG_IGNOREDSTSHIFTS = TEXT( "Ignore DST Shifts" );

const TCHAR* CONFIG_USEDRAGDROPMINTIME = TEXT( "Use DragDrop Min Time" );
const TCHAR* CONFIG_DRAGDROPMINTIME = TEXT( "DragDrop Min Time" );

// stranky konfiguracniho dialogu
const TCHAR* CONFIG_LASTFOCUSEDPAGE = TEXT( "Last Focused Page" );
const TCHAR* CONFIG_VIEWANDEDITEXPAND = TEXT( "Viewers And Editors Expanded" );
const TCHAR* CONFIG_PACKEPAND = TEXT( "Packers And Unpackers Expanded" );
const TCHAR* CONFIG_CONFIGURATION_HEIGHT = TEXT( "Configuration Height" );

const TCHAR* CONFIG_MENUINDEX_REG = TEXT( "Menu Index" );
const TCHAR* CONFIG_MENUBREAK_REG = TEXT( "Menu Break" );
const TCHAR* CONFIG_MENUWIDTH_REG = TEXT( "Menu Width" );
const TCHAR* CONFIG_TOOLBARINDEX_REG = TEXT( "ToolBar Index" );
const TCHAR* CONFIG_TOOLBARBREAK_REG = TEXT( "ToolBar Break" );
const TCHAR* CONFIG_TOOLBARWIDTH_REG = TEXT( "ToolBar Width" );
const TCHAR* CONFIG_PLUGINSBARINDEX_REG = TEXT( "PluginsBar Index" );
const TCHAR* CONFIG_PLUGINSBARBREAK_REG = TEXT( "PluginsBar Break" );
const TCHAR* CONFIG_PLUGINSBARWIDTH_REG = TEXT( "PluginsBar Width" );
const TCHAR* CONFIG_USERMENUINDEX_REG = TEXT( "User Menu Index" );
const TCHAR* CONFIG_USERMENUBREAK_REG = TEXT( "User Menu Break" );
const TCHAR* CONFIG_USERMENUWIDTH_REG = TEXT( "User Menu Width" );
const TCHAR* CONFIG_USERMENULABELS_REG = TEXT( "User Menu Labels" );
const TCHAR* CONFIG_HOTPATHSINDEX_REG = TEXT( "Hot Paths Index" );
const TCHAR* CONFIG_HOTPATHSBREAK_REG = TEXT( "Hot Paths Break" );
const TCHAR* CONFIG_HOTPATHSWIDTH_REG = TEXT( "Hot Paths Width" );
const TCHAR* CONFIG_DRIVEBARINDEX_REG = TEXT( "Drive Bar Index" );
const TCHAR* CONFIG_DRIVEBARBREAK_REG = TEXT( "Drive Bar Break" );
const TCHAR* CONFIG_DRIVEBARWIDTH_REG = TEXT( "Drive Bar Width" );
const TCHAR* CONFIG_GRIPSVISIBLE_REG = TEXT( "Grips Visible" );

const TCHAR* SALAMANDER_CONFIRMATION_REG = TEXT( "Confirmation" );
const TCHAR* CONFIG_CNFRM_FILEDIRDEL = TEXT( "Files or Dirs Del" );
const TCHAR* CONFIG_CNFRM_NEDIRDEL = TEXT( "Non-empty Dir Del" );
const TCHAR* CONFIG_CNFRM_FILEOVER = TEXT( "File Overwrite" );
const TCHAR* CONFIG_CNFRM_DIROVER = TEXT( "Directory Overwrite" );
const TCHAR* CONFIG_CNFRM_SHFILEDEL = TEXT( "SH File Del" );
const TCHAR* CONFIG_CNFRM_SHDIRDEL = TEXT( "SH Dir Del" );
const TCHAR* CONFIG_CNFRM_SHFILEOVER = TEXT( "SH File Overwrite" );
const TCHAR* CONFIG_CNFRM_NTFSPRESS = TEXT( "NTFS Compress and Uncompress" );
const TCHAR* CONFIG_CNFRM_NTFSCRYPT = TEXT( "NTFS Encrypt and Decrypt" );
const TCHAR* CONFIG_CNFRM_DAD = TEXT( "Drag and Drop" );
const TCHAR* CONFIG_CNFRM_CLOSEARCHIVE = TEXT( "Close Archive" );
const TCHAR* CONFIG_CNFRM_CLOSEFIND = TEXT( "Close Find" );
const TCHAR* CONFIG_CNFRM_STOPFIND = TEXT( "Stop Find" );
const TCHAR* CONFIG_CNFRM_CREATETARGETPATH = TEXT( "Create Target Path" );
const TCHAR* CONFIG_CNFRM_ALWAYSONTOP = TEXT( "Always on Top" );
const TCHAR* CONFIG_CNFRM_ONSALCLOSE = TEXT( "Close Salamander" );
const TCHAR* CONFIG_CNFRM_SENDEMAIL = TEXT( "Send Email" );
const TCHAR* CONFIG_CNFRM_ADDTOARCHIVE = TEXT( "Add To Archive" );
const TCHAR* CONFIG_CNFRM_CREATEDIR = TEXT( "Create Dir" );
const TCHAR* CONFIG_CNFRM_CHANGEDIRTC = TEXT( "Change Dir TC" );
const TCHAR* CONFIG_CNFRM_SHOWNAMETOCOMP = TEXT( "Show Names To Compare" );
const TCHAR* CONFIG_CNFRM_DSTSHIFTSIGNORED = TEXT( "DST Shifts Ignored" );
const TCHAR* CONFIG_CNFRM_DSTSHIFTSOCCURED = TEXT( "DST Shifts Occured" );
const TCHAR* CONFIG_CNFRM_COPYMOVEOPTIONSNS = TEXT( "Copy Move Options Not Supported" );

const TCHAR* SALAMANDER_DRVSPEC_REG = TEXT( "Drive Special Settings" );
const TCHAR* CONFIG_DRVSPEC_FLOPPY_MON = TEXT( "Floppy Automatic Refresh" );
const TCHAR* CONFIG_DRVSPEC_FLOPPY_SIMPLE = TEXT( "Floppy Simple Icons" );
const TCHAR* CONFIG_DRVSPEC_REMOVABLE_MON = TEXT( "Removable Automatic Refresh" );
const TCHAR* CONFIG_DRVSPEC_REMOVABLE_SIMPLE = TEXT( "Removable Simple Icons" );
const TCHAR* CONFIG_DRVSPEC_FIXED_MON = TEXT( "Fixed Automatic Refresh" );
const TCHAR* CONFIG_DRVSPEC_FIXED_SIMPLE = TEXT( "Fixed Simple Icons" );
const TCHAR* CONFIG_DRVSPEC_REMOTE_MON = TEXT( "Remote Automatic Refresh" );
const TCHAR* CONFIG_DRVSPEC_REMOTE_SIMPLE = TEXT( "Remote Simple Icons" );
const TCHAR* CONFIG_DRVSPEC_REMOTE_ACT = TEXT( "Remote Do Not Refresh on Activation" );
const TCHAR* CONFIG_DRVSPEC_CDROM_MON = TEXT( "CDROM Automatic Refresh" );
const TCHAR* CONFIG_DRVSPEC_CDROM_SIMPLE = TEXT( "CDROM Simple Icons" );

const TCHAR* SALAMANDER_HOTPATHS_REG = TEXT( "Hot Paths" );

const TCHAR* SALAMANDER_VIEWTEMPLATES_REG = TEXT( "View Templates" );

const TCHAR* SALAMANDER_VIEWER_REG = TEXT( "Viewer" );
const TCHAR* VIEWER_FINDFORWARD_REG = TEXT( "Forward Direction" );
const TCHAR* VIEWER_FINDWHOLEWORDS_REG = TEXT( "Whole Words" );
const TCHAR* VIEWER_FINDCASESENSITIVE_REG = TEXT( "Case Sensitive" );
const TCHAR* VIEWER_FINDTEXT_REG = TEXT( "Find Text" );
const TCHAR* VIEWER_FINDHEXMODE_REG = TEXT( "HEX-mode" );
const TCHAR* VIEWER_FINDREGEXP_REG = TEXT( "Regular Expression" );
const TCHAR* VIEWER_CONFIGCRLF_REG = TEXT( "EOL CRLF" );
const TCHAR* VIEWER_CONFIGCR_REG = TEXT( "EOL CR" );
const TCHAR* VIEWER_CONFIGLF_REG = TEXT( "EOL LF" );
const TCHAR* VIEWER_CONFIGNULL_REG = TEXT( "EOL NULL" );
const TCHAR* VIEWER_CONFIGTABSIZE_REG = TEXT( "Tabelator Size" );
const TCHAR* VIEWER_CONFIGDEFMODE_REG = TEXT( "Default Mode" );
const TCHAR* VIEWER_CONFIGTEXTMASK_REG = TEXT( "Text Masks" );
const TCHAR* VIEWER_CONFIGHEXMASK_REG = TEXT( "Hex Masks" );
const TCHAR* VIEWER_CONFIGUSECUSTOMFONT_REG = TEXT( "Viewer Use Custom Font" );
const TCHAR* VIEWER_CONFIGFONT_REG = TEXT( "Viewer Font" );
const TCHAR* VIEWER_WRAPTEXT_REG = TEXT( "Wrap Text" );
const TCHAR* VIEWER_CPAUTOSELECT_REG = TEXT( "Auto-Select" );
const TCHAR* VIEWER_DEFAULTCONVERT_REG = TEXT( "Default Convert" );
const TCHAR* VIEWER_AUTOCOPYSELECTION_REG = TEXT( "Auto-Copy Selection" );
const TCHAR* VIEWER_GOTOOFFSETISHEX_REG = TEXT( "Go to Offset Is Hex" );

const TCHAR* VIEWER_CONFIGSAVEWINPOS_REG = TEXT( "Save Window Position" );
const TCHAR* VIEWER_CONFIGWNDLEFT_REG = TEXT( "Left" );
const TCHAR* VIEWER_CONFIGWNDRIGHT_REG = TEXT( "Right" );
const TCHAR* VIEWER_CONFIGWNDTOP_REG = TEXT( "Top" );
const TCHAR* VIEWER_CONFIGWNDBOTTOM_REG = TEXT( "Bottom" );
const TCHAR* VIEWER_CONFIGWNDSHOW_REG = TEXT( "Show" );

const TCHAR* SALAMANDER_USERMENU_REG = TEXT( "User Menu" );
const TCHAR* USERMENU_ITEMNAME_REG = TEXT( "Item Name" );
const TCHAR* USERMENU_COMMAND_REG = TEXT( "Command" );
const TCHAR* USERMENU_ARGUMENTS_REG = TEXT( "Arguments" );
const TCHAR* USERMENU_INITDIR_REG = TEXT( "Initial Directory" );
const TCHAR* USERMENU_SHELL_REG = TEXT( "Execute Through Shell" );
const TCHAR* USERMENU_USEWINDOW_REG = TEXT( "Open Shell Window" );
const TCHAR* USERMENU_CLOSE_REG = TEXT( "Close Shell Window" );
const TCHAR* USERMENU_SEPARATOR_REG = TEXT( "Separator" );
const TCHAR* USERMENU_SHOWINTOOLBAR_REG = TEXT( "Show In Toolbar" );
const TCHAR* USERMENU_TYPE_REG = TEXT( "Type" );
const TCHAR* USERMENU_ICON_REG = TEXT( "Icon" );

const TCHAR* SALAMANDER_VIEWERS_REG = TEXT( "Viewers" );
const TCHAR* SALAMANDER_ALTVIEWERS_REG = TEXT( "Alternative Viewers" );
const TCHAR* VIEWERS_MASKS_REG = TEXT( "Masks" );
const TCHAR* VIEWERS_COMMAND_REG = USERMENU_COMMAND_REG;
const TCHAR* VIEWERS_ARGUMENTS_REG = USERMENU_ARGUMENTS_REG;
const TCHAR* VIEWERS_INITDIR_REG = USERMENU_INITDIR_REG;
const TCHAR* VIEWERS_TYPE_REG = TEXT( "Type" );

const TCHAR* SALAMANDER_IZIP_REG = TEXT( "Internal ZIP Packer" );

const TCHAR* SALAMANDER_EDITORS_REG = TEXT( "Editors" );
const TCHAR* EDITORS_MASKS_REG = VIEWERS_MASKS_REG;
const TCHAR* EDITORS_COMMAND_REG = USERMENU_COMMAND_REG;
const TCHAR* EDITORS_ARGUMENTS_REG = USERMENU_ARGUMENTS_REG;
const TCHAR* EDITORS_INITDIR_REG = USERMENU_INITDIR_REG;

const TCHAR* SALAMANDER_VERSION_REG = TEXT( "Version" );
const TCHAR* SALAMANDER_VERSIONREG_REG = TEXT( "Configuration" );

const TCHAR* SALAMANDER_CUSTOMCOLORS_REG = TEXT( "Custom Colors" );

// barvy
const TCHAR* SALAMANDER_COLORS_REG = TEXT( "Colors" );
const TCHAR* SALAMANDER_CLR_FOCUS_ACTIVE_NORMAL_REG = TEXT( "Focus Active Normal" );
const TCHAR* SALAMANDER_CLR_FOCUS_ACTIVE_SELECTED_REG = TEXT( "Focus Active Selected" );
const TCHAR* SALAMANDER_CLR_FOCUS_INACTIVE_NORMAL_REG = TEXT( "Focus Inactive Normal" );
const TCHAR* SALAMANDER_CLR_FOCUS_INACTIVE_SELECTED_REG = TEXT( "Focus Inactive Selected" );
const TCHAR* SALAMANDER_CLR_FOCUS_BK_INACTIVE_NORMAL_REG = TEXT( "Focus Bk Inactive Normal" );
const TCHAR* SALAMANDER_CLR_FOCUS_BK_INACTIVE_SELECTED_REG = TEXT( "Focus Bk Inactive Selected" );

const TCHAR* SALAMANDER_CLR_ITEM_FG_NORMAL_REG = TEXT( "Item Fg Normal" );
const TCHAR* SALAMANDER_CLR_ITEM_FG_SELECTED_REG = TEXT( "Item Fg Selected" );
const TCHAR* SALAMANDER_CLR_ITEM_FG_FOCUSED_REG = TEXT( "Item Fg Focused" );
const TCHAR* SALAMANDER_CLR_ITEM_FG_FOCSEL_REG = TEXT( "Item Fg Focused and Selected" );
const TCHAR* SALAMANDER_CLR_ITEM_FG_HIGHLIGHT_REG = TEXT( "Item Fg Highlight" );

const TCHAR* SALAMANDER_CLR_ITEM_BK_NORMAL_REG = TEXT( "Item Bk Normal" );
const TCHAR* SALAMANDER_CLR_ITEM_BK_SELECTED_REG = TEXT( "Item Bk Selected" );
const TCHAR* SALAMANDER_CLR_ITEM_BK_FOCUSED_REG = TEXT( "Item Bk Focused" );
const TCHAR* SALAMANDER_CLR_ITEM_BK_FOCSEL_REG = TEXT( "Item Bk Focused and Selected" );
const TCHAR* SALAMANDER_CLR_ITEM_BK_HIGHLIGHT_REG = TEXT( "Item Bk Highlight" );

const TCHAR* SALAMANDER_CLR_ICON_BLEND_SELECTED_REG = TEXT( "Icon Blend Selected" );
const TCHAR* SALAMANDER_CLR_ICON_BLEND_FOCUSED_REG = TEXT( "Icon Blend Focused" );
const TCHAR* SALAMANDER_CLR_ICON_BLEND_FOCSEL_REG = TEXT( "Icon Blend Focused and Selected" );

const TCHAR* SALAMANDER_CLR_PROGRESS_FG_NORMAL_REG = TEXT( "Progress Fg Normal" );
const TCHAR* SALAMANDER_CLR_PROGRESS_FG_SELECTED_REG = TEXT( "Progress Fg Selected" );
const TCHAR* SALAMANDER_CLR_PROGRESS_BK_NORMAL_REG = TEXT( "Progress Bk Normal" );
const TCHAR* SALAMANDER_CLR_PROGRESS_BK_SELECTED_REG = TEXT( "Progress Bk Selected" );

const TCHAR* SALAMANDER_CLR_VIEWER_FG_NORMAL_REG = TEXT( "Viewer Fg Normal" );
const TCHAR* SALAMANDER_CLR_VIEWER_BK_NORMAL_REG = TEXT( "Viewer Bk Normal" );
const TCHAR* SALAMANDER_CLR_VIEWER_FG_SELECTED_REG = TEXT( "Viewer Fg Selected" );
const TCHAR* SALAMANDER_CLR_VIEWER_BK_SELECTED_REG = TEXT( "Viewer Bk Selected" );

const TCHAR* SALAMANDER_CLR_HOT_PANEL_REG = TEXT( "Hot Panel" );
const TCHAR* SALAMANDER_CLR_HOT_ACTIVE_REG = TEXT( "Hot Active" );
const TCHAR* SALAMANDER_CLR_HOT_INACTIVE_REG = TEXT( "Hot Inactive" );

const TCHAR* SALAMANDER_CLR_ACTIVE_CAPTION_FG_REG = TEXT( "Active Caption Fg" );
const TCHAR* SALAMANDER_CLR_ACTIVE_CAPTION_BK_REG = TEXT( "Active Caption Bk" );
const TCHAR* SALAMANDER_CLR_INACTIVE_CAPTION_FG_REG = TEXT( "Inactive Caption Fg" );
const TCHAR* SALAMANDER_CLR_INACTIVE_CAPTION_BK_REG = TEXT( "Inactive Caption Bk" );

const TCHAR* SALAMANDER_CLR_THUMBNAIL_FRAME_NORMAL_REG = TEXT( "Thumbnail Frame Normal" );
const TCHAR* SALAMANDER_CLR_THUMBNAIL_FRAME_SELECTED_REG = TEXT( "Thumbnail Frame Selected" );
const TCHAR* SALAMANDER_CLR_THUMBNAIL_FRAME_FOCUSED_REG = TEXT( "Thumbnail Frame Focused" );
const TCHAR* SALAMANDER_CLR_THUMBNAIL_FRAME_FOCSEL_REG = TEXT( "Thumbnail Frame Focused and Selected" );

const TCHAR* SALAMANDER_HLT = TEXT( "Panel Items Hilighting" );
const TCHAR* SALAMANDER_HLT_ITEM_MASKS = TEXT( "Masks" );
const TCHAR* SALAMANDER_HLT_ITEM_ATTR = TEXT( "Attributes" );
const TCHAR* SALAMANDER_HLT_ITEM_VALIDATTR = TEXT( "Valid Attributes" );
const TCHAR* SALAMANDER_HLT_ITEM_FG_NORMAL_REG = TEXT( "Item Fg Normal" );
const TCHAR* SALAMANDER_HLT_ITEM_FG_SELECTED_REG = TEXT( "Item Fg Selected" );
const TCHAR* SALAMANDER_HLT_ITEM_FG_FOCUSED_REG = TEXT( "Item Fg Focused" );
const TCHAR* SALAMANDER_HLT_ITEM_FG_FOCSEL_REG = TEXT( "Item Fg Focused and Selected" );
const TCHAR* SALAMANDER_HLT_ITEM_FG_HIGHLIGHT_REG = TEXT( "Item Fg Highlight" );
const TCHAR* SALAMANDER_HLT_ITEM_BK_NORMAL_REG = TEXT( "Item Bk Normal" );
const TCHAR* SALAMANDER_HLT_ITEM_BK_SELECTED_REG = TEXT( "Item Bk Selected" );
const TCHAR* SALAMANDER_HLT_ITEM_BK_FOCUSED_REG = TEXT( "Item Bk Focused" );
const TCHAR* SALAMANDER_HLT_ITEM_BK_FOCSEL_REG = TEXT( "Item Bk Focused and Selected" );
const TCHAR* SALAMANDER_HLT_ITEM_BK_HIGHLIGHT_REG = TEXT( "Item Bk Highlight" );

const TCHAR* SALAMANDER_CLRSCHEME_REG = TEXT( "Color Scheme" );

// Plugins
const TCHAR* SALAMANDER_PLUGINS = TEXT( "Plugins" );
const TCHAR* SALAMANDER_PLUGINS_NAME = TEXT( "Name" );
const TCHAR* SALAMANDER_PLUGINS_DLLNAME = TEXT( "DLL" );
const TCHAR* SALAMANDER_PLUGINS_VERSION = TEXT( "Version" );
const TCHAR* SALAMANDER_PLUGINS_COPYRIGHT = TEXT( "Copyright" );
const TCHAR* SALAMANDER_PLUGINS_EXTENSIONS = TEXT( "Extensions" );
const TCHAR* SALAMANDER_PLUGINS_DESCRIPTION = TEXT( "Description" );
const TCHAR* SALAMANDER_PLUGINS_LASTSLGNAME = TEXT( "LastSLGName" );
const TCHAR* SALAMANDER_PLUGINS_HOMEPAGE = TEXT( "HomePage" );
//const char *SALAMANDER_PLUGINS_PLGICONS = TEXT( "PluginIcons" );
const TCHAR* SALAMANDER_PLUGINS_PLGICONLIST = TEXT( "PluginIconList" );
const TCHAR* SALAMANDER_PLUGINS_PLGICONINDEX = TEXT( "PluginIconIndex" );
const TCHAR* SALAMANDER_PLUGINS_PLGSUBMENUICONINDEX = TEXT( "SubmenuIconIndex" );
const TCHAR* SALAMANDER_PLUGINS_SUBMENUINPLUGINSBAR = TEXT( "SubmenuInPluginsBar" );
const TCHAR* SALAMANDER_PLUGINS_THUMBMASKS = TEXT( "ThumbnailMasks" );
const TCHAR* SALAMANDER_PLUGINS_REGKEYNAME = TEXT( "Configuration Key" );
const TCHAR* SALAMANDER_PLUGINS_FSNAME = TEXT( "FS Name" );
const TCHAR* SALAMANDER_PLUGINS_FUNCTIONS = TEXT( "Functions" );
const TCHAR* SALAMANDER_PLUGINS_LOADONSTART = TEXT( "Load On Start" );
const TCHAR* SALAMANDER_PLUGINS_MENU = TEXT( "Menu" );
const TCHAR* SALAMANDER_PLUGINS_MENUITEMNAME = TEXT( "Name" );
const TCHAR* SALAMANDER_PLUGINS_MENUITEMSTATE = TEXT( "State" );
const TCHAR* SALAMANDER_PLUGINS_MENUITEMID = TEXT( "ID" );
const TCHAR* SALAMANDER_PLUGINS_MENUITEMSKILLLEVEL = TEXT( "Skill" );
const TCHAR* SALAMANDER_PLUGINS_MENUITEMICONINDEX = TEXT( "Icon" );
const TCHAR* SALAMANDER_PLUGINS_MENUITEMTYPE = TEXT( "Type" );
const TCHAR* SALAMANDER_PLUGINS_MENUITEMHOTKEY = TEXT( "HotKey" );
const TCHAR* SALAMANDER_PLUGINS_FSCMDNAME = TEXT( "FS Cmd Name" );
const TCHAR* SALAMANDER_PLUGINS_FSCMDICON = TEXT( "FS Cmd Icon" );
const TCHAR* SALAMANDER_PLUGINS_FSCMDVISIBLE = TEXT( "FS Cmd Visible" );
const TCHAR* SALAMANDER_PLUGINS_ISNETHOOD = TEXT( "Is Nethood" );
const TCHAR* SALAMANDER_PLUGINS_USESPASSWDMAN = TEXT( "Uses Password Manager" );

// Plugins: nasl. 8 retezcu je jen pro konverzi z konfigurace verze 6 a nizsich
const TCHAR* SALAMANDER_PLUGINS_PANELVIEW = TEXT( "Panel List" );
const TCHAR* SALAMANDER_PLUGINS_PANELEDIT = TEXT( "Panel Pack" );
const TCHAR* SALAMANDER_PLUGINS_CUSTPACK = TEXT( "Custom Pack" );
const TCHAR* SALAMANDER_PLUGINS_CUSTUNPACK = TEXT( "Custom Unpack" );
const TCHAR* SALAMANDER_PLUGINS_CONFIG = TEXT( "Configuration" );
const TCHAR* SALAMANDER_PLUGINS_LOADSAVE = TEXT( "Persistent" );
const TCHAR* SALAMANDER_PLUGINS_VIEWER = TEXT( "File Viewer" );
const TCHAR* SALAMANDER_PLUGINS_FS = TEXT( "File System" );

// Plugins Configuration
const TCHAR* SALAMANDER_PLUGINSCONFIG = TEXT( "Plugins Configuration" );

// Plugins Order
const TCHAR* SALAMANDER_PLUGINSORDER = TEXT( "Plugins Order" );
const TCHAR* SALAMANDER_PLUGINSORDER_SHOW = TEXT( "ShowInBar" );

// Packers & Unpackers
const TCHAR* SALAMANDER_PACKANDUNPACK = TEXT( "Packers & Unpackers" );
const TCHAR* SALAMANDER_CUSTOMPACKERS = TEXT( "Custom Packers" );
const TCHAR* SALAMANDER_CUSTOMUNPACKERS = TEXT( "Custom Unpackers" );
const TCHAR* SALAMANDER_PREDPACKERS = TEXT( "Predefined Packers" );
const TCHAR* SALAMANDER_ARCHIVEASSOC = TEXT( "Archive Association" );
// pro SALAMANDER_CUSTOMPACKERS i SALAMANDER_CUSTOMUNPACKERS
const TCHAR* SALAMANDER_ANOTHERPANEL = TEXT( "Use Another Panel" );
const TCHAR* SALAMANDER_PREFFERED = TEXT( "Preffered" );
const TCHAR* SALAMANDER_NAMEBYARCHIVE = TEXT( "Use Subdir Name By Archive" );
const TCHAR* SALAMANDER_SIMPLEICONSINARCHIVES = TEXT( "Simple Icons In Archives" );

const TCHAR* SALAMANDER_PWDMNGR_REG = TEXT( "Password Manager" );

//****************************************************************************
//
// GetUpgradeInfo
//
// Zkusi v konfiguracnim klici teto verze Salama najit "AutoImportConfig". Pokud ho nenajde
// nebo klic ulozeny v AutoImportConfig neexistuje, ukazuje do klice teto verze (coz je nesmysl)
// nebo obsahuje poskozenou (nedokoncene ukladani konfigurace) nebo prazdnou konfiguraci, vraci
// v 'autoImportConfig' FALSE. V opacnem pripade vraci v 'autoImportConfig' TRUE a
// v 'autoImportConfigFromKey' vraci cestu ke klici, ze ktereho se ma konfigurace importovat.
// Resi situace, kdy AutoImportConfig ukazuje na klic, ktery obsahuje AutoImportConfig
// na dalsi klic (jen projdeme na "cilovy" klic a mezilehle klice nechame beze zmen - pokud
// se import podari, stejne smazneme cilovy klic). Vraci FALSE jen pokud ma dojit k exitu softu.
//
// Pokud konfiguracni klic teto verze Salama obsahuje krome AutoImportConfig jeste taky
// klic "Configuration" (ocekavame, ze je to ulozena konfigurace), zeptame se usera jestli chce:
//   -pouzit aktualni konfiguraci a ignorovat konfiguraci stare verze (nemazal bych ji, sice
//    by to bylo logictejsi, ale zato by chudaci mohli prijit o data a zase tolik to registry
//    nezasira) - v tomto pripade ihned smaznout AutoImportConfig
//    (tohle se provede tise, pokud AutoImportConfig vede do konfiguracniho klice teto verze Salama)
//    (NABIZIME DEFAULTNE, protoze nevede ke ztrate dat, lidi radi odmackavaji msgboxy bez cteni)
//   -smazat aktualni konfiguraci a importovat konfiguraci stare verze - v tom pripade ihned
//    smaznout vse az na AutoImportConfig
//   -exit softu - jen vratit FALSE

BOOL GetUpgradeInfo(BOOL* autoImportConfig, char* autoImportConfigFromKey, int autoImportConfigFromKeySize)
{
    HKEY rootKey;
    DWORD saveInProgress; // dummy
    BOOL doNotExit = TRUE;
    if (autoImportConfigFromKeySize > 0)
        *autoImportConfigFromKey = 0;
    LoadSaveToRegistryMutex.Enter();
    int rounds = 0; // prevence zacykleni
    *autoImportConfig = FALSE;
    if (HANDLES_Q(RegOpenKeyEx(HKEY_CURRENT_USER, SalamanderConfigurationRoots[0], 0,
                               KEY_READ, &rootKey)) == ERROR_SUCCESS)
    {
        HKEY oldCfgKey;
        char oldKeyName[200];

        if (GetValue(rootKey, SALAMANDER_AUTO_IMPORT_CONFIG, REG_SZ, oldKeyName, 200))
        { // nasli jsme "AutoImportConfig"
        OPEN_AUTO_IMPORT_CONFIG_KEY:
            lstrcpyn(autoImportConfigFromKey, SalamanderConfigurationRoots[0], autoImportConfigFromKeySize);
            if (CutDirectory(autoImportConfigFromKey) &&
                SalPathAppend(autoImportConfigFromKey, oldKeyName, autoImportConfigFromKeySize) &&
                !IsTheSamePath(autoImportConfigFromKey, SalamanderConfigurationRoots[0]) &&     // klic ulozeny v AutoImportConfig neukazuje do klice teto verze
                HANDLES_Q(RegOpenKeyEx(HKEY_CURRENT_USER, autoImportConfigFromKey, 0, KEY_READ, // klic ulozeny v AutoImportConfig jde otevrit (jinak neexistuje?)
                                       &oldCfgKey)) == ERROR_SUCCESS)
            {
                // pokud aktualni "cilovy" klic obsahuje AutoImportConfig, projdeme jim...
                if (GetValue(oldCfgKey, SALAMANDER_AUTO_IMPORT_CONFIG, REG_SZ, oldKeyName, 200) && ++rounds <= 50)
                {
                    HANDLES(RegCloseKey(oldCfgKey));
                    goto OPEN_AUTO_IMPORT_CONFIG_KEY;
                }
                HKEY cfgKey;
                if (rounds <= 50 &&
                    !GetValue(oldCfgKey, SALAMANDER_SAVE_IN_PROGRESS, REG_DWORD, &saveInProgress, sizeof(DWORD)) &&
                    HANDLES_Q(RegOpenKeyEx(oldCfgKey, SALAMANDER_CONFIG_REG, 0, KEY_READ, &cfgKey)) == ERROR_SUCCESS)
                {
                    HANDLES(RegCloseKey(cfgKey));
                    *autoImportConfig = TRUE; // nejde o poskozenou ani prazdnou konfiguraci
                }
                HANDLES(RegCloseKey(oldCfgKey));
            }
        }
        if (*autoImportConfig) // zkontrolujeme jestli klic teto verze neobsahuje taky konfiguraci (krome "AutoImportConfig")
        {
            HKEY cfgKey;
            lstrcpyn(oldKeyName, SalamanderConfigurationRoots[0], 200);
            if (SalPathAppend(oldKeyName, SALAMANDER_CONFIG_REG, 200) &&
                HANDLES_Q(RegOpenKeyEx(HKEY_CURRENT_USER, oldKeyName, 0, KEY_READ, &cfgKey)) == ERROR_SUCCESS)
            {
                HANDLES(RegCloseKey(cfgKey));
                BOOL clearCfg = FALSE;
                if (!GetValue(rootKey, SALAMANDER_SAVE_IN_PROGRESS, REG_DWORD, &saveInProgress, sizeof(DWORD)))
                { // klic teto verze obsahuje neposkozenou konfiguraci, zeptame se usera co s tim
                    HANDLES(RegCloseKey(rootKey));
                    rootKey = NULL;
                    LoadSaveToRegistryMutex.Leave();

                    MSGBOXEX_PARAMS params;
                    memset(&params, 0, sizeof(params));
                    params.HParent = NULL;
                    params.Flags = MB_ABORTRETRYIGNORE | MB_ICONQUESTION | MB_SETFOREGROUND;
                    params.Caption = SALAMANDER_TEXT_VERSION;
                    lstrcpyn(oldKeyName, autoImportConfigFromKey, 200);
                    char* keyName;
                    if (!CutDirectory(oldKeyName, &keyName))
                        keyName = oldKeyName; // (teoreticky nemuze nastat)
                    char buf[1000];
                    sprintf(buf, "You have upgraded from %s (old version) to %s (new version). The configuration of the old "
                                 "version should be imported to the new version now, but there is already existing "
                                 "configuration for the new version. You can use this existing configuration (the configuration of "
                                 "the old version remains in registry, so you can import it later). Or you can overwrite "
                                 "this existing configuration (it would be lost) with the configuration of the old version. "
                                 "Or you can exit Open Salamander and solve this problem later.",
                            keyName, SALAMANDER_TEXT_VERSION);
                    params.Text = buf;
                    char aliasBtnNames[200];
                    sprintf(aliasBtnNames, "%d\t%s\t%d\t%s\t%d\t%s",
                            DIALOG_ABORT, "&Use Existing Configuration",
                            DIALOG_RETRY, "&Overwrite Existing Configuration",
                            DIALOG_IGNORE, "&Exit");
                    params.AliasBtnNames = aliasBtnNames;
                    int res = SalMessageBoxEx(&params);
                    switch (res)
                    {
                    case DIALOG_ABORT:
                        *autoImportConfig = FALSE;
                        break;
                    case DIALOG_RETRY:
                        clearCfg = TRUE;
                        break;

                    // case DIALOG_IGNORE:
                    default:
                        doNotExit = FALSE;
                        break;
                    }

                    LoadSaveToRegistryMutex.Enter();
                }
                else
                    clearCfg = TRUE; // obsazena konfigurace je poskozena, smazneme ji
                if (clearCfg &&
                    HANDLES_Q(RegOpenKeyEx(HKEY_CURRENT_USER, SalamanderConfigurationRoots[0], 0,
                                           KEY_READ | KEY_WRITE, &cfgKey)) == ERROR_SUCCESS)
                { // smazneme konfiguraci, nechame (respektive znovu vytvorime) tam jen "AutoImportConfig"
                    ClearKey(cfgKey);
                    lstrcpyn(oldKeyName, autoImportConfigFromKey, 200);
                    char* keyName;
                    if (!CutDirectory(oldKeyName, &keyName))
                        keyName = oldKeyName; // (teoreticky nemuze nastat)
                    SetValue(cfgKey, SALAMANDER_AUTO_IMPORT_CONFIG, REG_SZ, keyName, -1);
                    HANDLES(RegCloseKey(cfgKey));
                }
            }
        }
        if (rootKey != NULL)
            HANDLES(RegCloseKey(rootKey));
    }
    if (!*autoImportConfig && // klic teto verze neobsahuje "AutoImportConfig" nebo nevede na "platnou" starou konfiguraci
        HANDLES_Q(RegOpenKeyEx(HKEY_CURRENT_USER, SalamanderConfigurationRoots[0], 0,
                               KEY_READ | KEY_WRITE, &rootKey)) == ERROR_SUCCESS)
    { // v klici teto verze zrusime hodnotu "AutoImportConfig" (pokud tam vubec je, tak tam nema smysl)
        RegDeleteValue(rootKey, SALAMANDER_AUTO_IMPORT_CONFIG);
        HANDLES(RegCloseKey(rootKey));
    }
    LoadSaveToRegistryMutex.Leave();
    return doNotExit;
}

//****************************************************************************
//
// FindLanguageFromPrevVerOfSal
//
// Vytahne ze starsi verze Salamandera jazyk (pouzity .slg modul). Nejstarsi verze,
// ze ktere tuto informaci ziskavame je 2.53 beta 2 (prvni verze dodavana s vice jazyky: CZ+DE+EN).
// Pokud existuje konfigurace aktualni verze nebo nenajde takovy jazyk, vrati FALSE.
// Jinak vraci jazyk v 'slgName' (buffer MAX_PATH znaku).

BOOL FindLanguageFromPrevVerOfSal(char* slgName)
{
    HKEY hCfgKey;
    HKEY hRootKey;
    int rootIndex = 0;
    const TCHAR* root;
    DWORD saveInProgress; // dummy

    slgName[0] = 0;
    LoadSaveToRegistryMutex.Enter();
    do
    {
        // zjistim, jestli klic existuje a je pod nim ulozena konfigurace
        root = SalamanderConfigurationRoots[rootIndex];
        BOOL rootFound = HANDLES_Q(RegOpenKeyEx(HKEY_CURRENT_USER, root, 0, KEY_READ, &hRootKey)) == ERROR_SUCCESS;
        BOOL cfgFound = rootFound && HANDLES_Q(RegOpenKeyEx(hRootKey, SALAMANDER_CONFIG_REG, 0,
                                                            KEY_READ, &hCfgKey)) == ERROR_SUCCESS;
        if (cfgFound && GetValue(hRootKey, SALAMANDER_SAVE_IN_PROGRESS, REG_DWORD, &saveInProgress, sizeof(DWORD)))
        { // jde o poskozenou konfiguraci
            cfgFound = FALSE;
            HANDLES(RegCloseKey(hCfgKey));
        }
        DWORD configVersion = 1; // toto je konfig od 1.52 a starsi
        if (cfgFound)
        {
            HKEY actKey;
            if (HANDLES_Q(RegOpenKeyEx(hRootKey, SALAMANDER_VERSION_REG, 0, KEY_READ, &actKey) == ERROR_SUCCESS))
            {
                configVersion = 2; // toto je konfig od 1.6b1
                GetValue(actKey, SALAMANDER_VERSIONREG_REG, REG_DWORD, &configVersion, sizeof(DWORD));
                HANDLES(RegCloseKey(actKey));
            }
        }
        if (rootFound)
            HANDLES(RegCloseKey(hRootKey));
        if (cfgFound)
        {
            BOOL found = FALSE;
            if (rootIndex != 0 &&                      // jen pokud se jedna o jeden ze starsich klicu
                configVersion >= 59 /* 2.53 beta 2 */) // pred 2.53 beta 2 byla jen anglictina, tedy cteni nema smysl, nabidneme userovi defaultni jazyk systemu nebo rucni vyber jazyku
            {
                GetValue(hCfgKey, CONFIG_LANGUAGE_REG, REG_SZ, slgName, MAX_PATH);
                found = slgName[0] != 0;
            }
            HANDLES(RegCloseKey(hCfgKey));
            LoadSaveToRegistryMutex.Leave();
            return found;
        }
        rootIndex++;
    } while (rootIndex < SALCFG_ROOTS_COUNT);

    LoadSaveToRegistryMutex.Leave();
    return FALSE;
}

// ziska cislo z retezce (decimalni format cisla bez znamenka); vraci TRUE pokud tam bylo cislo;
// ignoruje white-spaces pred a za cislem
BOOL GetNumFromStr(const TCHAR* s, DWORD* retNum)
{
    DWORD n = 0;
    while (*s != 0 && *s <= ' ')
        s++;
    BOOL mayBeOK = *s >= '0' && *s <= '9';
    while (*s >= '0' && *s <= '9')
        n = 10 * n + (*s++ - '0');
    while (*s != 0 && *s <= ' ')
        s++;
    *retNum = n;
    return mayBeOK && *s == 0;
}

void CheckShutdownParams()
{
    // HKEY_CURRENT_USER\Control Panel\Desktop\WaitToKillAppTimeout=20000,REG_SZ  ... mene nez 20000, rvat!
    // HKEY_CURRENT_USER\Control Panel\Desktop\AutoEndTasks=0,REG_SZ              ... neni 0, rvat!
    // W2K a XP to maji, na Viste jsem to nenasel, ale pry je to tam taky (info z internetu)

    BOOL showWarning = FALSE;
    HKEY key;
    if ( Registry::Silent_Key_Open( HKEY_CURRENT_USER, TEXT_VIEW( "Control Panel\\Desktop" ), key ) )
    {
        String_TChar    num;
        DWORD           value;

        if (
            Registry::Silent_Value_Get( key, TEXT_VIEW( "WaitToKillAppTimeout" ), num )
            &&
            GetNumFromStr( num.Text_Get(), &value )
            &&
            ( value < 20000 )
        )
        {
            TRACE_E("CheckShutdownParams(): WaitToKillAppTimeout is '" << num.Text_Get() << "' (" << value << ")");
            showWarning = TRUE;
        }
        if (
            Registry::Silent_Value_Get( key, TEXT_VIEW( "AutoEndTasks" ), num )
            &&
            GetNumFromStr( num.Text_Get(), &value)
            &&
            ( value != 0 )
        )
        {
            TRACE_E("CheckShutdownParams(): AutoEndTasks is '" << num.Text_Get() << "' (" << value << ")");
            showWarning = TRUE;
        }
        Registry::Silent_Key_Close(key);
    }

    if (showWarning)
        SalMessageBox(NULL, LoadStr(IDS_CHANGEDSHUTDOWNPARS), SALAMANDER_TEXT_VERSION, MB_OK | MB_ICONWARNING);
}

BOOL MyRegRenameKey( HKEY key, const String_TChar_View& name, const String_TChar_View& name_new )
{
    BOOL ret = FALSE;
    // existuje i NtRenameKey, ale nejak jsem ji nerozchodil (vyzaduje UNICODE_STRING
    // a asi i klic otevreny pres NtOpenKey, kteremu se zadava klic pres OBJECT_ATTRIBUTES
    // inicializovany pres InitializeObjectAttributes), je to zbytecne komplikovane, nejde
    // o frekventovany kod, vyresim to pomalou, ale snadnou cestou... klic zkopiruju do
    // noveho, a pak original smazu
    HKEY key_new;
    if (!Registry::Silent_Key_Open( key, name_new, key_new)) // test neexistence ciloveho klice
    {
        if (Registry::Silent_Key_Create( key, name_new, key_new)) // vytvoreni ciloveho klice
        {
            // zkousel jsem i RegCopyTree (bez KEY_ALL_ACCESS neslapalo) a rychlost byla stejna jako SHCopyKey
            if (SHCopyKey(key, name.Text_Get(), key_new, 0) == ERROR_SUCCESS) // kopie do ciloveho klice
                ret = TRUE;
            Registry::Silent_Key_Close(key_new);
            if (ret)
                SHDeleteKey(key, name.Text_Get());
        }
        else
            TRACE_E("MyRegRenameKey(): unable to create target key: " << name_new.Text_Get() );
    }
    else
    {
        Registry::Silent_Key_Close( key_new );
        TRACE_E("MyRegRenameKey(): target key already exists: " << name_new.Text_Get());
    }
    return ret;
}

//****************************************************************************
//
// FindLatestConfiguration
//
// Pokusi se najit konfiguraci odpovidajici nasi verzi programu.
// Pokud se ji podari najit, bude nastavena promenna 'loadConfiguration' a funkce vrati
// TRUE. Pokud konfigurace jeste nebude existovat, funkce postupne prohleda stare
// konfigurace z pole 'SalamanderConfigurationRoots' (od nejmladsich k nejstarsim).
// Pokud nalezne nekterou z konfiguraci, zobrazi dialog a nabidne jeji konverzi do
// konfigurace soucasne a smazani z registry. Po zobrazeni posledniho dialogu vrati
// TRUE a nastavi promenne 'deleteConfigurations' a 'loadConfiguration' dle voleb
// uzivatele. Pokud uzivatel zvoli ukonceni aplikace, vrati funkce FALSE.
//

BOOL FindLatestConfiguration(BOOL* deleteConfigurations, const TCHAR*& loadConfiguration)
{
    HKEY hRootKey;
    loadConfiguration = NULL; // nechceme nahrat zadnou konfiguraci - pouziji se default hodnoty
    int rootIndex = 0;
    DWORD saveInProgress; // dummy
    HKEY hCfgKey;

    CImportConfigDialog dlg;
    ZeroMemory(dlg.ConfigurationExist, sizeof(dlg.ConfigurationExist)); // zadna z konfiguraci nenalezene
    dlg.DeleteConfigurations = deleteConfigurations;
    dlg.IndexOfConfigurationToLoad = -1;

    BOOL offerImportDlg = FALSE; // pokud existuje stara konfigurace nebo klice, nabidneme import

    LoadSaveToRegistryMutex.Enter();

    TCHAR   backup_text[200];
    _stprintf_s(backup_text, "%s.backup.63A7CD13", SalamanderConfigurationRoots[0]); // "63A7CD13" je prevence shody jmena klice s uzivatelskym
    auto    backup_view = String_TChar_View( backup_text );

    HKEY backupKey;
    BOOL backupFound = Registry::Silent_Key_Open( HKEY_CURRENT_USER, backup_view, backupKey);
    if (backupFound)
    {
        DWORD copyIsOK;
        if ( Registry::Silent_Value_Get( backupKey, String_TChar_View( SALAMANDER_COPY_IS_OK ), copyIsOK ))
            copyIsOK = 1; // backup je OK
        else
            copyIsOK = 0; // backup je vadny
        Registry::Silent_Key_Close( backupKey );
        if (!copyIsOK) // smazneme vadny backup, tvarime se, ze vubec neexistoval (asi se jen nestihl kompletne vytvorit)
        {
            TRACE_I("Configuration backup is incomplete, removing... " << backup_view.Text_Get());
            SHDeleteKey(HKEY_CURRENT_USER, backup_view.Text_Get());
            backupFound = FALSE;
        }
        else
            TRACE_I("Configuration backup is OK: " << backup_view.Text_Get());
    }

    do
    {
        const auto    root_view = String_TChar_View( SalamanderConfigurationRoots[rootIndex] );
        // zjistim, jestli klic existuje
        BOOL rootFound = Registry::Silent_Key_Open( HKEY_CURRENT_USER, root_view, hRootKey);
        if (
            rootFound
            &&
            (BOOL)Registry::Silent_Value_Get( hRootKey, String_TChar_View( SALAMANDER_SAVE_IN_PROGRESS ), saveInProgress )
        )
        { // jde o poskozenou konfiguraci
            TRACE_E("Configuration is corrupted!");
            rootFound = FALSE;
            Registry::Silent_Key_Close(hRootKey);
            if (rootIndex == 0 && backupFound) // pouzijeme backup, kdyz ho mame a nebudeme s tim prudit usera
            {
                TCHAR   corrupted_text[200];
                _stprintf_s(corrupted_text, "%s.corrupted.63A7CD13", root_view.Text_Get()); // "63A7CD13" je prevence shody jmena klice s uzivatelskym
                auto    corrupted_view = String_TChar_View( corrupted_text );


                SHDeleteKey(HKEY_CURRENT_USER, corrupted_view.Text_Get());           // pokud uz mame corrupted konfiguraci, odstranime ji, jedna staci
                if (
                    MyRegRenameKey(HKEY_CURRENT_USER, root_view, corrupted_view)
                    &&
                    MyRegRenameKey(HKEY_CURRENT_USER, backup_view, root_view)
                )
                {
                    backupFound = FALSE;
                    if ( Registry::Silent_Key_Create( HKEY_CURRENT_USER, root_view, hRootKey ) )
                    {
                        Registry::Silent_Value_Delete( hRootKey, String_TChar_View( SALAMANDER_COPY_IS_OK ));
                        Registry::Silent_Key_Close( hRootKey );
                    }
                    TRACE_I("Corrupted configuration was moved to: " << corrupted_view.Text_Get() );
                    TRACE_I("Using configuration backup instead ...");
                    continue; // v druhem kole nacteme konfiguraci ze zalohy vyrobene pri "critical shutdown"
                }
                else
                    TRACE_E("Unable to move corrupted configuration or configuration backup.");
            }

            if (rootIndex == 0) // u aktivni verze programu informujeme usera o poskozene konfiguraci a nechame ho klic zazalohovat, pak ho zkusime smazat (u starych verzi tuto poskozenou konfiguraci proste ignorujeme)
            {
                char buf[1500];
                _snprintf_s(buf, _TRUNCATE, LoadStr(IDS_CORRUPTEDCONFIGFOUND), root_view);
                LoadSaveToRegistryMutex.Leave();

                MSGBOXEX_PARAMS params;
                memset(&params, 0, sizeof(params));
                params.HParent = NULL;
                params.Flags = MB_OKCANCEL | MB_ICONERROR | MB_DEFBUTTON2;
                params.Caption = SALAMANDER_TEXT_VERSION;
                params.Text = buf;
                char aliasBtnNames[200];
                /* slouzi pro skript export_mnu.py, ktery generuje salmenu.mnu pro Translator
   nechame pro tlacitka msgboxu resit kolize hotkeys tim, ze simulujeme, ze jde o menu
MENU_TEMPLATE_ITEM MsgBoxButtons[] = 
{
  {MNTT_PB, 0
  {MNTT_IT, IDS_CORRUPTEDCONFIGREMOVEBTN
  {MNTT_IT, IDS_SELLANGEXITBUTTON
  {MNTT_PE, 0
};
*/
                sprintf(aliasBtnNames, "%d\t%s\t%d\t%s", DIALOG_OK, LoadStr(IDS_CORRUPTEDCONFIGREMOVEBTN), DIALOG_CANCEL, LoadStr(IDS_SELLANGEXITBUTTON));
                params.AliasBtnNames = aliasBtnNames;
                if (SalMessageBoxEx(&params) == IDCANCEL)
                {
                    CheckShutdownParams(); // pripadne jeste zobrazime tenhle warning (pokud si prejmenuji klic v registry, nemuseli by tu hlasku vubec potkat)
                    return FALSE;          // Exit
                }

                CheckShutdownParams();
                LoadSaveToRegistryMutex.Enter();
                if (HANDLES_Q(RegOpenKeyEx(HKEY_CURRENT_USER, root_view.Text_Get(), 0, KEY_READ | KEY_WRITE, &hRootKey)) == ERROR_SUCCESS)
                { // smazeme poskozenou konfiguraci (pokud tam jeste je - aneb user ji neprejmenoval kvuli zalohovani)
                    TRACE_I("Deleting corrupted configuration on user demand: " << root_view.Text_Get());
                    Registry::Silent_Key_Delete_Branch( hRootKey );
                    Registry::Silent_Key_Close(hRootKey);
                    Registry::Silent_Key_Delete(HKEY_CURRENT_USER, root_view);
                }
            }
        }
        BOOL cfgFound = rootFound && (BOOL)Registry::Silent_Key_Open( hRootKey, String_TChar_View( SALAMANDER_CONFIG_REG ), hCfgKey);
        if (rootFound)
            Registry::Silent_Key_Close(hRootKey);

        if (rootIndex == 0 && backupFound) // backup nepotrebujeme, smazneme ho
        {
            TRACE_I("Removing unnecessary configuration backup: " << backup_view.Text_Get());
            SHDeleteKey(HKEY_CURRENT_USER, backup_view.Text_Get());
            backupFound = FALSE;
        }

        if (cfgFound) // klice s konfiguraci rozpoznavame na zaklade existence podklice "Configuration" (pouha existence klice nestaci, protoze pod nim muze byt jen "AutoImportConfig")
        {
            Registry::Silent_Key_Close(hCfgKey);
            if (rootIndex == 0)
            {
                // jedna se o klic k aktivni verzi programu => potvrdime nacteni klice a vypadnem
                loadConfiguration = root_view.Text_Get();       //We can set this as root_view is defined from a static string.
                LoadSaveToRegistryMutex.Leave();
                return TRUE;
            }
            // jde o jeden ze starsich klicu

            // konfiguraci budeme nabizet pro import a smazani
            dlg.ConfigurationExist[rootIndex] = TRUE;
            offerImportDlg = TRUE;
        }
        rootIndex++;
    } while (rootIndex < SALCFG_ROOTS_COUNT);

    LoadSaveToRegistryMutex.Leave();

    if (offerImportDlg)
    {
        HWND hSplash = GetSplashScreenHandle(); // pokud existuje splash, docasne ho zhasneme
        if (hSplash != NULL)
            ShowWindow(hSplash, SW_HIDE);

        int dlgRet = (int)dlg.Execute();

        if (hSplash != NULL)
        {
            ShowWindow(hSplash, SW_SHOW);
            UpdateWindow(hSplash);
        }

        if (dlgRet == IDCANCEL)
        {
            return FALSE; // user chce zdrhnout ze Salama
        }
        if (dlg.IndexOfConfigurationToLoad != -1)
            loadConfiguration = SalamanderConfigurationRoots[dlg.IndexOfConfigurationToLoad];
    }
    return TRUE;
}

// maze klice dle pole vraceneho funkci FindLatestConfiguration

void CMainWindow::DeleteOldConfigurations(BOOL* deleteConfigurations, BOOL autoImportConfig, const TCHAR* autoImportConfigFromKey, BOOL doNotDeleteImportedCfg)
{
    // je co mazat?
    BOOL dirty = FALSE;
    if (autoImportConfig)
        dirty = TRUE;
    else
    {
        int rootIndex;
        for (rootIndex = 0; rootIndex < SALCFG_ROOTS_COUNT; rootIndex++)
        {
            if (deleteConfigurations[rootIndex])
            {
                dirty = TRUE;
                break;
            }
        }
    }
    if (dirty)
    {
        // podrizneme stare konfigurace
        HCURSOR hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        CWaitWindow analysing(HWindow, IDS_DELETINGCONFIGURATION, FALSE, ooStatic);
        analysing.Create();
        EnableWindow(HWindow, FALSE);
        LoadSaveToRegistryMutex.Enter();
        int rootIndex;
        for (rootIndex = 0; rootIndex < SALCFG_ROOTS_COUNT; rootIndex++)
        {
            if (deleteConfigurations[rootIndex])
            {
                HKEY hKey;
                const auto key_view = String_TChar_View( SalamanderConfigurationRoots[rootIndex] );
                if (Registry::Silent_Key_Create(HKEY_CURRENT_USER, key_view, hKey))
                {
                    Registry::Silent_Key_Delete_Branch( hKey );
                    Registry::Silent_Key_Close(hKey);
                    Registry::Silent_Key_Delete(HKEY_CURRENT_USER, key_view );
                }
            }
        }
        if (autoImportConfig) // vycistime starou konfiguraci (uz je ulozena do noveho klice) + v novem klici zrusime hodnotu "AutoImportConfig"
        {
            BOOL ok = FALSE;
            HKEY cfgKey;
            if (HANDLES_Q(RegOpenKeyEx(HKEY_CURRENT_USER, SalamanderConfigurationRoots[0], 0,
                                       KEY_READ | KEY_WRITE, &cfgKey)) == ERROR_SUCCESS)
            { // v novem klici zrusime hodnotu "AutoImportConfig"
                if (RegDeleteValue(cfgKey, SALAMANDER_AUTO_IMPORT_CONFIG) == ERROR_SUCCESS)
                    ok = TRUE;
                HANDLES(RegCloseKey(cfgKey));
            }
            if (!ok) // pokud k tomu dojde, zrejme neni problem, protoze jsme urcite taky nezapsali konfiguraci Salama (jde do stejneho klice) a cely UPGRADE bude potreba provest znovu
            {
                TRACE_E("CMainWindow::DeleteOldConfigurations(): unable to delete " << SALAMANDER_AUTO_IMPORT_CONFIG << " value from HKCU\\" << SalamanderConfigurationRoots[0]);
            }
            else // vycistime starou konfiguraci (uz je ulozena do noveho klice)
            {
                if (!doNotDeleteImportedCfg)
                {
                    if (HANDLES_Q(RegOpenKeyEx(HKEY_CURRENT_USER, autoImportConfigFromKey, 0, KEY_READ | KEY_WRITE, &cfgKey)) == ERROR_SUCCESS)
                    {
                        Registry::Silent_Key_Delete_Branch( cfgKey );
                        Registry::Silent_Key_Close( cfgKey );
                        Registry::Silent_Key_Delete( HKEY_CURRENT_USER, String_TChar_View( autoImportConfigFromKey ) );
                    }
                }
            }
        }
        LoadSaveToRegistryMutex.Leave();
        EnableWindow(HWindow, TRUE);
        DestroyWindow(analysing.HWindow);
        SetCursor(hOldCursor);
    }
}

//
// ****************************************************************************
// CMainWindow
//

void CMainWindow::SavePanelConfig(CPanelWindow* panel, HKEY hSalamander, const TCHAR* reg)
{
    HKEY actKey;
    if (CreateKey(hSalamander, reg, actKey))
    {
        DWORD value;
        value = panel->HeaderLineVisible;
        SetValue(actKey, PANEL_HEADER_REG, REG_DWORD, &value, sizeof(DWORD));
        SetValue(actKey, PANEL_PATH_REG, REG_SZ, panel->GetPath(), -1);
        value = panel->GetViewTemplateIndex();
        SetValue(actKey, PANEL_VIEW_REG, REG_DWORD, &value, sizeof(DWORD));
        value = panel->SortType;
        SetValue(actKey, PANEL_SORT_REG, REG_DWORD, &value, sizeof(DWORD));
        value = panel->ReverseSort;
        SetValue(actKey, PANEL_REVERSE_REG, REG_DWORD, &value, sizeof(DWORD));
        value = (panel->DirectoryLine->HWindow != NULL);
        SetValue(actKey, PANEL_DIRLINE_REG, REG_DWORD, &value, sizeof(DWORD));
        value = (panel->StatusLine->HWindow != NULL);
        SetValue(actKey, PANEL_STATUS_REG, REG_DWORD, &value, sizeof(DWORD));
        SetValue(actKey, PANEL_FILTER_ENABLE, REG_DWORD, &panel->FilterEnabled,
                 sizeof(DWORD));
        SetValue(actKey, PANEL_FILTER, REG_SZ, panel->Filter.GetMasksString(), -1);

        CloseKey(actKey);
    }
}

void CMainWindow::SaveConfig(HWND parent)
{
    CALL_STACK_MESSAGE1("CMainWindow::SaveConfig()");

    if (parent == NULL)
        parent = HWindow;

    if (SALAMANDER_ROOT_REG == NULL)
    {
        TRACE_E("SALAMANDER_ROOT_REG == NULL"); // nemusi byt chyba: pri UPGRADE tak resime ukonceni Salama bez ulozeni konfigurace (pokud user nema nainstalovane vsechny pluginy a zvoli Exit)
        return;
    }

    HCURSOR hOldCursor = NULL;
    if (GlobalSaveWaitWindow == NULL)
        hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    CWaitWindow analysing(parent, IDS_SAVINGCONFIGURATION, FALSE, ooStatic, TRUE);
    int savingProgress = 0;
    HWND oldPluginMsgBoxParent = PluginMsgBoxParent;
    if (GlobalSaveWaitWindow == NULL)
    {
        //TRACE_I("analysing.SetProgressPos() savingProgress="<<savingProgress);
        analysing.SetProgressMax(7 /* NUTNE SYNCHRONIZOVAT s CMainWindow::WindowProc::WM_USER_CLOSE_MAINWND !!! */); // o jednu min, at si uzijou na pohled na 100%
        analysing.Create();
        EnableWindow(parent, FALSE);

        // bude se volat i SaveConfiguration plug-inu -> nutne nastaveni parenta pro jejich messageboxy
        PluginMsgBoxParent = analysing.HWindow;
    }

    LoadSaveToRegistryMutex.Enter();

    HKEY salamander;
    if (CreateKey(HKEY_CURRENT_USER, SALAMANDER_ROOT_REG, salamander))
    {
        HKEY actKey;

        BOOL cfgIsOK = TRUE;
        BOOL deleteSALAMANDER_SAVE_IN_PROGRESS = !IsSetSALAMANDER_SAVE_IN_PROGRESS;
        if (deleteSALAMANDER_SAVE_IN_PROGRESS)
        {
            DWORD saveInProgress = 1;

            if (Registry::Silent_Value_Get( salamander, String_TChar_View( SALAMANDER_SAVE_IN_PROGRESS ), saveInProgress ))
            {                    // GetValueAux, protoze nechci hlasku o Load Configuration
                cfgIsOK = FALSE; // jde o poskozenou konfiguraci, ulozenim se neopravi (neuklada se komplet)
                TRACE_E("CMainWindow::SaveConfig(): unable to save configuration, configuration key in registry is corrupted");
            }
            else
            {
                saveInProgress = 1;
                SetValue(salamander, SALAMANDER_SAVE_IN_PROGRESS, REG_DWORD, &saveInProgress, sizeof(DWORD));
                IsSetSALAMANDER_SAVE_IN_PROGRESS = TRUE;
            }
        }

        if (cfgIsOK)
        {
            //--- version
            if (CreateKey(salamander, SALAMANDER_VERSION_REG, actKey))
            {
                DWORD newConfigVersion = THIS_CONFIG_VERSION;
                SetValue(actKey, SALAMANDER_VERSIONREG_REG, REG_DWORD, &newConfigVersion, sizeof(DWORD));
                CloseKey(actKey);
            }

            //---  window

            if (CreateKey(salamander, SALAMANDER_WINDOW_REG, actKey))
            {
                WINDOWPLACEMENT place;
                place.length = sizeof(WINDOWPLACEMENT);
                GetWindowPlacement(HWindow, &place);
                SetValue(actKey, WINDOW_LEFT_REG, REG_DWORD, &(place.rcNormalPosition.left), sizeof(DWORD));
                SetValue(actKey, WINDOW_RIGHT_REG, REG_DWORD, &(place.rcNormalPosition.right), sizeof(DWORD));
                SetValue(actKey, WINDOW_TOP_REG, REG_DWORD, &(place.rcNormalPosition.top), sizeof(DWORD));
                SetValue(actKey, WINDOW_BOTTOM_REG, REG_DWORD, &(place.rcNormalPosition.bottom), sizeof(DWORD));
                SetValue(actKey, WINDOW_SHOW_REG, REG_DWORD, &(place.showCmd), sizeof(DWORD));

                char buf[20];
                sprintf(buf, "%.1lf", SplitPosition * 100);
                SetValue(actKey, WINDOW_SPLIT_REG, REG_SZ, buf, -1);
                sprintf(buf, "%.1lf", BeforeZoomSplitPosition * 100);
                SetValue(actKey, WINDOW_BEFOREZOOMSPLIT_REG, REG_SZ, buf, -1);

                CloseKey(actKey);
            }

            if (Configuration.FindDialogWindowPlacement.length != 0)
            {
                if (CreateKey(salamander, FINDDIALOG_WINDOW_REG, actKey))
                {
                    SetValue(actKey, WINDOW_LEFT_REG, REG_DWORD, &(Configuration.FindDialogWindowPlacement.rcNormalPosition.left), sizeof(DWORD));
                    SetValue(actKey, WINDOW_RIGHT_REG, REG_DWORD, &(Configuration.FindDialogWindowPlacement.rcNormalPosition.right), sizeof(DWORD));
                    SetValue(actKey, WINDOW_TOP_REG, REG_DWORD, &(Configuration.FindDialogWindowPlacement.rcNormalPosition.top), sizeof(DWORD));
                    SetValue(actKey, WINDOW_BOTTOM_REG, REG_DWORD, &(Configuration.FindDialogWindowPlacement.rcNormalPosition.bottom), sizeof(DWORD));
                    SetValue(actKey, WINDOW_SHOW_REG, REG_DWORD, &(Configuration.FindDialogWindowPlacement.showCmd), sizeof(DWORD));

                    SetValue(actKey, FINDDIALOG_NAMEWIDTH_REG, REG_DWORD, &(Configuration.FindColNameWidth), sizeof(DWORD));
                    CloseKey(actKey);
                }
            }

            //---  left and right panel

            SavePanelConfig(LeftPanel, salamander, SALAMANDER_LEFTP_REG);
            SavePanelConfig(RightPanel, salamander, SALAMANDER_RIGHTP_REG);

            //---  default directories

            if (CreateKey(salamander, SALAMANDER_DEFDIRS_REG, actKey))
            {
                char name[2];
                name[1] = 0;
                char d;
                for (d = 'A'; d <= 'Z'; d++)
                {
                    name[0] = d;
                    char* path = DefaultDir[d - 'A'];
                    if (path[1] == ':' && path[2] == '\\' && path[3] != 0) // neni to "C:\"
                        SetValue(actKey, name, REG_SZ, path, -1);
                    else
                        DeleteValue(actKey, name);
                }
                CloseKey(actKey);
            }

            //---  password manager

            if (CreateKey(salamander, SALAMANDER_PWDMNGR_REG, actKey))
            {
                PasswordManager.Save(actKey);
                CloseKey(actKey);
            }

            //---  hot paths

            if (CreateKey(salamander, SALAMANDER_HOTPATHS_REG, actKey))
            {
                HotPaths.Save(actKey);
                CloseKey(actKey);
            }

            //--- view templates

            if (CreateKey(salamander, SALAMANDER_VIEWTEMPLATES_REG, actKey))
            {
                ViewTemplates.Save(actKey);
                CloseKey(actKey);
            }

            //---  Plugins
            HKEY configKey;
            HKEY orderKey;
            if (
                CreateKey(salamander, SALAMANDER_PLUGINS, actKey)
                &&
                CreateKey(salamander, SALAMANDER_PLUGINSCONFIG, configKey)
                &&
                CreateKey(salamander, SALAMANDER_PLUGINSORDER, orderKey)
            )
            {
                Plugins.Save(parent, actKey, configKey, orderKey);
                CloseKey(orderKey);
                CloseKey(actKey);
                CloseKey(configKey);
            }

            if (GlobalSaveWaitWindow == NULL)
                analysing.SetProgressPos(++savingProgress); // 1
            else
                GlobalSaveWaitWindow->SetProgressPos(++GlobalSaveWaitWindowProgress); // 1
            //TRACE_I("analysing.SetProgressPos() savingProgress="<<savingProgress);

            //---  Packers & Unpackers
            if (CreateKey(salamander, SALAMANDER_PACKANDUNPACK, actKey))
            {
                SetValue(actKey, SALAMANDER_SIMPLEICONSINARCHIVES, REG_DWORD, &(Configuration.UseSimpleIconsInArchives), sizeof(DWORD));

                //---  Custom Packers
                HKEY actSubKey;
                if (CreateKey(actKey, SALAMANDER_CUSTOMPACKERS, actSubKey))
                {
                    ClearKey(actSubKey);
                    HKEY itemKey;
                    char buf[30];
                    int i;
                    for (i = 0; i < PackerConfig.GetPackersCount(); i++)
                    {
                        itoa(i + 1, buf, 10);
                        if (CreateKey(actSubKey, buf, itemKey))
                        {
                            PackerConfig.Save(i, itemKey);
                            CloseKey(itemKey);
                        }
                        else
                            break;
                    }
                    SetValue(actSubKey, SALAMANDER_ANOTHERPANEL, REG_DWORD, &(Configuration.UseAnotherPanelForPack), sizeof(DWORD));
                    int pp = PackerConfig.GetPreferedPacker();
                    SetValue(actSubKey, SALAMANDER_PREFFERED, REG_DWORD, &pp, sizeof(DWORD));
                    CloseKey(actSubKey);
                }

                if (GlobalSaveWaitWindow == NULL)
                    analysing.SetProgressPos(++savingProgress); // 2
                else
                    GlobalSaveWaitWindow->SetProgressPos(++GlobalSaveWaitWindowProgress); // 2
                //TRACE_I("analysing.SetProgressPos() savingProgress="<<savingProgress);

                //---  Custom Unpackers
                if (CreateKey(actKey, SALAMANDER_CUSTOMUNPACKERS, actSubKey))
                {
                    ClearKey(actSubKey);
                    HKEY itemKey;
                    char buf[30];
                    int i;
                    for (i = 0; i < UnpackerConfig.GetUnpackersCount(); i++)
                    {
                        itoa(i + 1, buf, 10);
                        if (CreateKey(actSubKey, buf, itemKey))
                        {
                            UnpackerConfig.Save(i, itemKey);
                            CloseKey(itemKey);
                        }
                        else
                            break;
                    }
                    SetValue(actSubKey, SALAMANDER_ANOTHERPANEL, REG_DWORD, &(Configuration.UseAnotherPanelForUnpack), sizeof(DWORD));
                    SetValue(actSubKey, SALAMANDER_NAMEBYARCHIVE, REG_DWORD, &(Configuration.UseSubdirNameByArchiveForUnpack), sizeof(DWORD));
                    int pp = UnpackerConfig.GetPreferedUnpacker(); SetValue(actSubKey, SALAMANDER_PREFFERED, REG_DWORD, &pp, sizeof(DWORD));
                    CloseKey(actSubKey);
                }

                if (GlobalSaveWaitWindow == NULL)
                    analysing.SetProgressPos(++savingProgress); // 3
                else
                    GlobalSaveWaitWindow->SetProgressPos(++GlobalSaveWaitWindowProgress); // 3
                //TRACE_I("analysing.SetProgressPos() savingProgress="<<savingProgress);

                //---  Predefined Packers
                if (CreateKey(actKey, SALAMANDER_PREDPACKERS, actSubKey))
                {
                    ClearKey(actSubKey);
                    HKEY itemKey;
                    char buf[30];
                    int i;
                    for (i = 0; i < ArchiverConfig.GetArchiversCount(); i++)
                    {
                        itoa(i + 1, buf, 10);
                        if (CreateKey(actSubKey, buf, itemKey))
                        {
                            ArchiverConfig.Save(i, itemKey);
                            CloseKey(itemKey);
                        }
                        else
                            break;
                    }
                    CloseKey(actSubKey);
                }

                //---  Archive Association
                if (CreateKey(actKey, SALAMANDER_ARCHIVEASSOC, actSubKey))
                {
                    ClearKey(actSubKey);
                    HKEY itemKey;
                    char buf[30];
                    int i;
                    for (i = 0; i < PackerFormatConfig.GetFormatsCount(); i++)
                    {
                        itoa(i + 1, buf, 10);
                        if (CreateKey(actSubKey, buf, itemKey))
                        {
                            PackerFormatConfig.Save(i, itemKey);
                            CloseKey(itemKey);
                        }
                        else
                            break;
                    }
                    CloseKey(actSubKey);
                }

                CloseKey(actKey);
            }

            //---  configuration

            if (CreateKey(salamander, SALAMANDER_CONFIG_REG, actKey))
            {
                //---  top rebar begin
                SetValue(actKey, CONFIG_MENUINDEX_REG, REG_DWORD, &Configuration.MenuIndex, sizeof(DWORD));
                SetValue(actKey, CONFIG_MENUBREAK_REG, REG_DWORD, &Configuration.MenuBreak, sizeof(DWORD));
                SetValue(actKey, CONFIG_MENUWIDTH_REG, REG_DWORD, &Configuration.MenuWidth, sizeof(DWORD));
                SetValue(actKey, CONFIG_TOOLBARINDEX_REG, REG_DWORD, &Configuration.TopToolbarIndex, sizeof(DWORD));
                SetValue(actKey, CONFIG_TOOLBARBREAK_REG, REG_DWORD, &Configuration.TopToolbarBreak, sizeof(DWORD));
                SetValue(actKey, CONFIG_TOOLBARWIDTH_REG, REG_DWORD, &Configuration.TopToolbarWidth, sizeof(DWORD));
                SetValue(actKey, CONFIG_PLUGINSBARINDEX_REG, REG_DWORD, &Configuration.PluginsBarIndex, sizeof(DWORD));
                SetValue(actKey, CONFIG_PLUGINSBARBREAK_REG, REG_DWORD, &Configuration.PluginsBarBreak, sizeof(DWORD));
                SetValue(actKey, CONFIG_PLUGINSBARWIDTH_REG, REG_DWORD, &Configuration.PluginsBarWidth, sizeof(DWORD));
                SetValue(actKey, CONFIG_USERMENUINDEX_REG, REG_DWORD, &Configuration.UserMenuToolbarIndex, sizeof(DWORD));
                SetValue(actKey, CONFIG_USERMENUBREAK_REG, REG_DWORD, &Configuration.UserMenuToolbarBreak, sizeof(DWORD));
                SetValue(actKey, CONFIG_USERMENUWIDTH_REG, REG_DWORD, &Configuration.UserMenuToolbarWidth, sizeof(DWORD));
                SetValue(actKey, CONFIG_USERMENULABELS_REG, REG_DWORD, &Configuration.UserMenuToolbarLabels, sizeof(DWORD));
                SetValue(actKey, CONFIG_HOTPATHSINDEX_REG, REG_DWORD, &Configuration.HotPathsBarIndex, sizeof(DWORD));
                SetValue(actKey, CONFIG_HOTPATHSBREAK_REG, REG_DWORD, &Configuration.HotPathsBarBreak, sizeof(DWORD));
                SetValue(actKey, CONFIG_HOTPATHSWIDTH_REG, REG_DWORD, &Configuration.HotPathsBarWidth, sizeof(DWORD));
                SetValue(actKey, CONFIG_DRIVEBARINDEX_REG, REG_DWORD, &Configuration.DriveBarIndex, sizeof(DWORD));
                SetValue(actKey, CONFIG_DRIVEBARBREAK_REG, REG_DWORD, &Configuration.DriveBarBreak, sizeof(DWORD));
                SetValue(actKey, CONFIG_DRIVEBARWIDTH_REG, REG_DWORD, &Configuration.DriveBarWidth, sizeof(DWORD));
                SetValue(actKey, CONFIG_GRIPSVISIBLE_REG, REG_DWORD, &Configuration.GripsVisible, sizeof(DWORD));

                //---  top rebar end
                SetValue(actKey, CONFIG_FILENAMEFORMAT_REG, REG_DWORD, &Configuration.FileNameFormat, sizeof(DWORD));
                SetValue(actKey, CONFIG_SIZEFORMAT_REG, REG_DWORD, &Configuration.SizeFormat, sizeof(DWORD));
                SetValue(actKey, CONFIG_SELECTION_REG, REG_DWORD, &Configuration.IncludeDirs, sizeof(DWORD));
                SetValue(actKey, CONFIG_COPYFINDTEXT_REG, REG_DWORD, &Configuration.CopyFindText, sizeof(DWORD));
                SetValue(actKey, CONFIG_CLEARREADONLY_REG, REG_DWORD, &Configuration.ClearReadOnly, sizeof(DWORD));
                SetValue(actKey, CONFIG_PRIMARYCONTEXTMENU_REG, REG_DWORD, &Configuration.PrimaryContextMenu, sizeof(DWORD));
                SetValue(actKey, CONFIG_NOTHIDDENSYSTEM_REG, REG_DWORD, &Configuration.NotHiddenSystemFiles, sizeof(DWORD));
                SetValue(actKey, CONFIG_RECYCLEBIN_REG, REG_DWORD, &Configuration.UseRecycleBin, sizeof(DWORD));
                SetValue(actKey, CONFIG_RECYCLEMASKS_REG, REG_SZ, Configuration.RecycleMasks.GetMasksString(), -1);
                SetValue(actKey, CONFIG_SAVEONEXIT_REG, REG_DWORD, &Configuration.AutoSave, sizeof(DWORD));
                SetValue(actKey, CONFIG_SHOWGREPERRORS_REG, REG_DWORD, &Configuration.ShowGrepErrors, sizeof(DWORD));
                SetValue(actKey, CONFIG_FINDFULLROW_REG, REG_DWORD, &Configuration.FindFullRowSelect, sizeof(DWORD));
                SetValue(actKey, CONFIG_MINBEEPWHENDONE_REG, REG_DWORD, &Configuration.MinBeepWhenDone, sizeof(DWORD));
                SetValue(actKey, CONFIG_CLOSESHELL_REG, REG_DWORD, &Configuration.CloseShell, sizeof(DWORD));
                DWORD rightPanelFocused = (GetActivePanel() == RightPanel);
                SetValue(actKey, CONFIG_RIGHT_FOCUS_REG, REG_DWORD, &rightPanelFocused, sizeof(DWORD));
                SetValue(actKey, CONFIG_ALWAYSONTOP_REG, REG_DWORD, &Configuration.AlwaysOnTop, sizeof(DWORD));
                //      SetValue(actKey, CONFIG_FASTDIRMOVE_REG, REG_DWORD,
                //               &Configuration.FastDirectoryMove, sizeof(DWORD));
                SetValue(actKey, CONFIG_SORTUSESLOCALE_REG, REG_DWORD, &Configuration.SortUsesLocale, sizeof(DWORD));
                SetValue(actKey, CONFIG_SORTDETECTNUMBERS_REG, REG_DWORD, &Configuration.SortDetectNumbers, sizeof(DWORD));
                SetValue(actKey, CONFIG_SORTNEWERONTOP_REG, REG_DWORD, &Configuration.SortNewerOnTop, sizeof(DWORD));
                SetValue(actKey, CONFIG_SORTDIRSBYNAME_REG, REG_DWORD, &Configuration.SortDirsByName, sizeof(DWORD));
                SetValue(actKey, CONFIG_SORTDIRSBYEXT_REG, REG_DWORD, &Configuration.SortDirsByExt, sizeof(DWORD));
                SetValue(actKey, CONFIG_SAVEHISTORY_REG, REG_DWORD, &Configuration.SaveHistory, sizeof(DWORD));
                SetValue(actKey, CONFIG_SAVEWORKDIRS_REG, REG_DWORD, &Configuration.SaveWorkDirs, sizeof(DWORD));
                SetValue(actKey, CONFIG_ENABLECMDLINEHISTORY_REG, REG_DWORD, &Configuration.EnableCmdLineHistory, sizeof(DWORD));
                SetValue(actKey, CONFIG_SAVECMDLINEHISTORY_REG, REG_DWORD, &Configuration.SaveCmdLineHistory, sizeof(DWORD));
                //      SetValue(actKey, CONFIG_LANTASTICCHECK_REG, REG_DWORD, &Configuration.LantasticCheck, sizeof(DWORD));
                SetValue(actKey, CONFIG_ONLYONEINSTANCE_REG, REG_DWORD, &Configuration.OnlyOneInstance, sizeof(DWORD));
                SetValue(actKey, CONFIG_STATUSAREA_REG, REG_DWORD, &Configuration.StatusArea, sizeof(DWORD));
                SetValue(actKey, CONFIG_FULLROWSELECT_REG, REG_DWORD, &Configuration.FullRowSelect, sizeof(DWORD));
                SetValue(actKey, CONFIG_FULLROWHIGHLIGHT_REG, REG_DWORD, &Configuration.FullRowHighlight, sizeof(DWORD));
                SetValue(actKey, CONFIG_USEICONTINCTURE_REG, REG_DWORD, &Configuration.UseIconTincture, sizeof(DWORD));
                SetValue(actKey, CONFIG_SHOWPANELCAPTION_REG, REG_DWORD, &Configuration.ShowPanelCaption, sizeof(DWORD));
                SetValue(actKey, CONFIG_SHOWPANELZOOM_REG, REG_DWORD, &Configuration.ShowPanelZoom, sizeof(DWORD));
                SetValue(actKey, CONFIG_SINGLECLICK_REG, REG_DWORD, &Configuration.SingleClick, sizeof(DWORD));
                //      SetValue(actKey, CONFIG_SHOWTIPOFTHEDAY_REG, REG_DWORD, &Configuration.ShowTipOfTheDay, sizeof(DWORD));
                //      SetValue(actKey, CONFIG_LASTTIPOFTHEDAY_REG, REG_DWORD, &Configuration.LastTipOfTheDay, sizeof(DWORD));
                SetValue(actKey, CONFIG_INFOLINECONTENT_REG, REG_SZ, Configuration.InfoLineContent, -1);
                SetValue(actKey, CONFIG_IFPATHISINACCESSIBLEGOTOISMYDOCS_REG, REG_DWORD, &Configuration.IfPathIsInaccessibleGoToIsMyDocs, sizeof(DWORD));
                SetValue(actKey, CONFIG_IFPATHISINACCESSIBLEGOTO_REG, REG_SZ, Configuration.IfPathIsInaccessibleGoTo, -1);
                SetValue(actKey, CONFIG_HOTPATH_AUTOCONFIG, REG_DWORD, &Configuration.HotPathAutoConfig, sizeof(DWORD));
                SetValue(actKey, CONFIG_LASTUSEDSPEEDLIM_REG, REG_DWORD, &Configuration.LastUsedSpeedLimit, sizeof(DWORD));
                SetValue(actKey, CONFIG_QUICKSEARCHENTER_REG, REG_DWORD, &Configuration.QuickSearchEnterAlt, sizeof(DWORD));
                SetValue(actKey, CONFIG_CHD_SHOWMYDOC, REG_DWORD, &Configuration.ChangeDriveShowMyDoc, sizeof(DWORD));
                SetValue(actKey, CONFIG_CHD_SHOWCLOUDSTOR, REG_DWORD, &Configuration.ChangeDriveCloudStorage, sizeof(DWORD));
                SetValue(actKey, CONFIG_CHD_SHOWANOTHER, REG_DWORD, &Configuration.ChangeDriveShowAnother, sizeof(DWORD));
                SetValue(actKey, CONFIG_CHD_SHOWNET, REG_DWORD, &Configuration.ChangeDriveShowNet, sizeof(DWORD));
                SetValue(actKey, CONFIG_SEARCHFILECONTENT, REG_DWORD, &Configuration.SearchFileContent, sizeof(DWORD));
                SetValue(actKey, CONFIG_LASTPLUGINVER, REG_DWORD, &Configuration.LastPluginVer, sizeof(DWORD));
                SetValue(actKey, CONFIG_LASTPLUGINVER_OP, REG_DWORD, &Configuration.LastPluginVerOP, sizeof(DWORD));
                SetValue(actKey, CONFIG_USESALOPEN_REG, REG_DWORD, &Configuration.UseSalOpen, sizeof(DWORD));
                SetValue(actKey, CONFIG_NETWAREFASTDIRMOVE_REG, REG_DWORD, &Configuration.NetwareFastDirMove, sizeof(DWORD));
                if (Windows7AndLater) 
                    SetValue(actKey, CONFIG_ASYNCCOPYALG_REG, REG_DWORD, &Configuration.UseAsyncCopyAlg, sizeof(DWORD));
                SetValue(actKey, CONFIG_RELOAD_ENV_VARS_REG, REG_DWORD, &Configuration.ReloadEnvVariables, sizeof(DWORD));
                SetValue(actKey, CONFIG_QUICKRENAME_SELALL_REG, REG_DWORD, &Configuration.QuickRenameSelectAll, sizeof(DWORD));
                SetValue(actKey, CONFIG_EDITNEW_SELALL_REG, REG_DWORD, &Configuration.EditNewSelectAll, sizeof(DWORD));
                SetValue(actKey, CONFIG_SHIFTFORHOTPATHS_REG, REG_DWORD, &Configuration.ShiftForHotPaths, sizeof(DWORD));
                SetValue(actKey, CONFIG_LANGUAGE_REG, REG_SZ, Configuration.SLGName, -1);
                SetValue(actKey, CONFIG_USEALTLANGFORPLUGINS_REG, REG_DWORD, &Configuration.UseAsAltSLGInOtherPlugins, sizeof(DWORD));
                SetValue(actKey, CONFIG_ALTLANGFORPLUGINS_REG, REG_SZ, Configuration.AltPluginSLGName, -1);
                DWORD langChanged = (StrICmp(Configuration.SLGName, Configuration.LoadedSLGName) != 0); // TRUE pokud user zmenil jazyk Salama
                SetValue(actKey, CONFIG_LANGUAGECHANGED_REG, REG_DWORD, &langChanged, sizeof(DWORD));
                SetValue(actKey, CONFIG_SHOWSPLASHSCREEN_REG, REG_DWORD, &Configuration.ShowSplashScreen, sizeof(DWORD));
                SetValue(actKey, CONFIG_CONVERSIONTABLE_REG, REG_SZ, &Configuration.ConversionTable, -1);
                SetValue(actKey, CONFIG_SKILLLEVEL_REG, REG_DWORD, &Configuration.SkillLevel, sizeof(DWORD));
                SetValue(actKey, CONFIG_TITLEBARSHOWPATH_REG, REG_DWORD, &Configuration.TitleBarShowPath, sizeof(DWORD));
                SetValue(actKey, CONFIG_TITLEBARMODE_REG, REG_DWORD, &Configuration.TitleBarMode, sizeof(DWORD));
                SetValue(actKey, CONFIG_TITLEBARPREFIX_REG, REG_DWORD, &Configuration.UseTitleBarPrefix, sizeof(DWORD));
                SetValue(actKey, CONFIG_TITLEBARPREFIXTEXT_REG, REG_SZ, &Configuration.TitleBarPrefix, -1);
                SetValue(actKey, CONFIG_MAINWINDOWICONINDEX_REG, REG_DWORD, &Configuration.MainWindowIconIndex, sizeof(DWORD));
                SetValue(actKey, CONFIG_CLICKQUICKRENAME_REG, REG_DWORD, &Configuration.ClickQuickRename, sizeof(DWORD));
                SetValue(actKey, CONFIG_VISIBLEDRIVES_REG, REG_DWORD, &Configuration.VisibleDrives, sizeof(DWORD));
                SetValue(actKey, CONFIG_SEPARATEDDRIVES_REG, REG_DWORD, &Configuration.SeparatedDrives, sizeof(DWORD));
                SetValue(actKey, CONFIG_COMPAREBYTIME_REG, REG_DWORD, &Configuration.CompareByTime, sizeof(DWORD));
                SetValue(actKey, CONFIG_COMPAREBYSIZE_REG, REG_DWORD, &Configuration.CompareBySize, sizeof(DWORD));
                SetValue(actKey, CONFIG_COMPAREBYCONTENT_REG, REG_DWORD, &Configuration.CompareByContent, sizeof(DWORD));
                SetValue(actKey, CONFIG_COMPAREBYATTR_REG, REG_DWORD, &Configuration.CompareByAttr, sizeof(DWORD));
                SetValue(actKey, CONFIG_COMPAREBYSUBDIRS_REG, REG_DWORD, &Configuration.CompareSubdirs, sizeof(DWORD));
                SetValue(actKey, CONFIG_COMPAREBYSUBDIRSATTR_REG, REG_DWORD, &Configuration.CompareSubdirsAttr, sizeof(DWORD));
                SetValue(actKey, CONFIG_COMPAREONEPANELDIRS_REG, REG_DWORD, &Configuration.CompareOnePanelDirs, sizeof(DWORD));
                SetValue(actKey, CONFIG_COMPAREMOREOPTIONS_REG, REG_DWORD, &Configuration.CompareMoreOptions, sizeof(DWORD));
                SetValue(actKey, CONFIG_COMPAREIGNOREFILES_REG, REG_DWORD, &Configuration.CompareIgnoreFiles, sizeof(DWORD));
                SetValue(actKey, CONFIG_COMPAREIGNOREDIRS_REG, REG_DWORD, &Configuration.CompareIgnoreDirs, sizeof(DWORD));
                SetValue(actKey, CONFIG_CONFIGTIGNOREFILESMASKS_REG, REG_SZ, Configuration.CompareIgnoreFilesMasks.GetMasksString(), -1);
                SetValue(actKey, CONFIG_CONFIGTIGNOREDIRSMASKS_REG, REG_SZ, Configuration.CompareIgnoreDirsMasks.GetMasksString(), -1);

                SetValue(actKey, CONFIG_THUMBNAILSIZE_REG, REG_DWORD, &Configuration.ThumbnailSize, sizeof(DWORD));
                SetValue(actKey, CONFIG_KEEPPLUGINSSORTED_REG, REG_DWORD, &Configuration.KeepPluginsSorted, sizeof(DWORD));
                SetValue(actKey, CONFIG_SHOWSLGINCOMPLETE_REG, REG_DWORD, &Configuration.ShowSLGIncomplete, sizeof(DWORD));

                // POZOR: pri padu v icon overlay handleru se tyto hodnoty zapisuji primo do registry
                //        (zamezeni "nespustitelnosti" Salama), viz InformAboutIconOvrlsHanCrash()
                SetValue(actKey, CONFIG_ENABLECUSTICOVRLS_REG, REG_DWORD, &Configuration.EnableCustomIconOverlays, sizeof(DWORD));
                SetValue(actKey, CONFIG_DISABLEDCUSTICOVRLS_REG, REG_SZ, Configuration.DisabledCustomIconOverlays != NULL ? Configuration.DisabledCustomIconOverlays : "", -1);

                SetValue(actKey, CONFIG_EDITNEWFILE_USEDEFAULT_REG, REG_DWORD, &Configuration.UseEditNewFileDefault, sizeof(DWORD));
                SetValue(actKey, CONFIG_EDITNEWFILE_DEFAULT_REG, REG_SZ, Configuration.EditNewFileDefault, -1);

#ifndef _WIN64 // FIXME_X64_WINSCP
                SetValue(actKey, "Add x86-Only Plugins", REG_DWORD,
                         &Configuration.AddX86OnlyPlugins, sizeof(DWORD));
#endif // _WIN64

                HKEY actSubKey;
                if (CreateKey(actKey, SALAMANDER_CONFIRMATION_REG, actSubKey))
                {
                    SetValue(actSubKey, CONFIG_CNFRM_FILEDIRDEL, REG_DWORD, &Configuration.CnfrmFileDirDel, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_NEDIRDEL, REG_DWORD, &Configuration.CnfrmNEDirDel, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_FILEOVER, REG_DWORD, &Configuration.CnfrmFileOver, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_DIROVER, REG_DWORD, &Configuration.CnfrmDirOver, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_SHFILEDEL, REG_DWORD, &Configuration.CnfrmSHFileDel, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_SHDIRDEL, REG_DWORD, &Configuration.CnfrmSHDirDel, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_SHFILEOVER, REG_DWORD, &Configuration.CnfrmSHFileOver, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_NTFSPRESS, REG_DWORD, &Configuration.CnfrmNTFSPress, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_NTFSCRYPT, REG_DWORD, &Configuration.CnfrmNTFSCrypt, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_DAD, REG_DWORD, &Configuration.CnfrmDragDrop, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_CLOSEARCHIVE, REG_DWORD, &Configuration.CnfrmCloseArchive, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_CLOSEFIND, REG_DWORD, &Configuration.CnfrmCloseFind, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_STOPFIND, REG_DWORD, &Configuration.CnfrmStopFind, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_CREATETARGETPATH, REG_DWORD, &Configuration.CnfrmCreatePath, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_ALWAYSONTOP, REG_DWORD, &Configuration.CnfrmAlwaysOnTop, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_ONSALCLOSE, REG_DWORD, &Configuration.CnfrmOnSalClose, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_SENDEMAIL, REG_DWORD, &Configuration.CnfrmSendEmail, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_ADDTOARCHIVE, REG_DWORD, &Configuration.CnfrmAddToArchive, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_CREATEDIR, REG_DWORD, &Configuration.CnfrmCreateDir, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_CHANGEDIRTC, REG_DWORD, &Configuration.CnfrmChangeDirTC, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_SHOWNAMETOCOMP, REG_DWORD, &Configuration.CnfrmShowNamesToCompare, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_DSTSHIFTSIGNORED, REG_DWORD, &Configuration.CnfrmDSTShiftsIgnored, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_DSTSHIFTSOCCURED, REG_DWORD, &Configuration.CnfrmDSTShiftsOccured, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_CNFRM_COPYMOVEOPTIONSNS, REG_DWORD, &Configuration.CnfrmCopyMoveOptionsNS, sizeof(DWORD));

                    CloseKey(actSubKey);
                }

                if (CreateKey(actKey, SALAMANDER_DRVSPEC_REG, actSubKey))
                {
                    SetValue(actSubKey, CONFIG_DRVSPEC_FLOPPY_MON, REG_DWORD, &Configuration.DrvSpecFloppyMon, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_DRVSPEC_FLOPPY_SIMPLE, REG_DWORD, &Configuration.DrvSpecFloppySimple, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_DRVSPEC_REMOVABLE_MON, REG_DWORD, &Configuration.DrvSpecRemovableMon, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_DRVSPEC_REMOVABLE_SIMPLE, REG_DWORD, &Configuration.DrvSpecRemovableSimple, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_DRVSPEC_FIXED_MON, REG_DWORD, &Configuration.DrvSpecFixedMon, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_DRVSPEC_FIXED_SIMPLE, REG_DWORD, &Configuration.DrvSpecFixedSimple, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_DRVSPEC_REMOTE_MON, REG_DWORD, &Configuration.DrvSpecRemoteMon, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_DRVSPEC_REMOTE_SIMPLE, REG_DWORD, &Configuration.DrvSpecRemoteSimple, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_DRVSPEC_REMOTE_ACT, REG_DWORD, &Configuration.DrvSpecRemoteDoNotRefreshOnAct, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_DRVSPEC_CDROM_MON, REG_DWORD, &Configuration.DrvSpecCDROMMon, sizeof(DWORD));
                    SetValue(actSubKey, CONFIG_DRVSPEC_CDROM_SIMPLE, REG_DWORD, &Configuration.DrvSpecCDROMSimple, sizeof(DWORD));
                    CloseKey(actSubKey);
                }

                SetValue(actKey, CONFIG_TOPTOOLBAR_REG, REG_SZ, Configuration.TopToolBar, -1);
                SetValue(actKey, CONFIG_MIDDLETOOLBAR_REG, REG_SZ, Configuration.MiddleToolBar, -1);

                SetValue(actKey, CONFIG_LEFTTOOLBAR_REG, REG_SZ, Configuration.LeftToolBar, -1);
                SetValue(actKey, CONFIG_RIGHTTOOLBAR_REG, REG_SZ, Configuration.RightToolBar, -1);

                SetValue(actKey, CONFIG_TOPTOOLBARVISIBLE_REG, REG_DWORD, &Configuration.TopToolBarVisible, sizeof(DWORD));
                SetValue(actKey, CONFIG_PLGTOOLBARVISIBLE_REG, REG_DWORD, &Configuration.PluginsBarVisible, sizeof(DWORD));
                SetValue(actKey, CONFIG_MIDDLETOOLBARVISIBLE_REG, REG_DWORD, &Configuration.MiddleToolBarVisible, sizeof(DWORD));

                SetValue(actKey, CONFIG_USERMENUTOOLBARVISIBLE_REG, REG_DWORD, &Configuration.UserMenuToolBarVisible, sizeof(DWORD));
                SetValue(actKey, CONFIG_HOTPATHSBARVISIBLE_REG, REG_DWORD, &Configuration.HotPathsBarVisible, sizeof(DWORD));

                SetValue(actKey, CONFIG_DRIVEBARVISIBLE_REG, REG_DWORD, &Configuration.DriveBarVisible, sizeof(DWORD));
                SetValue(actKey, CONFIG_DRIVEBAR2VISIBLE_REG, REG_DWORD, &Configuration.DriveBar2Visible, sizeof(DWORD));

                SetValue(actKey, CONFIG_BOTTOMTOOLBARVISIBLE_REG, REG_DWORD, &Configuration.BottomToolBarVisible, sizeof(DWORD));

                //      SetValue(actKey, CONFIG_SPACESELCALCSPACE, REG_DWORD, &Configuration.SpaceSelCalcSpace, sizeof(DWORD));
                SetValue(actKey, CONFIG_USETIMERESOLUTION, REG_DWORD, &Configuration.UseTimeResolution, sizeof(DWORD));
                SetValue(actKey, CONFIG_TIMERESOLUTION, REG_DWORD, &Configuration.TimeResolution, sizeof(DWORD));
                SetValue(actKey, CONFIG_IGNOREDSTSHIFTS, REG_DWORD, &Configuration.IgnoreDSTShifts, sizeof(DWORD));
                SetValue(actKey, CONFIG_USEDRAGDROPMINTIME, REG_DWORD, &Configuration.UseDragDropMinTime, sizeof(DWORD));
                SetValue(actKey, CONFIG_DRAGDROPMINTIME, REG_DWORD, &Configuration.DragDropMinTime, sizeof(DWORD));

                SetValue(actKey, CONFIG_LASTFOCUSEDPAGE, REG_DWORD, &Configuration.LastFocusedPage, sizeof(DWORD));
                SetValue(actKey, CONFIG_CONFIGURATION_HEIGHT, REG_DWORD, &Configuration.ConfigurationHeight, sizeof(DWORD));
                SetValue(actKey, CONFIG_VIEWANDEDITEXPAND, REG_DWORD, &Configuration.ViewersAndEditorsExpanded, sizeof(DWORD));
                SetValue(actKey, CONFIG_PACKEPAND, REG_DWORD, &Configuration.PackersAndUnpackersExpanded, sizeof(DWORD));

                SetValue(actKey, CONFIG_CMDLINE_REG, REG_DWORD, &EditPermanentVisible, sizeof(DWORD));
                SetValue(actKey, CONFIG_CMDLFOCUS_REG, REG_DWORD, &EditMode, sizeof(DWORD));

                SetValue(actKey, CONFIG_USECUSTOMPANELFONT_REG, REG_DWORD, &UseCustomPanelFont, sizeof(DWORD));
                SaveLogFont(actKey, CONFIG_PANELFONT_REG, &LogFont);

                if (GlobalSaveWaitWindow == NULL)
                    analysing.SetProgressPos(++savingProgress); // 4
                else
                    GlobalSaveWaitWindow->SetProgressPos(++GlobalSaveWaitWindowProgress); // 4
                //TRACE_I("analysing.SetProgressPos() savingProgress="<<savingProgress);

                SaveHistory(actKey, CONFIG_NAMEDHISTORY_REG, FindNamedHistory, FIND_NAMED_HISTORY_SIZE, !Configuration.SaveHistory);
                SaveHistory(actKey, CONFIG_LOOKINHISTORY_REG, FindLookInHistory, FIND_LOOKIN_HISTORY_SIZE, !Configuration.SaveHistory);
                SaveHistory(actKey, CONFIG_GREPHISTORY_REG, FindGrepHistory, FIND_GREP_HISTORY_SIZE, !Configuration.SaveHistory);
                SaveHistory(actKey, CONFIG_SELECTHISTORY_REG, Configuration.SelectHistory, SELECT_HISTORY_SIZE, !Configuration.SaveHistory);
                SaveHistory(actKey, CONFIG_COPYHISTORY_REG, Configuration.CopyHistory, COPY_HISTORY_SIZE, !Configuration.SaveHistory);
                SaveHistory(actKey, CONFIG_CHANGEDIRHISTORY_REG, Configuration.ChangeDirHistory, CHANGEDIR_HISTORY_SIZE, !Configuration.SaveHistory);

                if (GlobalSaveWaitWindow == NULL)
                    analysing.SetProgressPos(++savingProgress); // 5
                else
                    GlobalSaveWaitWindow->SetProgressPos(++GlobalSaveWaitWindowProgress); // 5
                //TRACE_I("analysing.SetProgressPos() savingProgress="<<savingProgress);

                SaveHistory(actKey, CONFIG_VIEWERHISTORY_REG, ViewerHistory, VIEWER_HISTORY_SIZE, !Configuration.SaveHistory);
                SaveHistory(actKey, CONFIG_COMMANDHISTORY_REG, Configuration.EditHistory, EDIT_HISTORY_SIZE, !(Configuration.SaveHistory && Configuration.EnableCmdLineHistory && Configuration.SaveCmdLineHistory));
                SaveHistory(actKey, CONFIG_FILELISTHISTORY_REG, Configuration.FileListHistory, FILELIST_HISTORY_SIZE, !Configuration.SaveHistory);
                SaveHistory(actKey, CONFIG_CREATEDIRHISTORY_REG, Configuration.CreateDirHistory, CREATEDIR_HISTORY_SIZE, !Configuration.SaveHistory);
                SaveHistory(actKey, CONFIG_QUICKRENAMEHISTORY_REG, Configuration.QuickRenameHistory, QUICKRENAME_HISTORY_SIZE, !Configuration.SaveHistory);
                SaveHistory(actKey, CONFIG_EDITNEWHISTORY_REG, Configuration.EditNewHistory, EDITNEW_HISTORY_SIZE, !Configuration.SaveHistory);
                SaveHistory(actKey, CONFIG_CONVERTHISTORY_REG, Configuration.ConvertHistory, CONVERT_HISTORY_SIZE, !Configuration.SaveHistory);
                SaveHistory(actKey, CONFIG_FILTERHISTORY_REG, Configuration.FilterHistory, FILTER_HISTORY_SIZE, !Configuration.SaveHistory);

                if (DirHistory != NULL)
                    DirHistory->SaveToRegistry(actKey, CONFIG_WORKDIRSHISTORY_REG, !Configuration.SaveWorkDirs);

                if (GlobalSaveWaitWindow == NULL)
                    analysing.SetProgressPos(++savingProgress); // 6
                else
                    GlobalSaveWaitWindow->SetProgressPos(++GlobalSaveWaitWindowProgress); // 6
                //TRACE_I("analysing.SetProgressPos() savingProgress="<<savingProgress);

                if (CreateKey(actKey, CONFIG_COPYMOVEOPTIONS_REG, actSubKey))
                {
                    CopyMoveOptions.Save(actSubKey);
                    CloseKey(actSubKey);
                }

                if (CreateKey(actKey, CONFIG_FINDOPTIONS_REG, actSubKey))
                {
                    FindOptions.Registry_Save(actSubKey);
                    CloseKey(actSubKey);
                }

                if (CreateKey(actKey, CONFIG_FINDIGNORE_REG, actSubKey))
                {
                    FindIgnore.Save(actSubKey);
                    CloseKey(actSubKey);
                }

                SetValue(actKey, CONFIG_FILELISTNAME_REG, REG_SZ, Configuration.FileListName, -1);
                SetValue(actKey, CONFIG_FILELISTAPPEND_REG, REG_DWORD, &Configuration.FileListAppend, sizeof(DWORD));
                SetValue(actKey, CONFIG_FILELISTDESTINATION_REG, REG_DWORD, &Configuration.FileListDestination, sizeof(DWORD));

                CloseKey(actKey);
            }

            //---  viewer

            if (CreateKey(salamander, SALAMANDER_VIEWER_REG, actKey))
            {
                SetValue(actKey, VIEWER_FINDFORWARD_REG, REG_DWORD, &GlobalFindDialog.Forward, sizeof(DWORD));
                SetValue(actKey, VIEWER_FINDWHOLEWORDS_REG, REG_DWORD, &GlobalFindDialog.WholeWords, sizeof(DWORD));
                SetValue(actKey, VIEWER_FINDCASESENSITIVE_REG, REG_DWORD, &GlobalFindDialog.CaseSensitive, sizeof(DWORD));
                SetValue(actKey, VIEWER_FINDREGEXP_REG, REG_DWORD, &GlobalFindDialog.Regular, sizeof(DWORD));
                SetValue(actKey, VIEWER_FINDTEXT_REG, REG_SZ, GlobalFindDialog.Text, -1);
                SetValue(actKey, VIEWER_FINDHEXMODE_REG, REG_DWORD, &GlobalFindDialog.HexMode, sizeof(DWORD));

                SetValue(actKey, VIEWER_CONFIGCRLF_REG, REG_DWORD, &Configuration.EOL_CRLF, sizeof(DWORD));
                SetValue(actKey, VIEWER_CONFIGCR_REG, REG_DWORD, &Configuration.EOL_CR, sizeof(DWORD));
                SetValue(actKey, VIEWER_CONFIGLF_REG, REG_DWORD,&Configuration.EOL_LF, sizeof(DWORD));
                SetValue(actKey, VIEWER_CONFIGNULL_REG, REG_DWORD, &Configuration.EOL_NULL, sizeof(DWORD));
                SetValue(actKey, VIEWER_CONFIGTABSIZE_REG, REG_DWORD, &Configuration.TabSize, sizeof(DWORD));
                SetValue(actKey, VIEWER_CONFIGDEFMODE_REG, REG_DWORD, &Configuration.DefViewMode, sizeof(DWORD));
                SetValue(actKey, VIEWER_CONFIGTEXTMASK_REG, REG_SZ, Configuration.TextModeMasks.GetMasksString(), -1);
                SetValue(actKey, VIEWER_CONFIGHEXMASK_REG, REG_SZ, Configuration.HexModeMasks.GetMasksString(), -1);
                SetValue(actKey, VIEWER_CONFIGUSECUSTOMFONT_REG, REG_DWORD, &UseCustomViewerFont, sizeof(DWORD));
                SaveLogFont(actKey, VIEWER_CONFIGFONT_REG, &ViewerLogFont);
                SetValue(actKey, VIEWER_WRAPTEXT_REG, REG_DWORD, &Configuration.WrapText, sizeof(DWORD));
                SetValue(actKey, VIEWER_CPAUTOSELECT_REG, REG_DWORD, &Configuration.CodePageAutoSelect, sizeof(DWORD));
                SetValue(actKey, VIEWER_DEFAULTCONVERT_REG, REG_SZ, Configuration.DefaultConvert, -1);
                SetValue(actKey, VIEWER_AUTOCOPYSELECTION_REG, REG_DWORD, &Configuration.AutoCopySelection, sizeof(DWORD));
                SetValue(actKey, VIEWER_GOTOOFFSETISHEX_REG, REG_DWORD, &Configuration.GoToOffsetIsHex, sizeof(DWORD));

                SetValue(actKey, VIEWER_CONFIGSAVEWINPOS_REG, REG_DWORD, &Configuration.SavePosition, sizeof(DWORD));
                if (Configuration.WindowPlacement.length != 0)
                {
                    SetValue(actKey, VIEWER_CONFIGWNDLEFT_REG, REG_DWORD, &Configuration.WindowPlacement.rcNormalPosition.left, sizeof(DWORD));
                    SetValue(actKey, VIEWER_CONFIGWNDRIGHT_REG, REG_DWORD, &Configuration.WindowPlacement.rcNormalPosition.right, sizeof(DWORD));
                    SetValue(actKey, VIEWER_CONFIGWNDTOP_REG, REG_DWORD, &Configuration.WindowPlacement.rcNormalPosition.top, sizeof(DWORD));
                    SetValue(actKey, VIEWER_CONFIGWNDBOTTOM_REG, REG_DWORD, &Configuration.WindowPlacement.rcNormalPosition.bottom, sizeof(DWORD));
                    SetValue(actKey, VIEWER_CONFIGWNDSHOW_REG, REG_DWORD, &Configuration.WindowPlacement.showCmd, sizeof(DWORD));
                }

                CloseKey(actKey);
            }

            //---  user menu

            if (CreateKey(salamander, SALAMANDER_USERMENU_REG, actKey))
            {
                ClearKey(actKey);

                HKEY subKey;
                char buf[30];
                int i;
                for (i = 0; i < UserMenuItems->Count; i++)
                {
                    itoa(i + 1, buf, 10);
                    if (CreateKey(actKey, buf, subKey))
                    {
                        SetValue(subKey, USERMENU_ITEMNAME_REG, REG_SZ, UserMenuItems->At(i)->ItemName, -1);
                        SetValue(subKey, USERMENU_COMMAND_REG, REG_SZ, UserMenuItems->At(i)->UMCommand, -1);
                        SetValue(subKey, USERMENU_ARGUMENTS_REG, REG_SZ, UserMenuItems->At(i)->Arguments, -1);
                        SetValue(subKey, USERMENU_INITDIR_REG, REG_SZ, UserMenuItems->At(i)->InitDir, -1);
                        SetValue(subKey, USERMENU_SHELL_REG, REG_DWORD, &UserMenuItems->At(i)->ThroughShell, sizeof(DWORD));
                        SetValue(subKey, USERMENU_CLOSE_REG, REG_DWORD, &UserMenuItems->At(i)->CloseShell, sizeof(DWORD));
                        SetValue(subKey, USERMENU_USEWINDOW_REG, REG_DWORD, &UserMenuItems->At(i)->UseWindow, sizeof(DWORD));

                        SetValue(subKey, USERMENU_ICON_REG, REG_SZ, UserMenuItems->At(i)->Icon, -1);
                        SetValue(subKey, USERMENU_TYPE_REG, REG_DWORD, &UserMenuItems->At(i)->Type, sizeof(DWORD));
                        SetValue(subKey, USERMENU_SHOWINTOOLBAR_REG, REG_DWORD, &UserMenuItems->At(i)->ShowInToolbar, sizeof(DWORD));

                        CloseKey(subKey);
                    }
                    else
                        break;
                }
                CloseKey(actKey);
            }

            //---  internal ZIP packer

            if (Configuration.ConfigVersion < 6 && // jen stary config, jinak klic nevytvarime+necistime
                CreateKey(salamander, SALAMANDER_IZIP_REG, actKey))
            {
                ClearKey(actKey);

                CloseKey(actKey);

                DeleteKey(salamander, SALAMANDER_IZIP_REG);
            }

            //---  viewers

            SaveViewers(salamander, SALAMANDER_VIEWERS_REG, ViewerMasks);
            SaveViewers(salamander, SALAMANDER_ALTVIEWERS_REG, AltViewerMasks);

            //---  editors

            SaveEditors(salamander, SALAMANDER_EDITORS_REG, EditorMasks);

            if (GlobalSaveWaitWindow == NULL)
                analysing.SetProgressPos(++savingProgress); // 7
            else
                GlobalSaveWaitWindow->SetProgressPos(++GlobalSaveWaitWindowProgress); // 7
            //TRACE_I("analysing.SetProgressPos() savingProgress="<<savingProgress);

            //---  colors
            if (CreateKey(salamander, SALAMANDER_CUSTOMCOLORS_REG, actKey))
            {
                char buff[10];
                int i;
                for (i = 0; i < NUMBER_OF_CUSTOMCOLORS; i++)
                {
                    itoa(i + 1, buff, 10);
                    SaveRGB(actKey, buff, CustomColors[i]);
                }

                CloseKey(actKey);
            }

            if (CreateKey(salamander, SALAMANDER_COLORS_REG, actKey))
            {
                DWORD scheme = 4; // custom
                if (CurrentColors == SalamanderColors)
                    scheme = 0;
                else if (CurrentColors == ExplorerColors)
                    scheme = 1;
                else if (CurrentColors == NortonColors)
                    scheme = 2;
                else if (CurrentColors == NavigatorColors)
                    scheme = 3;
                SetValue(actKey, SALAMANDER_CLRSCHEME_REG, REG_DWORD, &scheme, sizeof(DWORD));

                SaveRGBF(actKey, SALAMANDER_CLR_FOCUS_ACTIVE_NORMAL_REG, UserColors[FOCUS_ACTIVE_NORMAL]);
                SaveRGBF(actKey, SALAMANDER_CLR_FOCUS_ACTIVE_SELECTED_REG, UserColors[FOCUS_ACTIVE_SELECTED]);
                SaveRGBF(actKey, SALAMANDER_CLR_FOCUS_INACTIVE_NORMAL_REG, UserColors[FOCUS_FG_INACTIVE_NORMAL]);
                SaveRGBF(actKey, SALAMANDER_CLR_FOCUS_INACTIVE_SELECTED_REG, UserColors[FOCUS_FG_INACTIVE_SELECTED]);
                SaveRGBF(actKey, SALAMANDER_CLR_FOCUS_BK_INACTIVE_NORMAL_REG, UserColors[FOCUS_BK_INACTIVE_NORMAL]);
                SaveRGBF(actKey, SALAMANDER_CLR_FOCUS_BK_INACTIVE_SELECTED_REG, UserColors[FOCUS_BK_INACTIVE_SELECTED]);

                SaveRGBF(actKey, SALAMANDER_CLR_ITEM_FG_NORMAL_REG, UserColors[ITEM_FG_NORMAL]);
                SaveRGBF(actKey, SALAMANDER_CLR_ITEM_FG_SELECTED_REG, UserColors[ITEM_FG_SELECTED]);
                SaveRGBF(actKey, SALAMANDER_CLR_ITEM_FG_FOCUSED_REG, UserColors[ITEM_FG_FOCUSED]);
                SaveRGBF(actKey, SALAMANDER_CLR_ITEM_FG_FOCSEL_REG, UserColors[ITEM_FG_FOCSEL]);
                SaveRGBF(actKey, SALAMANDER_CLR_ITEM_FG_HIGHLIGHT_REG, UserColors[ITEM_FG_HIGHLIGHT]);

                SaveRGBF(actKey, SALAMANDER_CLR_ITEM_BK_NORMAL_REG, UserColors[ITEM_BK_NORMAL]);
                SaveRGBF(actKey, SALAMANDER_CLR_ITEM_BK_SELECTED_REG, UserColors[ITEM_BK_SELECTED]);
                SaveRGBF(actKey, SALAMANDER_CLR_ITEM_BK_FOCUSED_REG, UserColors[ITEM_BK_FOCUSED]);
                SaveRGBF(actKey, SALAMANDER_CLR_ITEM_BK_FOCSEL_REG, UserColors[ITEM_BK_FOCSEL]);
                SaveRGBF(actKey, SALAMANDER_CLR_ITEM_BK_HIGHLIGHT_REG, UserColors[ITEM_BK_HIGHLIGHT]);

                SaveRGBF(actKey, SALAMANDER_CLR_ICON_BLEND_SELECTED_REG, UserColors[ICON_BLEND_SELECTED]);
                SaveRGBF(actKey, SALAMANDER_CLR_ICON_BLEND_FOCUSED_REG, UserColors[ICON_BLEND_FOCUSED]);
                SaveRGBF(actKey, SALAMANDER_CLR_ICON_BLEND_FOCSEL_REG, UserColors[ICON_BLEND_FOCSEL]);

                SaveRGBF(actKey, SALAMANDER_CLR_PROGRESS_FG_NORMAL_REG, UserColors[PROGRESS_FG_NORMAL]);
                SaveRGBF(actKey, SALAMANDER_CLR_PROGRESS_FG_SELECTED_REG, UserColors[PROGRESS_FG_SELECTED]);
                SaveRGBF(actKey, SALAMANDER_CLR_PROGRESS_BK_NORMAL_REG, UserColors[PROGRESS_BK_NORMAL]);
                SaveRGBF(actKey, SALAMANDER_CLR_PROGRESS_BK_SELECTED_REG, UserColors[PROGRESS_BK_SELECTED]);

                SaveRGBF(actKey, SALAMANDER_CLR_HOT_PANEL_REG, UserColors[HOT_PANEL]);
                SaveRGBF(actKey, SALAMANDER_CLR_HOT_ACTIVE_REG, UserColors[HOT_ACTIVE]);
                SaveRGBF(actKey, SALAMANDER_CLR_HOT_INACTIVE_REG, UserColors[HOT_INACTIVE]);

                SaveRGBF(actKey, SALAMANDER_CLR_ACTIVE_CAPTION_FG_REG, UserColors[ACTIVE_CAPTION_FG]);
                SaveRGBF(actKey, SALAMANDER_CLR_ACTIVE_CAPTION_BK_REG, UserColors[ACTIVE_CAPTION_BK]);
                SaveRGBF(actKey, SALAMANDER_CLR_INACTIVE_CAPTION_FG_REG, UserColors[INACTIVE_CAPTION_FG]);
                SaveRGBF(actKey, SALAMANDER_CLR_INACTIVE_CAPTION_BK_REG, UserColors[INACTIVE_CAPTION_BK]);

                SaveRGBF(actKey, SALAMANDER_CLR_THUMBNAIL_FRAME_NORMAL_REG, UserColors[THUMBNAIL_FRAME_NORMAL]);
                SaveRGBF(actKey, SALAMANDER_CLR_THUMBNAIL_FRAME_SELECTED_REG, UserColors[THUMBNAIL_FRAME_SELECTED]);
                SaveRGBF(actKey, SALAMANDER_CLR_THUMBNAIL_FRAME_FOCUSED_REG, UserColors[THUMBNAIL_FRAME_FOCUSED]);
                SaveRGBF(actKey, SALAMANDER_CLR_THUMBNAIL_FRAME_FOCSEL_REG, UserColors[THUMBNAIL_FRAME_FOCSEL]);

                SaveRGBF(actKey, SALAMANDER_CLR_VIEWER_FG_NORMAL_REG, ViewerColors[VIEWER_FG_NORMAL]);
                SaveRGBF(actKey, SALAMANDER_CLR_VIEWER_BK_NORMAL_REG, ViewerColors[VIEWER_BK_NORMAL]);
                SaveRGBF(actKey, SALAMANDER_CLR_VIEWER_FG_SELECTED_REG, ViewerColors[VIEWER_FG_SELECTED]);
                SaveRGBF(actKey, SALAMANDER_CLR_VIEWER_BK_SELECTED_REG, ViewerColors[VIEWER_BK_SELECTED]);

                // ulozim barvy pro highlighting souboru
                HKEY hHltKey;
                if (CreateKey(actKey, SALAMANDER_HLT, hHltKey))
                {
                    ClearKey(hHltKey);
                    HKEY hSubKey;
                    char buf[30];
                    int i;
                    for (i = 0; i < HighlightMasks->Count; i++)
                    {
                        itoa(i + 1, buf, 10);
                        if (CreateKey(hHltKey, buf, hSubKey))
                        {
                            CHighlightMasksItem* item = HighlightMasks->At(i);
                            SetValue(hSubKey, SALAMANDER_HLT_ITEM_MASKS, REG_SZ, item->Masks->GetMasksString(), -1);
                            SetValue(hSubKey, SALAMANDER_HLT_ITEM_ATTR, REG_DWORD, &item->Attr, sizeof(DWORD));
                            SetValue(hSubKey, SALAMANDER_HLT_ITEM_VALIDATTR, REG_DWORD, &item->ValidAttr, sizeof(DWORD));

                            SaveRGBF(hSubKey, SALAMANDER_HLT_ITEM_FG_NORMAL_REG, item->NormalFg);
                            SaveRGBF(hSubKey, SALAMANDER_HLT_ITEM_FG_SELECTED_REG, item->SelectedFg);
                            SaveRGBF(hSubKey, SALAMANDER_HLT_ITEM_FG_FOCUSED_REG, item->FocusedFg);
                            SaveRGBF(hSubKey, SALAMANDER_HLT_ITEM_FG_FOCSEL_REG, item->FocSelFg);
                            SaveRGBF(hSubKey, SALAMANDER_HLT_ITEM_FG_HIGHLIGHT_REG, item->HighlightFg);

                            SaveRGBF(hSubKey, SALAMANDER_HLT_ITEM_BK_NORMAL_REG, item->NormalBk);
                            SaveRGBF(hSubKey, SALAMANDER_HLT_ITEM_BK_SELECTED_REG, item->SelectedBk);
                            SaveRGBF(hSubKey, SALAMANDER_HLT_ITEM_BK_FOCUSED_REG, item->FocusedBk);
                            SaveRGBF(hSubKey, SALAMANDER_HLT_ITEM_BK_FOCSEL_REG, item->FocSelBk);
                            SaveRGBF(hSubKey, SALAMANDER_HLT_ITEM_BK_HIGHLIGHT_REG, item->HighlightBk);
                            CloseKey(hSubKey);
                        }
                    }
                    CloseKey(hHltKey);
                }
                CloseKey(actKey);
            }

            if (GlobalSaveWaitWindow == NULL)
                analysing.SetProgressPos(++savingProgress); // 8
            else
                GlobalSaveWaitWindow->SetProgressPos(++GlobalSaveWaitWindowProgress); // 8
            //TRACE_I("analysing.SetProgressPos() savingProgress="<<savingProgress);

            if (deleteSALAMANDER_SAVE_IN_PROGRESS)
            {
                DeleteValue(salamander, SALAMANDER_SAVE_IN_PROGRESS);
                IsSetSALAMANDER_SAVE_IN_PROGRESS = FALSE;
            }
        }
        CloseKey(salamander);
    }

    LoadSaveToRegistryMutex.Leave();

    if (GlobalSaveWaitWindow == NULL)
    {
        EnableWindow(parent, TRUE);
        PluginMsgBoxParent = oldPluginMsgBoxParent;
        DestroyWindow(analysing.HWindow);
        SetCursor(hOldCursor);
    }
}

void CMainWindow::LoadPanelConfig(char* panelPath, CPanelWindow* panel, HKEY hSalamander, const TCHAR* reg)
{
    HKEY actKey;
    if (OpenKey(hSalamander, reg, actKey))
    {
        DWORD value;
        if (GetValue(actKey, PANEL_PATH_REG, REG_SZ, panelPath, MAX_PATH))
        {
            if (GetValue(actKey, PANEL_HEADER_REG, REG_DWORD, &value, sizeof(DWORD)))
                panel->HeaderLineVisible = value;
            if (GetValue(actKey, PANEL_VIEW_REG, REG_DWORD, &value, sizeof(DWORD)))
            {
                if (Configuration.ConfigVersion < 13 && !value) // konverze: Detailed byl ulozen jako FALSE
                    value = 2;
                panel->SelectViewTemplate(value, FALSE, FALSE, VALID_DATA_ALL, FALSE, TRUE);
            }
            if (GetValue(actKey, PANEL_REVERSE_REG, REG_DWORD, &value, sizeof(DWORD)))
                panel->ReverseSort = value;
            if (GetValue(actKey, PANEL_SORT_REG, REG_DWORD, &value, sizeof(DWORD)))
            {
                if (value > stAttr)
                    value = stName;
                panel->SortType = (CSortType)value;
            }
            if (GetValue(actKey, PANEL_DIRLINE_REG, REG_DWORD, &value, sizeof(DWORD)))
                if ((BOOL)value != (panel->DirectoryLine->HWindow != NULL))
                    panel->ToggleDirectoryLine();
            if (GetValue(actKey, PANEL_STATUS_REG, REG_DWORD, &value, sizeof(DWORD)))
                if ((BOOL)value != (panel->StatusLine->HWindow != NULL))
                    panel->ToggleStatusLine();
            GetValue(actKey, PANEL_FILTER_ENABLE, REG_DWORD, &panel->FilterEnabled,
                     sizeof(DWORD));

            char filter[MAX_PATH];
            if (!GetValue(actKey, PANEL_FILTER, REG_SZ, filter, MAX_PATH))
            {
                filter[0] = 0;
                if (Configuration.ConfigVersion < 22)
                {
                    char* filterHistory[1];
                    filterHistory[0] = NULL;
                    LoadHistory(actKey, PANEL_FILTERHISTORY_REG, filterHistory, 1);
                    if (filterHistory[0] != NULL) // at se nacita i pocatecni stav filtru
                    {
                        DWORD filterInverse = FALSE;
                        if (panel->FilterEnabled && Configuration.ConfigVersion < 14) // konverze: byl zrusen checkbox pro inverzni filtr
                            GetValue(actKey, PANEL_FILTER_INVERSE, REG_DWORD, &filterInverse, sizeof(DWORD));
                        if (filterInverse)
                            strcpy(filter, "|");
                        else
                            filter[0] = 0;
                        strcat(filter, filterHistory[0]);
                        free(filterHistory[0]);
                    }
                }
                else
                    panel->FilterEnabled = FALSE;
            }
            if (filter[0] != 0)
                panel->Filter.SetMasksString(filter);

            panel->UpdateFilterSymbol();
            int errPos;
            if (!panel->Filter.PrepareMasks(errPos))
            {
                panel->Filter.SetMasksString("*.*");
                panel->Filter.PrepareMasks(errPos);
            }
        }

        CloseKey(actKey);
    }
}

void LoadIconOvrlsInfo(const TCHAR* root)
{
    HKEY hSalamander;
    if (OpenKey(HKEY_CURRENT_USER, root, hSalamander))
    {
        HKEY actKey;
        DWORD configVersion = 1; // toto je konfig od 1.52 a starsi
        if (OpenKey(hSalamander, SALAMANDER_VERSION_REG, actKey))
        {
            configVersion = 2; // toto je konfig od 1.6b1
            GetValue(actKey, SALAMANDER_VERSIONREG_REG, REG_DWORD,
                     &configVersion, sizeof(DWORD));
            CloseKey(actKey);
        }
        if (OpenKey(hSalamander, SALAMANDER_CONFIG_REG, actKey))
        {
            ClearListOfDisabledCustomIconOverlays();
            DWORD disabledCustomIconOverlaysBufSize;

            if (
                GetValue(actKey, CONFIG_ENABLECUSTICOVRLS_REG, REG_DWORD,&Configuration.EnableCustomIconOverlays, sizeof(DWORD))
                &&
                GetSize(actKey, CONFIG_DISABLEDCUSTICOVRLS_REG, REG_SZ, disabledCustomIconOverlaysBufSize)
            )
            {
                if (disabledCustomIconOverlaysBufSize > 1) // <= 1 znamena prazdny string a na to staci NULL
                {
                    Configuration.DisabledCustomIconOverlays = (char*)malloc(disabledCustomIconOverlaysBufSize);

                    if (Configuration.DisabledCustomIconOverlays == NULL)
                    {
                        TRACE_E(LOW_MEMORY);
                        Configuration.EnableCustomIconOverlays = FALSE; // z bezpecnostnich duvodu (icon overlay handlery dost padaji)
                    }
                    else
                    {
                        if (!GetValue(actKey, CONFIG_DISABLEDCUSTICOVRLS_REG, REG_SZ,Configuration.DisabledCustomIconOverlays, disabledCustomIconOverlaysBufSize))
                        {
                            free(Configuration.DisabledCustomIconOverlays);
                            Configuration.DisabledCustomIconOverlays = NULL;
                            Configuration.EnableCustomIconOverlays = FALSE; // z bezpecnostnich duvodu (icon overlay handlery dost padaji)
                        }
                    }
                }
            }
            else
            {
                if (configVersion >= 41) // pokud v novych konfiguracich chybi tato hodnota, zakazeme overlaye (ve starsich verzich tyto promenne nebyly, tedy nejde o chybu - overlaye nechame zapnute)
                    Configuration.EnableCustomIconOverlays = FALSE;
            }

            CloseKey(actKey);
        }
        CloseKey(hSalamander);
    }
}

BOOL CMainWindow::LoadConfig(BOOL importingOldConfig, const CCommandLineParams* cmdLineParams)
{
    CALL_STACK_MESSAGE2("CMainWindow::LoadConfig(%d)", importingOldConfig);
    if (SALAMANDER_ROOT_REG == NULL)
        return FALSE;

    LoadSaveToRegistryMutex.Enter();

    HKEY salamander;
    if (OpenKey(HKEY_CURRENT_USER, SALAMANDER_ROOT_REG, salamander))
    {
        HKEY actKey;
        BOOL ret = TRUE;

        IfExistSetSplashScreenText(LoadStr(IDS_STARTUP_CONFIG));

        Configuration.ConfigVersion = 1; // toto je konfig od 1.52 a starsi
                                         //--- version
        if (OpenKey(salamander, SALAMANDER_VERSION_REG, actKey))
        {
            Configuration.ConfigVersion = 2; // toto je konfig od 1.6b1
            GetValue(actKey, SALAMANDER_VERSIONREG_REG, REG_DWORD,
                     &Configuration.ConfigVersion, sizeof(DWORD));
            CloseKey(actKey);
        }

        //---  viewers

        EnterViewerMasksCS();
        LoadViewers(salamander, SALAMANDER_VIEWERS_REG, ViewerMasks);
        LeaveViewerMasksCS();
        LoadViewers(salamander, SALAMANDER_ALTVIEWERS_REG, AltViewerMasks);

        //---  editors

        LoadEditors(salamander, SALAMANDER_EDITORS_REG, EditorMasks);

        //---  colors
        if (OpenKey(salamander, SALAMANDER_CUSTOMCOLORS_REG, actKey))
        {
            char buff[10];
            int i;
            for (i = 0; i < NUMBER_OF_CUSTOMCOLORS; i++)
            {
                itoa(i + 1, buff, 10);
                LoadRGB(actKey, buff, CustomColors[i]);
            }

            CloseKey(actKey);
        }

        if (OpenKey(salamander, SALAMANDER_COLORS_REG, actKey))
        {
            DWORD scheme;
            CurrentColors = UserColors;
            if (GetValue(actKey, SALAMANDER_CLRSCHEME_REG, REG_DWORD, &scheme, sizeof(DWORD)))
            {
                // pridali jsme nove schema (DOS Navigator) na pozici 3
                if (Configuration.ConfigVersion < 28 && scheme == 3)
                    scheme = 4;

                if (scheme == 0)
                    CurrentColors = SalamanderColors;
                else if (scheme == 1)
                    CurrentColors = ExplorerColors;
                else if (scheme == 2)
                    CurrentColors = NortonColors;
                else if (scheme == 3)
                    CurrentColors = NavigatorColors;
            }

            LoadRGBF(actKey, SALAMANDER_CLR_ITEM_FG_NORMAL_REG, UserColors[ITEM_FG_NORMAL]);
            LoadRGBF(actKey, SALAMANDER_CLR_ITEM_FG_SELECTED_REG, UserColors[ITEM_FG_SELECTED]);
            LoadRGBF(actKey, SALAMANDER_CLR_ITEM_FG_FOCUSED_REG, UserColors[ITEM_FG_FOCUSED]);
            LoadRGBF(actKey, SALAMANDER_CLR_ITEM_FG_FOCSEL_REG, UserColors[ITEM_FG_FOCSEL]);
            LoadRGBF(actKey, SALAMANDER_CLR_ITEM_FG_HIGHLIGHT_REG, UserColors[ITEM_FG_HIGHLIGHT]);

            LoadRGBF(actKey, SALAMANDER_CLR_ITEM_BK_NORMAL_REG, UserColors[ITEM_BK_NORMAL]);
            LoadRGBF(actKey, SALAMANDER_CLR_ITEM_BK_SELECTED_REG, UserColors[ITEM_BK_SELECTED]);
            LoadRGBF(actKey, SALAMANDER_CLR_ITEM_BK_FOCUSED_REG, UserColors[ITEM_BK_FOCUSED]);
            LoadRGBF(actKey, SALAMANDER_CLR_ITEM_BK_FOCSEL_REG, UserColors[ITEM_BK_FOCSEL]);
            LoadRGBF(actKey, SALAMANDER_CLR_ITEM_BK_HIGHLIGHT_REG, UserColors[ITEM_BK_HIGHLIGHT]);

            LoadRGBF(actKey, SALAMANDER_CLR_FOCUS_ACTIVE_NORMAL_REG, UserColors[FOCUS_ACTIVE_NORMAL]);
            LoadRGBF(actKey, SALAMANDER_CLR_FOCUS_ACTIVE_SELECTED_REG, UserColors[FOCUS_ACTIVE_SELECTED]);
            LoadRGBF(actKey, SALAMANDER_CLR_FOCUS_INACTIVE_NORMAL_REG, UserColors[FOCUS_FG_INACTIVE_NORMAL]);
            LoadRGBF(actKey, SALAMANDER_CLR_FOCUS_INACTIVE_SELECTED_REG, UserColors[FOCUS_FG_INACTIVE_SELECTED]);
            if (!LoadRGBF(actKey, SALAMANDER_CLR_FOCUS_BK_INACTIVE_NORMAL_REG, UserColors[FOCUS_BK_INACTIVE_NORMAL]))
                UserColors[FOCUS_BK_INACTIVE_NORMAL] = UserColors[ITEM_BK_NORMAL]; // konverze starsich konfiguraci
            if (!LoadRGBF(actKey, SALAMANDER_CLR_FOCUS_BK_INACTIVE_SELECTED_REG, UserColors[FOCUS_BK_INACTIVE_SELECTED]))
                UserColors[FOCUS_BK_INACTIVE_SELECTED] = UserColors[ITEM_BK_NORMAL]; // konverze starsich konfiguraci

            LoadRGBF(actKey, SALAMANDER_CLR_ICON_BLEND_SELECTED_REG, UserColors[ICON_BLEND_SELECTED]);
            LoadRGBF(actKey, SALAMANDER_CLR_ICON_BLEND_FOCUSED_REG, UserColors[ICON_BLEND_FOCUSED]);
            LoadRGBF(actKey, SALAMANDER_CLR_ICON_BLEND_FOCSEL_REG, UserColors[ICON_BLEND_FOCSEL]);

            LoadRGBF(actKey, SALAMANDER_CLR_PROGRESS_FG_NORMAL_REG, UserColors[PROGRESS_FG_NORMAL]);
            LoadRGBF(actKey, SALAMANDER_CLR_PROGRESS_FG_SELECTED_REG, UserColors[PROGRESS_FG_SELECTED]);
            LoadRGBF(actKey, SALAMANDER_CLR_PROGRESS_BK_NORMAL_REG, UserColors[PROGRESS_BK_NORMAL]);
            LoadRGBF(actKey, SALAMANDER_CLR_PROGRESS_BK_SELECTED_REG, UserColors[PROGRESS_BK_SELECTED]);

            LoadRGBF(actKey, SALAMANDER_CLR_HOT_PANEL_REG, UserColors[HOT_PANEL]);
            LoadRGBF(actKey, SALAMANDER_CLR_HOT_ACTIVE_REG, UserColors[HOT_ACTIVE]);
            LoadRGBF(actKey, SALAMANDER_CLR_HOT_INACTIVE_REG, UserColors[HOT_INACTIVE]);

            LoadRGBF(actKey, SALAMANDER_CLR_ACTIVE_CAPTION_FG_REG, UserColors[ACTIVE_CAPTION_FG]);
            LoadRGBF(actKey, SALAMANDER_CLR_ACTIVE_CAPTION_BK_REG, UserColors[ACTIVE_CAPTION_BK]);
            LoadRGBF(actKey, SALAMANDER_CLR_INACTIVE_CAPTION_FG_REG, UserColors[INACTIVE_CAPTION_FG]);
            LoadRGBF(actKey, SALAMANDER_CLR_INACTIVE_CAPTION_BK_REG, UserColors[INACTIVE_CAPTION_BK]);

            LoadRGBF(actKey, SALAMANDER_CLR_THUMBNAIL_FRAME_NORMAL_REG, UserColors[THUMBNAIL_FRAME_NORMAL]);
            LoadRGBF(actKey, SALAMANDER_CLR_THUMBNAIL_FRAME_SELECTED_REG, UserColors[THUMBNAIL_FRAME_SELECTED]);
            LoadRGBF(actKey, SALAMANDER_CLR_THUMBNAIL_FRAME_FOCUSED_REG, UserColors[THUMBNAIL_FRAME_FOCUSED]);
            LoadRGBF(actKey, SALAMANDER_CLR_THUMBNAIL_FRAME_FOCSEL_REG, UserColors[THUMBNAIL_FRAME_FOCSEL]);

            LoadRGBF(actKey, SALAMANDER_CLR_VIEWER_FG_NORMAL_REG, ViewerColors[VIEWER_FG_NORMAL]);
            LoadRGBF(actKey, SALAMANDER_CLR_VIEWER_BK_NORMAL_REG, ViewerColors[VIEWER_BK_NORMAL]);
            LoadRGBF(actKey, SALAMANDER_CLR_VIEWER_FG_SELECTED_REG, ViewerColors[VIEWER_FG_SELECTED]);
            LoadRGBF(actKey, SALAMANDER_CLR_VIEWER_BK_SELECTED_REG, ViewerColors[VIEWER_BK_SELECTED]);

            // nactu barvy pro highlighting souboru
            HKEY hHltKey;
            if (OpenKey(actKey, SALAMANDER_HLT, hHltKey))
            {
                HKEY hSubKey;
                char buf[30];
                strcpy(buf, "1");
                int i = 1;
                HighlightMasks->DestroyMembers();
                while (OpenKey(hHltKey, buf, hSubKey))
                {
                    char masks[MAX_PATH];
                    if (GetValue(hSubKey, SALAMANDER_HLT_ITEM_MASKS, REG_SZ, masks, MAX_PATH))
                    {
                        CHighlightMasksItem* item = new CHighlightMasksItem();
                        if (item == NULL || !item->Set(masks))
                        {
                            TRACE_E(LOW_MEMORY);
                            if (item != NULL)
                                delete item;
                            continue;
                        }
                        int errPos;
                        item->Masks->PrepareMasks(errPos);

                        GetValue(hSubKey, SALAMANDER_HLT_ITEM_ATTR, REG_DWORD, &item->Attr, sizeof(DWORD));
                        GetValue(hSubKey, SALAMANDER_HLT_ITEM_VALIDATTR, REG_DWORD, &item->ValidAttr, sizeof(DWORD));

                        LoadRGBF(hSubKey, SALAMANDER_HLT_ITEM_FG_NORMAL_REG, item->NormalFg);
                        LoadRGBF(hSubKey, SALAMANDER_HLT_ITEM_FG_SELECTED_REG, item->SelectedFg);
                        LoadRGBF(hSubKey, SALAMANDER_HLT_ITEM_FG_FOCUSED_REG, item->FocusedFg);
                        LoadRGBF(hSubKey, SALAMANDER_HLT_ITEM_FG_FOCSEL_REG, item->FocSelFg);
                        LoadRGBF(hSubKey, SALAMANDER_HLT_ITEM_FG_HIGHLIGHT_REG, item->HighlightFg);

                        LoadRGBF(hSubKey, SALAMANDER_HLT_ITEM_BK_NORMAL_REG, item->NormalBk);
                        LoadRGBF(hSubKey, SALAMANDER_HLT_ITEM_BK_SELECTED_REG, item->SelectedBk);
                        LoadRGBF(hSubKey, SALAMANDER_HLT_ITEM_BK_FOCUSED_REG, item->FocusedBk);
                        LoadRGBF(hSubKey, SALAMANDER_HLT_ITEM_BK_FOCSEL_REG, item->FocSelBk);
                        LoadRGBF(hSubKey, SALAMANDER_HLT_ITEM_BK_HIGHLIGHT_REG, item->HighlightBk);
                        HighlightMasks->Add(item);
                        if (!HighlightMasks->IsGood())
                        {
                            HighlightMasks->ResetState();
                            delete item;
                        }
                        itoa(++i, buf, 10);
                        CloseKey(hSubKey);
                    }
                }
                if (Configuration.ConfigVersion < 16) // pridame barveni Encrypted souboru/adresaru
                {
                    CHighlightMasksItem* hItem = new CHighlightMasksItem();
                    if (hItem != NULL)
                    {
                        HighlightMasks->Add(hItem);
                        hItem->Set("*.*");
                        int errPos;
                        hItem->Masks->PrepareMasks(errPos);
                        hItem->NormalFg = RGBF(19, 143, 13, 0); // barva vzata z Windows XP
                        hItem->FocusedFg = RGBF(19, 143, 13, 0);
                        hItem->ValidAttr = FILE_ATTRIBUTE_ENCRYPTED;
                        hItem->Attr = FILE_ATTRIBUTE_ENCRYPTED;
                    }
                }
                CloseKey(hHltKey);
            }

            ColorsChanged(FALSE, TRUE, TRUE); // sporime cas, nechame zmenit jen barvo-zavisle polozky

            CloseKey(actKey);
        }

        //---  window

        WINDOWPLACEMENT place;
        BOOL useWinPlacement = FALSE;
        if (OpenKey(salamander, SALAMANDER_WINDOW_REG, actKey))
        {
            place.length = sizeof(WINDOWPLACEMENT);
            GetWindowPlacement(HWindow, &place);
            if (
                GetValue(actKey, WINDOW_LEFT_REG, REG_DWORD, &(place.rcNormalPosition.left), sizeof(DWORD))
                &&
                GetValue(actKey, WINDOW_RIGHT_REG, REG_DWORD, &(place.rcNormalPosition.right), sizeof(DWORD))
                &&
                GetValue(actKey, WINDOW_TOP_REG, REG_DWORD, &(place.rcNormalPosition.top), sizeof(DWORD))
                &&
                GetValue(actKey, WINDOW_BOTTOM_REG, REG_DWORD, &(place.rcNormalPosition.bottom), sizeof(DWORD))
                &&
                GetValue(actKey, WINDOW_SHOW_REG, REG_DWORD, &(place.showCmd), sizeof(DWORD))
            )
            {
                char buf[20];
                if (GetValue(actKey, WINDOW_SPLIT_REG, REG_SZ, buf, 20))
                {
                    sscanf(buf, "%lf", &SplitPosition);
                    SplitPosition /= 100;
                    if (SplitPosition < 0)
                        SplitPosition = 0;
                    if (SplitPosition > 1)
                        SplitPosition = 1;
                }
                if (GetValue(actKey, WINDOW_BEFOREZOOMSPLIT_REG, REG_SZ, buf, 20))
                {
                    sscanf(buf, "%lf", &BeforeZoomSplitPosition);
                    BeforeZoomSplitPosition /= 100;
                    if (BeforeZoomSplitPosition < 0)
                        BeforeZoomSplitPosition = 0;
                    if (BeforeZoomSplitPosition > 1)
                        BeforeZoomSplitPosition = 1;
                }
                useWinPlacement = TRUE;
            }
            else
                ret = FALSE;

            CloseKey(actKey);
        }
        else
            ret = FALSE;

        if (OpenKey(salamander, FINDDIALOG_WINDOW_REG, actKey))
        {
            Configuration.FindDialogWindowPlacement.length = sizeof(WINDOWPLACEMENT);

            GetValue(actKey, WINDOW_LEFT_REG, REG_DWORD, &(Configuration.FindDialogWindowPlacement.rcNormalPosition.left), sizeof(DWORD));
            GetValue(actKey, WINDOW_RIGHT_REG, REG_DWORD, &(Configuration.FindDialogWindowPlacement.rcNormalPosition.right), sizeof(DWORD));
            GetValue(actKey, WINDOW_TOP_REG, REG_DWORD, &(Configuration.FindDialogWindowPlacement.rcNormalPosition.top), sizeof(DWORD));
            GetValue(actKey, WINDOW_BOTTOM_REG, REG_DWORD, &(Configuration.FindDialogWindowPlacement.rcNormalPosition.bottom), sizeof(DWORD));
            GetValue(actKey, WINDOW_SHOW_REG, REG_DWORD, &(Configuration.FindDialogWindowPlacement.showCmd), sizeof(DWORD));

            GetValue(actKey, FINDDIALOG_NAMEWIDTH_REG, REG_DWORD, &(Configuration.FindColNameWidth), sizeof(DWORD));
            CloseKey(actKey);
        }

        //---  default directories

        if (OpenKey(salamander, SALAMANDER_DEFDIRS_REG, actKey))
        {
            DWORD values;
            DWORD res = RegQueryInfoKey(actKey, NULL, 0, 0, NULL, NULL, NULL, &values, NULL, NULL, NULL, NULL);
            if (res == ERROR_SUCCESS)
            {
                char dir[4] = TEXT( " :\\" ); // reset DefaultDir
                char d;
                for (d = 'A'; d <= 'Z'; d++)
                {
                    dir[0] = d;
                    strcpy(DefaultDir[d - 'A'], dir);
                }

                char name[2];
                BYTE path[MAX_PATH];
                DWORD nameLen, dataLen, type;

                int i;
                for (i = 0; i < (int)values; i++)
                {
                    nameLen = 2;
                    dataLen = MAX_PATH;
                    res = RegEnumValue(actKey, i, name, &nameLen, 0, &type, path, &dataLen);
                    if (res == ERROR_SUCCESS)
                        if (type == REG_SZ)
                        {
                            char d2 = LowerCase[name[0]];
                            if (d2 >= 'a' && d2 <= 'z')
                            {
                                if (dataLen > 2 && LowerCase[path[0]] == d2 &&
                                    path[1] == ':' && path[2] == '\\')
                                    memmove(DefaultDir[d2 - 'a'], path, dataLen);
                                else
                                    SalMessageBox(HWindow, LoadStr(IDS_UNEXPECTEDVALUE), LoadStr(IDS_ERRORLOADCONFIG), MB_OK | MB_ICONEXCLAMATION);
                            }
                            else
                                SalMessageBox(HWindow, LoadStr(IDS_UNEXPECTEDVALUE), LoadStr(IDS_ERRORLOADCONFIG), MB_OK | MB_ICONEXCLAMATION);
                        }
                        else
                            SalMessageBox(HWindow, LoadStr(IDS_UNEXPECTEDVALUETYPE), LoadStr(IDS_ERRORLOADCONFIG), MB_OK | MB_ICONEXCLAMATION);
                    else
                        SalMessageBox(HWindow, GetErrorText(res), LoadStr(IDS_ERRORLOADCONFIG), MB_OK | MB_ICONEXCLAMATION);
                }
            }
            else if (res != ERROR_FILE_NOT_FOUND)
                SalMessageBox(HWindow, GetErrorText(res), LoadStr(IDS_ERRORLOADCONFIG), MB_OK | MB_ICONEXCLAMATION);
            CloseKey(actKey);
        }

        //---  password manager

        if (OpenKey(salamander, SALAMANDER_PWDMNGR_REG, actKey))
        {
            PasswordManager.Load(actKey);
            CloseKey(actKey);
        }

        //---  hot paths

        if (OpenKey(salamander, SALAMANDER_HOTPATHS_REG, actKey))
        {
            if (Configuration.ConfigVersion == 1) // je treba nakonvertit HotPaths
                HotPaths.Load1_52(actKey);
            else
                HotPaths.Load(actKey);

            CloseKey(actKey);
        }

        //--- view templates

        if (OpenKey(salamander, SALAMANDER_VIEWTEMPLATES_REG, actKey))
        {
            ViewTemplates.Load(actKey);
            CloseKey(actKey);
        }

        //---  Plugins Order
        if (OpenKey(salamander, SALAMANDER_PLUGINSORDER, actKey))
        {
            Plugins.LoadOrder(HWindow, actKey);
            CloseKey(actKey);
        }

        //---  Plugins
        if (OpenKey(salamander, SALAMANDER_PLUGINS, actKey)) // jinak default hodnoty
        {
            Plugins.Load(HWindow, actKey);
            CloseKey(actKey);
        }
        else
        {
            if (Configuration.ConfigVersion >= 6)
                Plugins.Clear(); // nechce ani defaultni archivatory ...
        }

        //---  Packers & Unpackers
        if (OpenKey(salamander, SALAMANDER_PACKANDUNPACK, actKey))
        {
            GetValue(actKey, SALAMANDER_SIMPLEICONSINARCHIVES, REG_DWORD, &(Configuration.UseSimpleIconsInArchives), sizeof(DWORD));
            //---  Custom Packers
            HKEY actSubKey;
            if (OpenKey(actKey, SALAMANDER_CUSTOMPACKERS, actSubKey))
            {
                PackerConfig.DeleteAllPackers();
                HKEY itemKey;
                char buf[30];
                int i = 1;
                strcpy(buf, "1");
                while (OpenKey(actSubKey, buf, itemKey))
                {
                    PackerConfig.Load(itemKey);
                    CloseKey(itemKey);
                    itoa(++i, buf, 10);
                }
                GetValue(actSubKey, SALAMANDER_ANOTHERPANEL, REG_DWORD,
                         &(Configuration.UseAnotherPanelForPack), sizeof(DWORD));
                int pp;
                if (GetValue(actSubKey, SALAMANDER_PREFFERED, REG_DWORD, &pp, sizeof(DWORD)))
                {
                    PackerConfig.SetPreferedPacker(pp);
                }
                CloseKey(actSubKey);
                // pridame novinky od minule verze :-)
                PackerConfig.AddDefault(Configuration.ConfigVersion);
            }
            //---  Custom Unpackers
            if (OpenKey(actKey, SALAMANDER_CUSTOMUNPACKERS, actSubKey))
            {
                UnpackerConfig.DeleteAllUnpackers();
                HKEY itemKey;
                char buf[30];
                int i = 1;
                strcpy(buf, "1");
                while (OpenKey(actSubKey, buf, itemKey))
                {
                    UnpackerConfig.Load(itemKey);
                    CloseKey(itemKey);
                    itoa(++i, buf, 10);
                }
                GetValue(actSubKey, SALAMANDER_ANOTHERPANEL, REG_DWORD, &(Configuration.UseAnotherPanelForUnpack), sizeof(DWORD));
                GetValue(actSubKey, SALAMANDER_NAMEBYARCHIVE, REG_DWORD, &(Configuration.UseSubdirNameByArchiveForUnpack), sizeof(DWORD));
                int pp;
                if (GetValue(actSubKey, SALAMANDER_PREFFERED, REG_DWORD, &pp, sizeof(DWORD)))
                {
                    UnpackerConfig.SetPreferedUnpacker(pp);
                }
                CloseKey(actSubKey);
                // pridame novinky od minule verze :-)
                UnpackerConfig.AddDefault(Configuration.ConfigVersion);
            }
            //---  Predefined Packers
            if (OpenKey(actKey, SALAMANDER_PREDPACKERS, actSubKey))
            {
                // j.r.
                // External Archivers Locations: default hodnoty se behem loadu konfigurace nemazou
                // jako doposud, pouze se aktualizuji. Pokud je v registry neuplny nebo neznamy zaznam,
                // suse se ignoruje. Pouze pokud sedi Title s nekterou z default hodnot, pouziji se
                // jeji cesty.
                // ArchiverConfig.DeleteAllArchivers();
                HKEY itemKey;
                char buf[30];
                int i = 1;
                strcpy(buf, "1");
                while (OpenKey(actSubKey, buf, itemKey))
                {
                    ArchiverConfig.Load(itemKey);
                    CloseKey(itemKey);
                    itoa(++i, buf, 10);
                }
                CloseKey(actSubKey);
                // pridame novinky od minule verze :-)
                // ArchiverConfig.AddDefault(Configuration.ConfigVersion); // j.r. nadale netreba volat
            }
            //---  Archive Association
            if (OpenKey(actKey, SALAMANDER_ARCHIVEASSOC, actSubKey))
            {
                PackerFormatConfig.DeleteAllFormats();
                HKEY itemKey;
                char buf[30];
                int i = 1;
                strcpy(buf, "1");
                while (OpenKey(actSubKey, buf, itemKey))
                {
                    PackerFormatConfig.Load(itemKey);
                    CloseKey(itemKey);
                    itoa(++i, buf, 10);
                }
                CloseKey(actSubKey);
                // pridame novinky od minule verze :-)
                PackerFormatConfig.AddDefault(Configuration.ConfigVersion);
                PackerFormatConfig.BuildArray();
            }
            CloseKey(actKey);
        }

        Plugins.CheckData(); // uprava nactenych dat

        //---  user menu

        IfExistSetSplashScreenText(LoadStr(IDS_STARTUP_USERMENU));

        if (OpenKey(salamander, SALAMANDER_USERMENU_REG, actKey))
        {
            HKEY subKey;
            char buf[30];
            strcpy(buf, "1");
            char name[MAX_PATH];
            char command[MAX_PATH];
            char arguments[USRMNUARGS_MAXLEN];
            char initDir[MAX_PATH];
            int throughShell, closeShell, useWindow;
            int showInToolbar, separator;
            CUserMenuItemType type;
            char icon[MAX_PATH];
            int i = 1;
            UserMenuItems->DestroyMembers();

            CUserMenuIconDataArr* bkgndReaderData = new CUserMenuIconDataArr();

            while (OpenKey(actKey, buf, subKey))
            {
                if (
                    GetValue(subKey, USERMENU_ITEMNAME_REG, REG_SZ, name, MAX_PATH)
                    &&
                    GetValue(subKey, USERMENU_COMMAND_REG, REG_SZ, command, MAX_PATH)
                    &&
                    GetValue(subKey, USERMENU_SHELL_REG, REG_DWORD, &throughShell, sizeof(DWORD))
                    &&
                    GetValue(subKey, USERMENU_CLOSE_REG, REG_DWORD, &closeShell, sizeof(DWORD))
                )
                {
                    if (
                        Configuration.ConfigVersion == 1 ||
                        !GetValue(subKey, USERMENU_ARGUMENTS_REG, REG_SZ, arguments, USRMNUARGS_MAXLEN)
                    )
                    {
                        // konvert z user-menu verze 1.52 na stavajici verzi
                        char* s = command;
                        while (*s != 0)
                        {
                            if (*s == '%' && *++s != '%')
                                break;
                            s++;
                        }
                        if (*s == 0)
                            *arguments = 0; // zadne parametry
                        else
                        {
                            s--;
                            while (--s >= command && *s != ' ')
                                ;
                            if (s < command)
                                *arguments = 0; // syntakticka chyba
                            else
                            {
                                *s++ = 0; // zarizneme command, s nastavime na prvni znak argumentu
                                char* st = arguments;
                                char* stEnd = arguments + sizeof(arguments) - 1;
                                while (*s != 0 && st < stEnd)
                                {
                                    if (*s == '%')
                                    {
                                        const TCHAR* add = TEXT( "" );
                                        switch (LowerCase[*++s])
                                        {
                                        case '%':
                                            add = TEXT( "%" );
                                            break;
                                        case 'd':
                                            add = TEXT( "$(Drive)" );
                                            break;
                                        case 'p':
                                            add = TEXT( "$(Path)" );
                                            break;
                                        case 'h':
                                            add = TEXT( "$(DOSPath)" );
                                            break;
                                        case 'f':
                                            add = TEXT( "$(Name)" );
                                            break;
                                        case 's':
                                            add = TEXT( "$(DOSName)" );
                                            break;
                                        }
                                        if (st + strlen(add) > stEnd)
                                            break;
                                        else
                                        {
                                            strcpy(st, add);
                                            st += strlen(add);
                                        }
                                    }
                                    else
                                        *st++ = *s;
                                    s++;
                                }
                                *st = 0;
                            }
                        }
                    }
                    if (Configuration.ConfigVersion == 1 ||
                        !GetValue(subKey, USERMENU_INITDIR_REG, REG_SZ, initDir, MAX_PATH))
                    {
                        strcpy(initDir, "$(Drive)$(Path)");
                    }
                    if (Configuration.ConfigVersion == 1 ||
                        !GetValue(subKey, USERMENU_USEWINDOW_REG, REG_DWORD, &useWindow, sizeof(DWORD)))
                    {
                        useWindow = TRUE;
                    }

                    if (Configuration.ConfigVersion == 1 ||
                        !GetValue(subKey, USERMENU_ICON_REG, REG_SZ, icon, MAX_PATH))
                    {
                        icon[0] = 0;
                    }

                    if (Configuration.ConfigVersion == 1 ||
                        !GetValue(subKey, USERMENU_SEPARATOR_REG, REG_DWORD, &separator, sizeof(DWORD)))
                    {
                        separator = FALSE;
                    }

                    if (!GetValue(subKey, USERMENU_TYPE_REG, REG_DWORD, &type, sizeof(DWORD)))
                    {
                        type = separator ? umitSeparator : umitItem;
                    }

                    if (Configuration.ConfigVersion == 1 ||
                        !GetValue(subKey, USERMENU_SHOWINTOOLBAR_REG, REG_DWORD, &showInToolbar, sizeof(DWORD)))
                    {
                        showInToolbar = TRUE;
                    }

                    CUserMenuItem* item = new CUserMenuItem(name, command, arguments, initDir, icon,
                                                            throughShell, closeShell, useWindow,
                                                            showInToolbar, type, bkgndReaderData);
                    if (item != NULL && item->IsGood())
                    {
                        UserMenuItems->Add(item);
                        if (!UserMenuItems->IsGood())
                        {
                            delete item;
                            UserMenuItems->ResetState();
                            break;
                        }
                    }
                    else
                    {
                        if (item != NULL)
                            delete item;
                        TRACE_E(LOW_MEMORY);
                        break;
                    }
                }
                else
                    break;
                itoa(++i, buf, 10);
                CloseKey(subKey);
            }

            UserMenuIconBkgndReader.StartBkgndReadingIcons(bkgndReaderData); // POZOR: uvolni 'bkgndReaderData'

            CloseKey(actKey);
        }

        IfExistSetSplashScreenText(LoadStr(IDS_STARTUP_CONFIG));

        //---  configuration

        DWORD cmdLine = 0, cmdLineFocus = 0;
        DWORD rightPanelFocused = FALSE;
        if (OpenKey(salamander, SALAMANDER_CONFIG_REG, actKey))
        {
            if (importingOldConfig)
            {
                GetValue(actKey, CONFIG_ONLYONEINSTANCE_REG, REG_DWORD,
                         &Configuration.OnlyOneInstance, sizeof(DWORD));
            }
            //---  top rebar begin
            GetValue(actKey, CONFIG_MENUINDEX_REG, REG_DWORD, &Configuration.MenuIndex, sizeof(DWORD));
            GetValue(actKey, CONFIG_MENUBREAK_REG, REG_DWORD, &Configuration.MenuBreak, sizeof(DWORD));
            GetValue(actKey, CONFIG_MENUWIDTH_REG, REG_DWORD, &Configuration.MenuWidth, sizeof(DWORD));
            GetValue(actKey, CONFIG_TOOLBARINDEX_REG, REG_DWORD, &Configuration.TopToolbarIndex, sizeof(DWORD));
            GetValue(actKey, CONFIG_TOOLBARBREAK_REG, REG_DWORD, &Configuration.TopToolbarBreak, sizeof(DWORD));
            GetValue(actKey, CONFIG_TOOLBARWIDTH_REG, REG_DWORD, &Configuration.TopToolbarWidth, sizeof(DWORD));
            GetValue(actKey, CONFIG_PLUGINSBARINDEX_REG, REG_DWORD, &Configuration.PluginsBarIndex, sizeof(DWORD));
            GetValue(actKey, CONFIG_PLUGINSBARBREAK_REG, REG_DWORD, &Configuration.PluginsBarBreak, sizeof(DWORD));
            GetValue(actKey, CONFIG_PLUGINSBARWIDTH_REG, REG_DWORD, &Configuration.PluginsBarWidth, sizeof(DWORD));
            GetValue(actKey, CONFIG_USERMENUINDEX_REG, REG_DWORD, &Configuration.UserMenuToolbarIndex, sizeof(DWORD));
            GetValue(actKey, CONFIG_USERMENUBREAK_REG, REG_DWORD, &Configuration.UserMenuToolbarBreak, sizeof(DWORD));
            GetValue(actKey, CONFIG_USERMENUWIDTH_REG, REG_DWORD, &Configuration.UserMenuToolbarWidth, sizeof(DWORD));
            GetValue(actKey, CONFIG_USERMENULABELS_REG, REG_DWORD, &Configuration.UserMenuToolbarLabels, sizeof(DWORD));
            GetValue(actKey, CONFIG_HOTPATHSINDEX_REG, REG_DWORD, &Configuration.HotPathsBarIndex, sizeof(DWORD));
            GetValue(actKey, CONFIG_HOTPATHSBREAK_REG, REG_DWORD, &Configuration.HotPathsBarBreak, sizeof(DWORD));
            GetValue(actKey, CONFIG_HOTPATHSWIDTH_REG, REG_DWORD, &Configuration.HotPathsBarWidth, sizeof(DWORD));
            GetValue(actKey, CONFIG_DRIVEBARINDEX_REG, REG_DWORD, &Configuration.DriveBarIndex, sizeof(DWORD));
            GetValue(actKey, CONFIG_DRIVEBARBREAK_REG, REG_DWORD, &Configuration.DriveBarBreak, sizeof(DWORD));
            GetValue(actKey, CONFIG_DRIVEBARWIDTH_REG, REG_DWORD, &Configuration.DriveBarWidth, sizeof(DWORD));
            GetValue(actKey, CONFIG_GRIPSVISIBLE_REG, REG_DWORD, &Configuration.GripsVisible, sizeof(DWORD));
            //---  top rebar end
            GetValue(actKey, CONFIG_FILENAMEFORMAT_REG, REG_DWORD, &Configuration.FileNameFormat, sizeof(DWORD));
            GetValue(actKey, CONFIG_SIZEFORMAT_REG, REG_DWORD, &Configuration.SizeFormat, sizeof(DWORD));
            // automaticka konverze z "mixed-case" na "partially-mixed-case"
            if (Configuration.FileNameFormat == 1)
                Configuration.FileNameFormat = 7;

            GetValue(actKey, CONFIG_SELECTION_REG, REG_DWORD, &Configuration.IncludeDirs, sizeof(DWORD));
            GetValue(actKey, CONFIG_COPYFINDTEXT_REG, REG_DWORD, &Configuration.CopyFindText, sizeof(DWORD));
            GetValue(actKey, CONFIG_CLEARREADONLY_REG, REG_DWORD, &Configuration.ClearReadOnly, sizeof(DWORD));
            GetValue(actKey, CONFIG_PRIMARYCONTEXTMENU_REG, REG_DWORD, &Configuration.PrimaryContextMenu, sizeof(DWORD));
            GetValue(actKey, CONFIG_NOTHIDDENSYSTEM_REG, REG_DWORD, &Configuration.NotHiddenSystemFiles, sizeof(DWORD));
            GetValue(actKey, CONFIG_RECYCLEBIN_REG, REG_DWORD, &Configuration.UseRecycleBin, sizeof(DWORD));
            // prasecinka, poskytneme MasksString, je zde kontrola rozsahu, o nic nejde
            GetValue(actKey, CONFIG_RECYCLEMASKS_REG, REG_SZ, Configuration.RecycleMasks.GetWritableMasksString(), MAX_PATH);
            GetValue(actKey, CONFIG_SAVEONEXIT_REG, REG_DWORD, &Configuration.AutoSave, sizeof(DWORD));
            GetValue(actKey, CONFIG_SHOWGREPERRORS_REG, REG_DWORD, &Configuration.ShowGrepErrors, sizeof(DWORD));
            GetValue(actKey, CONFIG_FINDFULLROW_REG, REG_DWORD, &Configuration.FindFullRowSelect, sizeof(DWORD));
            if (Configuration.ConfigVersion <= 6)
                Configuration.ShowGrepErrors = FALSE; // forcneme FALSE, abychom zbytecne neprudili (ostatni to taky tak delaji)
            GetValue(actKey, CONFIG_MINBEEPWHENDONE_REG, REG_DWORD, &Configuration.MinBeepWhenDone, sizeof(DWORD));
            GetValue(actKey, CONFIG_CLOSESHELL_REG, REG_DWORD, &Configuration.CloseShell, sizeof(DWORD));
            GetValue(actKey, CONFIG_RIGHT_FOCUS_REG, REG_DWORD, &rightPanelFocused, sizeof(DWORD));
            GetValue(actKey, CONFIG_ALWAYSONTOP_REG, REG_DWORD, &Configuration.AlwaysOnTop, sizeof(DWORD));
            //      GetValue(actKey, CONFIG_FASTDIRMOVE_REG, REG_DWORD,
            //               &Configuration.FastDirectoryMove, sizeof(DWORD));
            GetValue(actKey, CONFIG_SORTUSESLOCALE_REG, REG_DWORD, &Configuration.SortUsesLocale, sizeof(DWORD));
            GetValue(actKey, CONFIG_SORTDETECTNUMBERS_REG, REG_DWORD, &Configuration.SortDetectNumbers, sizeof(DWORD));
            GetValue(actKey, CONFIG_SORTNEWERONTOP_REG, REG_DWORD, &Configuration.SortNewerOnTop, sizeof(DWORD));
            GetValue(actKey, CONFIG_SORTDIRSBYNAME_REG, REG_DWORD, &Configuration.SortDirsByName, sizeof(DWORD));
            GetValue(actKey, CONFIG_SORTDIRSBYEXT_REG, REG_DWORD, &Configuration.SortDirsByExt, sizeof(DWORD));
            GetValue(actKey, CONFIG_SAVEHISTORY_REG, REG_DWORD, &Configuration.SaveHistory, sizeof(DWORD));
            GetValue(actKey, CONFIG_SAVEWORKDIRS_REG, REG_DWORD, &Configuration.SaveWorkDirs, sizeof(DWORD));
            GetValue(actKey, CONFIG_ENABLECMDLINEHISTORY_REG, REG_DWORD, &Configuration.EnableCmdLineHistory, sizeof(DWORD));
            GetValue(actKey, CONFIG_SAVECMDLINEHISTORY_REG, REG_DWORD, &Configuration.SaveCmdLineHistory, sizeof(DWORD));
            //      GetValue(actKey, CONFIG_LANTASTICCHECK_REG, REG_DWORD,
            //               &Configuration.LantasticCheck, sizeof(DWORD));
            GetValue(actKey, CONFIG_STATUSAREA_REG, REG_DWORD, &Configuration.StatusArea, sizeof(DWORD));
            if (!GetValue(actKey, CONFIG_FULLROWSELECT_REG, REG_DWORD, &Configuration.FullRowSelect, sizeof(DWORD)))
            {
                // nechceme konverzi - vnutime TRUE
                //        if (GetValue(actKey, CONFIG_EXPLORERLOOK_REG, REG_DWORD,
                //                     &Configuration.FullRowSelect, sizeof(DWORD)))
                //        {
                DeleteValue(actKey, CONFIG_EXPLORERLOOK_REG);
                //          Configuration.FullRowSelect = !Configuration.FullRowSelect;
                //        }
            }
            GetValue(actKey, CONFIG_FULLROWHIGHLIGHT_REG, REG_DWORD, &Configuration.FullRowHighlight, sizeof(DWORD));
            GetValue(actKey, CONFIG_USEICONTINCTURE_REG, REG_DWORD, &Configuration.UseIconTincture, sizeof(DWORD));
            GetValue(actKey, CONFIG_SHOWPANELCAPTION_REG, REG_DWORD, &Configuration.ShowPanelCaption, sizeof(DWORD));
            GetValue(actKey, CONFIG_SHOWPANELZOOM_REG, REG_DWORD, &Configuration.ShowPanelZoom, sizeof(DWORD));
            GetValue(actKey, CONFIG_SINGLECLICK_REG, REG_DWORD, &Configuration.SingleClick, sizeof(DWORD));
            //      GetValue(actKey, CONFIG_SHOWTIPOFTHEDAY_REG, REG_DWORD, &Configuration.ShowTipOfTheDay, sizeof(DWORD));
            //      GetValue(actKey, CONFIG_LASTTIPOFTHEDAY_REG, REG_DWORD, &Configuration.LastTipOfTheDay, sizeof(DWORD));
            GetValue(actKey, CONFIG_INFOLINECONTENT_REG, REG_SZ, Configuration.InfoLineContent, 200);
            GetValue(actKey, CONFIG_IFPATHISINACCESSIBLEGOTO_REG, REG_SZ, Configuration.IfPathIsInaccessibleGoTo, MAX_PATH);
            if (!GetValue(actKey, CONFIG_IFPATHISINACCESSIBLEGOTOISMYDOCS_REG, REG_DWORD, &Configuration.IfPathIsInaccessibleGoToIsMyDocs, sizeof(DWORD)))
            {
                char path[MAX_PATH];
                GetIfPathIsInaccessibleGoTo(path, TRUE);
                if (IsTheSamePath(path, Configuration.IfPathIsInaccessibleGoTo)) // user chce chodit do my-documents
                {
                    Configuration.IfPathIsInaccessibleGoToIsMyDocs = TRUE;
                    Configuration.IfPathIsInaccessibleGoTo[0] = 0;
                }
                else
                    Configuration.IfPathIsInaccessibleGoToIsMyDocs = FALSE;
            }
            GetValue(actKey, CONFIG_HOTPATH_AUTOCONFIG, REG_DWORD, &Configuration.HotPathAutoConfig, sizeof(DWORD));
            GetValue(actKey, CONFIG_LASTUSEDSPEEDLIM_REG, REG_DWORD, &Configuration.LastUsedSpeedLimit, sizeof(DWORD));
            GetValue(actKey, CONFIG_QUICKSEARCHENTER_REG, REG_DWORD, &Configuration.QuickSearchEnterAlt, sizeof(DWORD));
            GetValue(actKey, CONFIG_CHD_SHOWMYDOC, REG_DWORD, &Configuration.ChangeDriveShowMyDoc, sizeof(DWORD));
            GetValue(actKey, CONFIG_CHD_SHOWCLOUDSTOR, REG_DWORD, &Configuration.ChangeDriveCloudStorage, sizeof(DWORD));
            GetValue(actKey, CONFIG_CHD_SHOWANOTHER, REG_DWORD, &Configuration.ChangeDriveShowAnother, sizeof(DWORD));
            GetValue(actKey, CONFIG_CHD_SHOWNET, REG_DWORD, &Configuration.ChangeDriveShowNet, sizeof(DWORD));
            GetValue(actKey, CONFIG_SEARCHFILECONTENT, REG_DWORD, &Configuration.SearchFileContent, sizeof(DWORD));
            GetValue(actKey, CONFIG_LASTPLUGINVER, REG_DWORD, &Configuration.LastPluginVer, sizeof(DWORD));
            GetValue(actKey, CONFIG_LASTPLUGINVER_OP, REG_DWORD, &Configuration.LastPluginVerOP, sizeof(DWORD));
            GetValue(actKey, CONFIG_QUICKRENAME_SELALL_REG, REG_DWORD, &Configuration.QuickRenameSelectAll, sizeof(DWORD));
            GetValue(actKey, CONFIG_EDITNEW_SELALL_REG, REG_DWORD, &Configuration.EditNewSelectAll, sizeof(DWORD));
            if (!GetValue(actKey, CONFIG_USESALOPEN_REG, REG_DWORD, &Configuration.UseSalOpen, sizeof(DWORD)))
            {
                Configuration.UseSalOpen = FALSE; // default je nepouzivat
            }
            else
            {
                if (Configuration.ConfigVersion == 11) // v 1.6 beta 7 bylo zapnute ... vypneme
                {
                    Configuration.UseSalOpen = FALSE; // default je nepouzivat
                }
            }
            GetValue(actKey, CONFIG_NETWAREFASTDIRMOVE_REG, REG_DWORD, &Configuration.NetwareFastDirMove, sizeof(DWORD));
            if (Windows7AndLater)
                GetValue(actKey, CONFIG_ASYNCCOPYALG_REG, REG_DWORD, &Configuration.UseAsyncCopyAlg, sizeof(DWORD));
            GetValue(actKey, CONFIG_RELOAD_ENV_VARS_REG, REG_DWORD, &Configuration.ReloadEnvVariables, sizeof(DWORD));
            GetValue(actKey, CONFIG_SHIFTFORHOTPATHS_REG, REG_DWORD, &Configuration.ShiftForHotPaths, sizeof(DWORD));
            //      GetValue(actKey, CONFIG_LANGUAGE_REG, REG_SZ, Configuration.SLGName, MAX_PATH);
            //      GetValue(actKey, CONFIG_USEALTLANGFORPLUGINS_REG, REG_DWORD, &Configuration.UseAsAltSLGInOtherPlugins, sizeof(DWORD));
            //      GetValue(actKey, CONFIG_ALTLANGFORPLUGINS_REG, REG_SZ, Configuration.AltPluginSLGName, MAX_PATH);
            GetValue(actKey, CONFIG_CONVERSIONTABLE_REG, REG_SZ, &Configuration.ConversionTable, MAX_PATH);
            GetValue(actKey, CONFIG_SKILLLEVEL_REG, REG_DWORD, &Configuration.SkillLevel, sizeof(DWORD));
            GetValue(actKey, CONFIG_TITLEBARSHOWPATH_REG, REG_DWORD, &Configuration.TitleBarShowPath, sizeof(DWORD));
            GetValue(actKey, CONFIG_TITLEBARMODE_REG, REG_DWORD, &Configuration.TitleBarMode, sizeof(DWORD));
            GetValue(actKey, CONFIG_TITLEBARPREFIX_REG, REG_DWORD, &Configuration.UseTitleBarPrefix, sizeof(DWORD));
            GetValue(actKey, CONFIG_TITLEBARPREFIXTEXT_REG, REG_SZ, &Configuration.TitleBarPrefix, TITLE_PREFIX_MAX);
            GetValue(actKey, CONFIG_MAINWINDOWICONINDEX_REG, REG_DWORD, &Configuration.MainWindowIconIndex, sizeof(DWORD));
            if (Configuration.MainWindowIconIndex < 0 || Configuration.MainWindowIconIndex > MAINWINDOWICONS_COUNT)
                Configuration.MainWindowIconIndex = 0;
            GetValue(actKey, CONFIG_CLICKQUICKRENAME_REG, REG_DWORD, &Configuration.ClickQuickRename, sizeof(DWORD));
            GetValue(actKey, CONFIG_VISIBLEDRIVES_REG, REG_DWORD, &Configuration.VisibleDrives, sizeof(DWORD));
            GetValue(actKey, CONFIG_SEPARATEDDRIVES_REG, REG_DWORD, &Configuration.SeparatedDrives, sizeof(DWORD));
            GetValue(actKey, CONFIG_COMPAREBYTIME_REG, REG_DWORD, &Configuration.CompareByTime, sizeof(DWORD));
            if (!GetValue(actKey, CONFIG_COMPAREBYSIZE_REG, REG_DWORD, &Configuration.CompareBySize, sizeof(DWORD)))
            { // konverze ze starsi konfigurace - BySize bylo soucasti ByTime, takze proto nastaveni zkopirujeme
                Configuration.CompareBySize = Configuration.CompareByTime;
            }
            GetValue(actKey, CONFIG_COMPAREBYCONTENT_REG, REG_DWORD, &Configuration.CompareByContent, sizeof(DWORD));
            GetValue(actKey, CONFIG_COMPAREBYATTR_REG, REG_DWORD, &Configuration.CompareByAttr, sizeof(DWORD));
            GetValue(actKey, CONFIG_COMPAREBYSUBDIRS_REG, REG_DWORD, &Configuration.CompareSubdirs, sizeof(DWORD));
            GetValue(actKey, CONFIG_COMPAREBYSUBDIRSATTR_REG, REG_DWORD, &Configuration.CompareSubdirsAttr, sizeof(DWORD));

            GetValue(actKey, CONFIG_COMPAREONEPANELDIRS_REG, REG_DWORD, &Configuration.CompareOnePanelDirs, sizeof(DWORD));
            GetValue(actKey, CONFIG_COMPAREMOREOPTIONS_REG, REG_DWORD, &Configuration.CompareMoreOptions, sizeof(DWORD));
            GetValue(actKey, CONFIG_COMPAREIGNOREFILES_REG, REG_DWORD, &Configuration.CompareIgnoreFiles, sizeof(DWORD));
            GetValue(actKey, CONFIG_COMPAREIGNOREDIRS_REG, REG_DWORD, &Configuration.CompareIgnoreDirs, sizeof(DWORD));
            // prasecinka, poskytneme MasksString, je zde kontrola rozsahu, o nic nejde
            GetValue(actKey, CONFIG_CONFIGTIGNOREFILESMASKS_REG, REG_SZ, Configuration.CompareIgnoreFilesMasks.GetWritableMasksString(), MAX_PATH);
            GetValue(actKey, CONFIG_CONFIGTIGNOREDIRSMASKS_REG, REG_SZ, Configuration.CompareIgnoreDirsMasks.GetWritableMasksString(), MAX_PATH);
            int errPos;
            Configuration.CompareIgnoreFilesMasks.PrepareMasks(errPos);
            Configuration.CompareIgnoreDirsMasks.PrepareMasks(errPos);

            GetValue(actKey, CONFIG_THUMBNAILSIZE_REG, REG_DWORD, &Configuration.ThumbnailSize, sizeof(DWORD));
            LeftPanel->SetThumbnailSize(Configuration.ThumbnailSize);
            RightPanel->SetThumbnailSize(Configuration.ThumbnailSize);

            GetValue(actKey, CONFIG_KEEPPLUGINSSORTED_REG, REG_DWORD, &Configuration.KeepPluginsSorted, sizeof(DWORD));

            Configuration.ShowSLGIncomplete = TRUE;
            if (Configuration.ConfigVersion == THIS_CONFIG_VERSION)
            {
                GetValue(actKey, CONFIG_SHOWSLGINCOMPLETE_REG, REG_DWORD, &Configuration.ShowSLGIncomplete, sizeof(DWORD));
            }

            GetValue(actKey, CONFIG_EDITNEWFILE_USEDEFAULT_REG, REG_DWORD, &Configuration.UseEditNewFileDefault, sizeof(DWORD));
            GetValue(actKey, CONFIG_EDITNEWFILE_DEFAULT_REG, REG_SZ, Configuration.EditNewFileDefault, MAX_PATH);

#ifndef _WIN64 // FIXME_X64_WINSCP
            if (!GetValue(actKey, "Add x86-Only Plugins", REG_DWORD,
                          &Configuration.AddX86OnlyPlugins, sizeof(DWORD)))
            {
                Configuration.AddX86OnlyPlugins = TRUE;
            }
#endif // _WIN64

            HKEY actSubKey;
            if (OpenKey(actKey, SALAMANDER_CONFIRMATION_REG, actSubKey))
            {
                GetValue(actSubKey, CONFIG_CNFRM_FILEDIRDEL, REG_DWORD, &Configuration.CnfrmFileDirDel, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_NEDIRDEL, REG_DWORD, &Configuration.CnfrmNEDirDel, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_FILEOVER, REG_DWORD, &Configuration.CnfrmFileOver, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_DIROVER, REG_DWORD, &Configuration.CnfrmDirOver, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_SHFILEDEL, REG_DWORD, &Configuration.CnfrmSHFileDel, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_SHDIRDEL, REG_DWORD, &Configuration.CnfrmSHDirDel, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_SHFILEOVER, REG_DWORD, &Configuration.CnfrmSHFileOver, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_NTFSPRESS, REG_DWORD, &Configuration.CnfrmNTFSPress, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_NTFSCRYPT, REG_DWORD, &Configuration.CnfrmNTFSCrypt, sizeof(DWORD));
                if (Configuration.ConfigVersion != 1)
                    GetValue(actSubKey, CONFIG_CNFRM_DAD, REG_DWORD, &Configuration.CnfrmDragDrop, sizeof(DWORD));
                else // je-li to stary config, nacitame to o patro vejs
                    GetValue(actKey, "Confirm Drop Operations", REG_DWORD, &Configuration.CnfrmDragDrop, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_CLOSEARCHIVE, REG_DWORD, &Configuration.CnfrmCloseArchive, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_CLOSEFIND, REG_DWORD, &Configuration.CnfrmCloseFind, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_STOPFIND, REG_DWORD, &Configuration.CnfrmStopFind, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_CREATETARGETPATH, REG_DWORD, &Configuration.CnfrmCreatePath, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_ALWAYSONTOP, REG_DWORD, &Configuration.CnfrmAlwaysOnTop, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_ONSALCLOSE, REG_DWORD, &Configuration.CnfrmOnSalClose, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_SENDEMAIL, REG_DWORD, &Configuration.CnfrmSendEmail, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_ADDTOARCHIVE, REG_DWORD, &Configuration.CnfrmAddToArchive, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_CREATEDIR, REG_DWORD, &Configuration.CnfrmCreateDir, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_CHANGEDIRTC, REG_DWORD, &Configuration.CnfrmChangeDirTC, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_SHOWNAMETOCOMP, REG_DWORD, &Configuration.CnfrmShowNamesToCompare, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_DSTSHIFTSIGNORED, REG_DWORD, &Configuration.CnfrmDSTShiftsIgnored, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_DSTSHIFTSOCCURED, REG_DWORD, &Configuration.CnfrmDSTShiftsOccured, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_CNFRM_COPYMOVEOPTIONSNS, REG_DWORD, &Configuration.CnfrmCopyMoveOptionsNS, sizeof(DWORD));

                CloseKey(actSubKey);
            }

            if (OpenKey(actKey, SALAMANDER_DRVSPEC_REG, actSubKey))
            {
                GetValue(actSubKey, CONFIG_DRVSPEC_FLOPPY_MON, REG_DWORD, &Configuration.DrvSpecFloppyMon, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_DRVSPEC_FLOPPY_SIMPLE, REG_DWORD, &Configuration.DrvSpecFloppySimple, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_DRVSPEC_REMOVABLE_MON, REG_DWORD, &Configuration.DrvSpecRemovableMon, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_DRVSPEC_REMOVABLE_SIMPLE, REG_DWORD, &Configuration.DrvSpecRemovableSimple, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_DRVSPEC_FIXED_MON, REG_DWORD, &Configuration.DrvSpecFixedMon, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_DRVSPEC_FIXED_SIMPLE, REG_DWORD, &Configuration.DrvSpecFixedSimple, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_DRVSPEC_REMOTE_MON, REG_DWORD, &Configuration.DrvSpecRemoteMon, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_DRVSPEC_REMOTE_SIMPLE, REG_DWORD, &Configuration.DrvSpecRemoteSimple, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_DRVSPEC_REMOTE_ACT, REG_DWORD, &Configuration.DrvSpecRemoteDoNotRefreshOnAct, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_DRVSPEC_CDROM_MON, REG_DWORD, &Configuration.DrvSpecCDROMMon, sizeof(DWORD));
                GetValue(actSubKey, CONFIG_DRVSPEC_CDROM_SIMPLE, REG_DWORD, &Configuration.DrvSpecCDROMSimple, sizeof(DWORD));

                // pro stare verze forcneme cteni ikon na removable, protoze jsme zavedli floppy kategorii
                if (Configuration.ConfigVersion < 31)
                    Configuration.DrvSpecRemovableSimple = FALSE;

                CloseKey(actSubKey);
            }

            if (Configuration.ConfigVersion >= 8) // pro stare verze forcneme novou toolbaru
                GetValue(actKey, CONFIG_TOPTOOLBAR_REG, REG_SZ, Configuration.TopToolBar, 400);
            GetValue(actKey, CONFIG_MIDDLETOOLBAR_REG, REG_SZ, Configuration.MiddleToolBar, 400);
            GetValue(actKey, CONFIG_LEFTTOOLBAR_REG, REG_SZ, Configuration.LeftToolBar, 100);
            GetValue(actKey, CONFIG_RIGHTTOOLBAR_REG, REG_SZ, Configuration.RightToolBar, 100);
            // change drive button byl pouze jeden - ted zavadim dve tlacitka
            // a slucuji vsechny bitmapy do jedne

            if (Configuration.ConfigVersion <= 3 && Configuration.RightToolBar[0] != 0)
            {
                char tmp[5000];
                lstrcpyn(tmp, Configuration.RightToolBar, 5000);
                char num[50];

                Configuration.RightToolBar[0] = 0;

                BOOL first = TRUE;
                char* p = strtok(tmp, ",");
                while (p != NULL)
                {
                    int i = atoi(p);

                    // stary tbbeChangeDrive zamenim za tbbeChangeDriveR
                    //#define TBBE_CHANGE_DRIVE_R     51
                    if (i == 36)
                        i = 51;

                    if (!first)
                        strcat(Configuration.RightToolBar, ",");
                    sprintf(num, "%d", i);
                    strcat(Configuration.RightToolBar, num);
                    first = FALSE;
                    p = strtok(NULL, ",");
                }
            }

            if (TopToolBar != NULL)
                TopToolBar->Load(Configuration.TopToolBar);
            if (MiddleToolBar != NULL)
                MiddleToolBar->Load(Configuration.MiddleToolBar);
            if (LeftPanel->DirectoryLine->ToolBar != NULL)
                LeftPanel->DirectoryLine->ToolBar->Load(Configuration.LeftToolBar);
            if (RightPanel->DirectoryLine->ToolBar != NULL)
                RightPanel->DirectoryLine->ToolBar->Load(Configuration.RightToolBar);

            GetValue(actKey, CONFIG_TOPTOOLBARVISIBLE_REG, REG_DWORD, &Configuration.TopToolBarVisible, sizeof(DWORD));
            GetValue(actKey, CONFIG_PLGTOOLBARVISIBLE_REG, REG_DWORD, &Configuration.PluginsBarVisible, sizeof(DWORD));
            GetValue(actKey, CONFIG_MIDDLETOOLBARVISIBLE_REG, REG_DWORD, &Configuration.MiddleToolBarVisible, sizeof(DWORD));

            GetValue(actKey, CONFIG_USERMENUTOOLBARVISIBLE_REG, REG_DWORD, &Configuration.UserMenuToolBarVisible, sizeof(DWORD));
            GetValue(actKey, CONFIG_HOTPATHSBARVISIBLE_REG, REG_DWORD, &Configuration.HotPathsBarVisible, sizeof(DWORD));

            // pokud jde o starou verzi konfigurace a uzivatel ma naplnene user menu,
            // zobrazim UserMenuBar
            if (Configuration.ConfigVersion <= 3 && UserMenuItems->Count > 0)
                Configuration.UserMenuToolBarVisible = TRUE;

            GetValue(actKey, CONFIG_DRIVEBARVISIBLE_REG, REG_DWORD, &Configuration.DriveBarVisible, sizeof(DWORD));
            GetValue(actKey, CONFIG_DRIVEBAR2VISIBLE_REG, REG_DWORD, &Configuration.DriveBar2Visible, sizeof(DWORD));

            if (ret) // pokud vracime FALSE, vse bude vlozeno pozdeji
            {
                // bandy musime vlozit ve spravnem poradi dle jejich indexu
                BOOL menuInserted = FALSE; // menu je dulezite, musime jej vlozit za kazdou cenu
                // zakazujeme ukladani pozic behem nahazovani bandu, protoze by doslo k
                // prepsani jejich poradi
                int idx;
                for (idx = 0; idx < 10; idx++) // muzeme s klidem zkusit vic indexu, nez je bandu
                {
                    if (idx == Configuration.MenuIndex)
                    {
                        InsertMenuBand();
                        menuInserted = TRUE;
                    }
                    if (idx == Configuration.TopToolbarIndex && Configuration.TopToolBarVisible)
                        ToggleTopToolBar(FALSE);
                    if (idx == Configuration.PluginsBarIndex && Configuration.PluginsBarVisible)
                        TogglePluginsBar(FALSE);
                    if (idx == Configuration.UserMenuToolbarIndex && Configuration.UserMenuToolBarVisible)
                        ToggleUserMenuToolBar(FALSE);
                    if (idx == Configuration.HotPathsBarIndex && Configuration.HotPathsBarVisible)
                        ToggleHotPathsBar(FALSE);
                    if (idx == Configuration.DriveBarIndex && Configuration.DriveBarVisible)
                        ToggleDriveBar(Configuration.DriveBar2Visible, FALSE);
                }
                if (!menuInserted)
                {
                    TRACE_E("Inserting MenuBar. Configuration seems to be corrupted.");
                    Configuration.MenuIndex = 0;
                    InsertMenuBand();
                }
                if (Configuration.MiddleToolBarVisible)
                    ToggleMiddleToolBar();
                CreateAndInsertWorkerBand(); // na zaver vlozime workera
            }

            GetValue(actKey, CONFIG_BOTTOMTOOLBARVISIBLE_REG, REG_DWORD,
                     &Configuration.BottomToolBarVisible, sizeof(DWORD));
            if (Configuration.BottomToolBarVisible)
                ToggleBottomToolBar();

            //      GetValue(actKey, CONFIG_SPACESELCALCSPACE, REG_DWORD, &Configuration.SpaceSelCalcSpace, sizeof(DWORD));
            GetValue(actKey, CONFIG_USETIMERESOLUTION, REG_DWORD, &Configuration.UseTimeResolution, sizeof(DWORD));
            GetValue(actKey, CONFIG_TIMERESOLUTION, REG_DWORD, &Configuration.TimeResolution, sizeof(DWORD));
            GetValue(actKey, CONFIG_IGNOREDSTSHIFTS, REG_DWORD, &Configuration.IgnoreDSTShifts, sizeof(DWORD));
            GetValue(actKey, CONFIG_USEDRAGDROPMINTIME, REG_DWORD, &Configuration.UseDragDropMinTime, sizeof(DWORD));
            GetValue(actKey, CONFIG_DRAGDROPMINTIME, REG_DWORD, &Configuration.DragDropMinTime, sizeof(DWORD));

            GetValue(actKey, CONFIG_LASTFOCUSEDPAGE, REG_DWORD, &Configuration.LastFocusedPage, sizeof(DWORD));
            GetValue(actKey, CONFIG_CONFIGURATION_HEIGHT, REG_DWORD, &Configuration.ConfigurationHeight, sizeof(DWORD));
            GetValue(actKey, CONFIG_VIEWANDEDITEXPAND, REG_DWORD, &Configuration.ViewersAndEditorsExpanded, sizeof(DWORD));
            GetValue(actKey, CONFIG_PACKEPAND, REG_DWORD, &Configuration.PackersAndUnpackersExpanded, sizeof(DWORD));

            GetValue(actKey, CONFIG_CMDLINE_REG, REG_DWORD, &cmdLine, sizeof(DWORD));
            GetValue(actKey, CONFIG_CMDLFOCUS_REG, REG_DWORD, &cmdLineFocus, sizeof(DWORD));

            GetValue(actKey, CONFIG_USECUSTOMPANELFONT_REG, REG_DWORD, &UseCustomPanelFont, sizeof(DWORD));
            if (LoadLogFont(actKey, CONFIG_PANELFONT_REG, &LogFont) && UseCustomPanelFont)
            {
                // pokud uzivatel pouziva vlastni font, musime ho nyni napropagovat
                SetFont();
            }

            LoadHistory(actKey, CONFIG_NAMEDHISTORY_REG, FindNamedHistory, FIND_NAMED_HISTORY_SIZE);
            LoadHistory(actKey, CONFIG_LOOKINHISTORY_REG, FindLookInHistory, FIND_LOOKIN_HISTORY_SIZE);
            LoadHistory(actKey, CONFIG_GREPHISTORY_REG, FindGrepHistory, FIND_GREP_HISTORY_SIZE);
            LoadHistory(actKey, CONFIG_SELECTHISTORY_REG, Configuration.SelectHistory, SELECT_HISTORY_SIZE);
            //      Klukum (Honza Patera, Tomas Jelinek) se toto nelibilo, protoze si nahodi novou
            //      instanci a uz si nepamatuji co bylo minule. Daji (Un)Select a tam na ne ceka
            //      maska z posledne. FAR, VC, NC pri spusteni nove instance maji *.*. Budeme se chovat take tak.
            //      if (Configuration.SelectHistory[0] != NULL)  // at se nacita i pocatecni stav num +/- oznacovani
            //        strcpy(SelectionMask, Configuration.SelectHistory[0]);
            LoadHistory(actKey, CONFIG_COPYHISTORY_REG, Configuration.CopyHistory, COPY_HISTORY_SIZE);
            LoadHistory(actKey, CONFIG_CHANGEDIRHISTORY_REG, Configuration.ChangeDirHistory, CHANGEDIR_HISTORY_SIZE);
            LoadHistory(actKey, CONFIG_VIEWERHISTORY_REG, ViewerHistory, VIEWER_HISTORY_SIZE);
            LoadHistory(actKey, CONFIG_COMMANDHISTORY_REG, Configuration.EditHistory, EDIT_HISTORY_SIZE);
            LoadHistory(actKey, CONFIG_FILELISTHISTORY_REG, Configuration.FileListHistory, FILELIST_HISTORY_SIZE);
            LoadHistory(actKey, CONFIG_CREATEDIRHISTORY_REG, Configuration.CreateDirHistory, CREATEDIR_HISTORY_SIZE);
            LoadHistory(actKey, CONFIG_QUICKRENAMEHISTORY_REG, Configuration.QuickRenameHistory, QUICKRENAME_HISTORY_SIZE);
            LoadHistory(actKey, CONFIG_EDITNEWHISTORY_REG, Configuration.EditNewHistory, EDITNEW_HISTORY_SIZE);
            LoadHistory(actKey, CONFIG_CONVERTHISTORY_REG, Configuration.ConvertHistory, CONVERT_HISTORY_SIZE);
            LoadHistory(actKey, CONFIG_FILTERHISTORY_REG, Configuration.FilterHistory, FILTER_HISTORY_SIZE);
            if (DirHistory != NULL)
            {
                DirHistory->LoadFromRegistry(actKey, CONFIG_WORKDIRSHISTORY_REG);
                if (LeftPanel != NULL)
                    LeftPanel->DirectoryLine->SetHistory(DirHistory->HasPaths());
                if (RightPanel != NULL)
                    RightPanel->DirectoryLine->SetHistory(DirHistory->HasPaths());
            }

            if (OpenKey(actKey, CONFIG_COPYMOVEOPTIONS_REG, actSubKey))
            {
                CopyMoveOptions.Load(actSubKey);
                CloseKey(actSubKey);
            }

            if (OpenKey(actKey, CONFIG_FINDOPTIONS_REG, actSubKey))
            {
                FindOptions.Registry_Load(actSubKey, Configuration.ConfigVersion);
                CloseKey(actSubKey);
            }

            if (OpenKey(actKey, CONFIG_FINDIGNORE_REG, actSubKey))
            {
                FindIgnore.Load(actSubKey, Configuration.ConfigVersion);
                CloseKey(actSubKey);
            }

            GetValue(actKey, CONFIG_FILELISTNAME_REG, REG_SZ, Configuration.FileListName, MAX_PATH);
            GetValue(actKey, CONFIG_FILELISTAPPEND_REG, REG_DWORD, &Configuration.FileListAppend, sizeof(DWORD));
            GetValue(actKey, CONFIG_FILELISTDESTINATION_REG, REG_DWORD, &Configuration.FileListDestination, sizeof(DWORD));

            CloseKey(actKey);
        }

        //---  viewer

        if (OpenKey(salamander, SALAMANDER_VIEWER_REG, actKey))
        {
            GetValue(actKey, VIEWER_FINDFORWARD_REG, REG_DWORD, &GlobalFindDialog.Forward, sizeof(DWORD));
            GetValue(actKey, VIEWER_FINDWHOLEWORDS_REG, REG_DWORD, &GlobalFindDialog.WholeWords, sizeof(DWORD));
            GetValue(actKey, VIEWER_FINDCASESENSITIVE_REG, REG_DWORD, &GlobalFindDialog.CaseSensitive, sizeof(DWORD));
            GetValue(actKey, VIEWER_FINDREGEXP_REG, REG_DWORD, &GlobalFindDialog.Regular, sizeof(DWORD));
            GetValue(actKey, VIEWER_FINDTEXT_REG, REG_SZ, GlobalFindDialog.Text, FIND_TEXT_LEN);
            GetValue(actKey, VIEWER_FINDHEXMODE_REG, REG_DWORD, &GlobalFindDialog.HexMode, sizeof(DWORD));

            GetValue(actKey, VIEWER_CONFIGCRLF_REG, REG_DWORD, &Configuration.EOL_CRLF, sizeof(DWORD));
            GetValue(actKey, VIEWER_CONFIGCR_REG, REG_DWORD, &Configuration.EOL_CR, sizeof(DWORD));
            GetValue(actKey, VIEWER_CONFIGLF_REG, REG_DWORD, &Configuration.EOL_LF, sizeof(DWORD));
            GetValue(actKey, VIEWER_CONFIGNULL_REG, REG_DWORD, &Configuration.EOL_NULL, sizeof(DWORD));
            GetValue(actKey, VIEWER_CONFIGTABSIZE_REG, REG_DWORD, &Configuration.TabSize, sizeof(DWORD));
            GetValue(actKey, VIEWER_CONFIGDEFMODE_REG, REG_DWORD, &Configuration.DefViewMode, sizeof(DWORD));
            // prasecinka, poskytneme MasksString, je zde kontrola rozsahu, o nic nejde
            GetValue(actKey, VIEWER_CONFIGTEXTMASK_REG, REG_SZ, Configuration.TextModeMasks.GetWritableMasksString(), MAX_PATH);
            if (Configuration.ConfigVersion < 17 &&
                strcmp(Configuration.TextModeMasks.GetWritableMasksString(), "*.txt;*.602") == 0)
            {
                strcpy(Configuration.TextModeMasks.GetWritableMasksString(), "*.txt;*.602;*.xml");
            }
            int errPos;
            Configuration.TextModeMasks.PrepareMasks(errPos);
            // prasecinka, poskytneme MasksString, je zde kontrola rozsahu, o nic nejde
            GetValue(actKey, VIEWER_CONFIGHEXMASK_REG, REG_SZ, Configuration.HexModeMasks.GetWritableMasksString(), MAX_PATH);
            Configuration.HexModeMasks.PrepareMasks(errPos);

            GetValue(actKey, VIEWER_CONFIGUSECUSTOMFONT_REG, REG_DWORD, &UseCustomViewerFont, sizeof(DWORD));
            LoadLogFont(actKey, VIEWER_CONFIGFONT_REG, &ViewerLogFont); // jeste nemuze byt otevreny zadny viewer, neni nutne volat SetViewerFont()
            GetValue(actKey, VIEWER_WRAPTEXT_REG, REG_DWORD, &Configuration.WrapText, sizeof(DWORD));
            GetValue(actKey, VIEWER_CPAUTOSELECT_REG, REG_DWORD, &Configuration.CodePageAutoSelect, sizeof(DWORD));
            GetValue(actKey, VIEWER_DEFAULTCONVERT_REG, REG_SZ, Configuration.DefaultConvert, 200);
            GetValue(actKey, VIEWER_AUTOCOPYSELECTION_REG, REG_DWORD, &Configuration.AutoCopySelection, sizeof(DWORD));
            GetValue(actKey, VIEWER_GOTOOFFSETISHEX_REG, REG_DWORD, &Configuration.GoToOffsetIsHex, sizeof(DWORD));

            GetValue(actKey, VIEWER_CONFIGSAVEWINPOS_REG, REG_DWORD, &Configuration.SavePosition, sizeof(DWORD));
            BOOL plcmntExist = TRUE;
            plcmntExist &= GetValue(actKey, VIEWER_CONFIGWNDLEFT_REG, REG_DWORD, &Configuration.WindowPlacement.rcNormalPosition.left, sizeof(DWORD));
            plcmntExist &= GetValue(actKey, VIEWER_CONFIGWNDRIGHT_REG, REG_DWORD, &Configuration.WindowPlacement.rcNormalPosition.right, sizeof(DWORD));
            plcmntExist &= GetValue(actKey, VIEWER_CONFIGWNDTOP_REG, REG_DWORD, &Configuration.WindowPlacement.rcNormalPosition.top, sizeof(DWORD));
            plcmntExist &= GetValue(actKey, VIEWER_CONFIGWNDBOTTOM_REG, REG_DWORD, &Configuration.WindowPlacement.rcNormalPosition.bottom, sizeof(DWORD));
            plcmntExist &= GetValue(actKey, VIEWER_CONFIGWNDSHOW_REG, REG_DWORD, &Configuration.WindowPlacement.showCmd, sizeof(DWORD));
            if (plcmntExist)
                Configuration.WindowPlacement.length = sizeof(Configuration.WindowPlacement);

            CloseKey(actKey);
        }

        //---  left and right panel

        char leftPanelPath[MAX_PATH];
        char rightPanelPath[MAX_PATH];
        GetSystemDirectory(leftPanelPath, MAX_PATH);
        strcpy(rightPanelPath, leftPanelPath);
        char sysDefDir[MAX_PATH];
        lstrcpyn(sysDefDir, DefaultDir[LowerCase[leftPanelPath[0]] - 'a'], MAX_PATH);
        LoadPanelConfig(leftPanelPath, LeftPanel, salamander, SALAMANDER_LEFTP_REG);
        LoadPanelConfig(rightPanelPath, RightPanel, salamander, SALAMANDER_RIGHTP_REG);

        CloseKey(salamander);
        salamander = NULL;

        LoadSaveToRegistryMutex.Leave();

        //---  END OF LOADING CONFIGURATION

        if (cmdLine && !SystemPolicies.GetNoRun())
            PostMessage(HWindow, WM_COMMAND, CM_TOGGLEEDITLINE, TRUE);

        MSG msg; // nechame zpracovat rozeslane zpravy
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // provedeme nastaveni aktivniho panelu podle parametru z prikazove radky
        if (ret && cmdLineParams != NULL)
        {
            if (cmdLineParams->ActivatePanel == 1 && rightPanelFocused ||
                cmdLineParams->ActivatePanel == 2 && !rightPanelFocused)
            {
                rightPanelFocused = !rightPanelFocused;
            }
        }

        FocusPanel(rightPanelFocused ? RightPanel : LeftPanel);
        (rightPanelFocused ? RightPanel : LeftPanel)->SetCaretIndex(0, FALSE);
        if (cmdLineFocus)
            SendMessage(HWindow, WM_COMMAND, CM_EDITLINE, 0);

        // tady to nedelalo dobrotu
        // pokud byl panel nasmerovan na UNC cestu, ktera nebyla dostupna,
        // zustalo to tu cekat nekolik vterin
        //    LeftPanel->UpdateDriveIcon(TRUE);
        //    RightPanel->UpdateDriveIcon(TRUE);
        //    RefreshMenuAndTB(TRUE);

        HMENU h = GetSystemMenu(HWindow, FALSE);
        if (h != NULL)
        {
            CheckMenuItem(h, CM_ALWAYSONTOP, MF_BYCOMMAND | (Configuration.AlwaysOnTop ? MF_CHECKED : MF_UNCHECKED));

            char buff[200];
            MENUITEMINFO mii;
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_TYPE;
            mii.dwTypeData = buff;
            mii.cch = 199;

            GetMenuItemInfo(h, SC_MINIMIZE, FALSE, &mii);
            wsprintf(buff + strlen(buff), "\t%s+%s", LoadStr(IDS_SHIFT), LoadStr(IDS_ESCAPE));
            SetMenuItemInfo(h, SC_MINIMIZE, FALSE, &mii);

            mii.cch = 199;
            GetMenuItemInfo(h, SC_MAXIMIZE, FALSE, &mii);
            wsprintf(buff + strlen(buff), "\t%s+%s+%s", LoadStr(IDS_CTRL), LoadStr(IDS_SHIFT), LoadStr(IDS_F11));
            SetMenuItemInfo(h, SC_MAXIMIZE, FALSE, &mii);
        }

        SplashScreenCloseIfExist();
        if (Configuration.StatusArea)
            AddTrayIcon();

        SetWindowIcon();
        SetWindowTitle();

        SetWindowPos(HWindow,
                     Configuration.AlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                     0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        // ukazeme okno v plne parade
        if (useWinPlacement)
        {
            // z MSDN:
            // ShowCmd: 0 = SW_SHOWNORMAL
            //          3 = SW_SHOWMAXIMIZED
            //          7 = SW_SHOWMINNOACTIVE

            // pri startu nechceme minimalizovanou aplikaci - pouze pokud to user definoval
            // na urovni shortcuty
            if (!Configuration.StatusArea)
            {
                switch (CmdShow)
                {
                case SW_SHOWNORMAL:
                {
                    // pokud je v konfiguraci minimalizovane okno, otevreme ho restored
                    if (place.showCmd == SW_MINIMIZE)
                        place.showCmd = SW_RESTORE;
                    if (place.showCmd == SW_SHOWMINIMIZED)
                        place.showCmd = SW_SHOWNORMAL;
                    break;
                }

                // nastaveni v shortcute ma prioritu nad konfiguraci
                case SW_SHOWMINNOACTIVE:
                case SW_SHOWMAXIMIZED:
                {
                    place.showCmd = CmdShow;
                    break;
                }
                }
            }
            else
            {
                switch (CmdShow)
                {
                case SW_SHOWNORMAL:
                {
                    // pokud je v konfiguraci minimalizovane okno, otevreme ho restored
                    if (place.showCmd == SW_MINIMIZE)
                        place.showCmd = SW_RESTORE;
                    if (place.showCmd == SW_SHOWMINIMIZED)
                        place.showCmd = SW_SHOWNORMAL;
                    break;
                }

                // nastaveni v shortcute ma prioritu nad konfiguraci
                case SW_SHOWMINNOACTIVE:
                {
                    place.showCmd = SW_HIDE;
                    PostMessage(HWindow, WM_SYSCOMMAND, SC_MINIMIZE, 0);
                    break;
                }

                // nastaveni v shortcute ma prioritu nad konfiguraci
                case SW_SHOWMAXIMIZED:
                {
                    place.showCmd = CmdShow;
                    PostMessage(HWindow, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
                    break;
                }
                }
            }
            SetWindowPlacement(HWindow, &place);
        }
        LeftPanel->SetupListBoxScrollBars();
        RightPanel->SetupListBoxScrollBars();

        UpdateWindow(HWindow);

        // provedeme nastaveni cest v panelech podle parametru z prikazove radky (vsechny typy cest, i archivy a FS)
        BOOL leftPanelPathSet = FALSE;
        BOOL rightPanelPathSet = FALSE;
        if (ret && cmdLineParams != NULL)
        {
            if (cmdLineParams->LeftPath[0] == 0 && cmdLineParams->RightPath[0] == 0 && cmdLineParams->ActivePath[0] != 0)
            {
                if (GetActivePanel()->ChangeDirLite(cmdLineParams->ActivePath)) // nema smysl kombinovat s nastavenim leveho/praveho panelu
                {
                    if (rightPanelFocused)
                        rightPanelPathSet = TRUE;
                    else
                    {
                        leftPanelPathSet = TRUE;
                        LeftPanel->RefreshVisibleItemsArray(); // komentar nize viz "RefreshVisibleItemsArray"
                    }
                }
            }
            else
            {
                if (cmdLineParams->LeftPath[0] != 0)
                {
                    if (LeftPanel->ChangeDirLite(cmdLineParams->LeftPath))
                    {
                        leftPanelPathSet = TRUE;
                        LeftPanel->RefreshVisibleItemsArray(); // komentar nize viz "RefreshVisibleItemsArray"
                    }
                }
                if (cmdLineParams->RightPath[0] != 0)
                {
                    if (RightPanel->ChangeDirLite(cmdLineParams->RightPath))
                        rightPanelPathSet = TRUE;
                }
            }
        }

        // ulozime pole viditelnych polozek, normalne se to dela v idle, ale jestli to ma
        // byt pripravene pro priorizaci cteni ikon usermenu pred ikonami mimo viditelnou
        // cast panelu, musime se o to postarat "rucne" (nacitani ikon uz teda davno bezi,
        // ale lepsi ted nez jeste pozdeji, tohle minimalni zpozdeni se snad moc neprojevi)
        if (rightPanelPathSet)
            RightPanel->RefreshVisibleItemsArray();

        // leftPanelPath a rightPanelPath jsou jen diskove cesty, ani archivy, ani FS neukladame
        DWORD err, lastErr;
        BOOL pathInvalid, cut;
        BOOL tryNet = TRUE;
        if (!leftPanelPathSet)
        {
            if (SalCheckAndRestorePathWithCut(LeftPanel->HWindow, leftPanelPath, tryNet,
                                              err, lastErr, pathInvalid, cut, TRUE))
            {
                LeftPanel->ChangePathToDisk(LeftPanel->HWindow, leftPanelPath);
            }
            else
                LeftPanel->ChangeToRescuePathOrFixedDrive(LeftPanel->HWindow);
            LeftPanel->RefreshVisibleItemsArray(); // komentar vyse viz "RefreshVisibleItemsArray"
        }
        UpdateWindow(LeftPanel->HWindow); // zajisti vykresleni dir/info line hned po vykresleni obsahu panelu

        tryNet = TRUE;
        if (!rightPanelPathSet)
        {
            if (SalCheckAndRestorePathWithCut(RightPanel->HWindow, rightPanelPath, tryNet,
                                              err, lastErr, pathInvalid, cut, TRUE))
            {
                RightPanel->ChangePathToDisk(RightPanel->HWindow, rightPanelPath);
            }
            else
                RightPanel->ChangeToRescuePathOrFixedDrive(RightPanel->HWindow);
            RightPanel->RefreshVisibleItemsArray(); // komentar vyse viz "RefreshVisibleItemsArray"
        }
        UpdateWindow(RightPanel->HWindow); // zajisti vykresleni dir/info line hned po vykresleni obsahu panelu

        // obnova default-dir na systemovem disku (poskozeno - syst. root byl v obou panelech)
        lstrcpyn(DefaultDir[LowerCase[sysDefDir[0]] - 'a'], sysDefDir, MAX_PATH);
        // obnova DefaultDir
        MainWindow->UpdateDefaultDir(TRUE);

        return ret;
    }

    LoadSaveToRegistryMutex.Leave();

    return FALSE;
}
