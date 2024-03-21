#!/usr/bin/env python3

import sys, os
import libxml2


libxml2.debugMemory(1)
baseDir = os.path.join('msxsdtest', 'Particles')
filenames = os.listdir(baseDir)
mainXSD = str()
signature = str()
dictXSD = dict()

def gatherFiles():
    for file in filenames:
        if (file[-5] in ["a", "b", "c"]) and (file[-3:] == 'xsd'):
            # newfilename = string.replace(filename, ' ', '_')
            signature = file[:-5]
            mainXSD = signature + ".xsd"
            imports = []
            for sub in filenames:
                if (mainXSD != sub) and (sub[-3:] == 'xsd') and sub.startswith(signature):
                    imports.append(sub)
            if len(imports) != 0:
                dictXSD[mainXSD] = imports

def debugMsg(text):
    #pass
    print("DEBUG:", text)


def fixup():
    for mainXSD in dictXSD:
        debugMsg("fixing '%s'..." % mainXSD)
        schemaDoc = None
        xpmainCtx = None
        # Load the schema document.
        schemaFile = os.path.join(baseDir, mainXSD)
        schemaDoc = libxml2.parseFile(schemaFile)
        if (schemaDoc is None):
            print("ERROR: doc '%s' not found" % mainXSD)
            sys.exit(1)
        try:
            xpmainCtx = schemaDoc.xpathNewContext()
            xpmainCtx.xpathRegisterNs("xs", "http://www.w3.org/2001/XMLSchema")
            xpres = xpmainCtx.xpathEval("/xs:schema")
            if len(xpres) == 0:
                print("ERROR: doc '%s' has no <schema> element" % mainXSD)
                sys.exit(1)
            schemaElem = xpres[0]
            schemaNs = schemaElem.ns()
            # Select all <import>s.
            xpres = xpmainCtx.xpathEval("/xs:schema/xs:import")
            if len(xpres) != 0:
                for elem in xpres:
                    loc = elem.noNsProp("schemaLocation")
                    if (loc is not None):
                        debugMsg("  imports '%s'" % loc)
                        if loc in dictXSD[mainXSD]:
                            dictXSD[mainXSD].remove(loc)
            for loc in dictXSD[mainXSD]:
                # Read out the targetNamespace.
                impTargetNs = None
                impFile = os.path.join(baseDir, loc)
                impDoc = libxml2.parseFile(impFile)
                try:
                    xpimpCtx = impDoc.xpathNewContext()
                    try:
                        xpimpCtx.setContextDoc(impDoc)
                        xpimpCtx.xpathRegisterNs("xs", "http://www.w3.org/2001/XMLSchema")
                        xpres = xpimpCtx.xpathEval("/xs:schema")
                        impTargetNs = xpres[0].noNsProp("targetNamespace")
                    finally:
                        xpimpCtx.xpathFreeContext()
                finally:
                    impDoc.freeDoc()

                # Add the <import>.
                debugMsg("  adding <import namespace='%s' schemaLocation='%s'/>" % (impTargetNs, loc))
                newElem = schemaDoc.newDocNode(schemaNs, "import", None)
                if (impTargetNs is not None):
                    newElem.newProp("namespace", impTargetNs)
                newElem.newProp("schemaLocation", loc)
                if schemaElem.children is not None:
                    schemaElem.children.addPrevSibling(newElem)
                schemaDoc.saveFile(schemaFile)
        finally:
            xpmainCtx.xpathFreeContext()
            schemaDoc.freeDoc()

try:
    gatherFiles()
    fixup()
finally:
    libxml2.cleanupParser()
    if libxml2.debugMemory(1) != 0:
        print("Memory leak %d bytes" % (libxml2.debugMemory(1)))

