COLOR FGC=green
DISPLAY ROWS=60
@HISTORY 400
SERVER usbctl sta
@CD \xossrc\util
@COPY DOSCOM.C DOSCOM_FIXED.C
OWXC /hc /d1+ /i=xosinc:\\owxc\\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc /zp4 /zpw /za99 /ze /oarkhs /nm=DOSCOM DOSCOM.C
@DIR *.RUN
@DIR *.OBJ
@DIR *.ERR
@IF EXIST DOSCOM.RUN DOSCOM
@DIR
@ECHO Compilation and execution complete