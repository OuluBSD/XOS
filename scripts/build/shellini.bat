BATOPT ERRP=ON
COLOR FGC=green
DISPLAY ROWS=60
@HISTORY 400
SERVER usbctl sta
@CD \xossrc\util
@ECHO Attempting to build DOSCOM using xmake
@ECHO Current directory:
@DIR
@ECHO Running xmake to build DOSCOM...
XMAKE DOSCOM.RUN
@ECHO Build process completed.
@ECHO Checking for DOSCOM.RUN...
@DIR DOSCOM.RUN
@IF EXIST DOSCOM.RUN @ECHO DOSCOM successfully built and exists
@IF NOT EXIST DOSCOM.RUN @ECHO DOSCOM build failed or does not exist
@ECHO DOSCOM build process completed