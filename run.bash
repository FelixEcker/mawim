#!/bin/bash
# requires argtest, see https://github.com/FelixEcker/argtest.git

XEPHYR_RESOLUTION='1920x1080'
XEPHYR_DISPLAY=':101'

if argtest --help "$@" || argtest -? "$@"; then
  printf "  --help    -?\tDisplay this help text\n"
  printf "  --rebuild   \tRebuild mawim before execution\n"
  printf "  --release   \tUse the release binary\n"
  printf "  --gdb       \tAttach gdb after launch\n"
  printf "  --valgrind  \tRun wrapped in valgrind"
  printf "\n\n"
  printf "You can also populate \$MARIEBUILD_FLAGS for rebuilding,\n"
  printf "\$XEPHYR_RESOLUTION (default $XEPHYR_RESOLUTION) and\n"
  printf "\$XEPHYR_DISPLAY (default $XEPHYR_DISPLAY)\n"
  exit
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

if argtest --valgrind "$@"; then
  MAWIM_VALGRIND="yes"
  export MAWIM_VALGRIND
else
  unset MAWIM_VALGRIND
fi

if argtest --gdb "$@"; then
  xinit ./data/xinitrc -- "$XEPHYR" $XEPHYR_DISPLAY -ac -screen $XEPHYR_RESOLUTION -host-cursor &
  sleep 5
  mawim_pid=$(pgrep -n mawim)
  sudo -E gdb -p ${mawim_pid}
else
  xinit ./data/xinitrc -- "$XEPHYR" $XEPHYR_DISPLAY -ac -screen $XEPHYR_RESOLUTION -host-cursor
fi
