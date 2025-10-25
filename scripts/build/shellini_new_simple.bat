COLOR FGC=green
DISPLAY ROWS=60
@HISTORY 400
SERVER usbctl sta
C:
@CD \xossrc\util
@COPY DOSCOM.C C:\DOSCOM.C
C:
@CC DOSCOM.C
C:
@DIR > C:\compile_dir.txt
C:
@TYPE *.err > C:\error_output.txt 2>NUL
C:
@IF EXIST DOSCOM.RUN DOSCOM
C:
@DIR > C:\post_exec_dir.txt
C:
@COPY C:\compile_dir.txt C:\output.txt
C:
@TYPE C:\error_output.txt >> C:\output.txt
C:
@TYPE C:\post_exec_dir.txt >> C:\output.txt
@TYPE C:\DOSCOM.C > C:\source.txt