# Microsoft Developer Studio Project File - Name="iolsucker" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=iolsucker - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iolsucker.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iolsucker.mak" CFG="iolsucker - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iolsucker - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "iolsucker - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iolsucker - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../../unix/lib/glib-2.0/include" /I "../../../unix/include/glib-2.0" /I "../../lib" /I "../../../unix/include" /I "./../../unix/lib/gtk/include" /I "../../../unix/include/db" /I "../" /I "../../lib/libmisc" /D "NDEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE RSC /l 0x2c0a /d "NDEBUG"
# ADD RSC /l 0x2c0a /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib glib-2.0.lib libcurl.lib libmisc.lib libdb41.lib /nologo /subsystem:console /machine:I386 /libpath:"../../../unix/lib" /libpath:"../libmisc/Release"

!ELSEIF  "$(CFG)" == "iolsucker - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../../unix/lib/glib-2.0/include" /I "../../../unix/include/glib-2.0" /I "../../lib" /I "../../../unix/include" /I "./../../unix/lib/gtk/include" /I "../../../unix/include/db" /I ".." /I "../../lib/libmisc" /D "_DEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /YX /FD /GZ /c
# ADD BASE RSC /l 0x2c0a /d "_DEBUG"
# ADD RSC /l 0x2c0a /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib glib-2.0.lib libcurl.lib libmisc.lib libdb41.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"../../../unix/lib" /libpath:"../libmisc/Debug/"

!ENDIF 

# Begin Target

# Name "iolsucker - Win32 Release"
# Name "iolsucker - Win32 Debug"
# Begin Source File

SOURCE="..\..\src\cache.c"
# End Source File
# Begin Source File

SOURCE="..\..\src\cache.h"
# End Source File
# Begin Source File

SOURCE="..\..\src\common.c"
# End Source File
# Begin Source File

SOURCE="..\..\src\common.h"
# End Source File
# Begin Source File

SOURCE=..\..\src\config.h
# End Source File
# Begin Source File

SOURCE=..\..\src\forum.c
# End Source File
# Begin Source File

SOURCE=..\..\src\getpass.c
# End Source File
# Begin Source File

SOURCE=..\..\src\getpass.h
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=..\..\src\iol.c
# End Source File
# Begin Source File

SOURCE=..\..\src\iol.h
# End Source File
# Begin Source File

SOURCE=..\..\src\iolerr.c
# End Source File
# Begin Source File

SOURCE=.\iolsucker.rc
# End Source File
# Begin Source File

SOURCE=..\..\src\link.c
# End Source File
# Begin Source File

SOURCE=..\..\src\link.h
# End Source File
# Begin Source File

SOURCE=..\..\src\link_debug.c
# End Source File
# Begin Source File

SOURCE=..\..\src\main.c
# End Source File
# Begin Source File

SOURCE=..\..\src\main.h
# End Source File
# Begin Source File

SOURCE=..\..\src\opt.c
# End Source File
# Begin Source File

SOURCE=..\..\src\progress.c
# End Source File
# Begin Source File

SOURCE=..\..\src\progress.h
# End Source File
# Begin Source File

SOURCE="..\..\src\stringset.c"
# End Source File
# Begin Source File

SOURCE="..\..\src\stringset.h"
# End Source File
# Begin Source File

SOURCE=..\..\lib\trace.c
# End Source File
# Begin Source File

SOURCE=..\..\src\win32.c
# End Source File
# Begin Source File

SOURCE=..\..\src\win32.h
# End Source File
# End Target
# End Project
