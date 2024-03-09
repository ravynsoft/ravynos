#include <tvision/tv.h>
#include <turbo/scintilla.h>
#include <turbo/styles.h>
#include <turbo/tpath.h>
#include <string_view>
#include "utils.h"

#ifdef HAVE_MAGIC
#include <magic.h>
#endif

namespace turbo {

constexpr Language
    Language::CPP {"//", "/*", "*/"},
    Language::Makefile {"#"},
    Language::Asm {";"},
    Language::JavaScript {"//", "/*", "*/"},
    Language::Rust {"//", "/*", "*/"},
    Language::Python {"#"},
    Language::Bash {"#"},
    Language::Diff,
    Language::JSON {"//", "/*", "*/"},
    Language::HTML {{}, "<!--", "-->"},
    Language::XML {{}, "<!--", "-->"},
    Language::VB {"'"},
    Language::Perl {"#"},
    Language::Batch {"rem "},
    Language::LaTex {"%"},
    Language::Lua {"--", "--[[", "]]"},
    Language::Ada {"--"},
    Language::Lisp {";"},
    Language::Ruby {"#"},
    Language::Tcl {"#"},
    Language::VBScript {"#"},
    Language::MATLAB {"%"},
    Language::CSS {{}, "/*", "*/"},
    Language::YAML {"#"},
    Language::Erlang {"%"},
    Language::Smalltalk {{}, "\"", "\""},
    Language::Markdown,
    Language::Properties {"#"},
    Language::CSharp {"//", "/*", "*/"},
    Language::Basic {"'"},
    Language::Pascal {"//", "{", "}"},
    Language::SQL {"--", "/*", "*/"};

static const const_unordered_map<std::string_view, const Language *> mime2lang = {
    {"text/x-c++",                  &Language::CPP},
    {"text/x-c",                    &Language::CPP},
    {"text/x-script.python",        &Language::Python},
    {"application/json",            &Language::JSON},
    {"text/x-shellscript",          &Language::Bash},
    {"text/x-makefile",             &Language::Makefile},
    {"text/x-diff",                 &Language::Diff},
    {"text/html",                   &Language::HTML},
};

static const const_unordered_map<std::string_view, const Language *> ext2lang = {
    {".js",                         &Language::JavaScript},
    {".jsx",                        &Language::JavaScript},
    {".mjs",                        &Language::JavaScript},
    {".asm",                        &Language::Asm},
    {".s",                          &Language::Asm},
    {".S",                          &Language::Asm},
    {".c",                          &Language::CPP},
    {".cc",                         &Language::CPP},
    {".cpp",                        &Language::CPP},
    {".cxx",                        &Language::CPP},
    {".h",                          &Language::CPP},
    {".hh",                         &Language::CPP},
    {".hpp",                        &Language::CPP},
    {".hxx",                        &Language::CPP},
    {".py",                         &Language::Python},
    {".htm",                        &Language::HTML},
    {".html",                       &Language::HTML},
    {".mhtml",                      &Language::HTML},
    {".xml",                        &Language::XML},
    {".vb",                         &Language::VB},
    {".pl",                         &Language::Perl},
    {".pm",                         &Language::Perl},
    {".bat",                        &Language::Batch},
    {".tex",                        &Language::LaTex},
    {".lua",                        &Language::Lua},
    {".diff",                       &Language::Diff},
    {".ads",                        &Language::Ada},
    {".adb",                        &Language::Ada},
    {".lsp",                        &Language::Lisp},
    {".rb",                         &Language::Ruby},
    {".tcl",                        &Language::Tcl},
    {".vbs",                        &Language::VBScript},
    {".m",                          &Language::MATLAB},
    {".css",                        &Language::CSS},
    {".erl",                        &Language::Erlang},
    {".hrl",                        &Language::Erlang},
    {".st",                         &Language::Smalltalk},
    {".md",                         &Language::Markdown},
    {".rs",                         &Language::Rust},
    {".java",                       &Language::CPP},
    {"Makefile",                    &Language::Makefile},
    {"PKGBUILD",                    &Language::Bash},
    {".json",                       &Language::JSON},
    {"eslintrc",                    &Language::JSON},
    {".jshintrc",                   &Language::JSON},
    {".jsonld",                     &Language::JSON},
    {".ipynb",                      &Language::JSON},
    {".babelrc",                    &Language::JSON},
    {".prettierrc",                 &Language::JSON},
    {".stylelintrc",                &Language::JSON},
    {".jsonc",                      &Language::JSON},
    {".jscop",                      &Language::JSON},
    {".yml",                        &Language::YAML},
    {".yaml",                       &Language::YAML},
    {".clang-format",               &Language::YAML},
    {".clang-tidy",                 &Language::YAML},
    {".mir",                        &Language::YAML},
    {".apinotes",                   &Language::YAML},
    {".ifs",                        &Language::YAML},
    {".sh",                         &Language::Bash},
    {".bashrc",                     &Language::Bash},
    {".zshrc",                      &Language::Bash},
    {".ini",                        &Language::Properties},
    {".properties",                 &Language::Properties},
    {".cs",                         &Language::CSharp},
    {".bas",                        &Language::Basic},
    {".frm",                        &Language::Basic},
    {".cls",                        &Language::Basic},
    {".pas",                        &Language::Pascal},
    {".sql",                        &Language::SQL},
};

const Language *detectFileLanguage(const char *filePath)
{
    const Language *lang = nullptr;
    {
        auto ext = TPath::extname(filePath);
        if (!ext.empty())
            lang = ext2lang[ext];
        else
        {
            auto name = TPath::basename(filePath);
            lang = ext2lang[name];
        }
    }
#ifdef HAVE_MAGIC
    if (!lang) {
        magic_t magic_cookie = magic_open(MAGIC_MIME_TYPE);
        if (magic_cookie) {
            if (magic_load(magic_cookie, nullptr) == 0)
            {
                const char *mimeType = magic_file(magic_cookie, filePath);
                if (mimeType)
                    lang = mime2lang[mimeType];
            }
            if ( magic_setflags(magic_cookie, MAGIC_MIME_ENCODING) == 0 &&
                 magic_load(magic_cookie, nullptr) == 0 )
            {
                const char *mimeEncoding = magic_file(magic_cookie, filePath);
                if (mimeEncoding)
                    ;
            }
        }
        magic_close(magic_cookie);
    }
#endif
    return lang;
}

extern constexpr ColorScheme schemeDefault =
{
    /* sNormal           */ {{}      , {}                       },
    /* sSelection        */ {'\x1'   , '\x7'                    },
    /* sWhitespace       */ {'\x5'   , {}                       },
    /* sCtrlChar         */ {'\xD'   , {}                       },
    /* sLineNums         */ {'\x6'   , {}                       },
    /* sKeyword1         */ {'\xE'   , {}                       },
    /* sKeyword2         */ {'\xA'   , {}                       },
    /* sMisc             */ {'\x9'   , {}      , slBold         },
    /* sPreprocessor     */ {'\x2'   , {}                       },
    /* sOperator         */ {'\xD'   , {}                       },
    /* sComment          */ {'\x6'   , {}                       },
    /* sStringLiteral    */ {'\xC'   , {}                       },
    /* sCharLiteral      */ {'\xC'   , {}                       },
    /* sNumberLiteral    */ {'\x3'   , {}                       },
    /* sEscapeSequence   */ {'\xB'   , {}                       },
    /* sError            */ {'\xF'   , '\xC'                    },
    /* sBraceMatch       */ {'\xE'   , {}      , slBold         },
    /* sReplaceHighlight */ {'\x0'   , '\xA'                    },
};

constexpr LexerSettings::StyleMapping stylesC[] =
{
    {SCE_C_DEFAULT,                 sNormal},
    {SCE_C_COMMENT,                 sComment},
    {SCE_C_COMMENTLINE,             sComment},
    {SCE_C_COMMENTDOC,              sComment},
    {SCE_C_NUMBER,                  sNumberLiteral},
    {SCE_C_WORD,                    sKeyword1},
    {SCE_C_STRING,                  sStringLiteral},
    {SCE_C_CHARACTER,               sCharLiteral},
    {SCE_C_PREPROCESSOR,            sPreprocessor},
    {SCE_C_OPERATOR,                sOperator},
    {SCE_C_COMMENTLINEDOC,          sComment},
    {SCE_C_WORD2,                   sKeyword2},
    {SCE_C_GLOBALCLASS,             sMisc},
    {SCE_C_PREPROCESSORCOMMENT,     sComment},
    {SCE_C_PREPROCESSORCOMMENTDOC,  sComment},
    {SCE_C_ESCAPESEQUENCE,          sEscapeSequence},
};

constexpr LexerSettings::KeywordMapping keywordsC[] =
{
    {0,
"alignas alignof and and_eq asm auto bitand bitor break case catch class compl "
"concept consteval constexpr constinit const_cast continue co_await co_return "
"co_yield decltype default delete do dynamic_cast else enum explicit export "
"false final for friend goto if import inline module namespace new noexcept not "
"not_eq nullptr operator or or_eq override private protected public "
"reinterpret_cast return sizeof static_assert static_cast struct switch "
"template this throw true try typedef typeid typename union using virtual while "
"xor xor_eq "
    },
    {1,
"bool char char8_t char16_t char32_t const double extern float int long mutable "
"register static short signed unsigned thread_local void volatile wchar_t int8_t "
"uint8_t int16_t uint16_t int32_t uint32_t int64_t uint64_t size_t ptrdiff_t "
"intptr_t uintptr_t far near uchar ushort uint ulong "
    },
    {3,
"std"
    },
};

constexpr LexerSettings::PropertyMapping propertiesC[] =
{
    {"styling.within.preprocessor",         "1"},
    {"lexer.cpp.track.preprocessor",        "0"},
    {"lexer.cpp.escape.sequence",           "1"},
};

constexpr LexerSettings::StyleMapping stylesMake[] =
{
    {SCE_MAKE_DEFAULT,              sNormal},
    {SCE_MAKE_COMMENT,              sComment},
    {SCE_MAKE_TARGET,               sKeyword1},
    {SCE_MAKE_IDENTIFIER,           sPreprocessor},
    {SCE_MAKE_PREPROCESSOR,         sPreprocessor},
    {SCE_MAKE_OPERATOR,             sOperator},
};

constexpr LexerSettings::StyleMapping stylesAsm[] =
{
    {SCE_ASM_DEFAULT,               sNormal},
    {SCE_ASM_COMMENT,               sComment},
    {SCE_ASM_COMMENTBLOCK,          sComment},
    {SCE_ASM_NUMBER,                sNumberLiteral},
    {SCE_ASM_CPUINSTRUCTION,        sKeyword1},
    {SCE_ASM_MATHINSTRUCTION,       sKeyword1},
    {SCE_ASM_STRING,                sStringLiteral},
    {SCE_ASM_CHARACTER,             sCharLiteral},
    {SCE_ASM_DIRECTIVE,             sPreprocessor},
};

constexpr LexerSettings::KeywordMapping keywordsJavaScript[] =
{
    {0,
"await break case catch continue default do else export false finally "
"for get if import new null return set super switch this throw true try while "
"with yield"
    },
    {1,
"async class const debugger delete enum eval extends function in instanceof let "
"static typeof var void"
    },
    {3,
"arguments Array ArrayBuffer AsyncFunction Atomics BigInt BigInt64Array "
"BigUint64Array Boolean DataView Date Error EvalError Float32Array Float64Array "
"Function Generator GeneratorFunction globalThis Infinity Int8Array Int16Array "
"Int32Array InternalError Intl JSON Map Math NaN Number Object Promise Proxy "
"RangeError ReferenceError Reflect RegExp Set String SyntaxError TypeError URIError "
"SharedArrayBuffer Symbol Uint8Array Uint8ClampedArray Uint16Array Uint32Array "
"undefined WeakMap WeakSet WebAssembly"
    },
};

constexpr LexerSettings::StyleMapping stylesHTML[] =
{
    // HTML
    { SCE_H_DEFAULT, sNormal},
    { SCE_H_TAG, sKeyword1},
    { SCE_H_TAGUNKNOWN, sError},
    { SCE_H_ATTRIBUTE, sKeyword2},
    { SCE_H_ATTRIBUTEUNKNOWN, sKeyword2},
    { SCE_H_NUMBER, sNumberLiteral},
    { SCE_H_DOUBLESTRING, sStringLiteral},
    { SCE_H_SINGLESTRING, sStringLiteral},
    { SCE_H_OTHER, sNormal},
    { SCE_H_COMMENT, sComment},
    { SCE_H_ENTITY, sNormal},
    { SCE_H_TAGEND, sKeyword1},
    { SCE_H_XMLSTART, sKeyword1},
    { SCE_H_XMLEND,  sKeyword1},
    { SCE_H_SCRIPT, sStringLiteral},
    { SCE_H_ASP, sStringLiteral},
    { SCE_H_ASPAT, sStringLiteral},
    { SCE_H_CDATA, sStringLiteral},
    { SCE_H_QUESTION, sNormal},
    { SCE_H_VALUE, sCharLiteral},
    { SCE_H_XCCOMMENT, sComment},
    // SGML (DTD)
    { SCE_H_SGML_DEFAULT, sNormal},
    { SCE_H_SGML_COMMAND, sKeyword2},
    { SCE_H_SGML_1ST_PARAM, sNormal},
    { SCE_H_SGML_DOUBLESTRING, sStringLiteral},
    { SCE_H_SGML_SIMPLESTRING, sStringLiteral},
    { SCE_H_SGML_ERROR, sError},
    { SCE_H_SGML_SPECIAL, sKeyword1},
    { SCE_H_SGML_ENTITY, sNormal},
    { SCE_H_SGML_COMMENT, sComment},
    { SCE_H_SGML_1ST_PARAM_COMMENT, sComment},
    { SCE_H_SGML_BLOCK_DEFAULT, sNormal},
    // JavaScript
    { SCE_HJ_DEFAULT, sNormal },
    { SCE_HJ_COMMENT, sComment },
    { SCE_HJ_COMMENTLINE, sComment },
    { SCE_HJ_COMMENTDOC, sComment },
    { SCE_HJ_NUMBER, sNumberLiteral },
    { SCE_HJ_WORD, sNormal },
    { SCE_HJ_KEYWORD, sKeyword1 },
    { SCE_HJ_DOUBLESTRING, sStringLiteral },
    { SCE_HJ_SINGLESTRING, sCharLiteral },
    { SCE_HJ_SYMBOLS, sCtrlChar },
    { SCE_HJ_REGEX, sMisc }
};

constexpr LexerSettings::KeywordMapping keywordsHTML[] =
{
    {0, // HTML
"a abbr address area article aside audio b base bdi bdo blockquote "
"body br button canvas caption cite code col colgroup data datalist dd del "
"details dfn dialog div dl dt em embed fieldset figure footer form h1 h2 h3 "
"h4 h5 h6 head header hgroup hr html i iframe img input ins kbd keygen label "
"legend li link main map mark menu menuitem meta meter nav noscript object "
"ol optgroup option output p param pre progress q rb rp rt rtc ruby s samp "
"script section select small source span strong style sub summary sup table "
"tbody td template textarea tfoot th thead time title tr track u ul var video "
"wbr strike tt acronym applet basefont big center dir font frame frameset noframes "
    // SVG
"svg animate animateMotion animateTransform circle clipPath defs desc discard "
"ellipse feBlend feColorMatrix feComponentTrasfer feComposite feConvolveMatrix "
"feDiffuseLighting feDisplacementMap feDistantLight feDropShadow feFlood feFuncA "
"feFuncB feFuncG feFuncR feGaussianBlur feImage feMerge feMergeNode feMorphology "
"feOffset fePointLight feSpecularLighting feSpotLight feTile feTurbulence filter "
"foreignObject g hatch hatchpath image line linearGradient marker mask metadata "
"mpath path pattern polygon polyline radialGradient rect set stop style switch "
"symbol text textPath title tspan use view "
    },
    {1, // JavaScript
"of await break case catch continue default do else export false finally for "
"get if import new null return set super switch this throw true try while with "
"yield async class const debugger delete enum eval extends function in instanceof "
"let static typeof var void undefined "
    },
    {2, // VBScript
" "
    },
    {3, // Python
" "
    },
    {4, // PHP
" "
    },
    {5, // SGML (DTD)
"DOCTYPE ELEMENT ATTLIST ENTITY PCDATA CDATA EMPTY SHORTREF USEMAP NOTATION IMPLIED "
"NDATA SYSTEM SGML HTML PUBLIC "
    },
};

constexpr LexerSettings::PropertyMapping propertiesHTML[] =
{
    {"asp.default.language",                "1"},
    {"html.tags.case.sensitive",            "0"},
    {"lexer.xml.allow.scripts",             "1"},
    {"lexer.html.mako",                     "0"},
    {"lexer.html.django",                   "0"},
    {"fold",                                "0"},
    {"fold.html",                           "0"},
    {"fold.html.preprocessor",              "0"},
    {"fold.compact",                        "0"},
    {"fold.hypertext.comment",              "0"},
    {"fold.hypertext.heredoc",              "0"},
    {"fold.xml.at.tag.open",                "0"},
};

constexpr LexerSettings::StyleMapping  stylesRust[] =
{
    {SCE_RUST_DEFAULT,              sNormal},
    {SCE_RUST_COMMENTBLOCK,         sComment},
    {SCE_RUST_COMMENTLINE,          sComment},
    {SCE_RUST_COMMENTBLOCKDOC,      sComment},
    {SCE_RUST_COMMENTLINEDOC,       sComment},
    {SCE_RUST_NUMBER,               sNumberLiteral},
    {SCE_RUST_WORD,                 sKeyword1},
    {SCE_RUST_WORD2,                sKeyword2},
    {SCE_RUST_STRING,               sStringLiteral},
    {SCE_RUST_STRINGR,              sStringLiteral},
    {SCE_RUST_CHARACTER,            sCharLiteral},
    {SCE_RUST_MACRO,                sPreprocessor},
    {SCE_RUST_OPERATOR,             sOperator},
    {SCE_RUST_LIFETIME,             sMisc},
    {SCE_RUST_BYTESTRING,           sEscapeSequence},
    {SCE_RUST_BYTESTRINGR,          sEscapeSequence},
    {SCE_RUST_BYTECHARACTER,        sEscapeSequence},
};

constexpr LexerSettings::KeywordMapping keywordsRust[] =
{
    {0,
"as break const continue crate dyn else enum extern false fn for if impl in let "
"loop match mod move mut pub ref return self Self static struct super trait true "
"type unsafe use where while "
    },
    {1,
"bool u8 u16 u32 u64 u128 i8 i16 i32 i64 i128 f32 f64 usize isize char str Pair "
"Box box String List"
    },
};

constexpr LexerSettings::StyleMapping  stylesPython[] =
{
    {SCE_P_DEFAULT,                 sNormal},
    {SCE_P_COMMENTLINE,             sComment},
    {SCE_P_NUMBER,                  sNumberLiteral},
    {SCE_P_STRING,                  sStringLiteral},
    {SCE_P_CHARACTER,               sCharLiteral},
    {SCE_P_WORD,                    sKeyword1},
    {SCE_P_TRIPLE,                  sStringLiteral},
    {SCE_P_TRIPLEDOUBLE,            sStringLiteral},
    {SCE_P_CLASSNAME,               sNormal},
    {SCE_P_DEFNAME,                 sNormal},
    {SCE_P_OPERATOR,                sOperator},
    {SCE_P_IDENTIFIER,              sNormal},
    {SCE_P_COMMENTBLOCK,            sComment},
    {SCE_P_STRINGEOL,               sNormal},
    {SCE_P_WORD2,                   sMisc},
    {SCE_P_DECORATOR,               sPreprocessor},
    {SCE_P_FSTRING,                 sStringLiteral},
    {SCE_P_FCHARACTER,              sCharLiteral},
    {SCE_P_FTRIPLE,                 sStringLiteral},
    {SCE_P_FTRIPLEDOUBLE,           sStringLiteral},
};

constexpr LexerSettings::KeywordMapping keywordsPython[] =
{
    {0,
"and as assert break class continue def del elif else except exec finally for "
"from global if import in is lambda not or pass print raise return try while "
"with yield"
    },
    {1,
"int float complex list tuple range str bytes bytearray memoryview set frozenset "
"dict "
    },
};

constexpr LexerSettings::PropertyMapping propertiesPython[] =
{
    {"lexer.python.keywords2.no.sub.identifiers",       "1"},
};

constexpr LexerSettings::StyleMapping  stylesBash[] =
{
    {SCE_SH_DEFAULT,                sNormal},
    {SCE_SH_ERROR,                  sError},
    {SCE_SH_COMMENTLINE,            sComment},
    {SCE_SH_NUMBER,                 sNumberLiteral},
    {SCE_SH_WORD,                   sKeyword2},
    {SCE_SH_STRING,                 sStringLiteral},
    {SCE_SH_CHARACTER,              sCharLiteral},
    {SCE_SH_OPERATOR,               sOperator},
    {SCE_SH_IDENTIFIER,             sNormal},
    {SCE_SH_SCALAR,                 sKeyword1},
    {SCE_SH_PARAM,                  sKeyword1},
    {SCE_SH_BACKTICKS,              sKeyword1},
    {SCE_SH_HERE_DELIM,             sMisc},
    {SCE_SH_HERE_Q,                 sMisc},
};

constexpr LexerSettings::KeywordMapping keywordsBash[] =
{
    {0,
// Keywords
"case do done elif else esac fi for function if in select then time until while "
// Builtins
"alias bg bind break builtin caller cd command compgen complete compopt continue "
"declare dirs disown echo enable eval exec exit export fc fg getopts hash help "
"history jobs kill let local logout mapfile popd printf pushd pwd read readarray "
"readonly return set shift shopt source suspend test times trap type typeset ulimit "
"umask unalias unset wait "
    },
};

constexpr LexerSettings::StyleMapping  stylesRuby[] =
{
    {SCE_RB_DEFAULT,                sNormal},
    {SCE_RB_ERROR,                  sError},
    {SCE_RB_COMMENTLINE,            sComment},
    {SCE_RB_POD,                    sMisc},
    {SCE_RB_NUMBER,                 sNumberLiteral},
    {SCE_RB_WORD,                   sKeyword1},
    {SCE_RB_STRING,                 sStringLiteral},
    {SCE_RB_CHARACTER,              sCharLiteral},
    {SCE_RB_CLASSNAME,              sNormal},
    {SCE_RB_DEFNAME,                sNormal},
    {SCE_RB_OPERATOR,               sOperator},
    {SCE_RB_IDENTIFIER,             sNormal},
    {SCE_RB_REGEX,                  sNormal},
    {SCE_RB_GLOBAL,                 sNormal},
    {SCE_RB_SYMBOL,                 sKeyword2},
    {SCE_RB_MODULE_NAME,            sNormal},
    {SCE_RB_INSTANCE_VAR,           sNormal},
    {SCE_RB_CLASS_VAR,              sNormal},
    {SCE_RB_BACKTICKS,              sKeyword1},
    {SCE_RB_HERE_DELIM,             sMisc},
    {SCE_RB_HERE_Q,                 sMisc},
    {SCE_RB_STRING_QQ,              sMisc},
    {SCE_RB_STRING_QX,              sMisc},
    {SCE_RB_STRING_QR,              sMisc},
    {SCE_RB_STRING_QW,              sMisc},
    {SCE_RB_WORD_DEMOTED,           sNormal},
    {SCE_RB_STDIN,                  sMisc},
    {SCE_RB_STDOUT,                 sMisc},
    {SCE_RB_STDERR,                 sMisc},
    {SCE_RB_UPPER_BOUND,            sMisc},
};

constexpr LexerSettings::KeywordMapping keywordsRuby[] =
{
    {0,
"__ENCODING__ __LINE__ __FILE__ BEGIN END "
"alias and begin break case class def defined? do else elsif end ensure false "
"for if in module next nil not or redo rescue retry return self super then true "
"undef unless until when while yield "
"public require require_relative "
    },
};

constexpr LexerSettings::StyleMapping stylesJSON[] =
{
    {SCE_JSON_DEFAULT,              sNormal},
    {SCE_JSON_NUMBER,               sNumberLiteral},
    {SCE_JSON_STRING,               sStringLiteral},
    {SCE_JSON_STRINGEOL,            sStringLiteral},
    {SCE_JSON_PROPERTYNAME,         sPreprocessor},
    {SCE_JSON_ESCAPESEQUENCE,       sEscapeSequence},
    {SCE_JSON_LINECOMMENT,          sComment},
    {SCE_JSON_BLOCKCOMMENT,         sComment},
    {SCE_JSON_OPERATOR,             sOperator},
    {SCE_JSON_URI,                  sStringLiteral},
    {SCE_JSON_COMPACTIRI,           sKeyword2},
    {SCE_JSON_KEYWORD,              sKeyword1},
    {SCE_JSON_LDKEYWORD,            sKeyword2},
    {SCE_JSON_ERROR,                sError},
};

constexpr LexerSettings::KeywordMapping keywordsJSON[] =
{
    {0, "false true null"},
    {1,
"@id @context @type @value @language @container @list @set @reverse @index "
"@base @vocab @graph "
    },
};

constexpr LexerSettings::PropertyMapping propertiesJSON[] =
{
    {"lexer.json.escape.sequence", "1"},
    {"lexer.json.allow.comments", "1"},
};

constexpr LexerSettings::StyleMapping stylesYAML[] =
{
    {SCE_YAML_DEFAULT,              sNormal},
    {SCE_YAML_COMMENT,              sComment},
    {SCE_YAML_IDENTIFIER,           sKeyword1},
    {SCE_YAML_KEYWORD,              sKeyword2},
    {SCE_YAML_NUMBER,               sNumberLiteral},
    {SCE_YAML_REFERENCE,            sMisc},
    {SCE_YAML_DOCUMENT,             sMisc},
    {SCE_YAML_TEXT,                 sStringLiteral},
    {SCE_YAML_ERROR,                sError},
    {SCE_YAML_OPERATOR,             sOperator},
};

constexpr LexerSettings::KeywordMapping keywordsYAML[] =
{
    {0, "true false yes no"},
};

constexpr LexerSettings::KeywordMapping keywordsCSharp[] =
{
    {0,
"abstract as base bool break case catch checked class const continue default "
"delegate do else enum event explicit extern false finally fixed for foreach "
"goto if implicit in interface internal is lock namespace new null object operator "
"out override params private protected public readonly ref return sealed sizeof "
"stackalloc static struct switch this throw true try typeof unchecked unsafe "
"using virtual void volatile while "
"add and alias ascending args async await by descending dynamic equals file from "
"get global group init into join let managed nameof not notnull on or orderby "
"partial record remove required scoped select set unmanaged value var when where "
"with yield "
    },
    {1,
"bool byte sbyte char decimal double float in uint nint nuint long ulong short ushort "
"object string dynamic "
    },
    {3,
"System"
    },
};

constexpr LexerSettings::StyleMapping stylesProperties[] =
{
    {SCE_PROPS_DEFAULT,             sNormal},
    {SCE_PROPS_COMMENT,             sComment},
    {SCE_PROPS_SECTION,             sKeyword2},
    {SCE_PROPS_ASSIGNMENT,          sOperator},
    {SCE_PROPS_DEFVAL,              sMisc},
    {SCE_PROPS_KEY,                 sKeyword1},
};

constexpr LexerSettings::StyleMapping stylesBasic[] =
{
    {SCE_B_DEFAULT,                 sNormal},
    {SCE_B_COMMENT,                 sComment},
    {SCE_B_NUMBER,                  sNumberLiteral},
    {SCE_B_KEYWORD,                 sKeyword1},
    {SCE_B_STRING,                  sStringLiteral},
    {SCE_B_PREPROCESSOR,            sPreprocessor},
    {SCE_B_OPERATOR,                sOperator},
    {SCE_B_IDENTIFIER,              sNormal},
    {SCE_B_DATE,                    sCharLiteral},
    {SCE_B_STRINGEOL,               sNormal},
    {SCE_B_KEYWORD2,                sKeyword2},
    {SCE_B_KEYWORD3,                sKeyword2},
    {SCE_B_KEYWORD4,                sKeyword2},
    {SCE_B_CONSTANT,                sNormal},
    {SCE_B_ASM,                     sMisc},
    {SCE_B_LABEL,                   sSelection},
    {SCE_B_ERROR,                   sError},
    {SCE_B_HEXNUMBER,               sNumberLiteral},
    {SCE_B_BINNUMBER,               sNumberLiteral},
    {SCE_B_COMMENTBLOCK,            sComment},
    {SCE_B_DOCLINE,                 sMisc},
    {SCE_B_DOCBLOCK,                sMisc},
    {SCE_B_DOCKEYWORD,              sMisc},
};

constexpr LexerSettings::KeywordMapping keywordsBasic[] =
{
    {0,
    // QBasic
"and as call case do loop if then else elseif end exit for next function gosub "
"goto not on or return select sub to type until wend while xor "
    // VB6
"false new true "
    },
    {1,
    // QBasic
"abs absolute access alias append asc atn beep binary bload bsave byval "
"cdbl chain chdir chr cint circle clear clng close cls color command common "
"const cos csng csrlin cvd cvdmbf cvi cvl cvs cvsmbf data date declare library "
"def seg defdbl defint deflng defsng defstr dim double draw "
"environ eof eqv erase erl err error exp field files fix free freefile "
"get hex imp inkey inp input instr int integer interrupt "
"interruptx key kill lbound lcase left len line list loc locate lock lof log "
"long lprint using lset ltrim mid mkd mkdir mkdmbf mki mkl mks mksmbf mod name "
"oct off strig open com out output paint palette pcopy peek "
"play pmap point poke pos preset print pset put random randomize read redim reset restore "
"resume right rmdir rnd rset rtrim run sadd screen seek sgn shared shell "
"sin single sleep sound space spc sqr step stick stop str string swap system "
"tab tan time timer ubound ucase unlock val varptr varseg view "
"wait width window write option explicit base "
    // VB6
"addressof attribute appactivate begin beginproperty boolean byref chdrive class "
"collection defbool defbyte defdate defdec defcur defobj defvar deletesetting "
"each endproperty enum event filecopy friend global implements in is load lib "
"like me nothing null object local optional compare module paramarray private "
"property public raiseevent savepicture savesetting sendkeys set setattr text "
"typeof unload variant version with withevents "
    // QB64
"_acceptfiledrop _acos _acosh _allowfullscreen _alpha _alpha32 _arrcot _arccsc _arcsec _asin _asinh "
"_assert _atan2 _atanh _autodisplay _axis _backgroundcolor _bit _bin _bin _blend _blink "
"_blue _blue32 _button _buttonchange _byte _capslock _ceil _cinp _clearcolor _clip _clipboard "
"_clipboardimage _colorchoosedialog _commandcount _connected _connectionaddress "
"_consoleinput _consoletitle _continue _controlchr _copyimage _copypalette _cot _coth _cosh "
"_csc _csch _cw _cwd _d2g _d2r _defaultcolor _define _deflate _delay _depthbuffer _desktopheight "
"_desktopwidth _dest _device _deviceinput _devices _dir _direxists _display _displayorder _dontblend "
"_dontwait _droppedfile _environcount _errorline _errormessage "
"_fileexists _finishdrop _float _font _fontheight _fontwidth _freefont _freeimage "
"_freetimer _fullscreen _g2d _g2r _glrender _green _green32 _height _hide _hypot _icon _inclerrofile "
"_inclerroline _inflate _inputbox _instrrev _integer64 _keyclear _keyhit _keydown _lastaxis "
"_lastbutton _lastwheel _limit _loadfont _loadimage _maptriangle _mapunicode _mem _memcopy _memelement "
"_memexists _memfill _memfree _memget _memimage _memnew _memput _memsound _messagebox _middle "
"_mk _mousebutton _mousehide _mouseinput _mousemove _mousemovementx _mousemovementy _mouseshow _mousewheel "
"_mousex _mousey _newimage _notifypopup _numlock _offset _openclient _openconnection "
"_openfiledialog _openhost _os _palettecolor _pi _pixelsize _preserve "
"_printimage _printmode _printstring _printwidth _putimage _r2d _r2g _red _red32 _readbit _resetbit "
"_resize _resizeheight _resizewidth _rgb _rgb32 _rgba _rgba32 _rol _ror _round _savefiledialog "
"_sec _sech _selectfolderdialog _screenclick _screenexists _screenhide _screenicon "
"_screenimage _screenmove _screenprint _screenshow _screenx _screeny _scrollock _setalpha "
"_setbit _shellhide _shl _shr _sinh _smooth _sndbal _sndclose _sndcopy _sndgetpos _sndlen _sndlimit _sndloop "
"_sndnew _sndopen _sndopenraw _sndpause _sndpaused _sndplay _sndplaycopy _sndplayfile _sndplaying "
"_sndrate _sndraw _sndrawdone _sndrawlen _sndsetpos _sndstop _sndvol _source _startdir _statuscode "
"_strcmp _stricmp _tanh _title _title _togglebit _totaldroppedfiles _trim _unsigned "
"_wheel _windowhandle _windowhasfocus _echo _exeicon _asserts _checking _console _debug _let "
"_midisoundfont _noprefix _unstable _versioninfo _virtualkeyboard _dynamic _include _static _explicitarray "
    },
};

constexpr LexerSettings::KeywordMapping keywordsVbNet[] =
{
    {0,
"and andalso as call case catch class continue do each else elseif end endif "
"enum exit false finally for function gosub goto if in loop new operator next "
"not or orelse return structure sub then throw to true try using wend when "
"while with xor "
    },
    {1,
"addhandler addressof alias boolean byref byte byval cbool "
"cbyte cchar cdate cdbl cdec char cint constraint clng cobj const csbyte "
"cshort csng cstr ctype cuint culng cushort date decimal declare default delegate dim "
"directcast double erase error event "
"friend get gettype getxmlnamespace global handles implements "
"statement imports inherits integer interface is isnot let lib like long me mod "
"module mustinherit mustoverride mybase myclass nameof namespace narrowing "
"nothing notinheritable notoverridable object of on option optional "
"out overloads overridable overrides paramarray partial private property protected public "
"raiseevent readonly redim removehandler resume sbyte select set shadows shared "
"short single static step stop string synclock trycast "
"typeof uinteger ulong ushort variant widening withevents writeonly "
    }
};

constexpr LexerSettings::StyleMapping  stylesPascal[] =
{
    {SCE_PAS_DEFAULT,               sNormal},
    {SCE_PAS_IDENTIFIER,            sNormal},
    {SCE_PAS_COMMENT,               sComment},
    {SCE_PAS_COMMENT2,              sComment},
    {SCE_PAS_COMMENTLINE,           sComment},
    {SCE_PAS_PREPROCESSOR,          sPreprocessor},
    {SCE_PAS_PREPROCESSOR2,         sPreprocessor},
    {SCE_PAS_NUMBER,                sNumberLiteral},
    {SCE_PAS_HEXNUMBER,             sNumberLiteral},
    {SCE_PAS_WORD,                  sKeyword1},
    {SCE_PAS_STRING,                sStringLiteral},
    {SCE_PAS_STRINGEOL,             sStringLiteral},
    {SCE_PAS_CHARACTER,             sCharLiteral},
    {SCE_PAS_OPERATOR,              sOperator},
    {SCE_PAS_ASM,                   sMisc}
};

constexpr LexerSettings::KeywordMapping keywordsPascal[] =
{
    {0,
// Pascal
"and array asm begin break case const constructor continue destructor div do "
"downto else end false file for function goto if implementation in inline interface "
"label mod nil not object of on operator or packed procedure program record "
"repeat set shl shr string then to true type unit until uses var while with xor "

"as class constref dispose except exit exports finalization finally inherited "
"initialization is library new on out property raise self threadvar try far near "

"absolute abstract alias assembler cdecl Cppdecl default export external forward "
"generic index local name nostackframe oldfpccall override pascal private protected "
"public published read register reintroduce safecall softfloat specialize stdcall "
"virtual write "

// Delphi
"resourcestring dispinterface strict nodefault stored automated final readonly "
"unsafe reference varargs contains helper overload implements winapi delayed package "
"requires deprecated resident writeonly dispid platform dynamic sealed experimental "
"message static "
    }
};

constexpr LexerSettings::StyleMapping  stylesTeX[] =
{
    {SCE_L_DEFAULT,                 sNormal},
    {SCE_L_COMMAND,                 sKeyword1},
    {SCE_L_TAG,                     sKeyword2},
    {SCE_L_MATH,                    sCharLiteral},
    {SCE_L_COMMENT,                 sComment},
    {SCE_L_TAG2,                    sKeyword2},
    {SCE_L_MATH2,                   sCharLiteral},
    {SCE_L_COMMENT2,                sComment},
    {SCE_L_VERBATIM,                sStringLiteral},
    {SCE_L_SHORTCMD,                sKeyword1},
    {SCE_L_SPECIAL,                 sCtrlChar},
    {SCE_L_CMDOPT,                  sOperator},
    {SCE_L_ERROR,                   sError},
};

constexpr LexerSettings::StyleMapping  stylesSQL[] =
{
    {SCE_SQL_DEFAULT,                sNormal},
    {SCE_SQL_COMMENT,                sComment},
    {SCE_SQL_COMMENTLINE,            sComment},
    {SCE_SQL_COMMENTDOC,             sComment},
    {SCE_SQL_NUMBER,                 sNumberLiteral},
    {SCE_SQL_WORD,                   sKeyword1},
    {SCE_SQL_STRING,                 sStringLiteral},
    {SCE_SQL_CHARACTER,              sCharLiteral},
    {SCE_SQL_SQLPLUS,                sMisc},
    {SCE_SQL_SQLPLUS_PROMPT,         sMisc},
    {SCE_SQL_OPERATOR,               sOperator},
    {SCE_SQL_IDENTIFIER,             sNormal},
    {SCE_SQL_SQLPLUS_COMMENT,        sComment},
    {SCE_SQL_COMMENTLINEDOC,         sComment},
    {SCE_SQL_WORD2,                  sKeyword2},
    {SCE_SQL_COMMENTDOCKEYWORD,      sKeyword2},
    {SCE_SQL_COMMENTDOCKEYWORDERROR, sError},
    {SCE_SQL_USER1,                  sKeyword2},
    {SCE_SQL_USER2,                  sKeyword2},
    {SCE_SQL_USER3,                  sKeyword2},
    {SCE_SQL_USER4,                  sKeyword2},
    {SCE_SQL_QUOTEDIDENTIFIER,       sNormal},
    {SCE_SQL_QOPERATOR,              sOperator},
};

constexpr LexerSettings::KeywordMapping keywordsSQL[] =
{
    {0,
"a abort abs absolute access action ada add admin after aggregate alias all allocate also alter always analyse "
"analyze and any are array as asc asensitive assertion assignment asymmetric at atomic attribute attributes audit "
"authorization auto_increment avg avg_row_length backup backward before begin bernoulli between bigint binary bit "
"bit_length bitvar blob bool boolean both breadth break browse bulk by c cache call called cardinality cascade "
"cascaded case cast catalog catalog_name ceil ceiling chain change char char_length character character_length "
"character_set_catalog character_set_name character_set_schema characteristics characters check checked checkpoint "
"checksum class class_origin clob close cluster clustered coalesce cobol collate collation collation_catalog "
"collation_name collation_schema collect column column_name columns command_function command_function_code comment "
"commit committed completion compress compute condition condition_number connect connection connection_name constraint "
"constraint_catalog constraint_name constraint_schema constraints constructor contains containstable continue "
"conversion convert copy corr corresponding count covar_pop covar_samp create createdb createrole createuser cross "
"csv cube cume_dist current current_date current_default_transform_group current_path current_role current_time "
"current_timestamp current_transform_group_for_type current_user cursor cursor_name cycle data database databases "
"date datetime datetime_interval_code datetime_interval_precision day day_hour day_microsecond day_minute day_second "
"dayofmonth dayofweek dayofyear dbcc deallocate dec decimal declare default defaults deferrable deferred defined "
"definer degree delay_key_write delayed delete delimiter delimiters dense_rank deny depth deref derived desc describe "
"descriptor destroy destructor deterministic diagnostics dictionary disable disconnect disk dispatch distinct "
"distinctrow distributed div do domain double drop dual dummy dump dynamic dynamic_function dynamic_function_code "
"each element else elseif enable enclosed encoding encrypted end end-exec enum equals errlvl escape escaped every "
"except exception exclude excluding exclusive exec execute existing exists exit exp explain external extract false "
"fetch fields file fillfactor filter final first float float4 float8 floor flush following for force foreign fortran "
"forward found free freetext freetexttable freeze from full fulltext function fusion g general generated get global "
"go goto grant granted grants greatest group grouping handler having header heap hierarchy high_priority hold holdlock "
"host hosts hour hour_microsecond hour_minute hour_second identified identity identity_insert identitycol if ignore "
"ilike immediate immutable implementation implicit in include including increment index indicator infile infix inherit "
"inherits initial initialize initially inner inout input insensitive insert insert_id instance instantiable instead "
"int int1 int2 int3 int4 int8 integer intersect intersection interval into invoker is isam isnull isolation iterate "
"join k key key_member key_type keys kill lancompiler language large last last_insert_id lateral lead leading least "
"leave left length less level like limit lineno lines listen ln load local localtime localtimestamp location locator "
"lock login logs long longblob longtext loop low_priority lower m map match matched max max_rows maxextents maxvalue "
"mediumblob mediumint mediumtext member merge message_length message_octet_length message_text method middleint "
"min min_rows minus minute minute_microsecond minute_second minvalue mlslabel mod mode modifies modify module month "
"monthname more move multiset mumps myisam name names national natural nchar nclob nesting new next no "
"no_write_to_binlog noaudit nocheck nocompress nocreatedb nocreaterole nocreateuser noinherit nologin nonclustered "
"none normalize normalized nosuperuser not nothing notify notnull nowait null nullable nullif nulls number numeric "
"object octet_length octets of off offline offset offsets oids old on online only open opendatasource openquery "
"openrowset openxml operation operator optimize option optionally options or order ordering ordinality others out "
"outer outfile output over overlaps overlay overriding owner pack_keys pad parameter parameter_mode parameter_name "
"parameter_ordinal_position parameter_specific_catalog parameter_specific_name parameter_specific_schema parameters "
"partial partition pascal password path pctfree percent percent_rank percentile_cont percentile_disc placing plan pli "
"position postfix power preceding precision prefix preorder prepare prepared preserve primary print prior privileges "
"proc procedural procedure process processlist public purge quote raid0 raiserror range rank raw read reads readtext "
"real recheck reconfigure recursive ref references referencing regexp regr_avgx regr_avgy regr_count regr_intercept "
"regr_r2 regr_slope regr_sxx regr_sxy regr_syy reindex relative release reload rename repeat repeatable replace "
"replication require reset resignal resource restart restore restrict result return returned_cardinality returned_length "
"returned_octet_length returned_sqlstate returns revoke right rlike role rollback rollup routine routine_catalog "
"routine_name routine_schema row row_count row_number rowcount rowguidcol rowid rownum rows rule save savepoint scale "
"schema schema_name schemas scope scope_catalog scope_name scope_schema scroll search second second_microsecond section "
"security select self sensitive separator sequence serializable server_name session session_user set setof sets setuser "
"share show shutdown signal similar simple size smallint some soname source space spatial specific specific_name "
"specifictype sql sql_big_result sql_big_selects sql_big_tables sql_calc_found_rows sql_log_off sql_log_update "
"sql_low_priority_updates sql_select_limit sql_small_result sql_warnings sqlca sqlcode sqlerror sqlexception sqlstate "
"sqlwarning sqrt ssl stable start starting state statement static statistics status stddev_pop stddev_samp stdin stdout "
"storage straight_join strict string structure style subclass_origin sublist submultiset substring successful sum "
"superuser symmetric synonym sysdate sysid system system_user table table_name tables tablesample tablespace temp "
"template temporary terminate terminated text textsize than then ties time timestamp timezone_hour timezone_minute "
"tinyblob tinyint tinytext to toast top top_level_count trailing tran transaction transaction_active "
"transactions_committed transactions_rolled_back transform transforms translate translation treat trigger "
"trigger_catalog trigger_name trigger_schema trim true truncate trusted tsequal type uescape uid unbounded uncommitted "
"under undo unencrypted union unique unknown unlisten unlock unnamed unnest unsigned until update updatetext upper "
"usage use user user_defined_type_catalog user_defined_type_code user_defined_type_name user_defined_type_schema "
"using utc_date utc_time utc_timestamp vacuum valid validate validator value values var_pop var_samp varbinary varchar "
"varchar2 varcharacter variable variables varying verbose view volatile waitfor when whenever where while width_bucket "
"window with within without work write writetext x509 xor year year_month zerofill zone "
    }
};

constexpr LexerSettings::PropertyMapping propertiesSQL[] =
{
    {"lexer.sql.backticks.identifier", "1"},
    {"lexer.sql.numbersign.comment", "1"},
};

constexpr struct { const Language *language; LexerSettings lexer; } builtInLexers[] =
{
    {&Language::CPP, {SCLEX_CPP, stylesC, keywordsC, propertiesC}},
    {&Language::Makefile, {SCLEX_MAKEFILE, stylesMake, nullptr, nullptr}},
    {&Language::Asm, {SCLEX_ASM, stylesAsm, nullptr, nullptr}},
    {&Language::JavaScript, {SCLEX_CPP, stylesC, keywordsJavaScript, propertiesC}},
    {&Language::Rust, {SCLEX_RUST, stylesRust, keywordsRust, nullptr}},
    {&Language::Python, {SCLEX_PYTHON, stylesPython, keywordsPython, propertiesPython}},
    {&Language::Bash, {SCLEX_BASH, stylesBash, keywordsBash, nullptr}},
    {&Language::Ruby, {SCLEX_RUBY, stylesRuby, keywordsRuby, nullptr}},
    {&Language::JSON, {SCLEX_JSON, stylesJSON, keywordsJSON, propertiesJSON}},
    {&Language::YAML, {SCLEX_YAML, stylesYAML, keywordsYAML, nullptr}},
    {&Language::HTML, {SCLEX_HTML, stylesHTML, keywordsHTML, propertiesHTML}},
    {&Language::Properties, {SCLEX_PROPERTIES, stylesProperties, nullptr, nullptr}},
    {&Language::CSharp, {SCLEX_CPP, stylesC, keywordsCSharp, propertiesC}},
    {&Language::VB, {SCLEX_VB, stylesBasic, keywordsVbNet, nullptr}},
    {&Language::Basic, {SCLEX_VB, stylesBasic, keywordsBasic, nullptr}},
    {&Language::Pascal, {SCLEX_PASCAL, stylesPascal, keywordsPascal, nullptr}},
    {&Language::LaTex, {SCLEX_LATEX, stylesTeX, nullptr, nullptr}},
    {&Language::SQL, {SCLEX_SQL, stylesSQL, keywordsSQL, propertiesSQL}},
};

TColorAttr coalesce(TColorAttr from, TColorAttr into)
{
    auto f_fg = ::getFore(from), f_bg = ::getBack(from),
         i_fg = ::getFore(into), i_bg = ::getBack(into);
    return {
        f_fg.isDefault() ? i_fg : f_fg,
        f_bg.isDefault() ? i_bg : f_bg,
        ::getStyle(from),
    };
}

const LexerSettings *findBuiltInLexer(const Language *language)
{
    for (const auto &l : builtInLexers)
        if (l.language == language)
            return &l.lexer;
    return nullptr;
}

} // namespace turbo
