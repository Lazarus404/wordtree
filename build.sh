#!/usr/bin/env bash
# Builds jdll/wordtree.jdll, the native library, for this machine.
#
# The Jello headers come from $JELLO_ROOT/include, else from beside the `jello` on PATH: the
# include/ of an installed Jello, or vm/src/include in a checkout of Jello's repository. CC
# chooses the C compiler (cc by default; llvm-mingw's clang on Windows), and CFLAGS adds flags.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"

die() {
  echo "build.sh: $*" >&2
  exit 1
}

include_dir() {
  local jello candidate
  if [[ -n "${JELLO_ROOT:-}" ]]; then
    echo "$JELLO_ROOT/include"
    return
  fi
  jello="$(command -v jello)" || die "jello is not on PATH; set JELLO_ROOT to Jello's root"
  for candidate in "$(dirname "$jello")/../include" "$(dirname "$jello")/../src/include"; do
    if [[ -f "$candidate/jello/jdll.h" ]]; then
      echo "$candidate"
      return
    fi
  done
  die "cannot find jello/jdll.h beside $jello; set JELLO_ROOT to Jello's root"
}

INCLUDE="$(include_dir)"
CC="${CC:-cc}"
case "$(uname -s)" in
  Darwin) SHARED=(-bundle) ;;
  MINGW* | MSYS* | CYGWIN*) SHARED=(-shared) ;;
  *) SHARED=(-fPIC -shared) ;;
esac

# librope is vendored as its author wrote it, so its warnings are not ours to fix.
mkdir -p "$ROOT/build/obj"
# shellcheck disable=SC2086
"$CC" -std=c11 -O2 -w -fPIC ${CFLAGS:-} -c "$ROOT/jdll/librope/rope.c" -o "$ROOT/build/obj/rope.o"
# shellcheck disable=SC2086
"$CC" -std=c11 -O2 -Wall -Wextra ${CFLAGS:-} "${SHARED[@]}" -I"$INCLUDE" \
  "$ROOT/jdll/wordtree.c" "$ROOT/build/obj/rope.o" -o "$ROOT/jdll/wordtree.jdll"
echo "built jdll/wordtree.jdll"
