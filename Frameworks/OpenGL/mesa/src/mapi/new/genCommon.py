#!/usr/bin/env python3

# (C) Copyright 2015, NVIDIA CORPORATION.
# All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# on the rights to use, copy, modify, merge, publish, distribute, sub
# license, and/or sell copies of the Software, and to permit persons to whom
# the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
# IBM AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
# Authors:
#    Kyle Brenneman <kbrenneman@nvidia.com>

import collections
import re
import sys
import xml.etree.ElementTree as etree

import os
GLAPI = os.path.join(os.path.dirname(__file__), "..", "glapi", "gen")
sys.path.insert(0, GLAPI)
import static_data

MAPI_TABLE_NUM_DYNAMIC = 4096

_LIBRARY_FEATURE_NAMES = {
    # libGL and libGLdiapatch both include every function.
    "gl" : None,
    "gldispatch" : None,
    "opengl" : frozenset(( "GL_VERSION_1_0", "GL_VERSION_1_1",
        "GL_VERSION_1_2", "GL_VERSION_1_3", "GL_VERSION_1_4", "GL_VERSION_1_5",
        "GL_VERSION_2_0", "GL_VERSION_2_1", "GL_VERSION_3_0", "GL_VERSION_3_1",
        "GL_VERSION_3_2", "GL_VERSION_3_3", "GL_VERSION_4_0", "GL_VERSION_4_1",
        "GL_VERSION_4_2", "GL_VERSION_4_3", "GL_VERSION_4_4", "GL_VERSION_4_5",
    )),
    "glesv1" : frozenset(("GL_VERSION_ES_CM_1_0", "GL_OES_point_size_array")),
    "glesv2" : frozenset(("GL_ES_VERSION_2_0", "GL_ES_VERSION_3_0",
            "GL_ES_VERSION_3_1", "GL_ES_VERSION_3_2",
    )),
}

def getFunctions(xmlFiles):
    """
    Reads an XML file and returns all of the functions defined in it.

    xmlFile should be the path to Khronos's gl.xml file. The return value is a
    sequence of FunctionDesc objects, ordered by slot number.
    """
    roots = [ etree.parse(xmlFile).getroot() for xmlFile in xmlFiles ]
    return getFunctionsFromRoots(roots)

def getFunctionsFromRoots(roots):
    functions = {}
    for root in roots:
        for func in _getFunctionList(root):
            functions[func.name] = func
    functions = functions.values()

    # Sort the function list by name.
    functions = sorted(functions, key=lambda f: f.name)

    # Lookup for fixed offset/slot functions and use it if available.
    # Assign a slot number to each function. This isn't strictly necessary,
    # since you can just look at the index in the list, but it makes it easier
    # to include the slot when formatting output.

    next_slot = 0
    for i in range(len(functions)):
        name = functions[i].name[2:]

        if name in static_data.offsets:
            functions[i] = functions[i]._replace(slot=static_data.offsets[name])
        elif not name.endswith("ARB") and name + "ARB" in static_data.offsets:
            functions[i] = functions[i]._replace(slot=static_data.offsets[name + "ARB"])
        elif not name.endswith("EXT") and name + "EXT" in static_data.offsets:
            functions[i] = functions[i]._replace(slot=static_data.offsets[name + "EXT"])
        else:
            functions[i] = functions[i]._replace(slot=next_slot)
            next_slot += 1

    return functions

def getExportNamesFromRoots(target, roots):
    """
    Goes through the <feature> tags from gl.xml and returns a set of OpenGL
    functions that a library should export.

    target should be one of "gl", "gldispatch", "opengl", "glesv1", or
    "glesv2".
    """
    featureNames = _LIBRARY_FEATURE_NAMES[target]
    if featureNames is None:
        return set(func.name for func in getFunctionsFromRoots(roots))

    names = set()
    for root in roots:
        features = []
        for featElem in root.findall("feature"):
            if featElem.get("name") in featureNames:
                features.append(featElem)
        for featElem in root.findall("extensions/extension"):
            if featElem.get("name") in featureNames:
                features.append(featElem)
        for featElem in features:
            for commandElem in featElem.findall("require/command"):
                names.add(commandElem.get("name"))
    return names

class FunctionArg(collections.namedtuple("FunctionArg", "type name")):
    @property
    def dec(self):
        """
        Returns a "TYPE NAME" string, suitable for a function prototype.
        """
        rv = str(self.type)
        if not rv.endswith("*"):
            rv += " "
        rv += self.name
        return rv

class FunctionDesc(collections.namedtuple("FunctionDesc", "name rt args slot")):
    def hasReturn(self):
        """
        Returns true if the function returns a value.
        """
        return (self.rt != "void")

    @property
    def decArgs(self):
        """
        Returns a string with the types and names of the arguments, as you
        would use in a function declaration.
        """
        if not self.args:
            return "void"
        else:
            return ", ".join(arg.dec for arg in self.args)

    @property
    def callArgs(self):
        """
        Returns a string with the names of the arguments, as you would use in a
        function call.
        """
        return ", ".join(arg.name for arg in self.args)

    @property
    def basename(self):
        assert self.name.startswith("gl")
        return self.name[2:]

def _getFunctionList(root):
    for elem in root.findall("commands/command"):
        yield _parseCommandElem(elem)

def _parseCommandElem(elem):
    protoElem = elem.find("proto")
    (rt, name) = _parseProtoElem(protoElem)

    args = []
    for ch in elem.findall("param"):
        # <param> tags have the same format as a <proto> tag.
        args.append(FunctionArg(*_parseProtoElem(ch)))
    func = FunctionDesc(name, rt, tuple(args), slot=None)

    return func

def _parseProtoElem(elem):
    # If I just remove the tags and string the text together, I'll get valid C code.
    text = _flattenText(elem)
    text = text.strip()
    m = re.match(r"^(.+)\b(\w+)(?:\s*\[\s*(\d*)\s*\])?$", text, re.S)
    if m:
        typename = _fixupTypeName(m.group(1))
        name = m.group(2)
        if m.group(3):
            # HACK: glPathGlyphIndexRangeNV defines an argument like this:
            # GLuint baseAndCount[2]
            # Convert it to a pointer and hope for the best.
            typename += "*"
        return (typename, name)
    else:
        raise ValueError("Can't parse element %r -> %r" % (elem, text))

def _flattenText(elem):
    """
    Returns the text in an element and all child elements, with the tags
    removed.
    """
    text = ""
    if elem.text is not None:
        text = elem.text
    for ch in elem:
        text += _flattenText(ch)
        if ch.tail is not None:
            text += ch.tail
    return text

def _fixupTypeName(typeName):
    """
    Converts a typename into a more consistent format.
    """

    rv = typeName.strip()

    # Replace "GLvoid" with just plain "void".
    rv = re.sub(r"\bGLvoid\b", "void", rv)

    # Remove the vendor suffixes from types that have a suffix-less version.
    rv = re.sub(r"\b(GLhalf|GLintptr|GLsizeiptr|GLint64|GLuint64)(?:ARB|EXT|NV|ATI)\b", r"\1", rv)

    rv = re.sub(r"\bGLDEBUGPROCKHR\b", "GLDEBUGPROC", rv)

    # Clear out any leading and trailing whitespace.
    rv = rv.strip()

    # Remove any whitespace before a '*'
    rv = re.sub(r"\s+\*", r"*", rv)

    # Change "foo*" to "foo *"
    rv = re.sub(r"([^\*])\*", r"\1 *", rv)

    # Condense all whitespace into a single space.
    rv = re.sub(r"\s+", " ", rv)

    return rv

