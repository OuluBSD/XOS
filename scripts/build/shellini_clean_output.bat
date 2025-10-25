BATOPT ERRP=ON
COLOR FGC=green
DISPLAY ROWS=60
@HISTORY 400
SERVER usbctl sta
@CD \xossrc\util
@COPY DOSCOM.C DOSCOM_FIXED.C
@ECHO Starting compilation of DOSCOM.C > output.txt
@ECHO ====================================== >> output.txt
OWXC /hc /d1+ /i=xosinc:\\owxc\\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc /zp4 /zpw /za99 /ze /oarkhs /nm=DOSCOM DOSCOM.C >> output.txt 2>&1
@IF EXIST DOSCOM.RUN (
  @ECHO ====================================== >> output.txt
  @ECHO Running DOSCOM >> output.txt
  @ECHO ====================================== >> output.txt
  DOSCOM >> output.txt 2>&1
)
@ECHO ====================================== >> output.txt
@ECHO Compilation and execution complete >> output.txt
@COPY output.txt ..\..\output.txt