# Microsoft Developer Studio Generated NMAKE File, Based on iolsucker.dsp
!IF "$(CFG)" == ""
CFG=iolsucker - Win32 Debug
!MESSAGE No configuration specified. Defaulting to iolsucker - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "iolsucker - Win32 Release" && "$(CFG)" != "iolsucker - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iolsucker - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\iolsucker.exe"

!ELSE 

ALL : "libmiscdll - Win32 Release" "$(OUTDIR)\iolsucker.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"libmiscdll - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\curlerr.obj"
	-@erase "$(INTDIR)\getpass.obj"
	-@erase "$(INTDIR)\iol.obj"
	-@erase "$(INTDIR)\link.obj"
	-@erase "$(INTDIR)\link_debug.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\trace.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\iolsucker.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "../../lib" /I "../../../unix/include" /I "../../../unix/include/glib-2.0" /D "NDEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\iolsucker.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\iolsucker.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=libcurl.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib libxml2.lib glib-2.0.lib libmisc.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\iolsucker.pdb" /machine:I386 /out:"$(OUTDIR)\iolsucker.exe" /libpath:"../../../unix/lib" /libpath:"../libmiscdll/Release" 
LINK32_OBJS= \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\curlerr.obj" \
	"$(INTDIR)\getpass.obj" \
	"$(INTDIR)\iol.obj" \
	"$(INTDIR)\link.obj" \
	"$(INTDIR)\link_debug.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\trace.obj" \
	"..\libmiscdll\Release\libmiscdll.lib"

"$(OUTDIR)\iolsucker.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "iolsucker - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\iolsucker.exe"

!ELSE 

ALL : "libmiscdll - Win32 Debug" "$(OUTDIR)\iolsucker.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"libmiscdll - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\curlerr.obj"
	-@erase "$(INTDIR)\getpass.obj"
	-@erase "$(INTDIR)\iol.obj"
	-@erase "$(INTDIR)\link.obj"
	-@erase "$(INTDIR)\link_debug.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\trace.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\iolsucker.exe"
	-@erase "$(OUTDIR)\iolsucker.ilk"
	-@erase "$(OUTDIR)\iolsucker.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "../../lib" /I "../../../unix/include" /I "../../../unix/include/glib-2.0" /D "_DEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\iolsucker.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\iolsucker.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=libcurl.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib libxml2.lib glib-2.0.lib libmisc.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\iolsucker.pdb" /debug /machine:I386 /out:"$(OUTDIR)\iolsucker.exe" /pdbtype:sept /libpath:"../../../unix/lib" /libpath:"../libmisc/Debug/" 
LINK32_OBJS= \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\curlerr.obj" \
	"$(INTDIR)\getpass.obj" \
	"$(INTDIR)\iol.obj" \
	"$(INTDIR)\link.obj" \
	"$(INTDIR)\link_debug.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\trace.obj" \
	"..\libmiscdll\Debug\libmiscdll.lib"

"$(OUTDIR)\iolsucker.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("iolsucker.dep")
!INCLUDE "iolsucker.dep"
!ELSE 
!MESSAGE Warning: cannot find "iolsucker.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "iolsucker - Win32 Release" || "$(CFG)" == "iolsucker - Win32 Debug"

!IF  "$(CFG)" == "iolsucker - Win32 Release"

"libmiscdll - Win32 Release" : 
   cd "\juam\iolsucker-2.0.0\win32\libmiscdll"
   $(MAKE) /$(MAKEFLAGS) /F ".\libmiscdll.mak" CFG="libmiscdll - Win32 Release" 
   cd "..\iolsucker"

"libmiscdll - Win32 ReleaseCLEAN" : 
   cd "\juam\iolsucker-2.0.0\win32\libmiscdll"
   $(MAKE) /$(MAKEFLAGS) /F ".\libmiscdll.mak" CFG="libmiscdll - Win32 Release" RECURSE=1 CLEAN 
   cd "..\iolsucker"

!ELSEIF  "$(CFG)" == "iolsucker - Win32 Debug"

"libmiscdll - Win32 Debug" : 
   cd "\juam\iolsucker-2.0.0\win32\libmiscdll"
   $(MAKE) /$(MAKEFLAGS) /F ".\libmiscdll.mak" CFG="libmiscdll - Win32 Debug" 
   cd "..\iolsucker"

"libmiscdll - Win32 DebugCLEAN" : 
   cd "\juam\iolsucker-2.0.0\win32\libmiscdll"
   $(MAKE) /$(MAKEFLAGS) /F ".\libmiscdll.mak" CFG="libmiscdll - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\iolsucker"

!ENDIF 

SOURCE=..\..\src\config.c

"$(INTDIR)\config.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\curlerr.c

"$(INTDIR)\curlerr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\getpass.c

"$(INTDIR)\getpass.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\iol.c

"$(INTDIR)\iol.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\link.c

"$(INTDIR)\link.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\link_debug.c

"$(INTDIR)\link_debug.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\progress.c

"$(INTDIR)\progress.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\lib\trace.c

"$(INTDIR)\trace.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

