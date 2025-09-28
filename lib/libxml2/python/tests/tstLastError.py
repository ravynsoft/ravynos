#!/usr/bin/env python3
import sys, unittest

import setup_test
import libxml2

class TestCase(unittest.TestCase):

    def runTest(self):
        self.test1()
        self.test2()

    def setUp(self):
        libxml2.debugMemory(1)

    def tearDown(self):
        libxml2.cleanupParser()
        if libxml2.debugMemory(1) != 0:
            self.fail("Memory leak %d bytes" % (libxml2.debugMemory(1),))
        else:
            print("OK")

    def failUnlessXmlError(self,f,args,exc,domain,code,message,level,file,line):
        """Run function f, with arguments args and expect an exception exc;
        when the exception is raised, check the libxml2.lastError for
        expected values."""
        # disable the default error handler
        def noerr(ctx, str):
            pass
        # None is not acceptable as function.
        libxml2.registerErrorHandler(noerr,None)
        try:
            f(*args)
        except exc:
            e = libxml2.lastError()
            if e is None:
                self.fail("lastError not set")
            if 0:
                print("domain = ",e.domain())
                print("code = ",e.code())
                print("message =",repr(e.message()))
                print("level =",e.level())
                print("file =",e.file())
                print("line =",e.line())
                print()
            self.assertEqual(domain,e.domain())
            self.assertEqual(code,e.code())
            self.assertEqual(message,e.message())
            self.assertEqual(level,e.level())
            self.assertEqual(file,e.file())
            self.assertEqual(line,e.line())
        else:
            self.fail("exception %s should have been raised" % exc)

    def test1(self):
        """Test readFile with a file that does not exist"""
        self.failUnlessXmlError(libxml2.readFile,
                        ("dummy.xml",None,0),
                        libxml2.treeError,
                        domain=libxml2.XML_FROM_IO,
                        code=libxml2.XML_IO_LOAD_ERROR,
                        message='failed to load external entity "dummy.xml"\n',
                        level=libxml2.XML_ERR_WARNING,
                        file=None,
                        line=0)

    def test2(self):
        """Test a well-formedness error: we get the last error only"""
        s = "<x>\n<a>\n</x>"
        self.failUnlessXmlError(libxml2.readMemory,
                        (s,len(s),"dummy.xml",None,0),
                        libxml2.treeError,
                        domain=libxml2.XML_FROM_PARSER,
                        code=libxml2.XML_ERR_TAG_NAME_MISMATCH,
                        message='Opening and ending tag mismatch: a line 2 and x\n',
                        level=libxml2.XML_ERR_FATAL,
                        file='dummy.xml',
                        line=3)

if __name__ == "__main__":
    test = TestCase()
    test.setUp()
    test.test1()
    test.test2()
    test.tearDown()
