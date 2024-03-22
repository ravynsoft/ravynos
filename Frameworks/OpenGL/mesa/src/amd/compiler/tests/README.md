Tests are wrapped in a `BEGIN_TEST`/`END_TEST` and write data to the `output` file pointer. Tests have checks against the output. They are single line comments prefixed with certain characters:

- `!` fails the test if the current line does not match the pattern
- `>>` skips to the first line which matches the pattern, or fails the test if there is none
- `;` executes python code to extend the pattern syntax by inserting functions into the variable dictionary, fail the test, insert more checks or consume characters from the output

Before this prefix, there can be a `~` to only perform the check for certain
variants (a regex directly following the `~` is used).

# Pattern Syntax
Patterns can define variables which can be accessed in both python code and the pattern itself. These are useful for readability or dealing with unstable identifiers in the output. Variable identifiers are sequences of digits, ascii letters or `_` (though they cannot start with a digit).

- `\` can be used to match the following literal character without interpreting it.
- Most characters expect the same characters in the output.
- A sequence of spaces in the pattern expects a sequence of spaces or tabs in the output.
- A `#` in the pattern expects an unsigned integer in the output. The `#` can be followed by an identifier to store the integer in a variable.
- A `$` in the pattern stores the output until the first whitespace character into a variable.
- A `%` in the pattern followed by an identifier is the same as a `#` but it expects a `%` before the integer in the output. It basically matches a ACO temporary.
- A `@` calls a variable as a function. It can be followed by an argument string wrapped in `(` and `)`.

# Functions
- `s64`, `s96`, `s128`, `v2`, `v3`, etc, expand to a pattern which matches a disassembled instruction's definition or operand. It later checks that the size and alignment is what's expected.
- `match_func` expands to a sequence of `$` and inserts functions with expand to the extracted output
- `search_re` consumes the rest of the line and fails the test if the pattern is not found
