---

<img src="branding/kj_banner.png">

---

<div align=center style="float: left">
  <img src="https://img.shields.io/github/license/sentinelkly/konjour">
  <img src="https://img.shields.io/badge/version-alpha-yellow">
</div>

## About
Konjour is a very lightweight, portable, and minimal C/C++ build system. Konjour was designed with the goal of allowing quick C/C++ project setup without needing to worry about learning the ins and outs of bigger and more complex build systems such as Cmake. Unlike Cmake or Premake, Konjour handles the entire compilation procedure, instead of relying on outside systems like Make or Visual Studio Solutions.

### NOTE: Konjour is still in very early development. Everything is subject to change and issues may occur!

## How to build
For first time builds, konjour can be compiled with a simple build script:
```sh
#For unix and powershell
./build.[sh|bat]

#For cmd 
build.bat
```
This will compile and build a very quick debug build of konjour. You can use this debug build to
then bootstrap itself using the konjour.cfg:
```sh
./konjour-dbg konjour.cfg
```
That will then build the proper and release ready konjour build.

## Beta Roadmap
- [X] Multithreaded compilation
- [ ] More bug fixes and parsing sanitisation
- [ ] Avoid rebuilding unchanged targets
- [ ] Easier to read output
- [ ] Pre-compiled header support
- [ ] Configurable konjour settings
- [ ] Allow for field concatenation
- [ ] Cmake interoperability 
- [ ] Support for MSVC
- [ ] Basic meta-scripting
- [ ] Artefact sorted building
- [ ] Support for custom variables

## Current Issues
- [ ] 'Dodgy' code fixes
- [ ] Certain syntax errors go unnoticed 
