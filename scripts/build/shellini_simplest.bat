COLOR FGC=green
DISPLAY ROWS=60
@HISTORY 400
SERVER usbctl sta
@CD \xossrc\util
@COPY DOSCOM.C DOSCOM_FIXED.C
@CC DOSCOM_FIXED.C > compile.log 2>&1
@DIR > dir.log
@IF EXIST DOSCOM_FIXED.RUN DOSCOM_FIXED > output.log 2>&1
@DIR >> output.log
@COPY output.log ..\..\output.txt
@COPY compile.log ..\..\compile_output.txt