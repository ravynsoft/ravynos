#!/usr/bin/env python3
# Copyright © 2019, 2022 Intel Corporation
# SPDX-License-Identifier: MIT

from __future__ import annotations
import argparse
import copy
import intel_genxml
import pathlib
import xml.etree.ElementTree as et
import typing


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('files', nargs='*',
                        default=pathlib.Path(__file__).parent.glob('*.xml'),
                        type=pathlib.Path)
    parser.add_argument('--validate', action='store_true')
    parser.add_argument('--quiet', action='store_true')
    args: Args = parser.parse_args()

    for filename in args.files:
        if not args.quiet:
            print('Processing {}... '.format(filename), end='', flush=True)

        genxml = intel_genxml.GenXml(filename)

        if args.validate:
            assert genxml.is_equivalent_xml(genxml.sorted_copy()), \
                f'{filename} is invalid, run gen_sort_tags.py and commit that'
        else:
            genxml.sort()
            genxml.write_file()

        if not args.quiet:
            print('done.')


if __name__ == '__main__':
    main()
