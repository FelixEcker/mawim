# MaWiM
Marie's Window Manager - A simple X11 tiling window manager written in c.

## Building
### Requirements
* C Compiler
* Xlib
* [libmcfg_2](https://github.com/FelixEcker/mcfg_2)

### Compilation
MaWiM is built using [mariebuild](https://github.com/FelixEcker/mariebuild) and
requries version 0.5.0 or higher. If you have mariebuild installed simply run
one of the following commands to build:
```bash
# For a debug build
$ mb 

# For a release build
$ mb -t clean && mb -t release
```

## Debug Running
MaWiM supplies the `run.bash` script, this requires
[argtest](https://github.com/FelixEcker/argtest) and Xephyr to function. For
further information run
```bash
$ bash run.bash -?
```
