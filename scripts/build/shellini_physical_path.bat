BATOPT ERRP=ON
COLOR FGC=green
DISPLAY ROWS=60
@HISTORY 400
SERVER usbctl sta
@CD \xossrc\util
@ECHO Setting up environment and starting compilation of DOSCOM.C
@ECHO Note: This may fail if the logical device xosinc is not properly mapped
@ECHO Attempting to call compiler with physical path instead of logical path
@ECHO This will use the headers from \xossys\inc\owxc\
\xossys\cmd\w0wvc /hc /d1+ /i=\xossys\inc\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc /zp4 /zpw /za99 /ze /oarkhs /nm=DOSCOM DOSCOM.C
@ECHO Checking for DOSCOM.RUN executable...
@DIR DOSCOM.RUN
@ECHO If DOSCOM.RUN was created, running it now:
@IF EXIST DOSCOM.RUN DOSCOM
@ECHO Process complete.