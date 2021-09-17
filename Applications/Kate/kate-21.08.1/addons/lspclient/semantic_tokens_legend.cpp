/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: MIT
*/
#include "semantic_tokens_legend.h"

#include <KSyntaxHighlighting/Theme>
#include <KTextEditor/Editor>

SemanticTokensLegend::SemanticTokensLegend(QObject *parent)
    : QObject(parent)
{
    themeChange(KTextEditor::Editor::instance());
    connect(KTextEditor::Editor::instance(), &KTextEditor::Editor::configChanged, this, &SemanticTokensLegend::themeChange);
}

enum {
    FuncAttr = 0,
    TypeAttr,
    MacroAttr,
    CommentAttr,
    VarParamAttr,
    ConstantAttr,
    KeywordAttr,
};

void SemanticTokensLegend::themeChange(KTextEditor::Editor *e)
{
    if (!e) {
        return;
    }

    using Style = KSyntaxHighlighting::Theme::TextStyle;

    const auto theme = e->theme();

    QColor f = QColor::fromRgba(theme.textColor(Style::Function));
    QColor fs = QColor::fromRgba(theme.selectedTextColor(Style::Function));
    if (!fixedAttrs[FuncAttr]) {
        fixedAttrs[FuncAttr] = new KTextEditor::Attribute();
    }
    fixedAttrs[FuncAttr]->setForeground(f);
    fixedAttrs[FuncAttr]->setSelectedForeground(fs);
    fixedAttrs[FuncAttr]->setFontBold(theme.isBold(Style::Function));
    fixedAttrs[FuncAttr]->setFontItalic(theme.isItalic(Style::Function));

    if (!fixedAttrs[VarParamAttr]) {
        fixedAttrs[VarParamAttr] = new KTextEditor::Attribute();
    }
    {
        // This is only for function parameter which are not
        // directly supported by themes so we load some hard
        // coded values here for some of the themes we have
        // and for others we just read the "Variable" text-style.
        QColor v;
        QColor vs;
        bool italic = false;
        static const char MonokaiVP[] = "#fd971f";
        static const char DraculaVP[] = "#ffb86c";
        static const char AyuDarkLightVP[] = "#a37acc";
        static const char AyuMirageVP[] = "#d4bfff";
        if (theme.name() == QStringLiteral("Monokai")) {
            v = QColor(MonokaiVP);
            vs = v;
            italic = true;
        } else if (theme.name() == QStringLiteral("Dracula")) {
            v = QColor(DraculaVP);
            vs = v;
            italic = true;
        } else if (theme.name() == QStringLiteral("ayu Light") || theme.name() == QStringLiteral("ayu Dark")) {
            v = QColor(AyuDarkLightVP);
            vs = v;
        } else if (theme.name() == QStringLiteral("ayu Mirage")) {
            v = QColor(AyuMirageVP);
            vs = v;
        } else {
            v = QColor::fromRgba(theme.textColor(Style::Variable));
            italic = theme.isItalic(Style::Variable);
            vs = QColor::fromRgba(theme.selectedTextColor(Style::Variable));
        }

        fixedAttrs[VarParamAttr]->setForeground(v);
        fixedAttrs[VarParamAttr]->setSelectedForeground(vs);
        fixedAttrs[VarParamAttr]->setFontBold(theme.isBold(Style::Variable));
        fixedAttrs[VarParamAttr]->setFontItalic(italic);
    }

    QColor c = QColor::fromRgba(theme.textColor(Style::Constant));
    QColor cs = QColor::fromRgba(theme.selectedTextColor(Style::Constant));
    if (!fixedAttrs[ConstantAttr]) {
        fixedAttrs[ConstantAttr] = new KTextEditor::Attribute();
    }
    fixedAttrs[ConstantAttr]->setForeground(c);
    fixedAttrs[ConstantAttr]->setSelectedForeground(cs);
    fixedAttrs[ConstantAttr]->setFontBold(theme.isBold(Style::Constant));
    fixedAttrs[ConstantAttr]->setFontItalic(theme.isItalic(Style::Constant));

    QColor k = QColor::fromRgba(theme.textColor(Style::Keyword));
    QColor ks = QColor::fromRgba(theme.selectedTextColor(Style::Keyword));
    if (!fixedAttrs[KeywordAttr]) {
        fixedAttrs[KeywordAttr] = new KTextEditor::Attribute();
    }
    fixedAttrs[KeywordAttr]->setForeground(k);
    fixedAttrs[KeywordAttr]->setSelectedForeground(ks);
    fixedAttrs[KeywordAttr]->setFontBold(theme.isBold(Style::Keyword));
    fixedAttrs[KeywordAttr]->setFontItalic(theme.isItalic(Style::Keyword));

    QColor cm = QColor::fromRgba(theme.textColor(Style::Comment));
    QColor cms = QColor::fromRgba(theme.selectedTextColor(Style::Comment));
    if (!fixedAttrs[CommentAttr]) {
        fixedAttrs[CommentAttr] = new KTextEditor::Attribute();
    }
    fixedAttrs[CommentAttr]->setForeground(cm);
    fixedAttrs[CommentAttr]->setSelectedForeground(cms);
    fixedAttrs[CommentAttr]->setFontBold(theme.isBold(Style::Comment));
    fixedAttrs[CommentAttr]->setFontItalic(theme.isItalic(Style::Comment));

    QColor p = QColor::fromRgba(theme.textColor(Style::Preprocessor));
    QColor ps = QColor::fromRgba(theme.selectedTextColor(Style::Preprocessor));
    if (!fixedAttrs[MacroAttr]) {
        fixedAttrs[MacroAttr] = new KTextEditor::Attribute();
    }
    fixedAttrs[MacroAttr]->setForeground(p);
    fixedAttrs[MacroAttr]->setSelectedForeground(ps);
    fixedAttrs[MacroAttr]->setFontBold(theme.isBold(Style::Preprocessor));
    fixedAttrs[MacroAttr]->setFontItalic(theme.isItalic(Style::Preprocessor));

    QColor tp = QColor::fromRgba(theme.textColor(Style::DataType));
    QColor tps = QColor::fromRgba(theme.selectedTextColor(Style::DataType));
    if (!fixedAttrs[TypeAttr]) {
        fixedAttrs[TypeAttr] = new KTextEditor::Attribute();
    }
    fixedAttrs[TypeAttr]->setForeground(tp);
    fixedAttrs[TypeAttr]->setSelectedForeground(tps);
    fixedAttrs[TypeAttr]->setFontBold(theme.isBold(Style::DataType));
    fixedAttrs[TypeAttr]->setFontItalic(theme.isItalic(Style::DataType));
}

void SemanticTokensLegend::initialize(const std::vector<QString> &types)
{
    std::vector<TokenType> tokenTypes(types.size());
    int i = 0;
    for (const auto &type : types) {
        if (type == QStringLiteral("type"))
            tokenTypes[i] = TokenType::Type;
        else if (type == QStringLiteral("class"))
            tokenTypes[i] = TokenType::Class;
        else if (type == QStringLiteral("enum"))
            tokenTypes[i] = TokenType::Enum;
        else if (type == QStringLiteral("type"))
            tokenTypes[i] = TokenType::Type;
        else if (type == QStringLiteral("interface"))
            tokenTypes[i] = TokenType::Interface;
        else if (type == QStringLiteral("struct"))
            tokenTypes[i] = TokenType::Struct;
        else if (type == QStringLiteral("typeParameter"))
            tokenTypes[i] = TokenType::TypeParameter;
        else if (type == QStringLiteral("parameter"))
            tokenTypes[i] = TokenType::Parameter;
        else if (type == QStringLiteral("variable"))
            tokenTypes[i] = TokenType::Variable;
        else if (type == QStringLiteral("property"))
            tokenTypes[i] = TokenType::Property;
        else if (type == QStringLiteral("enumMember"))
            tokenTypes[i] = TokenType::EnumMember;
        else if (type == QStringLiteral("event"))
            tokenTypes[i] = TokenType::Event;
        else if (type == QStringLiteral("function"))
            tokenTypes[i] = TokenType::Function;
        else if (type == QStringLiteral("method"))
            tokenTypes[i] = TokenType::Method;
        else if (type == QStringLiteral("macro"))
            tokenTypes[i] = TokenType::Macro;
        else if (type == QStringLiteral("keyword"))
            tokenTypes[i] = TokenType::Keyword;
        else if (type == QStringLiteral("modifier"))
            tokenTypes[i] = TokenType::Modifier;
        else if (type == QStringLiteral("comment"))
            tokenTypes[i] = TokenType::Comment; // Can mean inactive code
        else if (type == QStringLiteral("string"))
            tokenTypes[i] = TokenType::String;
        else if (type == QStringLiteral("number"))
            tokenTypes[i] = TokenType::Number;
        else if (type == QStringLiteral("regexp"))
            tokenTypes[i] = TokenType::Regexp;
        else if (type == QStringLiteral("operator"))
            tokenTypes[i] = TokenType::Operator;
        else if (type == QStringLiteral("namespace"))
            tokenTypes[i] = TokenType::Namespace;
        else
            tokenTypes[i] = TokenType::Unsupported;
        i++;
    }
    totalTokenTypes = tokenTypes.size();
    refresh(tokenTypes);
}

void SemanticTokensLegend::refresh(const std::vector<TokenType> &tokenTypes)
{
    sharedAttrs.resize(tokenTypes.size());
    for (size_t i = 0; i < tokenTypes.size(); ++i) {
        switch (tokenTypes.at(i)) {
        case Type:
        case Class:
        case Interface:
        case Struct:
        case Enum:
            sharedAttrs[i] = fixedAttrs[TypeAttr];
            break;
        case Namespace:
            sharedAttrs[i] = fixedAttrs[KeywordAttr];
            break;
        case TypeParameter:
        case Parameter:
            sharedAttrs[i] = fixedAttrs[VarParamAttr];
            break;
        case Macro:
            sharedAttrs[i] = fixedAttrs[MacroAttr];
            break;
        case Function:
        case Method:
            sharedAttrs[i] = fixedAttrs[FuncAttr];
            break;
        case EnumMember:
            sharedAttrs[i] = fixedAttrs[ConstantAttr];
            break;
        case Comment:
            sharedAttrs[i] = fixedAttrs[CommentAttr];
            break;
            // Only these for now
        default:
            sharedAttrs[i] = {};
        }
    }
}
