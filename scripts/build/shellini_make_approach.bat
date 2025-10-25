BATOPT ERRP=ON
COLOR FGC=green
DISPLAY ROWS=60
@HISTORY 400
SERVER usbctl sta
@CD \xossrc\util
@COPY DOSCOM.C DOSCOM_FIXED.C
@ECHO Starting build of DOSCOM.C from makefile
@ECHO Attempting to make DOSCOM.RUN directly
@MAKE /F makefile.mak DOSCOM.RUN
@ECHO If that failed, trying to build object first
@MAKE /F makefile.mak DOSCOM.obj
@IF EXIST DOSCOM.obj
  @ECHO Object file created, attempting link
  @ECHO Link command would be executed here
@ECHO Checking for executable...
@DIR DOSCOM.RUN
@IF EXIST DOSCOM.RUN
  @ECHO Running DOSCOM with the following output:
  DOSCOM
@ECHO Process complete.