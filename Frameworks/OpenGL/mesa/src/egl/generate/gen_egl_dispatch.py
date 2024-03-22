#!/usr/bin/env python3

# (C) Copyright 2016, NVIDIA CORPORATION.
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

"""
Generates dispatch functions for EGL.

The list of functions and arguments is read from the Khronos's XML files, with
additional information defined in the module eglFunctionList.
"""

import argparse
import collections
import eglFunctionList
import sys
import textwrap

import os
NEWAPI = os.path.join(os.path.dirname(__file__), "..", "..", "mapi", "new")
sys.path.insert(0, NEWAPI)
import genCommon

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("target", choices=("header", "source"),
            help="Whether to build the source or header file.")
    parser.add_argument("xml_files", nargs="+", help="The XML files with the EGL function lists.")

    args = parser.parse_args()

    xmlFunctions = genCommon.getFunctions(args.xml_files)
    xmlByName = dict((f.name, f) for f in xmlFunctions)
    functions = []
    for (name, eglFunc) in eglFunctionList.EGL_FUNCTIONS:
        func = xmlByName[name]
        eglFunc = fixupEglFunc(func, eglFunc)
        functions.append((func, eglFunc))

    # Sort the function list by name.
    functions = sorted(functions, key=lambda f: f[0].name)

    if args.target == "header":
        text = generateHeader(functions)
    elif args.target == "source":
        text = generateSource(functions)
    sys.stdout.write(text)

def fixupEglFunc(func, eglFunc):
    result = dict(eglFunc)
    if result.get("prefix") is None:
        result["prefix"] = ""

    if result.get("extension") is not None:
        text = "defined(" + result["extension"] + ")"
        result["extension"] = text

    if result["method"] in ("none", "custom"):
        return result

    if result["method"] not in ("display", "device", "current"):
        raise ValueError("Invalid dispatch method %r for function %r" % (result["method"], func.name))

    if func.hasReturn():
        if result.get("retval") is None:
            result["retval"] = getDefaultReturnValue(func.rt)

    return result

def generateHeader(functions):
    text = textwrap.dedent(r"""
    #ifndef G_EGLDISPATCH_STUBS_H
    #define G_EGLDISPATCH_STUBS_H

    #ifdef __cplusplus
    extern "C" {
    #endif

    #include <EGL/egl.h>
    #include <EGL/eglext.h>
    #include <EGL/eglmesaext.h>
    #include <EGL/eglext_angle.h>
    #include <GL/mesa_glinterop.h>
    #include "glvnd/libeglabi.h"

    """.lstrip("\n"))

    text += "enum {\n"
    for (func, eglFunc) in functions:
        text += generateGuardBegin(func, eglFunc)
        text += "    __EGL_DISPATCH_" + func.name + ",\n"
        text += generateGuardEnd(func, eglFunc)
    text += "    __EGL_DISPATCH_COUNT\n"
    text += "};\n"

    for (func, eglFunc) in functions:
        if eglFunc["inheader"]:
            text += generateGuardBegin(func, eglFunc)
            text += "{f.rt} EGLAPIENTRY {ex[prefix]}{f.name}({f.decArgs});\n".format(f=func, ex=eglFunc)
            text += generateGuardEnd(func, eglFunc)

    text += textwrap.dedent(r"""
    #ifdef __cplusplus
    }
    #endif
    #endif // G_EGLDISPATCH_STUBS_H
    """)
    return text

def generateSource(functions):
    # First, sort the function list by name.
    text = ""
    text += '#include "egldispatchstubs.h"\n'
    text += '#include "g_egldispatchstubs.h"\n'
    text += '#include <stddef.h>\n'
    text += "\n"

    for (func, eglFunc) in functions:
        if eglFunc["method"] not in ("custom", "none"):
            text += generateGuardBegin(func, eglFunc)
            text += generateDispatchFunc(func, eglFunc)
            text += generateGuardEnd(func, eglFunc)

    text += "\n"
    text += "const char * const __EGL_DISPATCH_FUNC_NAMES[__EGL_DISPATCH_COUNT + 1] = {\n"
    for (func, eglFunc) in functions:
        text += generateGuardBegin(func, eglFunc)
        text += '    "' + func.name + '",\n'
        text += generateGuardEnd(func, eglFunc)
    text += "    NULL\n"
    text += "};\n"

    text += "const __eglMustCastToProperFunctionPointerType __EGL_DISPATCH_FUNCS[__EGL_DISPATCH_COUNT + 1] = {\n"
    for (func, eglFunc) in functions:
        text += generateGuardBegin(func, eglFunc)
        if eglFunc["method"] != "none":
            text += "    (__eglMustCastToProperFunctionPointerType) " + eglFunc.get("prefix", "") + func.name + ",\n"
        else:
            text += "    NULL, // " + func.name + "\n"
        text += generateGuardEnd(func, eglFunc)
    text += "    NULL\n"
    text += "};\n"

    return text

def generateGuardBegin(func, eglFunc):
    ext = eglFunc.get("extension")
    if ext is not None:
        return "#if " + ext + "\n"
    else:
        return ""

def generateGuardEnd(func, eglFunc):
    if eglFunc.get("extension") is not None:
        return "#endif\n"
    else:
        return ""

def generateDispatchFunc(func, eglFunc):
    text = ""

    if eglFunc.get("static"):
        text += "static "
    elif eglFunc.get("public"):
        text += "PUBLIC "
    text += textwrap.dedent(
    r"""
    {f.rt} EGLAPIENTRY {ef[prefix]}{f.name}({f.decArgs})
    {{
        typedef {f.rt} EGLAPIENTRY (* _pfn_{f.name})({f.decArgs});
    """).lstrip("\n").format(f=func, ef=eglFunc)

    if func.hasReturn():
        text += "    {f.rt} _ret = {ef[retval]};\n".format(f=func, ef=eglFunc)

    text += "    _pfn_{f.name} _ptr_{f.name} = (_pfn_{f.name}) ".format(f=func)
    if eglFunc["method"] == "current":
        text += "__eglDispatchFetchByCurrent(__EGL_DISPATCH_{f.name});\n".format(f=func)

    elif eglFunc["method"] in ("display", "device"):
        if eglFunc["method"] == "display":
            lookupFunc = "__eglDispatchFetchByDisplay"
            lookupType = "EGLDisplay"
        else:
            assert eglFunc["method"] == "device"
            lookupFunc = "__eglDispatchFetchByDevice"
            lookupType = "EGLDeviceEXT"

        lookupArg = None
        for arg in func.args:
            if arg.type == lookupType:
                lookupArg = arg.name
                break
        if lookupArg is None:
            raise ValueError("Can't find %s argument for function %s" % (lookupType, func.name,))

        text += "{lookupFunc}({lookupArg}, __EGL_DISPATCH_{f.name});\n".format(
                f=func, lookupFunc=lookupFunc, lookupArg=lookupArg)
    else:
        raise ValueError("Unknown dispatch method: %r" % (eglFunc["method"],))

    text += "    if(_ptr_{f.name} != NULL) {{\n".format(f=func)
    text += "        "
    if func.hasReturn():
        text += "_ret = "
    text += "_ptr_{f.name}({f.callArgs});\n".format(f=func)
    text += "    }\n"

    if func.hasReturn():
        text += "    return _ret;\n"
    text += "}\n"
    return text

def getDefaultReturnValue(typename):
    if typename.endswith("*"):
        return "NULL"
    elif typename == "EGLDisplay":
        return "EGL_NO_DISPLAY"
    elif typename == "EGLContext":
        return "EGL_NO_CONTEXT"
    elif typename == "EGLSurface":
        return "EGL_NO_SURFACE"
    elif typename == "EGLBoolean":
        return "EGL_FALSE";

    return "0"

if __name__ == "__main__":
    main()

