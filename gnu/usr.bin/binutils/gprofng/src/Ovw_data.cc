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
#include <memory.h>
#include <values.h>
#include <assert.h>
#include "Data_window.h"
#include "Exp_Layout.h"
#include "Table.h"
#include "Ovw_data.h"
#include "Sample.h"
#include "data_pckts.h"
#include "util.h"
#include "i18n.h"

void
Ovw_data::sum (Ovw_data *data)
{
  Ovw_item data_totals = data->get_totals ();
  if (totals == NULL)
    {
      totals = reset_item (new Ovw_item);
      *totals = data_totals;
      totals->start.tv_sec = totals->end.tv_sec = -1;
      totals->start.tv_nsec = totals->end.tv_nsec = 0;
    }
  else
    {
      tsadd (&totals->duration, &data_totals.duration);
      tsadd (&totals->tlwp, &data_totals.tlwp);
      if (tstodouble (totals->duration) != 0)
	totals->nlwp = tstodouble (totals->tlwp) / tstodouble (totals->duration);

      for (int i = 0, size = totals->size; i < size; i++)
	tsadd (&totals->values[i].t, &data_totals.values[i].t);
    }
}

Ovw_data::Ovw_item *
Ovw_data::reset_item (Ovw_data::Ovw_item *item)
{
  memset (item, 0, sizeof (*item));
  return item;
}

Ovw_data::Ovw_item
Ovw_data::get_totals ()
{
  // This routine will return the totals values for item in the sample.
  // Compute maximums and totals only once, and save the result.
  // On subsequent calls, just return the saved result.
  // If maximums is NULL, then totals is also NULL
  if (totals != NULL)
    return *totals;

  timestruc_t zero = {0, 0};
  totals = reset_item (new Ovw_item);
  totals->start.tv_sec = MAXINT; // new
  totals->start.tv_nsec = MAXINT; // new
  totals->start_label = totals->end_label = NTXT ("Total");
  totals->type = VT_HRTIME;

  int nsampsel = 0;
  for (int index = 0; index < size (); index++)
    {
      Ovw_item item = fetch (index);
      nsampsel++;

      // Compute totals
      for (int i = 0; i < OVW_NUMVALS + 1; i++)
	tsadd (&totals->values[i].t, &item.values[i].t);

      int_max (&totals->states, item.states);
      tsadd (&totals->total.t, &item.total.t);
      int_max (&totals->size, item.size);
      tsadd (&totals->duration, &item.duration);
      tsadd (&totals->tlwp, &item.tlwp);
      totals->number += item.number;
      if (tscmp (&totals->start, &item.start) > 0)
	totals->start = item.start;
      if (tscmp (&totals->end, &item.end) < 0)
	totals->end = item.end;
    }

  if (totals->start.tv_sec == MAXINT && totals->start.tv_nsec == MAXINT)
    totals->start = zero;
  totals->nlwp = tstodouble (totals->tlwp) / tstodouble (totals->duration);

  if (nsampsel == 0)
    {
      totals->size = OVW_NUMVALS + 1;
      totals->start.tv_sec = totals->end.tv_sec = -1;
      totals->start.tv_nsec = totals->end.tv_nsec = 0;
      totals->nlwp = -1;
    }
  return *totals;
}

Ovw_data::Ovw_item
Ovw_data::get_labels ()
{
  Ovw_item ovw_item;
  Value *values;
  memset (&ovw_item, 0, sizeof (Ovw_item));
  values = &ovw_item.values[0];

  char *stateUNames[/*LMS_NUM_STATES*/] = LMS_STATE_USTRINGS;
  values[0].l = dbe_strdup (GTXT ("Leftover"));
  values[OVW_LMS_USER + 1].l = stateUNames[LMS_USER];
  values[OVW_LMS_SYSTEM + 1].l = stateUNames[LMS_SYSTEM];
  values[OVW_LMS_WAIT_CPU + 1].l = stateUNames[LMS_WAIT_CPU];
  values[OVW_LMS_USER_LOCK + 1].l = stateUNames[LMS_USER_LOCK];
  values[OVW_LMS_TFAULT + 1].l = stateUNames[LMS_TFAULT];
  values[OVW_LMS_DFAULT + 1].l = stateUNames[LMS_DFAULT];
  values[OVW_LMS_KFAULT + 1].l = stateUNames[LMS_KFAULT];
  values[OVW_LMS_SLEEP + 1].l = stateUNames[LMS_SLEEP];
  values[OVW_LMS_STOPPED + 1].l = stateUNames[LMS_STOPPED];
  values[OVW_LMS_TRAP + 1].l = stateUNames[LMS_TRAP];

  ovw_item.size = OVW_NUMVALS + 1;
  ovw_item.states = 0;
  ovw_item.type = VT_LABEL;
  return ovw_item;
}

Ovw_data::Ovw_data ()
{
  packets = NULL;
  ovw_items = new Vector<Ovw_item*>;
  totals = NULL;
}

Ovw_data::Ovw_data (DataView *_packets, hrtime_t exp_start)
{
  packets = _packets;
  ovw_items = new Vector<Ovw_item*>;
  totals = NULL;
  long npackets = packets->getSize ();
  for (long index = 0; index < npackets; index++)
    {
      Ovw_item *ovw_item = new Ovw_item;
      memset (ovw_item, 0, sizeof (Ovw_item));
      Sample *sample = (Sample*) packets->getObjValue (PROP_SMPLOBJ, index);
      extract_data (ovw_item, sample);
      hr2timestruc (&ovw_item->start, sample->get_start_time () - exp_start);
      hr2timestruc (&ovw_item->end, sample->get_end_time () - exp_start);
      //  No need to check for duration, as duration has to be > 0.
      //  If not, it would have been found out in yyparse.
      tssub (&ovw_item->duration, &ovw_item->end, &ovw_item->start);
      ovw_item->number = sample->get_number ();
      ovw_item->start_label = sample->get_start_label ();
      ovw_item->end_label = sample->get_end_label ();

      int size = ovw_item->size;
      for (int j = 0; j < size; j++)
	tsadd (&ovw_item->tlwp, &ovw_item->values[j].t);
      if (tstodouble (ovw_item->duration) != 0)
	ovw_item->nlwp = tstodouble (ovw_item->tlwp) /
		tstodouble (ovw_item->duration);
      ovw_items->append (ovw_item);
    }
}

Ovw_data::~Ovw_data ()
{
  ovw_items->destroy ();
  delete ovw_items;
  delete totals;
}

void
Ovw_data::extract_data (Ovw_data::Ovw_item *ovw_item, Sample *sample)
{
  // This routine break out the data in "data" into buckets in "ovw_item"
  int index;
  int states;
  timestruc_t sum, rtime;
  timestruc_t zero = {0, 0};
  Value *values;
  PrUsage *prusage = sample->get_usage ();
  if (prusage == NULL)
    prusage = new PrUsage;

  values = &ovw_item->values[0];
  hr2timestruc (&values[OVW_LMS_USER + 1].t, prusage->pr_utime);
  hr2timestruc (&values[OVW_LMS_SYSTEM + 1].t, prusage->pr_stime);
  hr2timestruc (&values[OVW_LMS_WAIT_CPU + 1].t, prusage->pr_wtime);
  hr2timestruc (&values[OVW_LMS_USER_LOCK + 1].t, prusage->pr_ltime);
  hr2timestruc (&values[OVW_LMS_TFAULT + 1].t, prusage->pr_tftime);
  hr2timestruc (&values[OVW_LMS_DFAULT + 1].t, prusage->pr_dftime);
  hr2timestruc (&values[OVW_LMS_TRAP + 1].t, prusage->pr_ttime);
  hr2timestruc (&values[OVW_LMS_KFAULT + 1].t, prusage->pr_kftime);
  hr2timestruc (&values[OVW_LMS_SLEEP + 1].t, prusage->pr_slptime);
  hr2timestruc (&values[OVW_LMS_STOPPED + 1].t, prusage->pr_stoptime);
  ovw_item->size = OVW_NUMVALS + 1;

  //XXX: Compute values[0] as rtime - sum_of(other_times)
  sum = zero;
  states = 0;
  for (index = 1; index < ovw_item->size; index++)
    {
      if (values[index].t.tv_sec != 0 || values[index].t.tv_nsec != 0)
	states++;
      tsadd (&sum, &values[index].t);
    }

  //  If the sum of all times is greater than rtime then adjust
  //  rtime to be equal to sum and also adjust the pr_rtime field
  hr2timestruc (&rtime, prusage->pr_rtime);
  if (tscmp (&sum, &rtime) > 0)
    {
      ovw_item->total.t = sum;
      values[0].t = zero;
    }
  else
    {
      ovw_item->total.t = rtime;
      tssub (&rtime, &rtime, &sum);
      tsadd (&values[0].t, &rtime);
      states++;
    }
  ovw_item->type = VT_HRTIME;
  ovw_item->states = states;
}
