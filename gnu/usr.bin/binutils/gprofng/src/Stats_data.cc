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
//#include <search.h>     //  For the tsearch stuff
#include <assert.h>
//#include <stdlib.h>
#include "Table.h"
#include "Sample.h"
#include "Stats_data.h"
#include "util.h"
#include "i18n.h"

//XXX:	The fundamental problem with this package of routines is that
//XXX:	they should look a lot more like overview data. The result
//XXX:	is that it is not possible to share as much code as would
//XXX:	otherwise be possible.

int
Stats_data::size ()
{
  // Return the number of Stats_item values associated with "this".
  if (stats_items == NULL)
    return 0;
  return stats_items->size ();
}

Stats_data::Stats_item
Stats_data::fetch (int index)
{
  // Routine will return the "index"'th Stats_item associated with "this".
  assert (index >= 0 && index < stats_items->size ());
  return *(stats_items->fetch (index));
}

Stats_data::Stats_data ()
{
  // this constructor for use with sum()
  packets = NULL;
  stats_items = NULL;
}

Stats_data::Stats_data (DataView *_packets)
{
  packets = _packets;
  stats_items = NULL;
  compute_data ();  // reads all data
}

Stats_data::~Stats_data ()
{
  if (stats_items)
    {
      stats_items->destroy ();
      delete stats_items;
    }
}

void
Stats_data::sum (Stats_data *data)
{
  int index;
  Stats_item *stats_item, *data_item;
  if (stats_items == NULL)
    {
      stats_items = new Vector<Stats_item*>;
      Vec_loop (Stats_item*, data->stats_items, index, data_item)
      {
	stats_item = create_stats_item (data_item->value.ll, data_item->label);
	stats_items->append (stats_item);
      }
    }
  else
    {
      Vec_loop (Stats_item*, data->stats_items, index, data_item)
      {
	stats_items->fetch (index)->value.ll += data_item->value.ll;
      }
    }
}

Stats_data::Stats_item *
Stats_data::create_stats_item (long long v, char *l)
{
  Stats_data::Stats_item *st_it;
  st_it = new Stats_data::Stats_item;
  st_it->label = l;
  st_it->value.sign = false;
  st_it->value.ll = v;
  st_it->value.tag = VT_LLONG;
  return st_it;
}

PrUsage *
Stats_data::fetchPrUsage (long index)
{
  // Return the data values corresponding to the "index"'th sample.
  PrUsage *prusage;
  if (packets->getSize () > 0)
    {
      Sample* sample = (Sample*) packets->getObjValue (PROP_SMPLOBJ, index);
      prusage = sample->get_usage ();
      if (prusage != NULL)
	return prusage;
    }
  return new PrUsage;
}

void
Stats_data::compute_data ()
{
  Stats_data::Stats_item *stats_item;
  PrUsage *tots, *temp;
  stats_items = new Vector<Stats_data::Stats_item*>;

  // Precomputation is needed.
  long size = packets->getSize ();
  tots = new PrUsage ();
  for (long index = 0; index < size; index++)
    {
      temp = fetchPrUsage (index);
      tots->pr_tstamp += temp->pr_tstamp;
      tots->pr_create += temp->pr_create;
      tots->pr_term += temp->pr_term;
      tots->pr_rtime += temp->pr_rtime;
      tots->pr_utime += temp->pr_utime;
      tots->pr_stime += temp->pr_stime;
      tots->pr_ttime += temp->pr_ttime;
      tots->pr_tftime += temp->pr_tftime;
      tots->pr_dftime += temp->pr_dftime;
      tots->pr_kftime += temp->pr_kftime;
      tots->pr_slptime += temp->pr_slptime;
      tots->pr_ltime += temp->pr_ltime;
      tots->pr_wtime += temp->pr_wtime;
      tots->pr_stoptime += temp->pr_stoptime;
      tots->pr_minf += temp->pr_minf;
      tots->pr_majf += temp->pr_majf;
      tots->pr_nswap += temp->pr_nswap;
      tots->pr_inblk += temp->pr_inblk;
      tots->pr_oublk += temp->pr_oublk;
      tots->pr_msnd += temp->pr_msnd;
      tots->pr_mrcv += temp->pr_mrcv;
      tots->pr_sigs += temp->pr_sigs;
      tots->pr_vctx += temp->pr_vctx;
      tots->pr_ictx += temp->pr_ictx;
      tots->pr_sysc += temp->pr_sysc;
      tots->pr_ioch += temp->pr_ioch;
    }
  stats_item = create_stats_item ((long long) tots->pr_minf,
				  GTXT ("Minor Page Faults"));
  stats_items->append (stats_item);
  stats_item = create_stats_item ((long long) tots->pr_majf,
				  GTXT ("Major Page Faults"));
  stats_items->append (stats_item);
  stats_item = create_stats_item ((long long) tots->pr_nswap,
				  GTXT ("Process swaps"));
  stats_items->append (stats_item);
  stats_item = create_stats_item ((long long) tots->pr_inblk,
				  GTXT ("Input blocks"));
  stats_items->append (stats_item);
  stats_item = create_stats_item ((long long) tots->pr_oublk,
				  GTXT ("Output blocks"));
  stats_items->append (stats_item);
  stats_item = create_stats_item ((long long) tots->pr_msnd,
				  GTXT ("Messages sent"));
  stats_items->append (stats_item);
  stats_item = create_stats_item ((long long) tots->pr_mrcv,
				  GTXT ("Messages received"));
  stats_items->append (stats_item);
  stats_item = create_stats_item ((long long) tots->pr_sigs,
				  GTXT ("Signals handled"));
  stats_items->append (stats_item);
  stats_item = create_stats_item ((long long) tots->pr_vctx,
				  GTXT ("Voluntary context switches"));
  stats_items->append (stats_item);
  stats_item = create_stats_item ((long long) tots->pr_ictx,
				  GTXT ("Involuntary context switches"));
  stats_items->append (stats_item);
  stats_item = create_stats_item ((long long) tots->pr_sysc,
				  GTXT ("System calls"));
  stats_items->append (stats_item);
  stats_item = create_stats_item ((long long) tots->pr_ioch,
				  GTXT ("Characters of I/O"));
  stats_items->append (stats_item);
  delete tots;
}
