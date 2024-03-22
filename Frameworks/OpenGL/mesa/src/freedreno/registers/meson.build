# Copyright © 2019 Google, Inc

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

xml_files = [
  'adreno.xml',
  'freedreno_copyright.xml',
  'rules-ng.xsd',
]

gen_header_py = files('gen_header.py')

freedreno_xml_header_files = []

foreach f : xml_files
  _name = f + '.h'
  freedreno_xml_header_files += custom_target(
    _name,
    input: [gen_header_py, f],
    output: _name,
    command: [prog_python, '@INPUT0@', '--rnn', rnn_src_path, '--xml', '@INPUT1@', 'c-defines'],
    capture: true,
  )

  _gzname = f + '.gz'
  custom_target(
    _gzname,
    input: f,
    output: _gzname,
    command: [prog_gzip, '-kc', '@INPUT@'],
    capture: true,
    install_dir: rnn_install_path,
    install: install_fd_decode_tools,
    build_by_default: install_fd_decode_tools,
  )
endforeach

subdir('adreno')
