# minilang

A small compiler and interpreter in C++. Type a program at the console (or give
it a file); it checks the program, compiles it to an instruction table, and runs it.

It combines three course assignments into one program:

| Part | Files | Job |
|---|---|---|
| 1. Lexical analysis | `LexAnalyzer.h/.cpp` | source text -> token/lexeme pairs |
| 2. Syntax analysis | `SyntaxAnalyzer.h/.cpp` | validates the tokens, builds the symbol table |
| 3. Code generation + execution | `main.cpp` (`Compiler`, `Expr`, `Stmt` classes) | tokens -> instruction table -> run |

The parts are used as originally written. `main()` at the bottom of
`src/main.cpp` is the only new logic: it feeds each part's output to the next
through in-memory streams instead of the files the separate programs used.

## Build and run

Needs a C++20 compiler (the original code uses `std::map::contains`) and `make`.

```sh
make
./minilang                          # interactive mode
./minilang examples/factorial.mini  # run a file
./minilang -v examples/hello.mini   # also show what each part produces
make test                           # regression suite
```

### Interactive mode

Type a program, then a line containing only `run`:

```
$ ./minilang
minilang interactive mode
  Type a program, then a line with only 'run' to compile and execute it.
  Type 'quit' or press Ctrl-D to exit.

1> var
2>   integer n, m;
3> main
4>   input(n)
5>   m = n * 2;
6>   output(m)
7> end
8> run
---- running ----
input value for variable: n
21
42

1> quit
```

A mistake prints the error and returns to the prompt. `input(...)` reads from
the lines you type after `run`. `./minilang < program.mini` also works.

### Verbose mode (`-v`)

Shows what each original part prints: the lexer's token/lexeme pairs, the
parser's "Source code is correct" and symbol table, and the `dump()` of the
instruction table, then the program's output.

Exit codes: `0` ok, `1` lexical error, `2` syntax error, `4` runtime error.

## The language

```
var
  integer a, b;
  string s;
main
  a = 5;
  while (a > 0) loop
    if (a % 2 == 0) then output("even") else output(a) end if
    a = a - 1;
  end loop
end
```

```
PROG       -> VDEC main STMTLIST end
VDEC       -> var VARS {VARS} | (empty)
VARS       -> TYPE id {, id} ;
TYPE       -> integer | string
STMTLIST   -> STMT {STMT} | (empty)
STMT       -> IFSTMT | WHILESTMT | ASSIGNSTMT | INPUTSTMT | OUTPUTSTMT
IFSTMT     -> if ( LOGEXPR ) then STMTLIST ELSEPART end if
ELSEPART   -> else STMTLIST | (empty)
WHILESTMT  -> while ( LOGEXPR ) loop STMTLIST end loop
ASSIGNSTMT -> id = ARITHEXPR ;  |  id = STRTERM ;     (type of id must match)
INPUTSTMT  -> input ( id )
OUTPUTSTMT -> output ( NUMTERM ) | output ( STRTERM )
LOGEXPR    -> RELEXPR {LOGICOP RELEXPR}
RELEXPR    -> ARITHEXPR RELOP ARITHEXPR
ARITHEXPR  -> NUMTERM {ARITHOP NUMTERM}
NUMTERM    -> number | integer-id
STRTERM    -> text | string-id
LOGICOP    -> and | or
RELOP      -> == | != | < | <= | > | >=
ARITHOP    -> + | - | * | / | %
```

- `input(...)` and `output(...)` are **not** followed by `;`; assignments are.
- Integers start at `0`, strings at `""`.
- Precedence, tightest first: `* / %`, `+ -`, comparisons, `and`, `or`.
- `input(...)` reads one whitespace-delimited word per variable.

## What was changed from the original files

`docs/changes-from-original.diff` has the full diff. In summary:

| File | Change |
|---|---|
| `LexAnalyzer.h`, `LexAnalyzer.cpp` | **None** (byte-identical). |
| `SyntaxAnalyzer.cpp` | The class definition moved, unchanged, into the new `SyntaxAnalyzer.h` (it had no header, so `main.cpp` couldn't use it). The demo `main()` was removed. **One bug fix:** `inputstmt()` now consumes the closing `)` of `input(x)`; before, any program using `input` failed to parse. |
| `SyntaxAnalyzer.h` | New: the include guard plus the original class definition. |
| `main.cpp` | Everything up to and including `dump()` is unchanged. Some `#include`s and a small terminal-detection helper (`isatty`) were added at the top, and the old `main()` (which read `data12.txt` / `vars12.txt`) was replaced by the driver. |

What the driver does around the original code:

- Passes the lexer's output to the parser and compiler through string streams,
  and embeds the language's token/lexeme table (the old `lexemes.txt`).
- Captures the text the lexer and parser print, shows it only on errors or with
  `-v`, and converts the parser's `name : type` symbol lines into the
  `name type` lines the compiler reads.
- Clears the compiler's global state between programs so an interactive session
  can run several.
- Strips `\r` and turns tabs into spaces before scanning (the scanner only
  splits words on spaces, so files saved on Windows or indented with tabs failed).
- Catches exceptions during `run()` (e.g. `stoi` on a non-numeric `input`).

## Known limitations of the original code (left as-is)

These come from the original parts and were deliberately not changed:

- **Parser:** string comparisons (`if (s == "x")`) and string concatenation
  (`s = a + b;`) are rejected, since the parser's grammar predates the final
  assignment's `STREXPR` rule. The code generator already evaluates them.
- **Parser:** error messages give only the token/lexeme pair, with no line number.
- **Code generator:** division or modulo by zero crashes the process.
- **Code generator:** an integer expression that mixes in strings is not
  supported; typing a non-number for an integer `input` gives `Runtime error: stoi`.
- **Lexer:** escape sequences in strings are kept literally (`"\""` prints
  with its backslashes).

## Tests

`make test` runs every `.mini` file in `examples/` and `tests/cases/` and
compares output and exit code with the files beside it (`.out`, optional `.in`
for stdin, optional `.code`), then runs a set of interactive-mode sessions piped
in. `tests/run_tests.sh --update` regenerates the `.out` files; review the diff
before committing.

## Credits

Built as a group project for a Programming Languages course. Names from the
original source headers: Daniel McCarthy and Jonathan Gomez (syntax analyzer);
Aron Bartoszek, Adam Stahly, Daniel McCarthy and Nico Ruiz (code generation).
