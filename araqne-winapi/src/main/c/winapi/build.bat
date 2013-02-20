@echo off
msbuild /p:Targets="Clean;Build" /p:Configuration="Release" /p:Platform="x64" winapi.sln 
msbuild /p:Targets="Clean;Build" /p:Configuration="Release" /p:Platform="win32" winapi.sln 
