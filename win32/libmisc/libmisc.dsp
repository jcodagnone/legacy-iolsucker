# Microsoft Developer Studio Project File - Name="libmisc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libmisc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libmisc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libmisc.mak" CFG="libmisc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libmisc - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libmisc - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libmisc - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../lib/libmisc" /I "../" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE RSC /l 0x2c0a /d "NDEBUG"
# ADD RSC /l 0x2c0a /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libmisc - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../lib" /I "../" /I "../../lib/libmisc" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "HAVE_CONFIG_H" /YX /FD /GZ /c
# ADD BASE RSC /l 0x2c0a /d "_DEBUG"
# ADD RSC /l 0x2c0a /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libmisc - Win32 Release"
# Name "libmisc - Win32 Debug"
# Begin Group "libtrio"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\lib\libmisc\trio\compare.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\trio\strio.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\trio\trio.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\trio\trio.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\trio\triodef.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\trio\trionan.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\trio\trionan.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\trio\triop.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\trio\triostr.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\trio\triostr.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\lib\libmisc\basename.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\basename.h
# End Source File
# Begin Source File

SOURCE="..\..\lib\libmisc\dirent.c"
# End Source File
# Begin Source File

SOURCE="..\..\lib\libmisc\dirent_.h"
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\dirname.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\dirname.h
# End Source File
# Begin Source File

SOURCE="..\..\lib\libmisc\dirstack.c"
# End Source File
# Begin Source File

SOURCE="..\..\lib\libmisc\dirstack.h"
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\dll.h
# End Source File
# Begin Source File

SOURCE="..\..\lib\libmisc\ftw_.c"
# End Source File
# Begin Source File

SOURCE="..\..\lib\libmisc\ftw_.h"
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\i18n.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\mkrdir.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\mkrdir.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\newopt.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\newopt.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\queue.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\queue.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\snprintf.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\snprintf.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\stm.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\stm.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\strdup.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\strdup.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\trace.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\trace.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\libmisc\unix.h
# End Source File
# End Target
# End Project
