# Microsoft Developer Studio Project File - Name="iolwizard" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=iolwizard - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iolwizard.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iolwizard.mak" CFG="iolwizard - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iolwizard - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "iolwizard - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iolwizard - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../../unix/include/glib-2.0" /I "../../lib" /I "../../../unix/include" /I "../../../unix/lib/glib-2.0/include" /I "../../../unix/lib/gtk+/include" /I "../../../unix/lib/gtk/include" /I ".." /D "NDEBUG" /D "_WINDOWS" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x2c0a /d "NDEBUG"
# ADD RSC /l 0x2c0a /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib gdk.lib gtk.lib glib-2.0.lib shell32.lib libmisc.lib /nologo /subsystem:windows /machine:I386 /libpath:"../../../unix/lib" /libpath:"../libmisc/Release"

!ELSEIF  "$(CFG)" == "iolwizard - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../../unix/include/glib-2.0" /I "../../lib" /I "../../../unix/include" /I "../../../unix/lib/glib-2.0/include" /I "../../../unix/lib/gtk+/include" /I "../../../unix/lib/gtk/include" /I ".." /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x2c0a /d "_DEBUG"
# ADD RSC /l 0x2c0a /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib gdk.lib gtk.lib glib-2.0.lib shell32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"../../../unix/lib" /libpath:"../libmisc/Debug"

!ENDIF 

# Begin Target

# Name "iolwizard - Win32 Release"
# Name "iolwizard - Win32 Debug"
# Begin Source File

SOURCE=..\..\iolwizard\error_dlg.c
# End Source File
# Begin Source File

SOURCE=..\..\iolwizard\error_dlg.h
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\iolwizard.rc
# End Source File
# Begin Source File

SOURCE=..\..\iolwizard\main.c
# End Source File
# Begin Source File

SOURCE=..\..\src\opt.c
# End Source File
# Begin Source File

SOURCE=..\..\iolwizard\ui.c
# End Source File
# Begin Source File

SOURCE=..\..\iolwizard\ui.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32.c
# End Source File
# Begin Source File

SOURCE=..\..\src\win32.h
# End Source File
# End Target
# End Project
