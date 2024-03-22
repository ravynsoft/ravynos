#!/usr/bin/env python3

import argparse
import os
import platform
import subprocess

# This list contains symbols that _might_ be exported for some platforms
PLATFORM_SYMBOLS = [
    '_GLOBAL_OFFSET_TABLE_',
    '__bss_end__',
    '__bss_start__',
    '__bss_start',
    '__cxa_guard_abort',
    '__cxa_guard_acquire',
    '__cxa_guard_release',
    '__cxa_allocate_dependent_exception',
    '__cxa_allocate_exception',
    '__cxa_begin_catch',
    '__cxa_call_unexpected',
    '__cxa_current_exception_type',
    '__cxa_current_primary_exception',
    '__cxa_decrement_exception_refcount',
    '__cxa_deleted_virtual',
    '__cxa_demangle',
    '__cxa_end_catch',
    '__cxa_free_dependent_exception',
    '__cxa_free_exception',
    '__cxa_get_exception_ptr',
    '__cxa_get_globals',
    '__cxa_get_globals_fast',
    '__cxa_increment_exception_refcount',
    '__cxa_new_handler',
    '__cxa_pure_virtual',
    '__cxa_rethrow',
    '__cxa_rethrow_primary_exception',
    '__cxa_terminate_handler',
    '__cxa_throw',
    '__cxa_uncaught_exception',
    '__cxa_uncaught_exceptions',
    '__cxa_unexpected_handler',
    '__dynamic_cast',
    '__emutls_get_address',
    '__gxx_personality_v0',
    '__end__',
    '__odr_asan._glapi_Context',
    '__odr_asan._glapi_Dispatch',
    '_bss_end__',
    '_edata',
    '_end',
    '_fini',
    '_init',
    '_fbss',
    '_fdata',
    '_ftext',
]

def get_symbols_nm(nm, lib):
    '''
    List all the (non platform-specific) symbols exported by the library
    using `nm`
    '''
    symbols = []
    platform_name = platform.system()
    output = subprocess.check_output([nm, '-gP', lib],
                                     stderr=open(os.devnull, 'w')).decode("ascii")
    for line in output.splitlines():
        fields = line.split()
        if len(fields) == 2 or fields[1] == 'U':
            continue
        symbol_name = fields[0]
        if platform_name == 'Linux' or platform_name == 'GNU' or platform_name.startswith('GNU/'):
            if symbol_name in PLATFORM_SYMBOLS:
                continue
        elif platform_name == 'Darwin':
            assert symbol_name[0] == '_'
            symbol_name = symbol_name[1:]
        symbols.append(symbol_name)
    return symbols


def get_symbols_dumpbin(dumpbin, lib):
    '''
    List all the (non platform-specific) symbols exported by the library
    using `dumpbin`
    '''
    symbols = []
    output = subprocess.check_output([dumpbin, '/exports', lib],
                                     stderr=open(os.devnull, 'w')).decode("ascii")
    for line in output.splitlines():
        fields = line.split()
        # The lines with the symbols are made of at least 4 columns; see details below
        if len(fields) < 4:
            continue
        try:
            # Making sure the first 3 columns are a dec counter, a hex counter
            # and a hex address
            _ = int(fields[0], 10)
            _ = int(fields[1], 16)
            _ = int(fields[2], 16)
        except ValueError:
            continue
        symbol_name = fields[3]
        # De-mangle symbols
        if symbol_name[0] == '_' and '@' in symbol_name:
            symbol_name = symbol_name[1:].split('@')[0]
        symbols.append(symbol_name)
    return symbols


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--symbols-file',
                        action='store',
                        required=True,
                        help='path to file containing symbols')
    parser.add_argument('--lib',
                        action='store',
                        required=True,
                        help='path to library')
    parser.add_argument('--nm',
                        action='store',
                        help='path to binary (or name in $PATH)')
    parser.add_argument('--dumpbin',
                        action='store',
                        help='path to binary (or name in $PATH)')
    parser.add_argument('--ignore-symbol',
                        action='append',
                        help='do not process this symbol')
    args = parser.parse_args()

    try:
        if platform.system() == 'Windows':
            if not args.dumpbin:
                parser.error('--dumpbin is mandatory')
            lib_symbols = get_symbols_dumpbin(args.dumpbin, args.lib)
        else:
            if not args.nm:
                parser.error('--nm is mandatory')
            lib_symbols = get_symbols_nm(args.nm, args.lib)
    except:
        # We can't run this test, but we haven't technically failed it either
        # Return the GNU "skip" error code
        exit(77)
    mandatory_symbols = []
    optional_symbols = []
    with open(args.symbols_file) as symbols_file:
        qualifier_optional = '(optional)'
        for line in symbols_file.readlines():

            # Strip comments
            line = line.split('#')[0]
            line = line.strip()
            if not line:
                continue

            # Line format:
            # [qualifier] symbol
            qualifier = None
            symbol = None

            fields = line.split()
            if len(fields) == 1:
                symbol = fields[0]
            elif len(fields) == 2:
                qualifier = fields[0]
                symbol = fields[1]
            else:
                print(args.symbols_file + ': invalid format: ' + line)
                exit(1)

            # The only supported qualifier is 'optional', which means the
            # symbol doesn't have to be exported by the library
            if qualifier and not qualifier == qualifier_optional:
                print(args.symbols_file + ': invalid qualifier: ' + qualifier)
                exit(1)

            if qualifier == qualifier_optional:
                optional_symbols.append(symbol)
            else:
                mandatory_symbols.append(symbol)

    unknown_symbols = []
    for symbol in lib_symbols:
        if symbol in mandatory_symbols:
            continue
        if symbol in optional_symbols:
            continue
        if args.ignore_symbol and symbol in args.ignore_symbol:
            continue
        if symbol[:2] == '_Z':
            # As ajax found out, the compiler intentionally exports symbols
            # that we explicitly asked it not to export, and we can't do
            # anything about it:
            # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=36022#c4
            continue
        unknown_symbols.append(symbol)

    missing_symbols = [
        sym for sym in mandatory_symbols if sym not in lib_symbols
    ]

    for symbol in unknown_symbols:
        print(args.lib + ': unknown symbol exported: ' + symbol)

    for symbol in missing_symbols:
        print(args.lib + ': missing symbol: ' + symbol)

    if unknown_symbols or missing_symbols:
        exit(1)
    exit(0)


if __name__ == '__main__':
    main()
