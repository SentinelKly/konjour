@echo off

mkdir bin
g++ src/konjour.cc -I"vendor" -o bin/konjour-dbg.exe