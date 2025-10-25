COLOR FGC=green
DISPLAY ROWS=60
@HISTORY 400
SERVER usbctl sta
@CD \xossrc\util
@COPY DOSCOM.C C:\DOSCOM.C
@CC DOSCOM.C > C:\compile_output.txt 2>&1
@DIR C:\ >> C:\compile_output.txt
@IF EXIST DOSCOM.RUN DOSCOM > C:\doscom_output.txt 2>&1
@IF EXIST DOSCOM.RUN @DIR C:\ >> C:\doscom_output.txt
@COPY C:\compile_output.txt C:\output.txt
@TYPE C:\DOSCOM.C > C:\source.txt