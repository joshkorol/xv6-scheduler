@ECHO OFF
setlocal
SET parent=%~dp0
FOR %%a IN ("%parent%\..\..\..") DO SET "root=%%~fa"
@ECHO ON
docker run --rm -it --name="xv6" -v %root%:/xv6 -w="/xv6" jackwolfard/cs3210:latest
