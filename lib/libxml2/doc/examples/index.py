#!/usr/bin/env python3
#
# Indexes the examples and build an XML description
#
import glob
import sys
try:
    import libxml2
except:
    print("libxml2 python bindings not available")
    sys.exit(1)
sys.path.insert(0, "..")
from apibuild import CParser, escape

examples = []
extras = ['examples.xsl', 'index.html', 'index.py']
tests = []
sections = {}
symbols = {}
api_dict = None
api_doc = None

def load_api():
    global api_dict
    global api_doc

    if api_dict != None:
        return
    api_dict = {}
    try:
        print("loading ../libxml2-api.xml")
        api_doc = libxml2.parseFile("../libxml2-api.xml")
    except:
        print("failed to parse ../libxml2-api.xml")
        sys.exit(1)

def find_symbol(name):
    global api_dict
    global api_doc

    if api_doc == None:
        load_api()

    if name == None:
        return
    if name in api_dict:
        return api_dict[name]
    ctxt = api_doc.xpathNewContext()
    res = ctxt.xpathEval("/api/symbols/*[@name = '%s']" % (name))
    if type(res) == type([]) and len(res) >= 1:
        if len(res) > 1:
            print("Found %d references to %s in the API" % (len(res), name))
        node = res[0]
        typ = node.name
        file = node.xpathEval("string(@file)")
        info = node.xpathEval("string(info)")
    else:
        print("Reference %s not found in the API" % (name))
        return None
    ret = (typ, file, info)
    api_dict[name] = ret
    return ret

def parse_top_comment(filename, comment):
    res = {}
    lines = comment.split("\n")
    item = None
    for line in lines:
        while line != "" and (line[0] == ' ' or line[0] == '\t'):
            line = line[1:]
        while line != "" and line[0] == '*':
            line = line[1:]
        while line != "" and (line[0] == ' ' or line[0] == '\t'):
            line = line[1:]
        try:
            (it, line) = line.split(":", 1)
            item = it
            while line != "" and (line[0] == ' ' or line[0] == '\t'):
                line = line[1:]
            if item in res:
                res[item] = res[item] + " " + line
            else:
                res[item] = line
        except:
            if item != None:
                if item in res:
                    res[item] = res[item] + " " + line
                else:
                    res[item] = line
    return res

def parse(filename, output):
    global symbols
    global sections

    parser = CParser(filename)
    parser.collect_references()
    idx = parser.parse()
    info = parse_top_comment(filename, parser.top_comment)
    output.write("  <example filename='%s'>\n" % filename)
    try:
        synopsis = info['synopsis']
        output.write("    <synopsis>%s</synopsis>\n" % escape(synopsis));
    except:
        print("Example %s lacks a synopsis description" % (filename))
    try:
        purpose = info['purpose']
        output.write("    <purpose>%s</purpose>\n" % escape(purpose));
    except:
        print("Example %s lacks a purpose description" % (filename))
    try:
        usage = info['usage']
        output.write("    <usage>%s</usage>\n" % escape(usage));
    except:
        print("Example %s lacks an usage description" % (filename))
    try:
        test = info['test']
        output.write("    <test>%s</test>\n" % escape(test));
        progname=filename[0:-2]
        command=test.replace(progname, './' + progname, 1)
        tests.append(command)
    except:
        pass
    try:
        author = info['author']
        output.write("    <author>%s</author>\n" % escape(author));
    except:
        print("Example %s lacks an author description" % (filename))
    try:
        copy = info['copy']
        output.write("    <copy>%s</copy>\n" % escape(copy));
    except:
        print("Example %s lacks a copyright description" % (filename))
    try:
        section = info['section']
        output.write("    <section>%s</section>\n" % escape(section));
        if section in sections:
            sections[section].append(filename)
        else:
            sections[section] = [filename]
    except:
        print("Example %s lacks a section description" % (filename))
    for topic in sorted(info.keys()):
        if topic != "purpose" and topic != "usage" and \
           topic != "author" and topic != "copy" and \
           topic != "section" and topic != "synopsis" and topic != "test":
            str = info[topic]
            output.write("    <extra topic='%s'>%s</extra>\n" % (
                         escape(topic), escape(str)))
    output.write("    <includes>\n")
    for include in sorted(idx.includes.keys()):
        if include.find("libxml") != -1:
            id = idx.includes[include]
            line = id.get_lineno()
            output.write("      <include line='%d'>%s</include>\n" %
                         (line, escape(include)))
    output.write("    </includes>\n")
    output.write("    <uses>\n")
    for ref in sorted(idx.references.keys()):
        id = idx.references[ref]
        name = id.get_name()
        line = id.get_lineno()
        if name in symbols:
            sinfo = symbols[name]
            refs = sinfo[0]
            # gather at most 5 references per symbols
            if refs > 5:
                continue
            sinfo.append(filename)
            sinfo[0] = refs + 1
        else:
            symbols[name] = [1, filename]
        info = find_symbol(name)
        if info != None:
            type = info[0]
            file = info[1]
            output.write("      <%s line='%d' file='%s' name='%s'/>\n" % (type,
                         line, file, name))
        else:
            type = id.get_type()
            output.write("      <%s line='%d' name='%s'/>\n" % (type,
                         line, name))

    output.write("    </uses>\n")
    output.write("  </example>\n")

    return idx

def dump_symbols(output):
    global symbols

    output.write("  <symbols>\n")
    for symbol in sorted(symbols.keys()):
        output.write("    <symbol name='%s'>\n" % (symbol))
        info = symbols[symbol]
        i = 1
        while i < len(info):
            output.write("      <ref filename='%s'/>\n" % (info[i]))
            i = i + 1
        output.write("    </symbol>\n")
    output.write("  </symbols>\n")

def dump_sections(output):
    global sections

    output.write("  <sections>\n")
    for section in sorted(sections.keys()):
        output.write("    <section name='%s'>\n" % (section))
        info = sections[section]
        i = 0
        while i < len(info):
            output.write("      <example filename='%s'/>\n" % (info[i]))
            i = i + 1
        output.write("    </section>\n")
    output.write("  </sections>\n")

def dump_Makefile():
    for file in glob.glob('*.xml'):
        extras.append(file)
    for file in glob.glob('*.res'):
        extras.append(file)
    Makefile="""##
## This file is auto-generated by index.py
## DO NOT EDIT !!!
##

AM_CPPFLAGS = -I$(top_builddir)/include -I$(top_srcdir)/include
LDADD = $(top_builddir)/libxml2.la

CLEANFILES = *.tmp

rebuild:
\tcd $(srcdir) && $(PYTHON) index.py
\t$(MAKE) Makefile
\tcd $(srcdir) && xsltproc examples.xsl examples.xml
\t-cd $(srcdir) && xmllint --valid --noout index.html

.PHONY: rebuild

install-data-local: 
\t$(MKDIR_P) $(DESTDIR)$(docdir)/examples
\t-$(INSTALL) -m 0644 $(srcdir)/*.html $(srcdir)/*.c $(DESTDIR)$(docdir)/examples/

clean-local:
\ttest -f Makefile.am || rm -f test?.xml

"""
    examples.sort()
    extras.sort()
    tests.sort()
    EXTRA_DIST=""
    for extra in extras:
        EXTRA_DIST = EXTRA_DIST + " \\\n\t" + extra
    Makefile = Makefile + "EXTRA_DIST =%s\n\n" % (EXTRA_DIST)
    check_PROGRAMS=""
    for example in examples:
        check_PROGRAMS = check_PROGRAMS + " \\\n\t" + example
    Makefile = Makefile + "check_PROGRAMS =%s\n\n" % (check_PROGRAMS)
    for example in examples:
        Makefile = Makefile + "%s_SOURCES = %s.c\n\n" % (example, example)
    Makefile = Makefile + "valgrind: \n\t$(MAKE) CHECKER='valgrind' tests\n\n"
    Makefile = Makefile + "tests: $(check_PROGRAMS)\n"
    Makefile = Makefile + "\t@test -f Makefile.am || test -f test1.xml || $(LN_S) $(srcdir)/test?.xml .\n"
    Makefile = Makefile + "\t@(echo '## examples regression tests')\n"
    for test in tests:
        Makefile = Makefile + "\t@$(CHECKER) %s\n" % (test)
    Makefile = Makefile + "\t@rm *.tmp\n"
    try:
        old = open("Makefile.am", "r").read()
        if old != Makefile:
            n = open("Makefile.am", "w").write(Makefile)
            print("Updated Makefile.am")
    except:
        print("Failed to read or save Makefile.am")

if __name__ == "__main__":
    load_api()
    output = open("examples.xml", "w")
    output.write("<examples>\n")

    for file in sorted(glob.glob('*.c')):
        parse(file, output)
        examples.append(file[:-2])

    dump_symbols(output)
    dump_sections(output)
    output.write("</examples>\n")
    output.close()
    #dump_Makefile()

