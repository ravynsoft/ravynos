#!/usr/bin/env python3
#
# fontconfig/fc-lang/fc-lang.py
#
# Copyright © 2001-2002 Keith Packard
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

# fc-lang
#
# Read a set of language orthographies and build C declarations for
# charsets which can then be used to identify which languages are
# supported by a given font.
#
# TODO: this code is not very pythonic, a lot of it is a 1:1 translation
# of the C code and we could probably simplify it a bit
import argparse
import string
import sys
import os

# we just store the leaves in a dict, we can order the leaves later if needed
class CharSet:
    def __init__(self):
        self.leaves = {} # leaf_number -> leaf data (= 16 uint32)

    def add_char(self, ucs4):
        assert ucs4 < 0x01000000
        leaf_num = ucs4 >> 8
        if leaf_num in self.leaves:
            leaf = self.leaves[leaf_num]
        else:
            leaf = [0, 0, 0, 0, 0, 0, 0, 0] # 256/32 = 8
            self.leaves[leaf_num] = leaf
        leaf[(ucs4 & 0xff) >> 5] |= (1 << (ucs4 & 0x1f))
        #print('{:08x} [{:04x}] --> {}'.format(ucs4, ucs4>>8, leaf))

    def del_char(self, ucs4):
        assert ucs4 < 0x01000000
        leaf_num = ucs4 >> 8
        if leaf_num in self.leaves:
            leaf = self.leaves[leaf_num]
            leaf[(ucs4 & 0xff) >> 5] &= ~(1 << (ucs4 & 0x1f))
            # We don't bother removing the leaf if it's empty */
            #print('{:08x} [{:04x}] --> {}'.format(ucs4, ucs4>>8, leaf))

    def equals(self, other_cs):
        keys = sorted(self.leaves.keys())
        other_keys = sorted(other_cs.leaves.keys())
        if len(keys) != len(other_keys):
            return False
        for k1, k2 in zip(keys, other_keys):
            if k1 != k2:
                return False
            if not leaves_equal(self.leaves[k1], other_cs.leaves[k2]):
                return False
        return True

# Convert a file name into a name suitable for C declarations
def get_name(file_name):
    return file_name.split('.')[0]

# Convert a C name into a language name
def get_lang(c_name):
    return c_name.replace('_', '-').replace(' ', '').lower()

def read_orth_file(file_name):
    lines = []
    with open(file_name, 'r', encoding='utf-8') as orth_file:
        for num, line in enumerate(orth_file):
            if line.startswith('include '):
                include_fn = line[8:].strip()
                lines += read_orth_file(include_fn)
            else:
                # remove comments and strip whitespaces
                line = line.split('#')[0].strip()
                line = line.split('\t')[0].strip()
                # skip empty lines
                if line:
                    lines += [(file_name, num, line)]

    return lines

def leaves_equal(leaf1, leaf2):
    for v1, v2 in zip(leaf1, leaf2):
        if v1 != v2:
            return False
    return True

# Build a single charset from a source file
#
# The file format is quite simple, either
# a single hex value or a pair separated with a dash
def parse_orth_file(file_name, lines):
    charset = CharSet()
    for fn, num, line in lines:
        delete_char = line.startswith('-')
        if delete_char:
            line = line[1:]
        if line.find('-') != -1:
            parts = line.split('-')
        elif line.find('..') != -1:
            parts = line.split('..')
        else:
            parts = [line]

        start = int(parts.pop(0), 16)
        end = start
        if parts:
            end = int(parts.pop(0), 16)
        if parts:
            print('ERROR: {} line {}: parse error (too many parts)'.format(fn, num))

        for ucs4 in range(start, end+1):
            if delete_char:
                charset.del_char(ucs4)
            else:
                charset.add_char(ucs4)

    assert charset.equals(charset) # sanity check for the equals function

    return charset

if __name__=='__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('orth_files', nargs='+', help='List of .orth files')
    parser.add_argument('--directory', dest='directory', default=None)
    parser.add_argument('--template', dest='template_file', default=None)
    parser.add_argument('--output', dest='output_file', default=None)

    args = parser.parse_args()

    sets = []
    names = []
    langs = []
    country = []

    total_leaves = 0

    LangCountrySets = {}

    # Open output file
    if args.output_file:
        sys.stdout = open(args.output_file, 'w', encoding='utf-8')

    # Read the template file
    if args.template_file:
        tmpl_file = open(args.template_file, 'r', encoding='utf-8')
    else:
        tmpl_file = sys.stdin

    # Change into source dir if specified (after opening other files)
    if args.directory:
        os.chdir(args.directory)

    orth_entries = {}
    for i, fn in enumerate(args.orth_files):
        orth_entries[fn] = i

    for fn in sorted(orth_entries.keys()):
        lines = read_orth_file(fn)
        charset = parse_orth_file(fn, lines)

        sets.append(charset)

        name = get_name(fn)
        names.append(name)

        lang = get_lang(name)
        langs.append(lang)
        if lang.find('-') != -1:
            country.append(orth_entries[fn]) # maps to original index
            language_family = lang.split('-')[0]
            if not language_family in LangCountrySets:
              LangCountrySets[language_family] = []
            LangCountrySets[language_family] += [orth_entries[fn]]

        total_leaves += len(charset.leaves)

    # Find unique leaves
    leaves = []
    for s in sets:
       for leaf_num in sorted(s.leaves.keys()):
           leaf = s.leaves[leaf_num]
           is_unique = True
           for existing_leaf in leaves:
               if leaves_equal(leaf, existing_leaf):
                  is_unique = False
                  break
           #print('unique: ', is_unique)
           if is_unique:
               leaves.append(leaf)

    # Find duplicate charsets
    duplicate = []
    for i, s in enumerate(sets):
        dup_num = None
        if i >= 1:
            for j, s_cmp in enumerate(sets):
                if j >= i:
                    break
                if s_cmp.equals(s):
                    dup_num = j
                    break

        duplicate.append(dup_num)

    tn = 0
    off = {}
    for i, s in enumerate(sets):
        if duplicate[i]:
            continue
        off[i] = tn
        tn += len(s.leaves)

    # Scan the input until the marker is found
    # FIXME: this is a bit silly really, might just as well hardcode
    #        the license header in the script and drop the template
    for line in tmpl_file:
        if line.strip() == '@@@':
            break
        print(line, end='')

    print('/* total size: {} unique leaves: {} */\n'.format(total_leaves, len(leaves)))

    print('#define LEAF0       ({} * sizeof (FcLangCharSet))'.format(len(sets)))
    print('#define OFF0        (LEAF0 + {} * sizeof (FcCharLeaf))'.format(len(leaves)))
    print('#define NUM0        (OFF0 + {} * sizeof (uintptr_t))'.format(tn))
    print('#define SET(n)      (n * sizeof (FcLangCharSet) + offsetof (FcLangCharSet, charset))')
    print('#define OFF(s,o)    (OFF0 + o * sizeof (uintptr_t) - SET(s))')
    print('#define NUM(s,n)    (NUM0 + n * sizeof (FcChar16) - SET(s))')
    print('#define LEAF(o,l)   (LEAF0 + l * sizeof (FcCharLeaf) - (OFF0 + o * sizeof (intptr_t)))')
    print('#define fcLangCharSets (fcLangData.langCharSets)')
    print('#define fcLangCharSetIndices (fcLangData.langIndices)')
    print('#define fcLangCharSetIndicesInv (fcLangData.langIndicesInv)')

    assert len(sets) < 256 # FIXME: need to change index type to 16-bit below then

    print('''
static const struct {{
    FcLangCharSet  langCharSets[{}];
    FcCharLeaf     leaves[{}];
    uintptr_t      leaf_offsets[{}];
    FcChar16       numbers[{}];
    {}       langIndices[{}];
    {}       langIndicesInv[{}];
}} fcLangData = {{'''.format(len(sets), len(leaves), tn, tn,
                             'FcChar8 ', len(sets), 'FcChar8 ', len(sets)))

    # Dump sets
    print('{')
    for i, s in enumerate(sets):
        if duplicate[i]:
            j = duplicate[i]
        else:
            j = i
        print('    {{ "{}",  {{ FC_REF_CONSTANT, {}, OFF({},{}), NUM({},{}) }} }}, /* {} */'.format(
		langs[i], len(sets[j].leaves), i, off[j], i, off[j], i))

    print('},')

    # Dump leaves
    print('{')
    for l, leaf in enumerate(leaves):
        print('    {{ {{ /* {} */'.format(l), end='')
        for i in range(0, 8): # 256/32 = 8
            if i % 4 == 0:
                print('\n   ', end='')
            print(' 0x{:08x},'.format(leaf[i]), end='')
        print('\n    } },')
    print('},')

    # Dump leaves
    print('{')
    for i, s in enumerate(sets):
        if duplicate[i]:
            continue

        print('    /* {} */'.format(names[i]))

        for n, leaf_num in enumerate(sorted(s.leaves.keys())):
            leaf = s.leaves[leaf_num]
            if n % 4 == 0:
                print('   ', end='')
            found = [k for k, unique_leaf in enumerate(leaves) if leaves_equal(unique_leaf,leaf)] 
            assert found, "Couldn't find leaf in unique leaves list!"
            assert len(found) == 1
            print(' LEAF({:3},{:3}),'.format(off[i], found[0]), end='')
            if n % 4 == 3:
                print('')
        if len(s.leaves) % 4 != 0:
            print('')

    print('},')
	
    print('{')
    for i, s in enumerate(sets):
        if duplicate[i]:
            continue

        print('    /* {} */'.format(names[i]))

        for n, leaf_num in enumerate(sorted(s.leaves.keys())):
            leaf = s.leaves[leaf_num]
            if n % 8 == 0:
                print('   ', end='')
            print(' 0x{:04x},'.format(leaf_num), end='')
            if n % 8 == 7:
                print('')
        if len(s.leaves) % 8 != 0:
            print('')

    print('},')

    # langIndices
    print('{')
    for i, s in enumerate(sets):
        fn = '{}.orth'.format(names[i])
        print('    {}, /* {} */'.format(orth_entries[fn], names[i]))
    print('},')

    # langIndicesInv
    print('{')
    for i, k in enumerate(orth_entries.keys()):
        name = get_name(k)
        idx = names.index(name)
        print('    {}, /* {} */'.format(idx, name))
    print('}')

    print('};\n')

    print('#define NUM_LANG_CHAR_SET	{}'.format(len(sets)))
    num_lang_set_map = (len(sets) + 31) // 32;
    print('#define NUM_LANG_SET_MAP	{}'.format(num_lang_set_map))

    # Dump indices with country codes
    assert len(country) > 0
    assert len(LangCountrySets) > 0
    print('')
    print('static const FcChar32 fcLangCountrySets[][NUM_LANG_SET_MAP] = {')
    for k in sorted(LangCountrySets.keys()):
        langset_map = [0] * num_lang_set_map # initialise all zeros
        for entries_id in LangCountrySets[k]:
            langset_map[entries_id >> 5] |= (1 << (entries_id & 0x1f))
        print('    {', end='')
        for v in langset_map:
            print(' 0x{:08x},'.format(v), end='')
        print(' }}, /* {} */'.format(k))

    print('};\n')
    print('#define NUM_COUNTRY_SET {}\n'.format(len(LangCountrySets)))

    # Find ranges for each letter for faster searching
    # Dump sets start/finish for the fastpath
    print('static const FcLangCharSetRange  fcLangCharSetRanges[] = {\n')
    for c in string.ascii_lowercase: # a-z
        start = 9999
        stop = -1
        for i, s in enumerate(sets):
            if names[i].startswith(c):
                start = min(start,i)
                stop = max(stop,i)
        print('    {{ {}, {} }}, /* {} */'.format(start, stop, c))
    print('};\n')

    # And flush out the rest of the input file
    for line in tmpl_file:
        print(line, end='')
    
    sys.stdout.flush()
