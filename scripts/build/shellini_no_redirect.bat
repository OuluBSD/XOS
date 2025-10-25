COLOR FGC=green
DISPLAY ROWS=60
@HISTORY 400
SERVER usbctl sta
@CD \xossrc\util
@ECHO Fixing source file and compiling DOSCOM > log.txt
@COPY DOSCOM.C DOSCOM_BACKUP.C
@CC DOSCOM.C
@DIR *.RUN >> log.txt
@DIR *.OBJ >> log.txt
@DIR *.ERR >> log.txt
@IF EXIST DOSCOM.RUN DOSCOM
@DIR >> log.txt
@COPY log.txt ..\..\output.txt