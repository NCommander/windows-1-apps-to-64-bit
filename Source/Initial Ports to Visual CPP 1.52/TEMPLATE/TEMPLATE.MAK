# Microsoft Visual C++ generated build script - Do not modify

PROJ = TEMPLATE
DEBUG = 1
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\SRC\TEMPLATE\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC =             
FIRSTCPP =             
RC = rc
CFLAGS_D_WEXE = /nologo /W3 /FR /G2 /Zi /D_DEBUG /Od /AM /GA /Fd"TEMPLATE.PDB"
CFLAGS_R_WEXE = /nologo /W3 /FR /O1 /DNDEBUG /AM /GA
LFLAGS_D_WEXE = /NOLOGO /ONERROR:NOEXE /NOD /PACKC:61440 /CO /ALIGN:16 /STACK:10240
LFLAGS_R_WEXE = /NOLOGO /ONERROR:NOEXE /NOD /PACKC:61440 /ALIGN:16 /STACK:10240
LIBS_D_WEXE = oldnames libw commdlg shell olecli olesvr mlibcew
LIBS_R_WEXE = oldnames libw commdlg shell olecli olesvr mlibcew
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = TEMPLATE.DEF
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WEXE)
LFLAGS = $(LFLAGS_D_WEXE)
LIBS = $(LIBS_D_WEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WEXE)
LFLAGS = $(LFLAGS_R_WEXE)
LIBS = $(LIBS_R_WEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = TEMPINIT.SBR \
		TEMPNRES.SBR \
		TEMPRES.SBR


TEMPINIT_DEP = c:\src\template\template.h


TEMPNRES_DEP = c:\src\template\template.h


TEMPRES_DEP = c:\src\template\template.h


TEMPLATE_RCDEP = c:\src\template\template.h \
	c:\src\template\template.ico


all:	$(PROJ).EXE $(PROJ).BSC

TEMPINIT.OBJ:	TEMPINIT.C $(TEMPINIT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TEMPINIT.C

TEMPNRES.OBJ:	TEMPNRES.C $(TEMPNRES_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TEMPNRES.C

TEMPRES.OBJ:	TEMPRES.C $(TEMPRES_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TEMPRES.C

TEMPLATE.RES:	TEMPLATE.RC $(TEMPLATE_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r TEMPLATE.RC


$(PROJ).EXE::	TEMPLATE.RES

$(PROJ).EXE::	TEMPINIT.OBJ TEMPNRES.OBJ TEMPRES.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
TEMPINIT.OBJ +
TEMPNRES.OBJ +
TEMPRES.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\msvc\lib\+
c:\msvc\mfc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) TEMPLATE.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	TEMPLATE.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) TEMPLATE.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
