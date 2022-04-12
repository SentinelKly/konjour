@echo off

mkdir bin
gcc src/konjour.c src/cfg_parser.c src/gcc_builder.c src/err_handler.c -o ./bin/konjour-dbg.exe