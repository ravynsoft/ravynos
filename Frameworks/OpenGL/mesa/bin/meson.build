# Copyright © 2017 Eric Engestrom

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

git_sha1_gen_py = files('git_sha1_gen.py')
gen_vs_module_defs_py = files('gen_vs_module_defs.py')
gen_vs_module_defs_normal_command = [
  prog_python, gen_vs_module_defs_py,
  '--in_file', '@INPUT@', '--out_file', '@OUTPUT@',
  '--compiler_abi', cc.get_argument_syntax(),
  '--compiler_id', cc.get_id(), '--cpu_family', host_machine.cpu_family()
]
symbols_check = find_program('symbols-check.py')
install_megadrivers_py = find_program('install_megadrivers.py')
