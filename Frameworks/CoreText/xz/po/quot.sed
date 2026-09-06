# Sed script that converts quotations, by replacing ASCII quotation marks
# with Unicode quotation marks.
#
# Copyright (C) 2001 Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.
# This file is offered as-is, without any warranty.
#
# Written by Bruno Haible <bruno@clisp.org>, 2001.
#
s/"\([^"]*\)"/“\1”/g
s/`\([^`']*\)'/‘\1’/g
s/ '\([^`']*\)' / ‘\1’ /g
s/ '\([^`']*\)'$/ ‘\1’/g
s/^'\([^`']*\)' /‘\1’ /g
s/“”/""/g
