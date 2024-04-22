#!/usr/bin/env python3
#
# fontconfig/fc-case/fc-case.py
#
# Copyright © 2004 Keith Packard
# Copyright © 2019 Tim-Philipp Müller
#
# Permission to use, copy, modify, distribute, and sell this software and its
# documentation for any purpose is hereby granted without fee, provided that
# the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation, and that the name of the author(s) not be used in
# advertising or publicity pertaining to distribution of the software without
# specific, written prior permission.  The authors make no
# representations about the suitability of this software for any purpose.  It
# is provided "as is" without express or implied warranty.
#
# THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
# EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
# DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
# TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

from enum import Enum
import argparse
import string
import sys

class CaseFoldClass(Enum):
    COMMON = 1
    FULL = 2
    SIMPLE = 3
    TURKIC = 4

class CaseFoldMethod(Enum):
    RANGE = 0
    EVEN_ODD = 1
    FULL = 2

caseFoldClassMap = {
  'C' : CaseFoldClass.COMMON,
  'F' : CaseFoldClass.FULL,
  'S' : CaseFoldClass.SIMPLE,
  'T' : CaseFoldClass.TURKIC
}

folds = []

def ucs4_to_utf8(ucs4):
    utf8_rep = []
    
    if ucs4 < 0x80:
        utf8_rep.append(ucs4)
        bits = -6
    elif ucs4 < 0x800:
        utf8_rep.append(((ucs4 >> 6) & 0x1F) | 0xC0)
        bits = 0
    elif ucs4 < 0x10000:
        utf8_rep.append(((ucs4 >> 12) & 0x0F) | 0xE0)
        bits = 6
    elif ucs4 < 0x200000:
        utf8_rep.append(((ucs4 >> 18) & 0x07) | 0xF0)
        bits = 12
    elif ucs4 < 0x4000000:
        utf8_rep.append(((ucs4 >> 24) & 0x03) | 0xF8)
        bits = 18
    elif ucs4 < 0x80000000:
        utf8_rep.append(((ucs4 >> 30) & 0x01) | 0xFC)
        bits = 24
    else:
        return [];

    while bits >= 0:
        utf8_rep.append(((ucs4 >> bits) & 0x3F) | 0x80)
        bits-= 6

    return utf8_rep

def utf8_size(ucs4):
    return len(ucs4_to_utf8(ucs4))

case_fold_method_name_map = {
    CaseFoldMethod.RANGE: 'FC_CASE_FOLD_RANGE,',
    CaseFoldMethod.EVEN_ODD: 'FC_CASE_FOLD_EVEN_ODD,',
    CaseFoldMethod.FULL: 'FC_CASE_FOLD_FULL,',
}

if __name__=='__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('case_folding_file')
    parser.add_argument('--template', dest='template_file', default=None)
    parser.add_argument('--output', dest='output_file', default=None)

    args = parser.parse_args()

    minFoldChar = None
    maxFoldChar = None
    fold = None

    foldChars = []
    maxFoldChars = 0

    maxExpand = 0

    # Read the standard Unicode CaseFolding.txt file
    with open(args.case_folding_file, 'r', encoding='utf-8') as casefile:
        for cnt, line in enumerate(casefile):
            if not line or not line[0] in string.hexdigits:
                continue

            # print('Line {}: {}'.format(cnt, line.strip()))

            tokens = line.split('; ')

            if len(tokens) < 3:
                print('Not enough tokens in line {}'.format(cnt), file=sys.stderr)
                sys.exit(1)

            # Get upper case value
            upper = int(tokens.pop(0), 16)

            # Get class
            cfclass = caseFoldClassMap[tokens.pop(0)]

            # Get list of result characters
            lower = list(map(lambda s: int(s,16), tokens.pop(0).split()))

            # print('\t----> {:04X} {} {}'.format(upper, cfclass, lower))

            if not minFoldChar:
                minFoldChar = upper

            maxFoldChar = upper;

            if cfclass in [CaseFoldClass.COMMON, CaseFoldClass.FULL]:
                if len(lower) == 1:
                    # foldExtends
                    if fold and fold['method'] == CaseFoldMethod.RANGE:
                        foldExtends = (lower[0] - upper) == fold['offset'] and upper == fold['upper'] + fold['count']
                    elif fold and fold['method'] == CaseFoldMethod.EVEN_ODD:
                        foldExtends = (lower[0] - upper) == 1 and upper == (fold['upper'] + fold['count'] + 1)
                    else:
                        foldExtends = False

                    if foldExtends:
                        # This modifies the last fold item in the array too
                        fold['count'] = upper - fold['upper'] + 1;
                    else:
                        fold = {}
                        fold['upper'] = upper
                        fold['offset'] = lower[0] - upper;
                        if fold['offset'] == 1:
                            fold['method'] = CaseFoldMethod.EVEN_ODD
                        else:
                            fold['method'] = CaseFoldMethod.RANGE
                        fold['count'] = 1
                        folds.append(fold)
                    expand = utf8_size (lower[0]) - utf8_size(upper)
                else:
                    fold = {}
                    fold['upper'] = upper
                    fold['method'] = CaseFoldMethod.FULL
                    fold['offset'] = len(foldChars)

                    # add chars
                    for c in lower:
                        utf8_rep = ucs4_to_utf8(c)
                        # print('{} -> {}'.format(c,utf8_rep))
                        for utf8_char in utf8_rep:
                            foldChars.append(utf8_char)

                    fold['count'] = len(foldChars) - fold['offset']
                    folds.append(fold)

                    if fold['count'] > maxFoldChars:
                        maxFoldChars = fold['count']

                    expand = fold['count'] - utf8_size(upper)
                    if expand > maxExpand:
                        maxExpand = expand

    # Open output file
    if args.output_file:
        sys.stdout = open(args.output_file, 'w', encoding='utf-8')

    # Read the template file
    if args.template_file:
        tmpl_file = open(args.template_file, 'r', encoding='utf-8')
    else:
        tmpl_file = sys.stdin
    
    # Scan the input until the marker is found
    # FIXME: this is a bit silly really, might just as well harcode
    #        the license header in the script and drop the template
    for line in tmpl_file:
        if line.strip() == '@@@':
            break
        print(line, end='')
    
    # Dump these tables
    print('#define FC_NUM_CASE_FOLD\t{}'.format(len(folds)))
    print('#define FC_NUM_CASE_FOLD_CHARS\t{}'.format(len(foldChars)))
    print('#define FC_MAX_CASE_FOLD_CHARS\t{}'.format(maxFoldChars))
    print('#define FC_MAX_CASE_FOLD_EXPAND\t{}'.format(maxExpand))
    print('#define FC_MIN_FOLD_CHAR\t0x{:08x}'.format(minFoldChar))
    print('#define FC_MAX_FOLD_CHAR\t0x{:08x}'.format(maxFoldChar))
    print('')

    # Dump out ranges
    print('static const FcCaseFold    fcCaseFold[FC_NUM_CASE_FOLD] = {')
    for f in folds:
         short_offset = f['offset']
         if short_offset < -32367:
             short_offset += 65536
         if short_offset > 32368:
             short_offset -= 65536
         print('    {} 0x{:08x}, {:22s} 0x{:04x}, {:6d} {},'.format('{',
               f['upper'], case_fold_method_name_map[f['method']],
               f['count'], short_offset, '}'))
    print('};\n')

    # Dump out "other" values
    print('static const FcChar8\tfcCaseFoldChars[FC_NUM_CASE_FOLD_CHARS] = {')
    for n, c in enumerate(foldChars):
        if n == len(foldChars) - 1:
            end = ''
        elif n % 16 == 15:
            end = ',\n'
        else:
            end = ','
        print('0x{:02x}'.format(c), end=end)
    print('\n};')

    # And flush out the rest of the input file
    for line in tmpl_file:
        print(line, end='')
    
    sys.stdout.flush()
