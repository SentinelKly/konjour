@echo off

mkdir bin
gcc vendor/tomlc99/toml.c src/konjour.c -o ./bin/konjour-dbg.exe