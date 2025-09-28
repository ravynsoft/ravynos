# Test snippets

The files in this directory are used by the test suite to exercise various
behavior of Pod::Man or Pod::Text.  They use a pseudo-ini-file syntax with
free-form sections, normally an input and an output section and possibly
others.

Sections start with the section type in `[]`.  The contents are normally
free-form text.  The exception is an `[options]` section, where the
contents are key/value pairs, where the key is separated from the value
with whitespace.

Valid sections are:

```
    [name]
    The name of this test for status reporting
    
    [options]
    key value
    key value
    
    [input]
    POD input source.
    
    [output]
    The results of running some formatter on the input.
    
    [errors]
    Errors reported to standard error when running some formatter on the
    input.
    
    [exception]
    The text of an exception (with the file and line number information
    stripped) thrown by running some formatter on the input.
```

Files are organized into subdirectories named after the formatter, namely
man (Pod::Man), text (Pod::Text), color (Pod::Text::Color), overstrike
(Pod::Text::Overstrike), and termcap (Pod::Text::Termcap).

## Copyright and license

Copyright 2015, 2018, 2022 Russ Allbery <rra@cpan.org>

Copying and distribution of this file, with or without modification, are
permitted in any medium without royalty provided the copyright notice and
this notice are preserved.  This file is offered as-is, without any
warranty.

SPDX-License-Identifier: FSFAP
