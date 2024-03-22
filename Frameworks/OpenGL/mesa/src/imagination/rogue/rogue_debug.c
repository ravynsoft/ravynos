/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "nir/nir.h"
#include "rogue.h"
#include "util/macros.h"
#include "util/u_debug.h"

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

/**
 * \file rogue_debug.c
 *
 * \brief Contains debugging functions and data.
 */

static const struct debug_named_value rogue_debug_options[] = {
   { "nir", ROGUE_DEBUG_NIR, "Print NIR" },
   { "nir_passes", ROGUE_DEBUG_NIR_PASSES, "Print NIR passes" },
   { "ir", ROGUE_DEBUG_IR, "Print Rogue IR" },
   { "ir_passes", ROGUE_DEBUG_IR_PASSES, "Print Rogue IR passes" },
   { "ir_details",
     ROGUE_DEBUG_IR_DETAILS,
     "Print Rogue IR details (with ir/ir_passes enabled)" },
   { "vld_skip", ROGUE_DEBUG_VLD_SKIP, "Skip Rogue IR validation" },
   { "vld_nonfatal", ROGUE_DEBUG_VLD_NONFATAL, "Non-fatal Rogue IR validation" },
   DEBUG_NAMED_VALUE_END,
};

#define ROGUE_DEBUG_DEFAULT 0U
DEBUG_GET_ONCE_FLAGS_OPTION(rogue_debug,
                            "ROGUE_DEBUG",
                            rogue_debug_options,
                            ROGUE_DEBUG_DEFAULT)

PUBLIC
unsigned long rogue_debug = ROGUE_DEBUG_DEFAULT;

DEBUG_GET_ONCE_OPTION(rogue_color, "ROGUE_COLOR", NULL)

bool rogue_color = false;

static void rogue_debug_init_once(void)
{
   /* Get debug flags. */
   rogue_debug = debug_get_option_rogue_debug();

   /* Get/parse color option. */
   const char *color_opt = debug_get_option_rogue_color();
   if (!color_opt || !strcmp(color_opt, "auto") || !strcmp(color_opt, "a"))
      rogue_color = isatty(fileno(stdout));
   else if (!strcmp(color_opt, "on") || !strcmp(color_opt, "1"))
      rogue_color = true;
   else if (!strcmp(color_opt, "off") || !strcmp(color_opt, "0"))
      rogue_color = false;
}

PUBLIC
void rogue_debug_init(void)
{
   static once_flag flag = ONCE_FLAG_INIT;
   call_once(&flag, rogue_debug_init_once);
}
