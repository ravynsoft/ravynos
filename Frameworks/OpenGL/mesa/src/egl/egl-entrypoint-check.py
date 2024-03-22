#!/usr/bin/env python3

import argparse
from generate.eglFunctionList import EGL_FUNCTIONS as GLVND_ENTRYPOINTS


PREFIX1 = 'EGL_ENTRYPOINT('
PREFIX2 = 'EGL_ENTRYPOINT2('
SUFFIX = ')'


# These entrypoints should *not* be in the GLVND entrypoints
GLVND_EXCLUDED_ENTRYPOINTS = [
        # EGL_KHR_debug
        'eglDebugMessageControlKHR',
        'eglQueryDebugKHR',
        'eglLabelObjectKHR',
    ]


def check_entrypoint_sorted(entrypoints):
    print('Checking that EGL API entrypoints are sorted...')

    for i, _ in enumerate(entrypoints):
        # Can't compare the first one with the previous
        if i == 0:
            continue
        if entrypoints[i - 1] > entrypoints[i]:
            print('ERROR: ' + entrypoints[i] + ' should come before ' + entrypoints[i - 1])
            exit(1)

    print('All good :)')


def check_glvnd_entrypoints(egl_entrypoints, glvnd_entrypoints):
    print('Checking the GLVND entrypoints against the plain EGL ones...')
    success = True

    for egl_entrypoint in egl_entrypoints:
        if egl_entrypoint in GLVND_EXCLUDED_ENTRYPOINTS:
            continue
        if egl_entrypoint not in glvnd_entrypoints:
            print('ERROR: ' + egl_entrypoint + ' is missing from the GLVND entrypoints (src/egl/generate/eglFunctionList.py)')
            success = False

    for glvnd_entrypoint in glvnd_entrypoints:
        if glvnd_entrypoint not in egl_entrypoints:
            print('ERROR: ' + glvnd_entrypoint + ' is missing from the plain EGL entrypoints (src/egl/main/eglentrypoint.h)')
            success = False

    for glvnd_entrypoint in GLVND_EXCLUDED_ENTRYPOINTS:
        if glvnd_entrypoint in glvnd_entrypoints:
            print('ERROR: ' + glvnd_entrypoint + ' is should *not* be in the GLVND entrypoints (src/egl/generate/eglFunctionList.py)')
            success = False

    if success:
        print('All good :)')
    else:
        exit(1)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('header')
    args = parser.parse_args()

    with open(args.header) as header:
        lines = header.readlines()

    entrypoints = []
    for line in lines:
        line = line.strip()
        if line.startswith(PREFIX1):
            assert line.endswith(SUFFIX)
            entrypoints.append(line[len(PREFIX1):-len(SUFFIX)])
        if line.startswith(PREFIX2):
            assert line.endswith(SUFFIX)
            entrypoint = line[len(PREFIX2):-len(SUFFIX)]
            entrypoint = entrypoint.split(',')[0].strip()
            entrypoints.append(entrypoint)

    check_entrypoint_sorted(entrypoints)

    glvnd_entrypoints = [x[0] for x in GLVND_ENTRYPOINTS]

    check_glvnd_entrypoints(entrypoints, glvnd_entrypoints)

if __name__ == '__main__':
    main()
