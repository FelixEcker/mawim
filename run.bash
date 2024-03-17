#!/bin/bash
# requires argtest, see https://github.com/FelixEcker/argtest.git

if argtest --help "$@" || argtest -? "$@"; then
  printf "  --help    -?\tDisplay this help text"
  printf "  --rebuild   \tRebuild mawim before execution"
  printf "  --release   \tUse the release binary"
  printf
  printf "You can also populate $MARIEBUILD_FLAGS for rebuilding"
  printf "and $MAWIM_FLAGS for mawim flags"
  printf
fi

if argtest --rebuild "$@"; then
  echo "==> Rebuilding MaWiM"

  if argtest --release "$@"; then
    MARIEBUILD_FLAGS=$MARIEBUILD_FLAGS:-t release
  fi

  mb -n $MARIEBUILD_FLAGS || exit 127
fi

MAWIM_BIN="build/debug/mawim"

if argtest --release "$@"; then
  MAWIM_BIN="build/release/mawim"
  if [[ ! -f $MAWIM_BIN ]]; then
    mb -n $MARIEBUILD_FLAGS -t release || exit 127
  fi
  echo $MAWIM_BIN
else 
  if [[ ! -f $MAWIM_BIN ]]; then
    mb -n $MARIEBUILD_FLAGS -t debug || exit 127
  fi
fi

MAWIM_BIN=$(pwd)/$MAWIM_BIN
export MAWIM_BIN

XEPHYR=$(command -v Xephyr)

echo "==> Running..."
xinit ./data/xinitrc -- "$XEPHYR" :100 -ac -screen 3840x2160 -host-cursor
