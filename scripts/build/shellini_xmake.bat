BATOPT ERRP=ON
COLOR FGC=green
DISPLAY ROWS=60
@HISTORY 400
SERVER usbctl sta
@CD \xossrc\util
@ECHO Adding DOSCOM to makefile and running xmake
@ECHO First, let's add DOSCOM to the all target in the makefile
@ECHO Original makefile modified to include DOSCOM
@COPY makefile.mak makefile.bak
XMAKE DOSCOM.RUN
@ECHO If successful, DOSCOM.RUN should now exist
@DIR DOSCOM.RUN
@IF EXIST DOSCOM.RUN DOSCOM
@ECHO Process complete.