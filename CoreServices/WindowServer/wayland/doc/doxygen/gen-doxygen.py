#!/usr/bin/env python3

import argparse
import datetime
import errno
import os
import subprocess
import sys

# Custom configuration for each documentation format
doxygen_templates = {
    'xml': [
        'GENERATE_XML=YES\n',
        'XML_OUTPUT={format}/{section}\n',
        'INPUT= {files}\n',
    ],
    'html': [
        'GENERATE_HTML=YES\n',
        'HTML_OUTPUT={format}/{section}\n',
        'PROJECT_NAME=\"Wayland {section} API\"\n',
        'INPUT= {files}\n',
    ],
    'man': [
        'GENERATE_MAN=YES\n',
        'MAN_OUTPUT={format}\n',
        'MAN_SUBDIR=.\n',
        'JAVADOC_AUTOBRIEF=NO\n',
        'INPUT= {files}\n',
    ],
}

def load_doxygen_file(doxyfile):
    with open(doxyfile, 'r') as f:
        res = f.readlines()
    return res

def get_template(outformat):
    for (k,v) in doxygen_templates.items():
        if outformat.startswith(k):
            return v

def gen_doxygen_file(data, outformat, section, files):
    for l in get_template(outformat):
        data.append(l.format(format=outformat, section=section, files=' '.join(files)))
    return data

parser = argparse.ArgumentParser(description='Generate docs with Doxygen')
parser.add_argument('doxygen_file',
                    help='The doxygen file to use')
parser.add_argument('files',
                    help='The list of files to parse',
                    metavar='FILES',
                    nargs='+')
parser.add_argument('--builddir',
                    help='The build directory',
                    metavar='DIR',
                    default='.')
parser.add_argument('--section',
                    help='The section to build',
                    metavar='NAME',
                    default='Client')
parser.add_argument('--output-format',
                    help='The output format: xml, html, man',
                    metavar='FORMAT',
                    default='xml')
parser.add_argument('--stamp',
                    help='Stamp file to output',
                    metavar='STAMP_FILE',
                    nargs='?',
                    type=argparse.FileType('w'))

args = parser.parse_args()

# Merge the doxyfile with our custom templates
conf = load_doxygen_file(args.doxygen_file)
conf = gen_doxygen_file(conf, args.output_format, args.section, args.files)

# Doxygen is not clever enough to create the directories it
# needs beforehand
try:
    os.makedirs(os.path.join(args.builddir, args.output_format))
except OSError as e:
    if e.errno != errno.EEXIST:
        raise e

# Run Doxygen with the generated doxyfile
cmd = subprocess.Popen(['doxygen', '-'], stdin=subprocess.PIPE)
cmd.stdin.write(''.join(conf).encode('utf-8'))
cmd.stdin.close()
if cmd.wait() != 0:
    sys.exit(1)

# This is a bit of a hack; Doxygen will generate way more files than we
# want to install, but there's no way to know how many at configuration
# time. Since we want to install only the wl_* man pages anyway, we can
# delete the other files and let Meson install the whole man3 subdirectory
if args.output_format.startswith('man'):
    manpath = os.path.join(args.builddir, args.output_format)
    for filename in os.listdir(manpath):
        full_path = os.path.join(manpath, filename)
        if not filename.startswith('wl_'):
            os.remove(full_path)

if args.stamp:
   args.stamp.write(str(datetime.datetime.now()))
