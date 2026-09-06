# Sed script that inserts the file called HEADER before the header entry.
#
# Copyright (C) 2001 Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.
# This file is offered as-is, without any warranty.
#
# Written by Bruno Haible <bruno@clisp.org>, 2001.
#
# At each occurrence of a line starting with "msgid ", we execute the following
# commands. At the first occurrence, insert the file. At the following
# occurrences, do nothing. The distinction between the first and the following
# occurrences is achieved by looking at the hold space.
/^msgid /{
x
# Test if the hold space is empty.
s/m/m/
ta
# Yes it was empty. First occurrence. Read the file.
r HEADER
# Output the file's contents by reading the next line. But don't lose the
# current line while doing this.
g
N
bb
:a
# The hold space was nonempty. Following occurrences. Do nothing.
x
:b
}
