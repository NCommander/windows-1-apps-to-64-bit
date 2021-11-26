# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (MIPS) Application" 0x0501

!IF "$(CFG)" == ""
CFG=TemplatePort - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to TemplatePort - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "TemplatePort - Win32 Release" && "$(CFG)" !=\
 "TemplatePort - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "TemplatePort.mak" CFG="TemplatePort - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TemplatePort - Win32 Release" (based on "Win32 (MIPS) Application")
!MESSAGE "TemplatePort - Win32 Debug" (based on "Win32 (MIPS) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "TemplatePort - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "TemplatePort - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MIPSRel"
# PROP BASE Intermediate_Dir "MIPSRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "MIPSRel"
# PROP Intermediate_Dir "MIPSRel"
# PROP Target_Dir ""
OUTDIR=.\MIPSRel
INTDIR=.\MIPSRel

ALL : "$(OUTDIR)\TemplatePort.exe"

CLEAN : 
	-@erase "$(INTDIR)\TEMPINIT.OBJ"
	-@erase "$(INTDIR)\TEMPLATE.res"
	-@erase "$(INTDIR)\TEMPNRES.OBJ"
	-@erase "$(INTDIR)\TEMPRES.OBJ"
	-@erase "$(OUTDIR)\TemplatePort.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE MTL /nologo /D "NDEBUG" /mips
# ADD MTL /nologo /D "NDEBUG" /mips
MTL_PROJ=/nologo /D "NDEBUG" /mips 
# ADD BASE CPP /nologo /Gt0 /QMOb2000 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /Gt0 /QMOb2000 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /Gt0 /QMOb2000 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/TemplatePort.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\MIPSRel/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/TEMPLATE.res" /d "NDEBUG" 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:MIPS
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:MIPS
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/TemplatePort.pdb" /machine:MIPS\
 /out:"$(OUTDIR)/TemplatePort.exe" 
LINK32_OBJS= \
	"$(INTDIR)\TEMPINIT.OBJ" \
	"$(INTDIR)\TEMPLATE.res" \
	"$(INTDIR)\TEMPNRES.OBJ" \
	"$(INTDIR)\TEMPRES.OBJ"

"$(OUTDIR)\TemplatePort.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/TemplatePort.bsc" 
BSC32_SBRS= \
	

!ELSEIF  "$(CFG)" == "TemplatePort - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MIPSDbg"
# PROP BASE Intermediate_Dir "MIPSDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "MIPSDbg"
# PROP Intermediate_Dir "MIPSDbg"
# PROP Target_Dir ""
OUTDIR=.\MIPSDbg
INTDIR=.\MIPSDbg

ALL : "$(OUTDIR)\TemplatePort.exe"

CLEAN : 
	-@erase "$(INTDIR)\TEMPINIT.OBJ"
	-@erase "$(INTDIR)\TEMPLATE.res"
	-@erase "$(INTDIR)\TEMPNRES.OBJ"
	-@erase "$(INTDIR)\TEMPRES.OBJ"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\TemplatePort.exe"
	-@erase "$(OUTDIR)\TemplatePort.ilk"
	-@erase "$(OUTDIR)\TemplatePort.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE MTL /nologo /D "_DEBUG" /mips
# ADD MTL /nologo /D "_DEBUG" /mips
MTL_PROJ=/nologo /D "_DEBUG" /mips 
# ADD BASE CPP /nologo /Gt0 /QMOb2000 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /Gt0 /QMOb2000 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /Gt0 /QMOb2000 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/TemplatePort.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/"\
 /c 
CPP_OBJS=.\MIPSDbg/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/TEMPLATE.res" /d "_DEBUG" 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:MIPS
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:MIPS
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/TemplatePort.pdb" /debug /machine:MIPS\
 /out:"$(OUTDIR)/TemplatePort.exe" 
LINK32_OBJS= \
	"$(INTDIR)\TEMPINIT.OBJ" \
	"$(INTDIR)\TEMPLATE.res" \
	"$(INTDIR)\TEMPNRES.OBJ" \
	"$(INTDIR)\TEMPRES.OBJ"

"$(OUTDIR)\TemplatePort.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/TemplatePort.bsc" 
BSC32_SBRS= \
	

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "TemplatePort - Win32 Release"
# Name "TemplatePort - Win32 Debug"

!IF  "$(CFG)" == "TemplatePort - Win32 Release"

!ELSEIF  "$(CFG)" == "TemplatePort - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\TEMPRES.C
DEP_CPP_TEMPR=\
	".\TEMPLATE.H"\
	

"$(INTDIR)\TEMPRES.OBJ" : $(SOURCE) $(DEP_CPP_TEMPR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TEMPLATE.H

!IF  "$(CFG)" == "TemplatePort - Win32 Release"

!ELSEIF  "$(CFG)" == "TemplatePort - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TEMPLATE.RC
DEP_RSC_TEMPL=\
	".\TEMPLATE.H"\
	

"$(INTDIR)\TEMPLATE.res" : $(SOURCE) $(DEP_RSC_TEMPL) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TEMPNRES.C
DEP_CPP_TEMPN=\
	".\TEMPLATE.H"\
	

"$(INTDIR)\TEMPNRES.OBJ" : $(SOURCE) $(DEP_CPP_TEMPN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TEMPINIT.C
DEP_CPP_TEMPI=\
	".\TEMPLATE.H"\
	

"$(INTDIR)\TEMPINIT.OBJ" : $(SOURCE) $(DEP_CPP_TEMPI) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
