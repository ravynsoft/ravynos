#!/usr/bin/env python3
#
# this tests the Expand() API of the xmlTextReader interface
# this extract the Dragon bibliography entries from the XML specification
#
import setup_test
import libxml2
import os
import sys

# Memory debug specific
libxml2.debugMemory(1)

expect="""<bibl id="Aho" key="Aho/Ullman">Aho, Alfred V., 
Ravi Sethi, and Jeffrey D. Ullman.
<emph>Compilers:  Principles, Techniques, and Tools</emph>.
Reading:  Addison-Wesley, 1986, rpt. corr. 1988.</bibl>"""

basedir = os.path.dirname(os.path.realpath(__file__))
f = open(os.path.join(basedir, '../../test/valid/REC-xml-19980210.xml'), 'rb')
input = libxml2.inputBuffer(f)
reader = input.newTextReader("REC")
res=""
while reader.Read() > 0:
    while reader.Name() == 'bibl':
        node = reader.Expand()            # expand the subtree
        if node.xpathEval("@id = 'Aho'"): # use XPath on it
            res = res + node.serialize()
        if reader.Next() != 1:            # skip the subtree
            break;

if res != expect:
    print("Error: didn't get the expected output")
    print("got '%s'" % (res))
    print("expected '%s'" % (expect))
    

#
# cleanup
#
del input
del reader

# Memory debug specific
libxml2.cleanupParser()
if libxml2.debugMemory(1) == 0:
    print("OK")
else:
    print("Memory leak %d bytes" % (libxml2.debugMemory(1)))
