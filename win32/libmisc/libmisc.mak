# Microsoft Developer Studio Generated NMAKE File, Based on libmisc.dsp
!IF "$(CFG)" == ""
CFG=libmisc - Win32 Debug
!MESSAGE No configuration specified. Defaulting to libmisc - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "libmisc - Win32 Release" && "$(CFG)" != "libmisc - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libmisc - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\libmisc.lib"


CLEAN :
	-@erase "$(INTDIR)\basename.obj"
	-@erase "$(INTDIR)\dirname.obj"
	-@erase "$(INTDIR)\mkrdir.obj"
	-@erase "$(INTDIR)\newopt.obj"
	-@erase "$(INTDIR)\queue.obj"
	-@erase "$(INTDIR)\snprintf.obj"
	-@erase "$(INTDIR)\strdup.obj"
	-@erase "$(INTDIR)\trace.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\libmisc.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "../../lib" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\libmisc.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\libmisc.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\libmisc.lib" 
LIB32_OBJS= \
	"$(INTDIR)\basename.obj" \
	"$(INTDIR)\dirname.obj" \
	"$(INTDIR)\mkrdir.obj" \
	"$(INTDIR)\newopt.obj" \
	"$(INTDIR)\queue.obj" \
	"$(INTDIR)\snprintf.obj" \
	"$(INTDIR)\strdup.obj" \
	"$(INTDIR)\trace.obj"

"$(OUTDIR)\libmisc.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "libmisc - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\libmisc.lib"


CLEAN :
	-@erase "$(INTDIR)\basename.obj"
	-@erase "$(INTDIR)\dirname.obj"
	-@erase "$(INTDIR)\mkrdir.obj"
	-@erase "$(INTDIR)\newopt.obj"
	-@erase "$(INTDIR)\queue.obj"
	-@erase "$(INTDIR)\snprintf.obj"
	-@erase "$(INTDIR)\strdup.obj"
	-@erase "$(INTDIR)\trace.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\libmisc.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "../../lib" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\libmisc.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\libmisc.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\libmisc.lib" 
LIB32_OBJS= \
	"$(INTDIR)\basename.obj" \
	"$(INTDIR)\dirname.obj" \
	"$(INTDIR)\mkrdir.obj" \
	"$(INTDIR)\newopt.obj" \
	"$(INTDIR)\queue.obj" \
	"$(INTDIR)\snprintf.obj" \
	"$(INTDIR)\strdup.obj" \
	"$(INTDIR)\trace.obj"

"$(OUTDIR)\libmisc.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
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
!IF EXISTS("libmisc.dep")
!INCLUDE "libmisc.dep"
!ELSE 
!MESSAGE Warning: cannot find "libmisc.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "libmisc - Win32 Release" || "$(CFG)" == "libmisc - Win32 Debug"
SOURCE=..\..\lib\basename.c

"$(INTDIR)\basename.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\lib\dirname.c

"$(INTDIR)\dirname.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\lib\mkrdir.c

"$(INTDIR)\mkrdir.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\lib\newopt.c

"$(INTDIR)\newopt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\lib\queue.c

"$(INTDIR)\queue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\lib\snprintf.c

"$(INTDIR)\snprintf.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\lib\strdup.c

"$(INTDIR)\strdup.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\lib\trace.c

"$(INTDIR)\trace.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

