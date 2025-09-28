/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "config.h"
#include <assert.h>
#include "Sample.h"
#include "util.h"
#include "Exp_Layout.h"

Sample::Sample (int num)
{
  number = num;
  prusage = NULL;
  start_time = end_time = 0;
  start_label = end_label = NULL;
  validated = false;
}

Sample::~Sample ()
{
  delete prusage;
  free (start_label);
  free (end_label);
}

PrUsage *
Sample::get_usage ()
{
  if (validated == false)
    {
      validate_usage ();
      validated = true;
    }
  return prusage;
}

void
Sample::validate_usage ()
{
  if (prusage == NULL || validated)
    return;
  validated = true;

  // Make sure that none of the times are negative, force to zero if so
  if (prusage->pr_utime < 0)
    prusage->pr_utime = 0;
  if (prusage->pr_stime < 0)
    prusage->pr_stime = 0;
  if (prusage->pr_ttime < 0)
    prusage->pr_ttime = 0;
  if (prusage->pr_tftime < 0)
    prusage->pr_tftime = 0;
  if (prusage->pr_dftime < 0)
    prusage->pr_dftime = 0;
  if (prusage->pr_kftime < 0)
    prusage->pr_kftime = 0;
  if (prusage->pr_ltime < 0)
    prusage->pr_ltime = 0;
  if (prusage->pr_slptime < 0)
    prusage->pr_slptime = 0;
  if (prusage->pr_wtime < 0)
    prusage->pr_wtime = 0;
  if (prusage->pr_stoptime < 0)
    prusage->pr_stoptime = 0;
  if (prusage->pr_rtime < 0)
    prusage->pr_rtime = 0;

  // Now make sure that the sum of states is >= prusage->pr_rtime
  hrtime_t sum = prusage->pr_utime + prusage->pr_stime + prusage->pr_ttime
	  + prusage->pr_tftime + prusage->pr_dftime + prusage->pr_kftime
	  + prusage->pr_ltime + prusage->pr_slptime + prusage->pr_wtime
	  + prusage->pr_stoptime;

  sum = sum - prusage->pr_rtime;
  if (sum < 0)// increment sleep time to make it match
    prusage->pr_slptime = prusage->pr_slptime - sum;
}
