//===--- DriverOptions.cpp - Driver Options Table -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "tapi/Driver/DriverOptions.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Option/Option.h"

using namespace llvm;
using namespace llvm::opt;
using namespace tapi::internal;

/// Create prefix string literals used in TAPIOptions.td.
#define PREFIX(NAME, VALUE)                                                    \
  static constexpr llvm::StringLiteral NAME##_init[] = VALUE;                  \
  static constexpr llvm::ArrayRef<llvm::StringLiteral> NAME(                   \
      NAME##_init, std::size(NAME##_init) - 1);
#include "tapi/Driver/TAPIOptions.inc"
#undef PREFIX

static constexpr const StringLiteral prefixTable_init[] =
#define PREFIX_UNION(VALUES) VALUES
#include "tapi/Driver/TAPIOptions.inc"
#undef PREFIX_UNION
    ;
static constexpr const ArrayRef<StringLiteral>
    prefixTable(prefixTable_init, std::size(prefixTable_init) - 1);

/// Create table mapping all options defined in TAPIOptions.td.
static constexpr OptTable::Info infoTable[] = {
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS, PARAM,  \
               HELPTEXT, METAVAR, VALUES)                                      \
  {PREFIX, NAME,  HELPTEXT,    METAVAR,     OPT_##ID,  Option::KIND##Class,    \
   PARAM,  FLAGS, OPT_##GROUP, OPT_##ALIAS, ALIASARGS, VALUES},
#include "tapi/Driver/TAPIOptions.inc"
#undef OPTION
};

namespace {

/// \brief Create OptTable class for parsing actual command line arguments.
class DriverOptTable : public opt::PrecomputedOptTable {
public:
  DriverOptTable() : PrecomputedOptTable(infoTable, prefixTable) {}
};

} // end anonymous namespace.

OptTable *TAPI_INTERNAL::createDriverOptTable() { return new DriverOptTable(); }
