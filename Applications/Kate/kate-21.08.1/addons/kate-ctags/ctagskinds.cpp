/***************************************************************************
 *   SPDX-FileCopyrightText: 2001-2002 Bernd Gehrmann *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "ctagskinds.h"

#include <KLocalizedString>

struct CTagsKindMapping {
    char abbrev;
    const char *verbose;
};

struct CTagsExtensionMapping {
    const char *extension;
    const CTagsKindMapping *kinds;
};

static const CTagsKindMapping kindMappingAsm[] = {{'d', I18N_NOOP2("Tag Type", "define")},
                                                  {'l', I18N_NOOP2("Tag Type", "label")},
                                                  {'m', I18N_NOOP2("Tag Type", "macro")},
                                                  {0, nullptr}};

static const CTagsKindMapping kindMappingAsp[] = {{'f', I18N_NOOP2("Tag Type", "function")}, {'s', I18N_NOOP2("Tag Type", "subroutine")}, {0, nullptr}};

static const CTagsKindMapping kindMappingAwk[] = {{'f', I18N_NOOP2("Tag Type", "function")}, {0, nullptr}};

static const CTagsKindMapping kindMappingBeta[] = {{'f', I18N_NOOP2("Tag Type", "fragment definition")},
                                                   {'p', I18N_NOOP2("Tag Type", "any pattern")},
                                                   {'s', I18N_NOOP2("Tag Type", "slot")},
                                                   {'v', I18N_NOOP2("Tag Type", "pattern")},
                                                   {0, nullptr}};

static const CTagsKindMapping kindMappingC[] = {{'c', I18N_NOOP2("Tag Type", "class")},
                                                {'d', I18N_NOOP2("Tag Type", "macro")},
                                                {'e', I18N_NOOP2("Tag Type", "enumerator")},
                                                {'f', I18N_NOOP2("Tag Type", "function")},
                                                {'g', I18N_NOOP2("Tag Type", "enumeration")},
                                                {'m', I18N_NOOP2("Tag Type", "member")},
                                                {'n', I18N_NOOP2("Tag Type", "namespace")},
                                                {'p', I18N_NOOP2("Tag Type", "prototype")},
                                                {'s', I18N_NOOP2("Tag Type", "struct")},
                                                {'t', I18N_NOOP2("Tag Type", "typedef")},
                                                {'u', I18N_NOOP2("Tag Type", "union")},
                                                {'v', I18N_NOOP2("Tag Type", "variable")},
                                                {'x', I18N_NOOP2("Tag Type", "external variable")},
                                                {0, nullptr}};

static const CTagsKindMapping kindMappingCobol[] = {{'p', I18N_NOOP2("Tag Type", "paragraph")}, {0, nullptr}};

static const CTagsKindMapping kindMappingEiffel[] = {{'c', I18N_NOOP2("Tag Type", "class")},
                                                     {'f', I18N_NOOP2("Tag Type", "feature")},
                                                     {'l', I18N_NOOP2("Tag Type", "local entity")},
                                                     {0, nullptr}};

static const CTagsKindMapping kindMappingFortran[] = {{'b', I18N_NOOP2("Tag Type", "block")},
                                                      {'c', I18N_NOOP2("Tag Type", "common")},
                                                      {'e', I18N_NOOP2("Tag Type", "entry")},
                                                      {'f', I18N_NOOP2("Tag Type", "function")},
                                                      {'i', I18N_NOOP2("Tag Type", "interface")},
                                                      {'k', I18N_NOOP2("Tag Type", "type component")},
                                                      {'l', I18N_NOOP2("Tag Type", "label")},
                                                      {'L', I18N_NOOP2("Tag Type", "local")},
                                                      {'m', I18N_NOOP2("Tag Type", "module")},
                                                      {'n', I18N_NOOP2("Tag Type", "namelist")},
                                                      {'p', I18N_NOOP2("Tag Type", "program")},
                                                      {'s', I18N_NOOP2("Tag Type", "subroutine")},
                                                      {'t', I18N_NOOP2("Tag Type", "type")},
                                                      {'v', I18N_NOOP2("Tag Type", "variable")},
                                                      {0, nullptr}};

static const CTagsKindMapping kindMappingJava[] = {{'c', I18N_NOOP2("Tag Type", "class")},
                                                   {'f', I18N_NOOP2("Tag Type", "field")},
                                                   {'i', I18N_NOOP2("Tag Type", "interface")},
                                                   {'m', I18N_NOOP2("Tag Type", "method")},
                                                   {'p', I18N_NOOP2("Tag Type", "package")},
                                                   {0, nullptr}};

static const CTagsKindMapping kindMappingLisp[] = {{'f', I18N_NOOP2("Tag Type", "function")}, {0, nullptr}};

static const CTagsKindMapping kindMappingMake[] = {{'m', I18N_NOOP2("Tag Type", "macro")}, {0, nullptr}};

static const CTagsKindMapping kindMappingPascal[] = {{'f', I18N_NOOP2("Tag Type", "function")}, {'p', I18N_NOOP2("Tag Type", "procedure")}, {0, nullptr}};

static const CTagsKindMapping kindMappingPerl[] = {{'s', I18N_NOOP2("Tag Type", "subroutine")}, {0, nullptr}};

static const CTagsKindMapping kindMappingPHP[] = {{'c', I18N_NOOP2("Tag Type", "class")}, {'f', I18N_NOOP2("Tag Type", "function")}, {0, nullptr}};

static const CTagsKindMapping kindMappingPython[] = {{'c', I18N_NOOP2("Tag Type", "class")}, {'f', I18N_NOOP2("Tag Type", "function")}, {0, nullptr}};

static const CTagsKindMapping kindMappingRexx[] = {{'s', I18N_NOOP2("Tag Type", "subroutine")}, {0, nullptr}};

static const CTagsKindMapping kindMappingRuby[] = {{'c', I18N_NOOP2("Tag Type", "class")},
                                                   {'f', I18N_NOOP2("Tag Type", "function")},
                                                   {'m', I18N_NOOP2("Tag Type", "mixin")},
                                                   {0, nullptr}};

static const CTagsKindMapping kindMappingScheme[] = {{'f', I18N_NOOP2("Tag Type", "function")}, {'s', I18N_NOOP2("Tag Type", "set")}, {0, nullptr}};

static const CTagsKindMapping kindMappingSh[] = {{'f', I18N_NOOP2("Tag Type", "function")}, {0, nullptr}};

static const CTagsKindMapping kindMappingSlang[] = {{'f', I18N_NOOP2("Tag Type", "function")}, {'n', I18N_NOOP2("Tag Type", "namespace")}, {0, nullptr}};

static const CTagsKindMapping kindMappingTcl[] = {{'p', I18N_NOOP2("Tag Type", "procedure")}, {0, nullptr}};

static const CTagsKindMapping kindMappingVim[] = {{'f', I18N_NOOP2("Tag Type", "function")}, {0, nullptr}};

static CTagsExtensionMapping extensionMapping[] = {
    {"asm", kindMappingAsm},     {"s", kindMappingAsm},         {"S", kindMappingAsm},       {"asp", kindMappingAsp},
    {"asa", kindMappingAsp},     {"awk", kindMappingAwk},       {"c++", kindMappingC},       {"cc", kindMappingC},
    {"cp", kindMappingC},        {"cpp", kindMappingC},         {"cxx", kindMappingC},       {"h", kindMappingC},
    {"h++", kindMappingC},       {"hh", kindMappingC},          {"hp", kindMappingC},        {"hpp", kindMappingC},
    {"hxx", kindMappingC},       {"beta", kindMappingBeta},     {"cob", kindMappingCobol},   {"COB", kindMappingCobol},
    {"e", kindMappingEiffel},    {"f", kindMappingFortran},     {"for", kindMappingFortran}, {"ftn", kindMappingFortran},
    {"f77", kindMappingFortran}, {"f90", kindMappingFortran},   {"f95", kindMappingFortran}, {"java", kindMappingJava},
    {"cl", kindMappingLisp},     {"clisp", kindMappingLisp},    {"el", kindMappingLisp},     {"l", kindMappingLisp},
    {"lisp", kindMappingLisp},   {"lsp", kindMappingLisp},      {"ml", kindMappingLisp},     {"mak", kindMappingMake},
    {"p", kindMappingPascal},    {"pas", kindMappingPascal},    {"pl", kindMappingPerl},     {"pm", kindMappingPerl},
    {"perl", kindMappingPerl},   {"php", kindMappingPHP},       {"php3", kindMappingPHP},    {"phtml", kindMappingPHP},
    {"py", kindMappingPython},   {"python", kindMappingPython}, {"cmd", kindMappingRexx},    {"rexx", kindMappingRexx},
    {"rx", kindMappingRexx},     {"rb", kindMappingRuby},       {"sch", kindMappingScheme},  {"scheme", kindMappingScheme},
    {"scm", kindMappingScheme},  {"sm", kindMappingScheme},     {"SCM", kindMappingScheme},  {"SM", kindMappingScheme},
    {"sh", kindMappingSh},       {"SH", kindMappingSh},         {"bsh", kindMappingSh},      {"bash", kindMappingSh},
    {"ksh", kindMappingSh},      {"zsh", kindMappingSh},        {"sl", kindMappingSlang},    {"tcl", kindMappingTcl},
    {"wish", kindMappingTcl},    {"vim", kindMappingVim},       {nullptr, nullptr}};

static const CTagsKindMapping *findKindMapping(const char *pextension)
{
    CTagsExtensionMapping *pem = extensionMapping;
    while (pem->extension != nullptr) {
        if (strcmp(pem->extension, pextension) == 0) {
            return pem->kinds;
        }
        ++pem;
    }

    return nullptr;
}

QString CTagsKinds::findKind(const char *kindChar, const QString &extension)
{
    if (kindChar == nullptr || extension.isEmpty()) {
        return QString();
    }

    const CTagsKindMapping *kindMapping = findKindMapping(extension.toLocal8Bit().constData());
    if (kindMapping) {
        const CTagsKindMapping *pkm = kindMapping;
        while (pkm->verbose != nullptr) {
            if (pkm->abbrev == *kindChar) {
                return i18nc("Tag Type", pkm->verbose);
            }
            ++pkm;
        }
    }

    return QString();
}

QString CTagsKinds::findKindNoi18n(const char *kindChar, const QStringRef &extension)
{
    if (kindChar == nullptr || extension.isEmpty()) {
        return QString();
    }

    const CTagsKindMapping *kindMapping = findKindMapping(extension.toLocal8Bit().constData());
    if (kindMapping) {
        const CTagsKindMapping *pkm = kindMapping;
        while (pkm->verbose != nullptr) {
            if (pkm->abbrev == *kindChar) {
                return QString::fromLocal8Bit(pkm->verbose);
            }
            ++pkm;
        }
    }

    return QString();
}
