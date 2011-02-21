# Microsoft Developer Studio Project File - Name="winquake" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=winquake - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WinQuake.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WinQuake.mak" CFG="winquake - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "winquake - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "winquake - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "winquake - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /GX /O2 /Oy- /I ".\scitech\include" /I ".\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 .\dxsdk\sdk\lib\dxguid.lib .\scitech\lib\win32\vc\mgllt.lib winmm.lib wsock32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /profile /machine:I386 /out:"..\..\quake\wqpro.exe"
# SUBTRACT LINK32 /map /debug

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /ML /GX /ZI /Od /I ".\scitech\include" /I ".\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 .\dxsdk\sdk\lib\dxguid.lib .\scitech\lib\win32\vc\mgllt.lib winmm.lib wsock32.lib opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:".\Debug\wqpro.exe"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "winquake - Win32 Release"
# Name "winquake - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\banlog.c
# End Source File
# Begin Source File

SOURCE=.\cmd.c
# End Source File
# Begin Source File

SOURCE=.\common.c
# End Source File
# Begin Source File

SOURCE=.\conproc.c
# End Source File
# Begin Source File

SOURCE=.\console.c
# End Source File
# Begin Source File

SOURCE=.\crc.c
# End Source File
# Begin Source File

SOURCE=.\cvar.c
# End Source File
# Begin Source File

SOURCE=.\host.c
# End Source File
# Begin Source File

SOURCE=.\host_cmd.c
# End Source File
# Begin Source File

SOURCE=.\iplog.c
# End Source File
# Begin Source File

SOURCE=.\math.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\Release
InputPath=.\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp >                                                                                                                                                                                                    $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                    $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\Debug
InputPath=.\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp >                                                                                                                                                                                                    $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                    $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mathlib.c
# End Source File
# Begin Source File

SOURCE=.\model.c
# End Source File
# Begin Source File

SOURCE=.\net_dgrm.c
# End Source File
# Begin Source File

SOURCE=.\net_loop.c
# End Source File
# Begin Source File

SOURCE=.\net_main.c
# End Source File
# Begin Source File

SOURCE=.\net_win.c
# End Source File
# Begin Source File

SOURCE=.\net_wins.c
# End Source File
# Begin Source File

SOURCE=.\pr_cmds.c
# End Source File
# Begin Source File

SOURCE=.\pr_edict.c
# End Source File
# Begin Source File

SOURCE=.\pr_exec.c
# End Source File
# Begin Source File

SOURCE=.\r_main.c
# End Source File
# Begin Source File

SOURCE=.\security.c
# End Source File
# Begin Source File

SOURCE=.\sv_main.c
# End Source File
# Begin Source File

SOURCE=.\sv_move.c
# End Source File
# Begin Source File

SOURCE=.\sv_phys.c
# End Source File
# Begin Source File

SOURCE=.\sv_user.c
# End Source File
# Begin Source File

SOURCE=.\sys_a.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\sys_a.s
InputName=sys_a

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp >                                                                                                                                                                                                    $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                    $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\sys_a.s
InputName=sys_a

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp >                                                                                                                                                                                                    $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                    $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sys_win.c
# End Source File
# Begin Source File

SOURCE=.\wad.c
# End Source File
# Begin Source File

SOURCE=.\winquake.rc
# End Source File
# Begin Source File

SOURCE=.\world.c
# End Source File
# Begin Source File

SOURCE=.\worlda.s

!IF  "$(CFG)" == "winquake - Win32 Release"

# Begin Custom Build - mycoolbuild
OutDir=.\Release
InputPath=.\worlda.s
InputName=worlda

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp >                                                                                                                                                                                                    $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                    $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "winquake - Win32 Debug"

# Begin Custom Build - mycoolbuild
OutDir=.\Debug
InputPath=.\worlda.s
InputName=worlda

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath) 
	gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp >                                                                                                                                                                                                    $(OUTDIR)\$(InputName).asm 
	ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi                                                                                                                                                                                                    $(OUTDIR)\$(InputName).asm 
	del $(OUTDIR)\$(InputName).spp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zone.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\banlog.h
# End Source File
# Begin Source File

SOURCE=.\bspfile.h
# End Source File
# Begin Source File

SOURCE=.\cmd.h
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\conproc.h
# End Source File
# Begin Source File

SOURCE=.\console.h
# End Source File
# Begin Source File

SOURCE=.\crc.h
# End Source File
# Begin Source File

SOURCE=.\cvar.h
# End Source File
# Begin Source File

SOURCE=.\iplog.h
# End Source File
# Begin Source File

SOURCE=.\mathlib.h
# End Source File
# Begin Source File

SOURCE=.\model.h
# End Source File
# Begin Source File

SOURCE=.\modelgen.h
# End Source File
# Begin Source File

SOURCE=.\net.h
# End Source File
# Begin Source File

SOURCE=.\net_dgrm.h
# End Source File
# Begin Source File

SOURCE=.\net_loop.h
# End Source File
# Begin Source File

SOURCE=.\net_wins.h
# End Source File
# Begin Source File

SOURCE=.\pr_comp.h
# End Source File
# Begin Source File

SOURCE=.\progdefs.h
# End Source File
# Begin Source File

SOURCE=.\progs.h
# End Source File
# Begin Source File

SOURCE=.\protocol.h
# End Source File
# Begin Source File

SOURCE=.\quakeasm.h
# End Source File
# Begin Source File

SOURCE=.\quakedef.h
# End Source File
# Begin Source File

SOURCE=.\render.h
# End Source File
# Begin Source File

SOURCE=.\security.h
# End Source File
# Begin Source File

SOURCE=.\server.h
# End Source File
# Begin Source File

SOURCE=.\spritegn.h
# End Source File
# Begin Source File

SOURCE=.\sys.h
# End Source File
# Begin Source File

SOURCE=.\wad.h
# End Source File
# Begin Source File

SOURCE=.\winquake.h
# End Source File
# Begin Source File

SOURCE=.\world.h
# End Source File
# Begin Source File

SOURCE=.\zone.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icon3.ico
# End Source File
# Begin Source File

SOURCE=.\quake.ico
# End Source File
# End Group
# Begin Group "QuakeC Files"

# PROP Default_Filter "qc"
# Begin Source File

SOURCE=..\..\quake\games\basic\source\client.qc
# End Source File
# Begin Source File

SOURCE=..\..\quake\games\basic\source\defs.qc
# End Source File
# Begin Source File

SOURCE=..\..\quake\games\basic\source\internal.qc
# End Source File
# Begin Source File

SOURCE=..\..\quake\games\basic\source\misc.qc
# End Source File
# Begin Source File

SOURCE=..\..\quake\games\basic\source\world.qc
# End Source File
# End Group
# Begin Group "Loc Files"

# PROP Default_Filter ".loc"
# Begin Source File

SOURCE=..\..\Quake\Id1\locs\dm3.loc
# End Source File
# End Group
# Begin Source File

SOURCE=.\changes.log
# End Source File
# Begin Source File

SOURCE=..\release\pq_manual.txt
# End Source File
# Begin Source File

SOURCE=..\release\pq_readme.txt
# End Source File
# Begin Source File

SOURCE=..\release\pqsrc_readme.txt
# End Source File
# Begin Source File

SOURCE=..\release\qsecurity_readme.txt
# End Source File
# Begin Source File

SOURCE=..\release\sqpro_readme.txt
# End Source File
# End Target
# End Project
