#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Options {
    std::string defPath;
    std::string headerPath;
    std::string implPath;
};

struct MethodInfo {
    std::string returnType;
    std::string name;
    std::string args;
    bool isStatic{false};
    bool isVirtual{false};
    bool isLocalOnly{false};
};

static void
usage(const char *prog)
{
    std::cerr
        << "usage: " << prog << " --def <input.iig> --header <output.h> [--impl <output.cpp|/dev/null>] [-- ...cflags]\n"
        << "       " << prog << " <input.iig> -o <output.h>\n";
}

static bool
readFile(const std::string &path, std::string &out)
{
    std::ifstream f(path);
    if (!f) {
        return false;
    }
    std::stringstream ss;
    ss << f.rdbuf();
    out = ss.str();
    return true;
}

static bool
writeFile(const std::string &path, const std::string &content)
{
    std::ofstream f(path);
    if (!f) {
        return false;
    }
    f << content;
    return true;
}

static std::string
trim(const std::string &s)
{
    size_t b = 0;
    while (b < s.size() && std::isspace(static_cast<unsigned char>(s[b]))) {
        b++;
    }
    size_t e = s.size();
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) {
        e--;
    }
    return s.substr(b, e - b);
}


static std::string
rewriteIncludes(const std::string &input)
{
    std::stringstream in(input);
    std::ostringstream out;
    std::string line;
    std::regex incl(R"(^\s*#\s*include\s*<([^>]+)\.iig>\s*$)");
    std::smatch m;
    while (std::getline(in, line)) {
        if (std::regex_match(line, m, incl)) {
            out << "#include <" << m[1].str() << ".h>  /* .iig include */\n";
        } else {
            out << line << "\n";
        }
    }
    return out.str();
}

static std::string
removeExtendsClasses(const std::string &input)
{
    std::string out = input;
    size_t p = 0;
    while (true) {
        size_t start = out.find("class EXTENDS", p);
        if (start == std::string::npos) {
            break;
        }
        size_t brace = out.find('{', start);
        if (brace == std::string::npos) {
            break;
        }
        int depth = 1;
        size_t i = brace + 1;
        for (; i < out.size() && depth > 0; i++) {
            if (out[i] == '{') {
                depth++;
            } else if (out[i] == '}') {
                depth--;
            }
        }
        if (depth != 0) {
            break;
        }
        size_t semi = out.find(';', i);
        if (semi == std::string::npos) {
            break;
        }
        out.replace(start, (semi + 1) - start, "/* EXTENDS class omitted by iig-linux */\n");
        p = start;
    }
    return out;
}

static std::string
sanitizeDocumentationText(const std::string &input)
{
    std::string out = input;
    out = std::regex_replace(out, std::regex(R"(\bLOCALONLY\b)"), "");
    out = std::regex_replace(out, std::regex(R"(\bLOCAL\b)"), "");
    out = std::regex_replace(out, std::regex(R"(class\s+KERNEL\s+)"), "class ");
    out = std::regex_replace(out, std::regex(R"(\n\s*#\s*undef\s+KERNEL\s*\n)"), "\n");
    return out;
}

static std::string
sanitizeTypeText(std::string s)
{
    s = std::regex_replace(s, std::regex(R"(\bLOCALONLY\b)"), "");
    s = std::regex_replace(s, std::regex(R"(\bLOCAL\b)"), "");
    s = std::regex_replace(s, std::regex(R"(\bKERNEL\b)"), "");
    s = std::regex_replace(s, std::regex(R"(\bTARGET\b)"), "");
    s = std::regex_replace(s, std::regex(R"(\s+)"), " ");
    return trim(s);
}

static std::string
escapeMacroContinuation(const std::string &s)
{
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        if (c == '\\') {
            out += "\\\\";
        } else {
            out.push_back(c);
        }
    }
    return out;
}

static uint64_t
fnv1a64(const std::string &s)
{
    uint64_t h = 14695981039346656037ULL;
    for (unsigned char c : s) {
        h ^= static_cast<uint64_t>(c);
        h *= 1099511628211ULL;
    }
    return h;
}

static std::vector<std::string>
splitArgs(const std::string &args)
{
    std::vector<std::string> out;
    std::string cur;
    int angle = 0;
    int paren = 0;
    int bracket = 0;
    for (char c : args) {
        if (c == '<') {
            angle++;
        } else if (c == '>') {
            if (angle > 0) {
                angle--;
            }
        } else if (c == '(') {
            paren++;
        } else if (c == ')') {
            if (paren > 0) {
                paren--;
            }
        } else if (c == '[') {
            bracket++;
        } else if (c == ']') {
            if (bracket > 0) {
                bracket--;
            }
        }

        if (c == ',' && angle == 0 && paren == 0 && bracket == 0) {
            std::string t = trim(cur);
            if (!t.empty()) {
                out.push_back(t);
            }
            cur.clear();
            continue;
        }
        cur.push_back(c);
    }
    std::string t = trim(cur);
    if (!t.empty() && t != "void") {
        out.push_back(t);
    }
    return out;
}

static std::vector<MethodInfo>
parseMethods(const std::string &classText, const std::string &className)
{
    std::vector<MethodInfo> methods;
    std::string body = classText;
    body = std::regex_replace(body, std::regex(R"(/\*[\s\S]*?\*/)"), "");
    body = std::regex_replace(body, std::regex(R"(//.*?$)", std::regex::multiline), "");
    body = std::regex_replace(body, std::regex(R"(#pragma[^\n]*\n)"), "\n");

    std::stringstream ss(body);
    std::string line;
    std::string stmt;
    int depth = 0;
    while (std::getline(ss, line)) {
        std::string t = trim(line);
        if (t.empty()) {
            continue;
        }
        for (char c : line) {
            if (c == '{') {
                depth++;
            } else if (c == '}') {
                depth--;
            }
        }
        if (depth < 1) {
            continue;
        }
        if (t == "public:" || t == "private:" || t == "protected:") {
            continue;
        }

        stmt += " " + t;
        if (t.find(';') == std::string::npos) {
            continue;
        }

        std::string cand = trim(stmt);
        stmt.clear();
        if (cand.empty() || cand.find('(') == std::string::npos) {
            continue;
        }
        if (cand.rfind("typedef ", 0) == 0) {
            continue;
        }

        cand = cand.substr(0, cand.find(';'));

        size_t lp = cand.find('(');
        size_t rp = cand.rfind(')');
        if (lp == std::string::npos || rp == std::string::npos || rp < lp) {
            continue;
        }
        std::string left = trim(cand.substr(0, lp));
        std::string args = trim(cand.substr(lp + 1, rp - lp - 1));

        std::smatch m;
        std::regex nameRx(R"(([~A-Za-z_][A-Za-z0-9_]*)\s*$)");
        if (!std::regex_search(left, m, nameRx)) {
            continue;
        }
        std::string name = m[1].str();
        if (name == className || name == ("~" + className)) {
            continue;
        }
        left = trim(left.substr(0, m.position(1)));

        MethodInfo mi;
        mi.isLocalOnly = (cand.find("LOCALONLY") != std::string::npos);
        mi.isStatic = (left.find("static ") != std::string::npos);
        mi.isVirtual = (left.find("virtual ") != std::string::npos);
        mi.name = name;
        mi.returnType = sanitizeTypeText(left);
        mi.args = sanitizeTypeText(args);
        if (mi.returnType.empty()) {
            continue;
        }
        methods.push_back(mi);
    }
    return methods;
}

static std::string
emitGeneratedMetadata(const std::string &className, const std::vector<MethodInfo> &methods)
{
    std::ostringstream out;
    for (const auto &m : methods) {
        if (m.isLocalOnly) {
            continue;
        }
        std::ostringstream sig;
        sig << className << "::" << m.name << "(" << m.args << ")";
        uint64_t id = fnv1a64(sig.str());
        out << "#define " << className << "_" << m.name << "_ID            0x"
            << std::hex << std::nouppercase << id << "ULL" << std::dec << "\n";
    }
    out << "\n";

    for (const auto &m : methods) {
        if (m.isLocalOnly) {
            continue;
        }
        out << "#define " << className << "_" << m.name << "_Args \\\n";
        auto args = splitArgs(m.args);
        if (args.empty()) {
            out << "\n\n";
            continue;
        }
        for (size_t i = 0; i < args.size(); i++) {
            out << "        " << args[i];
            if (i + 1 < args.size()) {
                out << ", \\\n";
            } else {
                out << "\n\n";
            }
        }
    }

    out << "#define " << className << "_Methods \\\n\\\npublic:\\\n\\\n";
    out << "    virtual kern_return_t\\\n"
        << "    Dispatch(const IORPC rpc) APPLE_KEXT_OVERRIDE;\\\n\\\n";
    out << "    static kern_return_t\\\n"
        << "    _Dispatch(" << className << " * self, const IORPC rpc);\\\n\\\n";

    for (const auto &m : methods) {
        out << "    " << escapeMacroContinuation(m.returnType) << "\\\n"
            << "    " << m.name << "(\\\n";
        auto args = splitArgs(m.args);
        if (args.empty()) {
            out << ");\\\n\\\n";
            continue;
        }
        for (size_t i = 0; i < args.size(); i++) {
            out << "        " << escapeMacroContinuation(args[i]);
            if (i + 1 < args.size()) {
                out << ",\\\n";
            } else {
                if (!m.isLocalOnly && !m.isStatic) {
                    out << ",\\\n        OSDispatchMethod supermethod = NULL);\\\n\\\n";
                } else {
                    out << ");\\\n\\\n";
                }
            }
        }
    }

    out << "#define " << className << "_KernelMethods\n";
    out << "#define " << className << "_VirtualMethods\n";
    return out.str();
}

static bool
findMainClass(const std::string &input,
              std::string &className,
              size_t &classStart,
              size_t &classEnd)
{
    std::regex cls(R"(class\s+(?:KERNEL\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*:\s*public\s+[A-Za-z_][A-Za-z0-9_]*)");
    std::smatch m;
    std::string work = input;
    size_t searchOffset = 0;

    while (std::regex_search(work, m, cls)) {
        size_t abs = searchOffset + static_cast<size_t>(m.position(0));
        if (input.compare(abs, 13, "class EXTENDS") == 0) {
            searchOffset = abs + 13;
            work = input.substr(searchOffset);
            continue;
        }
        className = m[1].str();
        classStart = abs;
        size_t brace = input.find('{', classStart);
        if (brace == std::string::npos) {
            return false;
        }
        int depth = 1;
        size_t i = brace + 1;
        for (; i < input.size() && depth > 0; i++) {
            if (input[i] == '{') {
                depth++;
            } else if (input[i] == '}') {
                depth--;
            }
        }
        if (depth != 0) {
            return false;
        }
        size_t semi = input.find(';', i);
        if (semi == std::string::npos) {
            return false;
        }
        classEnd = semi + 1;
        return true;
    }

    return false;
}

static std::string
buildHeader(const std::string &input, const std::string &basename)
{
    std::string transformed = rewriteIncludes(input);
    transformed = removeExtendsClasses(transformed);

    std::string className;
    size_t classStart = 0;
    size_t classEnd = 0;
    if (!findMainClass(transformed, className, classStart, classEnd)) {
        return "/* iig(iig-linux) generated from " + basename + " */\n\n" + transformed;
    }

    size_t guardEnd = transformed.rfind("#endif");
    if (guardEnd == std::string::npos || guardEnd < classEnd) {
        guardEnd = transformed.size();
    }

    std::string prologue = transformed.substr(0, classStart);
    std::string classText = transformed.substr(classStart, classEnd - classStart);
    std::string epilogue = transformed.substr(guardEnd);

    std::string classTextDoc = sanitizeDocumentationText(classText);
    std::vector<MethodInfo> methods = parseMethods(classTextDoc, className);

    std::ostringstream out;
    out << "/* iig(iig-linux) generated from " << basename << " */\n\n";
    out << prologue;
    out << "\n/* source class " << className << " " << basename << " */\n\n";
    out << "#if __DOCUMENTATION__\n\n";
    out << classTextDoc << "\n\n";
    out << "#else /* __DOCUMENTATION__ */\n\n";
    out << "#if KERNEL\n";
    out << "#ifndef " << className << "_Methods\n#define " << className << "_Methods\n#endif\n";
    out << "#ifndef " << className << "_KernelMethods\n#define " << className << "_KernelMethods\n#endif\n";
    out << "#ifndef " << className << "_VirtualMethods\n#define " << className << "_VirtualMethods\n#endif\n";
    out << "#else /* !KERNEL */\n\n";
    out << "/* generated class " << className << " " << basename << " */\n\n";
    out << emitGeneratedMetadata(className, methods) << "\n";
    out << classTextDoc << "\n\n";
    out << "#endif /* !KERNEL */\n\n";
    out << "#endif /* !__DOCUMENTATION__ */\n\n";
    out << epilogue;
    return out.str();
}

static bool
parseArgs(int argc, const char **argv, Options &opts)
{
    bool sawDoubleDash = false;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--") {
            sawDoubleDash = true;
            continue;
        }
        if (sawDoubleDash) {
            continue;
        }
        if ((a == "--def") && (i + 1 < argc)) {
            opts.defPath = argv[++i];
            continue;
        }
        if ((a == "--header") && (i + 1 < argc)) {
            opts.headerPath = argv[++i];
            continue;
        }
        if ((a == "--impl") && (i + 1 < argc)) {
            opts.implPath = argv[++i];
            continue;
        }
        if ((a == "-o") && (i + 1 < argc)) {
            opts.headerPath = argv[++i];
            continue;
        }
        if ((a == "--xnu-root") && (i + 1 < argc)) {
            i++;
            continue;
        }
        if (a == "-h" || a == "--help") {
            return false;
        }
        if (!a.empty() && a[0] != '-' && opts.defPath.empty()) {
            opts.defPath = a;
            continue;
        }
    }

    return !opts.defPath.empty() && !opts.headerPath.empty();
}

} // namespace

int
main(int argc, const char **argv)
{
    Options opts;
    if (!parseArgs(argc, argv, opts)) {
        usage(argv[0]);
        return 1;
    }

    std::string input;
    if (!readFile(opts.defPath, input)) {
        std::cerr << "iig-linux: failed to read input: " << opts.defPath << "\n";
        return 1;
    }

    std::string base = std::filesystem::path(opts.defPath).filename().string();
    std::string header = buildHeader(input, base);

    if (!writeFile(opts.headerPath, header)) {
        std::cerr << "iig-linux: failed to write header: " << opts.headerPath << "\n";
        return 1;
    }

    if (!opts.implPath.empty() && opts.implPath != "/dev/null") {
        std::ostringstream impl;
        impl << "/* iig(iig-linux) generated from " << base << " */\n";
        impl << "/* implementation generation is intentionally minimal in iig-linux */\n";
        if (!writeFile(opts.implPath, impl.str())) {
            std::cerr << "iig-linux: failed to write impl: " << opts.implPath << "\n";
            return 1;
        }
    }

    return 0;
}

