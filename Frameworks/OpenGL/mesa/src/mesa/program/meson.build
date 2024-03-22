# Copyright © 2017 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

bison_command = []
if yacc_is_bison
  bison_command = [prog_bison, '-o', '@OUTPUT0@', '--defines=@OUTPUT1@', '@INPUT@']
else
  bison_command = [prog_bison, '-o', '@OUTPUT0@', '-H', '@OUTPUT1@', '@INPUT@']
endif

mesa_lex = custom_target(
  'mesa_lex',
  input : 'program_lexer.l',
  output : 'lex.yy.c',
  command : [prog_flex, '-o', '@OUTPUT@', '@INPUT@'],
)

program_parse_tab = custom_target(
  'program_parse_tab.[ch]',
  input : 'program_parse.y',
  output : ['program_parse.tab.c', 'program_parse.tab.h'],
  command : bison_command,
)
