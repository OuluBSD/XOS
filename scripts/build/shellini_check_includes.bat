BATOPT ERRP=ON
COLOR FGC=green
DISPLAY ROWS=60
@HISTORY 400
SERVER usbctl sta
@CD \xossrc\util
@ECHO Starting compilation of DOSCOM.C with fixed source
@ECHO First, let's check if the include files exist
@DIR \xossys\inc\owxc\XOS.H
@DIR \xossys\inc\owxc\PROGARG.H
@ECHO If the files exist above, trying to compile with corrected parameters
@ECHO The original typo in DOSCOM.C has been fixed (##include -> #include)
@ECHO Also fixed the header include (PROGHELP.H -> PROGARG.H)
\xossys\cmd\owxc /hc /d1+ /i=\xossys\inc\owxc\ /zq /6s /w3 /we /fpi87 /fp3 /s /mf /zc /zp4 /zpw /za99 /ze /oarkhs DOSCOM.C
@ECHO If successful, this should produce DOSCOM.obj
@DIR DOSCOM.OBJ
@ECHO Process complete.