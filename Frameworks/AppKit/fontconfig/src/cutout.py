import argparse
import subprocess
import json
import os
import re

if __name__== '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('input')
    parser.add_argument('output')
    parser.add_argument('buildroot')

    args = parser.parse_known_args()

    # c_args might contain things that are essential for crosscompilation, but
    # are not included in cc.cmd_array(), so we have to look them up ourselves
    host_cargs = []
    buildroot = args[0].buildroot
    with open(os.path.join(buildroot, 'meson-info', 'intro-buildoptions.json')) as json_file:
        bopts = json.load(json_file)
        for opt in bopts:
            if opt['name'] == 'c_args' and opt['section'] == 'compiler' and opt['machine'] == 'host':
                host_cargs = opt['value']
                break

    cpp = args[1]
    cpp_args = [i for i in host_cargs + [args[0].input] if not i.startswith('-g')]
    ret = subprocess.run(cpp + cpp_args, stdout=subprocess.PIPE, check=True)

    stdout = ret.stdout.decode('utf8')

    with open(args[0].output, 'w') as out:
        write = True
        for l in stdout.split('\n'):
            l = l.strip('\r')
            if l.startswith('CUT_OUT_BEGIN'):
                write = False

            if write and l:
                stripped = re.sub('^\s+', '', l)
                stripped = re.sub('\s*,\s*', ',', stripped)
                if not stripped.isspace() and stripped:
                    out.write('%s\n' % stripped)

            if l.startswith('CUT_OUT_END'):
                write = True
