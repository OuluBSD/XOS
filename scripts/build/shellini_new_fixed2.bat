COLOR FGC=green
DISPLAY ROWS=60
@HISTORY 400
SERVER usbctl sta
C:
@CD \xossrc\util
@COPY DOSCOM.C C:\DOSCOM.C
C:
@CC DOSCOM.C > C:\compile_output.txt 2>&1
@DIR C:\ >> C:\compile_output.txt
@IF EXIST DOSCOM.RUN (
  C:
  DOSCOM > C:\doscom_output.txt 2>&1
  @DIR C:\ >> C:\doscom_output.txt
)
C:
@COPY C:\compile_output.txt C:\output.txt
@TYPE C:\DOSCOM.C > C:\source.txt