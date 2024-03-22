# BSD 3-Clause License
#
# Copyright (c) 2018, pandas
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Based on https://github.com/pydata/pydata-sphinx-theme

from docutils import nodes

import sphinx
from sphinx.ext.autosummary import autosummary_table
from sphinx.locale import admonitionlabels

import types

class BootstrapHTML5TranslatorMixin:
    def __init__(self, *args, **kwds):
        super().__init__(*args, **kwds)
        self.settings.table_style = "table"

    def starttag(self, *args, **kwargs):
        """ensure an aria-level is set for any heading role"""
        if kwargs.get("ROLE") == "heading" and "ARIA-LEVEL" not in kwargs:
            kwargs["ARIA-LEVEL"] = "2"
        return super().starttag(*args, **kwargs)

    def visit_admonition(self, node, name: str = '') -> None:
        admonitionclasses = {
            'attention': 'alert-primary',
            'caution':   'alert-secondary',
            'danger':    'alert-danger',
            'error':     'alert-danger',
            'hint':      'alert-secondary',
            'important': 'alert-primary',
            'note':      'alert-info',
            'seealso':   'alert-info',
            'tip':       'alert-info',
            'warning':   'alert-warning',
        }

        self.body.append(self.starttag(
            node, 'div', CLASS=('alert ' + admonitionclasses[name])))
        if name:
            self.body.append(
                  self.starttag(node, 'div', '', CLASS='h5'))
            self.body.append(str(admonitionlabels[name]))
            self.body.append('</div>')

    def visit_table(self, node):
        # init the attributes
        atts = {}

        self._table_row_indices.append(0)

        # get the classes
        classes = [cls.strip(" \t\n") for cls in self.settings.table_style.split(",")]

        # we're looking at the 'real_table', which is wrapped by an autosummary
        if isinstance(node.parent, autosummary_table):
            classes += ["autosummary"]

        # add the width if set in a style attribute
        if "width" in node:
            atts["style"] = f'width: {node["width"]}'

        # add specific class if align is set
        if "align" in node:
            classes.append(f'table-{node["align"]}')

        tag = self.starttag(node, "table", CLASS=" ".join(classes), **atts)
        self.body.append(tag)

def setup_translators(app):
    if app.builder.default_translator_class is None:
        return

    if not app.registry.translators.items():
        translator = types.new_class(
            "BootstrapHTML5Translator",
            (
                BootstrapHTML5TranslatorMixin,
                app.builder.default_translator_class,
            ),
            {},
        )
        app.set_translator(app.builder.name, translator, override=True)
    else:
        for name, klass in app.registry.translators.items():
            if app.builder.format != "html":
                # Skip translators that are not HTML
                continue

            translator = types.new_class(
                "BootstrapHTML5Translator",
                (
                    BootstrapHTML5TranslatorMixin,
                    klass,
                ),
                {},
            )
            app.set_translator(name, translator, override=True)

def setup(app):
    app.connect("builder-inited", setup_translators)
