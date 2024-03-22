#!/usr/bin/env python3
"""
Check for and replace aliases with their new names from vk.xml
"""

import argparse
import pathlib
import subprocess
import sys
import xml.etree.ElementTree as et

THIS_FILE = pathlib.Path(__file__)
CWD = pathlib.Path.cwd()

VK_XML = THIS_FILE.parent / 'vk.xml'
EXCLUDE_PATHS = [
    VK_XML.relative_to(CWD).as_posix(),

    # These files come from other repos, there's no point checking and
    # fixing them here as that would be overwritten in the next sync.
    'src/amd/vulkan/radix_sort/',
    'src/virtio/venus-protocol/',
]


def get_aliases(xml_file: pathlib.Path):
    """
    Get all the aliases defined in vk.xml
    """
    xml = et.parse(xml_file)

    for node in ([]
        + xml.findall('.//enum[@alias]')
        + xml.findall('.//type[@alias]')
        + xml.findall('.//command[@alias]')
    ):
        # Some renames only apply to some APIs
        if 'api' in node.attrib and 'vulkan' not in node.attrib['api'].split(','):
            continue

        yield node.attrib['name'], node.attrib['alias']


def remove_prefix(string: str, prefix: str):
    """
    Remove prefix if string starts with it, and return the full string
    otherwise.
    """
    if not string.startswith(prefix):
        return string
    return string[len(prefix):]


# Function from https://stackoverflow.com/a/312464
def chunks(lst: list, n: int):
    """
    Yield successive n-sized chunks from lst.
    """
    for i in range(0, len(lst), n):
        yield lst[i:i + n]


def main(paths: list[str]):
    """
    Entrypoint; perform the search for all the aliases and replace them.
    """
    def prepare_identifier(identifier: str) -> str:
        prefixes_seen = []
        for prefix in [
            # Various macros prepend these, so they will not appear in the code using them.
            # List generated using this command:
            #   $ prefixes=$(git grep -woiE 'VK_\w+_' -- src/ ':!src/vulkan/registry/' | cut -d: -f2 | sort -u)
            #   $ for prefix in $prefixes; do grep -q $prefix src/vulkan/registry/vk.xml && echo "'$prefix',"; done
            # (the second part eliminates prefixes used only in mesa code and not upstream)
            'VK_BLEND_FACTOR_',
            'VK_BLEND_OP_',
            'VK_BORDER_COLOR_',
            'VK_COMMAND_BUFFER_RESET_',
            'VK_COMMAND_POOL_RESET_',
            'VK_COMPARE_OP_',
            'VK_COMPONENT_SWIZZLE_',
            'VK_DESCRIPTOR_TYPE_',
            'VK_DRIVER_ID_',
            'VK_DYNAMIC_STATE_',
            'VK_ERROR_',
            'VK_FORMAT_',
            'VK_IMAGE_ASPECT_MEMORY_PLANE_',
            'VK_IMAGE_ASPECT_PLANE_',
            'VK_IMAGE_USAGE_',
            'VK_NV_',
            'VK_PERFORMANCE_COUNTER_UNIT_',
            'VK_PIPELINE_BIND_POINT_',
            'VK_SAMPLER_ADDRESS_MODE_',
            'VK_SHADER_STAGE_TESSELLATION_',
            'VK_SHADER_STAGE_',
            'VK_STENCIL_OP_',
            'VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_',
            'VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_',
            'VK_STRUCTURE_TYPE_',
            'VK_USE_PLATFORM_',
            'VK_VERSION_',

            # Many places use the identifier without the `vk` prefix
            # (eg. with the driver name as a prefix instead)
            'VK_',
            'Vk',
            'vk',
        ]:
            # The order matters!  A shorter substring will match before a longer
            # one, hiding its matches.
            for prefix_seen in prefixes_seen:
                assert not prefix.startswith(prefix_seen), f'{prefix_seen} must come before {prefix}'
            prefixes_seen.append(prefix)

            identifier = remove_prefix(identifier, prefix)

        return identifier

    aliases = {}
    for old_name, alias_for in get_aliases(VK_XML):
        old_name = prepare_identifier(old_name)
        alias_for = prepare_identifier(alias_for)
        aliases[old_name] = alias_for

    print(f'Found {len(aliases)} aliases in {VK_XML.name}')

    # Some aliases have aliases
    recursion_needs_checking = True
    while recursion_needs_checking:
        recursion_needs_checking = False
        for old, new in aliases.items():
            if new in aliases:
                aliases[old] = aliases[new]
                recursion_needs_checking = True

    # Doing the whole search in a single command breaks grep, so only
    # look for 500 aliases at a time. Searching them one at a time would
    # be extremely slow.
    files_with_aliases = set()
    for aliases_chunk in chunks([*aliases], 500):
        grep_cmd = [
            'git',
            'grep',
            '-rlP',
            '|'.join(aliases_chunk),
        ] + paths
        search_output = subprocess.run(
            grep_cmd,
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
        ).stdout.decode()
        files_with_aliases.update(search_output.splitlines())


    def file_matches_path(file: str, path: str) -> bool:
        # if path is a folder; match any file within
        if path.endswith('/') and file.startswith(path):
            return True
        return file == path

    for excluded_path in EXCLUDE_PATHS:
        files_with_aliases = {
            file for file in files_with_aliases
            if not file_matches_path(file, excluded_path)
        }

    if not files_with_aliases:
        print('No alias found in any file.')
        sys.exit(0)

    print(f'{len(files_with_aliases)} files contain aliases:')
    print('\n'.join(f'- {file}' for file in sorted(files_with_aliases)))

    command = [
        'sed',
        '-i',
        ";".join([f's/{old}/{new}/g' for old, new in aliases.items()]),
    ]
    command += files_with_aliases
    subprocess.check_call(command, stderr=subprocess.DEVNULL)
    print('All aliases have been replaced')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('paths',
                        nargs=argparse.ZERO_OR_MORE,
                        default=['src/'],
                        help='Limit script to these paths (default: `src/`)')
    args = parser.parse_args()
    main(**vars(args))
