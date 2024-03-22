# Copyright © 2020-2021 Collabora Ltd
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import os
import pathlib
from urllib.parse import urlparse

def create_redirect(dst):
    tpl = '<html><head><meta http-equiv="refresh" content="0; url={0}"><script>window.location.replace("{0}")</script></head></html>'
    return tpl.format(dst)

def create_redirects(app, exception):
    if exception is not None or not app.builder.name == 'html':
        return
    for src, dst in app.config.html_redirects:
        path = os.path.join(app.outdir, '{0}.html'.format(src))

        os.makedirs(os.path.dirname(path), exist_ok=True)

        if urlparse(dst).scheme == "":
            dst = pathlib.posixpath.relpath(dst, start=os.path.dirname(src))
            if not os.path.isfile(os.path.join(os.path.dirname(path), dst)):
                raise Exception('{0} does not exitst'.format(dst))

        with open(path, 'w') as f:
            f.write(create_redirect(dst))

def setup(app):
    app.add_config_value('html_redirects', [], '')
    app.connect('build-finished', create_redirects)
