#ifndef TURBO_SCINTILLA_INTERNALS_H
#define TURBO_SCINTILLA_INTERNALS_H

// Define the standard order in which to include header files
// All platform headers should be included before Scintilla headers
// and each of these groups are then divided into directory groups.

// C standard library
#include <string.h>
#include <assert.h>

// C++ wrappers of C standard library

// C++ standard library
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <forward_list>
#include <algorithm>
#include <memory>
#include <chrono>

// Scintilla headers

// Non-platform-specific headers

// include
#include "include/Platform.h"
#include "include/Sci_Position.h"
#include "include/ILoader.h"
#include "include/ILexer.h"
#include "include/Scintilla.h"
#include "include/ScintillaWidget.h"
#include "include/SciLexer.h"

// lexlib
#include "lexlib/StringCopy.h"
#include "lexlib/PropSetSimple.h"
#include "lexlib/WordList.h"
#include "lexlib/LexAccessor.h"
#include "lexlib/Accessor.h"
#include "lexlib/StyleContext.h"
#include "lexlib/CharacterSet.h"
#include "lexlib/CharacterCategory.h"
#include "lexlib/LexerModule.h"
#include "lexlib/CatalogueModules.h"
#include "lexlib/OptionSet.h"
#include "lexlib/SparseState.h"
#include "lexlib/SubStyles.h"
#include "lexlib/DefaultLexer.h"
#include "lexlib/LexerBase.h"
#include "lexlib/LexerSimple.h"
#include "lexlib/LexerNoExceptions.h"

// src
#include "src/Catalogue.h"
#include "src/Position.h"
#include "src/IntegerRectangle.h"
#include "src/UniqueString.h"
#include "src/SplitVector.h"
#include "src/Partitioning.h"
#include "src/RunStyles.h"
#include "src/SparseVector.h"
#include "src/ContractionState.h"
#include "src/CellBuffer.h"
#include "src/PerLine.h"
#include "src/CallTip.h"
#include "src/KeyMap.h"
#include "src/Indicator.h"
#include "src/XPM.h"
#include "src/LineMarker.h"
#include "src/Style.h"
#include "src/ViewStyle.h"
#include "src/CharClassify.h"
#include "src/Decoration.h"
#include "src/CaseFolder.h"
#include "src/Document.h"
#include "src/RESearch.h"
#include "src/CaseConvert.h"
#include "src/UniConversion.h"
#include "src/DBCS.h"
#include "src/Selection.h"
#include "src/PositionCache.h"
#include "src/FontQuality.h"
#include "src/EditModel.h"
#include "src/MarginView.h"
#include "src/EditView.h"
#include "src/Editor.h"
#include "src/ElapsedPeriod.h"
#include "src/AutoComplete.h"
#include "src/ScintillaBase.h"
#include "src/ExternalLexer.h"

#endif // TURBO_SCINTILLA_INTERNALS_H
