# Microsoft Developer Studio Project File - Name="swc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=swc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "swc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "swc.mak" CFG="swc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "swc - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "swc - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "swc - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "swc - Win32 Debug"

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
# ADD CPP /nologo /MTd /W4 /WX /GX /Zi /Od /Gf /I "..\jpeg" /I "..\png" /I "..\zlib" /I "..\common" /D "WIN32" /D "_DEBUG" /D "_LIB" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "swc - Win32 Release"
# Name "swc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\chrtbl.cpp
# End Source File
# Begin Source File

SOURCE=.\datetimevs.cpp
# End Source File
# Begin Source File

SOURCE=.\Qdes.cpp
# End Source File
# Begin Source File

SOURCE=.\random.cpp
# End Source File
# Begin Source File

SOURCE=.\swap.c
# End Source File
# Begin Source File

SOURCE=.\swcbase.cpp
# End Source File
# Begin Source File

SOURCE=.\swcbutton.cpp
# End Source File
# Begin Source File

SOURCE=.\swccheckbox.cpp
# End Source File
# Begin Source File

SOURCE=.\swcclient16.cpp
# End Source File
# Begin Source File

SOURCE=.\swcclient32.cpp
# End Source File
# Begin Source File

SOURCE=.\swcdebug.cpp
# End Source File
# Begin Source File

SOURCE=.\swcdisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\swcdisplaybase.cpp
# End Source File
# Begin Source File

SOURCE=.\swcdisplaylinked.cpp
# End Source File
# Begin Source File

SOURCE=.\swcdropdown.cpp
# End Source File
# Begin Source File

SOURCE=.\swcdropdown2.cpp
# End Source File
# Begin Source File

SOURCE=.\swcedit.cpp
# End Source File
# Begin Source File

SOURCE=.\swcfile.cpp
# End Source File
# Begin Source File

SOURCE=.\swckbwedge.cpp
# End Source File
# Begin Source File

SOURCE=.\swclist.cpp
# End Source File
# Begin Source File

SOURCE=.\swcprint.cpp
# End Source File
# Begin Source File

SOURCE=.\swcrawprint.cpp
# End Source File
# Begin Source File

SOURCE=.\swcresponse.cpp
# End Source File
# Begin Source File

SOURCE=.\swcresponsechar.cpp
# End Source File
# Begin Source File

SOURCE=.\swcresponsenum.cpp
# End Source File
# Begin Source File

SOURCE=.\swcscale.cpp
# End Source File
# Begin Source File

SOURCE=.\swcscalelow.cpp
# End Source File
# Begin Source File

SOURCE=.\swcserver.cpp
# End Source File
# Begin Source File

SOURCE=.\swctabdisp.cpp
# End Source File
# Begin Source File

SOURCE=.\swctoplevel.cpp
# End Source File
# Begin Source File

SOURCE=.\swcwindows.cpp
# End Source File
# Begin Source File

SOURCE=.\winthread.cpp
# End Source File
# Begin Source File

SOURCE=.\xlatewinsocerror.cpp
# End Source File
# Begin Source File

SOURCE=.\xoserrmsg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
