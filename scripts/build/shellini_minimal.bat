BATOPT ERRP=ON
COLOR FGC=green
DISPLAY ROWS=60
@HISTORY 400
SERVER usbctl sta
@CD \xossrc\util
@COPY DOSCOM.C DOSCOM_FIXED.C
@ECHO Starting compilation of DOSCOM.C
OWXC /hc /d1+ /i=xosinc:\\owxc\\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc /zp4 /zpw /za99 /ze /oarkhs /nm=DOSCOM DOSCOM.C
@IF EXIST DOSCOM.RUN (
  @ECHO Running DOSCOM
  DOSCOM
)
@ECHO Compilation and execution complete