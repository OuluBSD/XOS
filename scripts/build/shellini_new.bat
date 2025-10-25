COLOR FGC=green
DISPLAY ROWS=60
@HISTORY 400
SERVER usbctl sta
@CD \xossrc\util
@TYPE DOSCOM.C > C:\first_test.txt
@COPY DOSCOM.C A:\DOSCOM.C
@TYPE A:\DOSCOM.C >> C:\first_test.txt
@CC DOSCOM.C > C:\compile_output.txt 2>&1
@DIR >> C:\compile_output.txt
@IF EXIST DOSCOM.RUN COPY DOSCOM.RUN A:\DOSCOM.RUN
@IF EXIST DOSCOM.RUN DOSCOM > C:\doscom_output.txt 2>&1
@DIR C:\ >> C:\dir_output.txt
@COPY C:\compile_output.txt C:\output.txt