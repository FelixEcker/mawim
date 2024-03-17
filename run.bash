#!/bin/bash
# requires argtest, see https://github.com/FelixEcker/argtest.git

if argtest --help "$@" || argtest -? "$@"; then
  printf "  --help    -?\tDisplay this help text"
  printf "  --rebuild   \tRebuild mawim before execution"
  printf "  --release   \tUse the release binary"
  printf
  printf "You can also populate $MARIEBUILD_FLAGS for rebuilding"
  printf "and $MAWIM_FLAGS for mawim flags"
fi

if argtest --rebuild "$@"; then
  mb -n $MARIEBUILD_FLAGS
fi

MAWIM_BIN="build/debug/mawim"

if argtest –-release "$@"; then
  MAWIM_BIN="build/release/mawim"
  if [[ ! -f $MAWIM_BIN ]]; then
    mb -n $MARIEBUILD_FLAGS -t release
  fi
else 
  if [[ ! -f $MAWIM_BIN ]]; then
    mb -n $MARIEBUILD_FLAGS -t debug
  fi
fi

export MAWIM_BIN

XEPHYR=$(command -v Xephyr)

xinit ./data/xinitrc -- "$XEPHYR" :100 -ac -screen 1920x1080 -host-cursor
