#!/bin/sh
# Runs every program in examples/ and tests/cases/ and compares the result
# with its expected output.
#
# For  name.mini  the runner looks for optional sibling files:
#   name.in    text fed to the program's standard input   (default: empty)
#   name.out   expected stdout + stderr                   (required)
#   name.code  expected exit code                         (default: 0)
#
# Usage: tests/run_tests.sh [path/to/minilang] [--update]
#   --update  rewrite every .out file from the current output (review the diff!)

BIN=${1:-./minilang}
UPDATE=0
[ "$2" = "--update" ] && UPDATE=1
[ "$1" = "--update" ] && { UPDATE=1; BIN=./minilang; }

ROOT=$(cd "$(dirname "$0")/.." && pwd)
pass=0
fail=0

for src in "$ROOT"/examples/*.mini "$ROOT"/tests/cases/*.mini; do
    base=${src%.mini}
    name=${src#"$ROOT"/}

    input=/dev/null
    [ -f "$base.in" ] && input="$base.in"
    want_code=0
    [ -f "$base.code" ] && want_code=$(cat "$base.code")

    actual=$("$BIN" "$src" < "$input" 2>&1)
    code=$?

    if [ "$UPDATE" -eq 1 ]; then
        printf '%s\n' "$actual" > "$base.out"
        echo "updated $name (exit $code)"
        continue
    fi

    if [ ! -f "$base.out" ]; then
        echo "FAIL  $name  (missing $base.out)"
        fail=$((fail + 1))
        continue
    fi

    expected=$(cat "$base.out")
    if [ "$actual" = "$expected" ] && [ "$code" -eq "$want_code" ]; then
        pass=$((pass + 1))
    else
        echo "FAIL  $name  (exit $code, wanted $want_code)"
        fail=$((fail + 1))
    fi
done

[ "$UPDATE" -eq 1 ] && exit 0

# ---- interactive (console) mode: programs are piped in, each ended by "run".
# check NAME EXPECTED_OUTPUT STDIN...   (stdout+stderr compared exactly)
check_interactive() {
    name=$1; expected=$2; stdin_text=$3
    actual=$(printf '%b' "$stdin_text" | "$BIN" 2>&1)
    if [ "$actual" = "$(printf '%b' "$expected")" ]; then
        pass=$((pass + 1))
    else
        echo "FAIL  interactive: $name"
        printf '  expected: %s\n  actual:   %s\n' "$expected" "$actual"
        fail=$((fail + 1))
    fi
}

PROG='main\n  output("hi")\nend\n'
check_interactive "runs a typed program"         'hi'        "${PROG}run\n"
check_interactive "runs at end of input"         'hi'        "${PROG}"
check_interactive "runs several programs"        'hi\nhi'    "${PROG}run\n${PROG}run\n"
check_interactive "quit stops the session"       'hi'        "${PROG}run\nquit\n${PROG}run\n"
check_interactive "empty session does nothing"   ''          "run\n\n  \nrun\n"
check_interactive "error does not end session"   'Error at token/lexeme pair: s_semi ;\nhi' \
    'main\n  output("x");\nend\nrun\n'"${PROG}"'run\n'
check_interactive "input() reads lines after run" 'input value for variable: n\n42' \
    'var\n  integer n, m;\nmain\n  input(n)\n  m = n * 2;\n  output(m)\nend\nrun\n21\n'
check_interactive "works with CRLF typing"       'hi'        'main\r\n  output("hi")\r\nend\r\nrun\r\n'

echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
