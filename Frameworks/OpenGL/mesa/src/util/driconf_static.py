#
# Copyright (C) 2021 Google, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

from mako.template import Template
from xml.etree import ElementTree
import argparse
import os

def dbg(str):
    if False:
        print(str)

cnt = 0
def cname(name):
    global cnt
    cnt = cnt + 1
    return name + '_' + str(cnt)

class Option(object):
    def __init__(self, xml):
        self.cname = cname('option')
        self.name = xml.attrib['name']
        self.value = xml.attrib['value']

class Application(object):
    def __init__(self, xml):
        self.cname = cname('application')
        self.name = xml.attrib['name']
        self.executable = xml.attrib.get('executable', None)
        self.executable_regexp = xml.attrib.get('executable_regexp', None)
        self.sha1 = xml.attrib.get('sha1', None)
        self.application_name_match = xml.attrib.get('application_name_match', None)
        self.application_versions = xml.attrib.get('application_versions', None)
        self.options = []

        for option in xml.findall('option'):
            self.options.append(Option(option))

class Engine(object):
    def __init__(self, xml):
        self.cname = cname('engine')
        self.engine_name_match = xml.attrib['engine_name_match']
        self.engine_versions = xml.attrib.get('engine_versions', None)
        self.options = []

        for option in xml.findall('option'):
            self.options.append(Option(option))

class Device(object):
    def __init__(self, xml):
        self.cname = cname('device')
        self.driver = xml.attrib.get('driver', None)
        self.device = xml.attrib.get('device', None)
        self.applications = []
        self.engines = []

        for application in xml.findall('application'):
            self.applications.append(Application(application))

        for engine in xml.findall('engine'):
            self.engines.append(Engine(engine))

class DriConf(object):
    def __init__(self, xmlpaths):
        self.devices = []
        for xmlpath in xmlpaths:
            root = ElementTree.parse(xmlpath).getroot()

            for device in root.findall('device'):
                self.devices.append(Device(device))


template = """\
/* Copyright (C) 2021 Google, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

struct driconf_option {
    const char *name;
    const char *value;
};

struct driconf_application {
    const char *name;
    const char *executable;
    const char *executable_regexp;
    const char *sha1;
    const char *application_name_match;
    const char *application_versions;
    unsigned num_options;
    const struct driconf_option *options;
};

struct driconf_engine {
    const char *engine_name_match;
    const char *engine_versions;
    unsigned num_options;
    const struct driconf_option *options;
};

struct driconf_device {
    const char *driver;
    const char *device;
    unsigned num_engines;
    const struct driconf_engine *engines;
    unsigned num_applications;
    const struct driconf_application *applications;
};

<%def name="render_options(cname, options)">
static const struct driconf_option ${cname}[] = {
%    for option in options:
    { .name = "${option.name}", .value = "${option.value}" },
%    endfor
};
</%def>

%for device in driconf.devices:
%    for engine in device.engines:
    ${render_options(engine.cname + '_options', engine.options)}
%    endfor

%if len(device.engines) > 0:
static const struct driconf_engine ${device.cname}_engines[] = {
%    for engine in device.engines:
    { .engine_name_match = "${engine.engine_name_match}",
%        if engine.engine_versions:
      .engine_versions = "${engine.engine_versions}",
%        endif
      .num_options = ${len(engine.options)},
      .options = ${engine.cname + '_options'},
    },
%    endfor
};
%endif

%    for application in device.applications:
    ${render_options(application.cname + '_options', application.options)}
%    endfor

%if len(device.applications) > 0:
static const struct driconf_application ${device.cname}_applications[] = {
%    for application in device.applications:
    { .name = "${application.name}",
%        if application.executable:
      .executable = "${application.executable}",
%        endif
%        if application.executable_regexp:
      .executable_regexp = "${application.executable_regexp}",
%        endif
%        if application.sha1:
      .sha1 = "${application.sha1}",
%        endif
%        if application.application_name_match:
      .application_name_match = "${application.application_name_match}",
%        endif
%        if application.application_versions:
      .application_versions = "${application.application_versions}",
%        endif
      .num_options = ${len(application.options)},
      .options = ${application.cname + '_options'},
    },
%    endfor
};
%endif

static const struct driconf_device ${device.cname} = {
%    if device.driver:
    .driver = "${device.driver}",
%    endif
%    if device.device:
    .device = "${device.device}",
%    endif
    .num_engines = ${len(device.engines)},
%    if len(device.engines) > 0:
    .engines = ${device.cname}_engines,
%    endif
    .num_applications = ${len(device.applications)},
%    if len(device.applications) > 0:
    .applications = ${device.cname}_applications,
%    endif
};
%endfor

static const struct driconf_device *driconf[] = {
%for device in driconf.devices:
    &${device.cname},
%endfor
};
"""

parser = argparse.ArgumentParser()
parser.add_argument('drirc',
                    nargs=argparse.ONE_OR_MORE,
                    help='drirc *.conf file(s) to statically include')
parser.add_argument('header',
                    help='C header file to output the static configuration to')
args = parser.parse_args()

xml = args.drirc
dst = args.header

with open(dst, 'w', encoding='utf-8') as f:
    f.write(Template(template).render(driconf=DriConf(xml)))

