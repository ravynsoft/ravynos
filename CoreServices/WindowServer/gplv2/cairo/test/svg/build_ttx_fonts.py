#!/usr/bin/env python3

# Build the ttx cairo svg test fonts from svg files
# The svg files use the naming convention
#  <font-name>.<char>.<test-name>.svg
# eg "circle.A.cx_cy_r.svg"
#
# <font-name> is use to create the name of the ttx font.
# <char> is a ascii hex character (uppercase) that the font fill map to the SVG description.
# <test-name> is a descriptive name of the SVG file and is not used to build the font…
#
# This script looks for all files matching the above pattern and
# creates one ttx font for each unique <font-name>. Each font will
# contain up to 16 characters. The SVG description of each character
# and the character that maps to the SVG description is obtained from
# the SVG file beginning with <font-name>.<char>.

import argparse
import os
import re
import sys
import xml.dom.minidom

TEMPLATE_FILE="svg-font-template.ttx"

glyph_names = {
    '0': 'zero',
    '1': 'one',
    '2': 'two',
    '3': 'three',
    '4': 'four',
    '5': 'five',
    '6': 'six',
    '7': 'seven',
    '8': 'eight',
    '9': 'nine',
    'A': 'A',
    'B': 'B',
    'C': 'C',
    'D': 'D',
    'E': 'E',
    'F': 'F'
}

# files is list of (char, filename)
def build_font(font_name, files, in_dir, out_dir, no_reformat):
    name = "cairo-svg-test-" + font_name
    doc = xml.dom.minidom.parse(os.path.join(in_dir, TEMPLATE_FILE))
    glyph_id = 1
    text_nl = doc.createTextNode('\n\n')
    for f in sorted(files):
        glyph_name = glyph_names[f[0]]

        glyph_order = doc.getElementsByTagName('GlyphOrder')[0]
        glyph_id_elem = doc.createElement('GlyphID')
        glyph_id_elem.setAttribute('id', str(glyph_id))
        glyph_id_elem.setAttribute('name', glyph_name)
        glyph_order.appendChild(glyph_id_elem)
        glyph_order.appendChild(text_nl)

        hmtx = doc.getElementsByTagName('hmtx')[0]
        mtx = doc.createElement('mtx')
        mtx.setAttribute('name', glyph_name)
        mtx.setAttribute('width', '1100')
        mtx.setAttribute('lsb', '0')
        hmtx.appendChild(mtx)

        cmap_format = doc.getElementsByTagName('cmap_format_4')[0]
        map = doc.createElement('map')
        map.setAttribute('code', hex(ord(f[0])))
        map.setAttribute('name', glyph_name)
        cmap_format.appendChild(map)

        glyf = doc.getElementsByTagName('glyf')[0]
        tt_glyph = doc.createElement('TTGlyph')
        tt_glyph.setAttribute('name', glyph_name)
        glyf.appendChild(tt_glyph)
        contour = doc.createElement('contour')
        tt_glyph.appendChild(contour)
        pt = doc.createElement('pt')
        pt.setAttribute('x', "0")
        pt.setAttribute('y', "0")
        pt.setAttribute('on', "1")
        contour.appendChild(pt)
        pt = doc.createElement('pt')
        pt.setAttribute('x', "0")
        pt.setAttribute('y', "1000")
        pt.setAttribute('on', "1")
        contour.appendChild(pt)
        pt = doc.createElement('pt')
        pt.setAttribute('x', "1000")
        pt.setAttribute('y', "1000")
        pt.setAttribute('on', "1")
        contour.appendChild(pt)
        pt = doc.createElement('pt')
        pt.setAttribute('x', "1000")
        pt.setAttribute('y', "0")
        pt.setAttribute('on', "1")
        contour.appendChild(pt)
        instructions = doc.createElement('instructions')
        tt_glyph.appendChild(instructions)

        svg = doc.getElementsByTagName('SVG')[0]
        svgdoc = doc.createElement('svgDoc')
        svgdoc.setAttribute('startGlyphID', str(glyph_id))
        svgdoc.setAttribute('endGlyphID', str(glyph_id))
        with open(os.path.join(in_dir, f[1]), 'r') as svg_file:
            svg_data = svg_file.read()
        cdata = doc.createCDATASection(svg_data)
        svgdoc.appendChild(cdata)
        svg.appendChild(svgdoc)
        glyph_id += 1

    name_record = doc.getElementsByTagName('namerecord')[0]
    name_record.firstChild.replaceWholeText(name.replace("-", " ").title())
    name_record = doc.getElementsByTagName('namerecord')[2]
    name_record.firstChild.replaceWholeText(name.replace("-", " ").title() + " Regular")

    ttx_filename = os.path.join(out_dir, name + '.ttx')
    ttf_filename = os.path.join(out_dir, name + '.ttf')
    with open(ttx_filename, 'w') as ttx_file:
        doc.writexml(ttx_file)

    if not no_reformat:
        # Convert to ttf and back to ttx. This reformats the ttx file
        # which allows better quality diffs.
        if os.path.exists(ttf_filename):
            os.remove(ttf_filename)
        os.system("ttx " + ttx_filename)
        os.remove(ttx_filename)
        os.system("ttx " + ttf_filename)
        os.remove(ttf_filename)

def build_file_list(in_dir):
    dict = {}
    regex_prog = re.compile(r"([^\.]+)\.(.)\.[^\.]+\.svg", re.ASCII)
    files = os.listdir(in_dir)
    for f in files:
        match = regex_prog.fullmatch(f)
        if match:
            fontname = match.group(1)
            character = match.group(2)
            if (fontname not in dict):
                dict[fontname] = [(character, f)];
            else:
                dict[fontname].append((character, f))
    return dict

if __name__=='__main__':
    parser = argparse.ArgumentParser(description='Build ttx fonts.')
    parser.add_argument("-i", nargs=1, metavar="indir", default=["."], help="Input directory")
    parser.add_argument("-o", nargs=1, metavar="outdir", default=["."], help="Output directory")
    parser.add_argument("-n", action='store_true', help="Don't reformat the output.")
    args = parser.parse_args()
    in_dir = args.i[0]
    out_dir = args.o[0]
    no_reformat = args.n
    file_list = build_file_list(in_dir)
    font_name = None
    for key, value in file_list.items():
        build_font(key, value, in_dir, out_dir, no_reformat)
