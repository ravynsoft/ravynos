# Coding style

- Indentation in tabs, 8 characters wide, spaces after the tabs where
  vertical alignment is required (see below)

**Note: this file uses spaces due to markdown rendering issues for tabs.
  Code must be implemented using tabs.**

- Max line width 80ch, do not break up printed strings though

- Break up long lines at logical groupings, one line for each logical group

```c
int a = somelongname() +
        someotherlongname();

if (a < 0 &&
    (b > 20 & d < 10) &&
    d != 0.0)


somelongfunctioncall(arg1,
                     arg2,
                     arg3);
```

- Function declarations: return type on separate line, {} on separate line,
  arguments broken up as above.

```c
static inline int
foobar(int a, int b)
{

}

void
somenamethatiswaytoolong(int a,
                         int b,
                         int c)
{
}
```

- `/* comments only */`, no `// comments`

- `variable_name`, not `VariableName` or `variableName`. same for functions.

- no typedefs of structs, enums, unions

- if it generates a compiler warning, it needs to be fixed
- if it generates a static checker warning, it needs to be fixed or
  commented

- declare variables when they are used first and try to keep them as local as possible.
  Exception: basic loop variables, e.g. for (int i = 0; ...) should always be
  declared inside the loop even where multiple loops exist

```c
int a;

if (foo) {
        int b = 10;

        a = get_value();
        usevalue(a, b);
}

if (bar) {
        a = get_value();
        useit(a);
}

int c = a * 100;
useit(c);
```

- avoid uninitialized variables where possible, declare them late instead.
  Note that most of libinput predates this style, try to stick with the code
  around you if in doubt.

  wrong:

```c
        int *a;
        int b = 7;

        ... some code ...

        a = zalloc(32);
```

  right:

```c
        int b = 7;
        ... some code ...

        int *a = zalloc(32);
```

- avoid calling non-obvious functions inside declaration blocks for multiple
  variables.

  bad:

```c
{
        int a = 7;
        int b = some_complicated_function();
        int *c = zalloc(32);
}
```

  better:
```c
{
        int a = 7;
        int *c = zalloc(32);

        int b = some_complicated_function();
}
```

  There is a bit of gut-feeling involved with this, but the goal is to make
  the variable values immediately recognizable.

- Where statements are near-identical and repeated, try to keep them
  identical:

  bad:
```c
int a = get_some_value(x++);
do_something(a);
a = get_some_value(x++);
do_something(a);
a = get_some_value(x++);
do_something(a);
```
  better:

```c
int a;
a = = get_some_value(x++);
do_something(a);
a = get_some_value(x++);
do_something(a);
a = get_some_value(x++);
do_something(a);
```

- if/else: { on the same line, no curly braces if both blocks are a single
  statement. If either if or else block are multiple statements, both must
  have curly braces.

```c
if (foo) {
        blah();
        bar();
} else {
        a = 10;
}
```

- public functions MUST be doxygen-commented, use doxygen's `@foo` rather than
  `\foo` notation

- `#include "config.h"` comes first, followed by system headers, followed by
  external library headers, followed by internal headers.
  sort alphabetically where it makes sense (specifically system headers)

```c
#include "config.h"

#include <stdio.h>
#include <string.h>

#include <libevdev/libevdev.h>

#include "libinput-private.h"
```

- goto jumps only to the end of the function, and only for good reasons
  (usually cleanup). goto never jumps backwards

- Use stdbool.h's bool for booleans within the library (instead of `int`).
  Exception: the public API uses int, not bool.

# Git commit message requirements

Our CI will check the commit messages for a few requirements. Below is the
list of what we expect from a git commit.

## Commit message content

A [good commit message](http://who-t.blogspot.com/2009/12/on-commit-messages.html) needs to
answer three questions:

- Why is it necessary? It may fix a bug, it may add a feature, it may
  improve performance, reliabilty, stability, or just be a change for the
  sake of correctness.
- How does it address the issue? For short obvious patches this part can be
  omitted, but it should be a high level description of what the approach
  was.
- What effects does the patch have? (In addition to the obvious ones, this
  may include benchmarks, side effects, etc.)

These three questions establish the context for the actual code changes, put
reviewers and others into the frame of mind to look at the diff and check if
the approach chosen was correct. A good commit message also helps
maintainers to decide if a given patch is suitable for stable branches or
inclusion in a distribution.

## Commit message format

The canonical git commit message format is:

```
one line as the subject line with a high-level note

full explanation of the patch follows after an empty line. This explanation
can be multiple paragraphs and is largely free-form. Markdown is not
supported.

You can include extra data where required like:
- benchmark one says 10s
- benchmark two says 12s
```

The subject line is the first thing everyone sees about this commit, so make
sure it's on point.

## Commit message technical requirements

- The commit message should use present tense (not past tense). Do write
  "change foo to bar", not "changed foo to bar".
- The text width of the commit should be 78 chars or less, especially the
  subject line.
- The author must be the name you usually identify as and email address. We do
  not accept the default `@users.noreply` gitlab addresses.
  ```
  git config --global user.name Your Name
  git config --global user.email your@email
  ```
