@ECHO OFF
@ECHO Starting DOSCOM compilation with xmake
@CD \XOSSRC\UTIL
@ECHO Running xmake to build DOSCOM...
@XMAKE DOSCOM.RUN
@ECHO If successful, DOSCOM.RUN should now exist
@DIR DOSCOM.RUN
@IF EXIST DOSCOM.RUN @ECHO DOSCOM compilation successful
@IF NOT EXIST DOSCOM.RUN @ECHO DOSCOM compilation failed
@ECHO DOSCOM compilation process completed