/* Imagination Technologies Meta opcode table.
   Copyright (C) 2013-2023 Free Software Foundation, Inc.
   Contributed by Imagination Technologies Ltd.

   This file is part of GDB and GAS.

   GDB and GAS are free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3, or (at
   your option) any later version.

   GDB and GAS are distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GDB or GAS; see the file COPYING3.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifdef __cplusplus
extern "C" {
#endif

enum metag_unit
{
  UNIT_CT,
  UNIT_D0,
  UNIT_D1,
  UNIT_A0,
  UNIT_A1,
  UNIT_PC,
  UNIT_RD,
  UNIT_TR,
  UNIT_TT,
  UNIT_FX,
  UNIT_DT,			/* DSP Template Table */
  UNIT_ACC_D0,
  UNIT_ACC_D1,
  UNIT_RAM_D0,
  UNIT_RAM_D1,
};

typedef struct
{
  const char *     name;
  enum metag_unit  unit;
  unsigned int     no;
} metag_reg;

static const metag_reg metag_regtab[] =
  {
    { "TXENABLE",   UNIT_CT,  0 },
    { "CT.0",       UNIT_CT,  0 },
    { "TXMODE",     UNIT_CT,  1 },
    { "CT.1",       UNIT_CT,  1 },
    { "TXSTATUS",   UNIT_CT,  2 },
    { "CT.2",       UNIT_CT,  2 },
    { "TXRPT",      UNIT_CT,  3 },
    { "CT.3",       UNIT_CT,  3 },
    { "TXTIMER",    UNIT_CT,  4 },
    { "CT.4",       UNIT_CT,  4 },
    { "TXL1START",  UNIT_CT,  5 },
    { "CT.5",       UNIT_CT,  5 },
    { "TXL1END",    UNIT_CT,  6 },
    { "CT.6",       UNIT_CT,  6 },
    { "TXL1COUNT",  UNIT_CT,  7 },
    { "CT.7",       UNIT_CT,  7 },
    { "TXL2START",  UNIT_CT,  8 },
    { "CT.8",       UNIT_CT,  8 },
    { "TXL2END",    UNIT_CT,  9 },
    { "CT.9",       UNIT_CT,  9 },
    { "TXL2COUNT",  UNIT_CT, 10 },
    { "CT.10",      UNIT_CT, 10 },
    { "TXBPOBITS",  UNIT_CT, 11 },
    { "CT.11",      UNIT_CT, 11 },
    { "TXMRSIZE",   UNIT_CT, 12 },
    { "CT.12",      UNIT_CT, 12 },
    { "TXTIMERI",   UNIT_CT, 13 },
    { "CT.13",      UNIT_CT, 13 },
    { "TXDRCTRL",   UNIT_CT, 14 },
    { "CT.14",      UNIT_CT, 14 },
    { "TXDRSIZE",   UNIT_CT, 15 },
    { "CT.15",      UNIT_CT, 15 },
    { "TXCATCH0",   UNIT_CT, 16 },
    { "CT.16",      UNIT_CT, 16 },
    { "TXCATCH1",   UNIT_CT, 17 },
    { "CT.17",      UNIT_CT, 17 },
    { "TXCATCH2",   UNIT_CT, 18 },
    { "CT.18",      UNIT_CT, 18 },
    { "TXCATCH3",   UNIT_CT, 19 },
    { "CT.19",      UNIT_CT, 19 },
    { "TXDEFR",     UNIT_CT, 20 },
    { "CT.20",      UNIT_CT, 20 },
    { "TXCPRS",     UNIT_CT, 21 },
    { "CT.21",      UNIT_CT, 21 },
    { "TXCLKCTRL",  UNIT_CT, 22 },
    { "CT.22",      UNIT_CT, 22 },
    { "TXINTERN0",  UNIT_CT, 23 },
    { "TXSTATE",    UNIT_CT, 23 },
    { "CT.23",      UNIT_CT, 23 },
    { "TXAMAREG0",  UNIT_CT, 24 },
    { "CT.24",      UNIT_CT, 24 },
    { "TXAMAREG1",  UNIT_CT, 25 },
    { "CT.25",      UNIT_CT, 25 },
    { "TXAMAREG2",  UNIT_CT, 26 },
    { "CT.26",      UNIT_CT, 26 },
    { "TXAMAREG3",  UNIT_CT, 27 },
    { "CT.27",      UNIT_CT, 27 },
    { "TXDIVTIME",  UNIT_CT, 28 },
    { "CT.28",      UNIT_CT, 28 },
    { "TXPRIVEXT",  UNIT_CT, 29 },
    { "CT.29",      UNIT_CT, 29 },
    { "TXTACTCYC",  UNIT_CT, 30 },
    { "TXACTCYC",   UNIT_CT, 30 },
    { "CT.30",      UNIT_CT, 30 },
    { "TXIDLECYC",  UNIT_CT, 31 },
    { "CT.31",      UNIT_CT, 31 },

    { "D0Re0",      UNIT_D0,  0 },
    { "D0.0",       UNIT_D0,  0 },
    { "D0Ar6",      UNIT_D0,  1 },
    { "D0.1",       UNIT_D0,  1 },
    { "D0Ar4",      UNIT_D0,  2 },
    { "D0.2",       UNIT_D0,  2 },
    { "D0Ar2",      UNIT_D0,  3 },
    { "D0.3",       UNIT_D0,  3 },
    { "D0FrT",      UNIT_D0,  4 },
    { "D0.4",       UNIT_D0,  4 },
    { "D0.5",       UNIT_D0,  5 },
    { "D0.6",       UNIT_D0,  6 },
    { "D0.7",       UNIT_D0,  7 },
    { "D0.8",       UNIT_D0,  8 },
    { "D0.9",       UNIT_D0,  9 },
    { "D0.10",      UNIT_D0, 10 },
    { "D0.11",      UNIT_D0, 11 },
    { "D0.12",      UNIT_D0, 12 },
    { "D0.13",      UNIT_D0, 13 },
    { "D0.14",      UNIT_D0, 14 },
    { "D0.15",      UNIT_D0, 15 },
    { "D0.16",      UNIT_D0, 16 },
    { "D0.17",      UNIT_D0, 17 },
    { "D0.18",      UNIT_D0, 18 },
    { "D0.19",      UNIT_D0, 19 },
    { "D0.20",      UNIT_D0, 20 },
    { "D0.21",      UNIT_D0, 21 },
    { "D0.22",      UNIT_D0, 22 },
    { "D0.23",      UNIT_D0, 23 },
    { "D0.24",      UNIT_D0, 24 },
    { "D0.25",      UNIT_D0, 25 },
    { "D0.26",      UNIT_D0, 26 },
    { "D0.27",      UNIT_D0, 27 },
    { "D0.28",      UNIT_D0, 28 },
    { "D0.29",      UNIT_D0, 29 },
    { "D0.30",      UNIT_D0, 30 },
    { "D0.31",      UNIT_D0, 31 },

    { "D1Re0",      UNIT_D1,  0 },
    { "D1.0",       UNIT_D1,  0 },
    { "D1Ar5",      UNIT_D1,  1 },
    { "D1.1",       UNIT_D1,  1 },
    { "D1Ar3",      UNIT_D1,  2 },
    { "D1.2",       UNIT_D1,  2 },
    { "D1Ar1",      UNIT_D1,  3 },
    { "D1.3",       UNIT_D1,  3 },
    { "D1RtP",      UNIT_D1,  4 },
    { "D1.4",       UNIT_D1,  4 },
    { "D1.5",       UNIT_D1,  5 },
    { "D1.6",       UNIT_D1,  6 },
    { "D1.7",       UNIT_D1,  7 },
    { "D1.8",       UNIT_D1,  8 },
    { "D1.9",       UNIT_D1,  9 },
    { "D1.10",      UNIT_D1, 10 },
    { "D1.11",      UNIT_D1, 11 },
    { "D1.12",      UNIT_D1, 12 },
    { "D1.13",      UNIT_D1, 13 },
    { "D1.14",      UNIT_D1, 14 },
    { "D1.15",      UNIT_D1, 15 },
    { "D1.16",      UNIT_D1, 16 },
    { "D1.17",      UNIT_D1, 17 },
    { "D1.18",      UNIT_D1, 18 },
    { "D1.19",      UNIT_D1, 19 },
    { "D1.20",      UNIT_D1, 20 },
    { "D1.21",      UNIT_D1, 21 },
    { "D1.22",      UNIT_D1, 22 },
    { "D1.23",      UNIT_D1, 23 },
    { "D1.24",      UNIT_D1, 24 },
    { "D1.25",      UNIT_D1, 25 },
    { "D1.26",      UNIT_D1, 26 },
    { "D1.27",      UNIT_D1, 27 },
    { "D1.28",      UNIT_D1, 28 },
    { "D1.29",      UNIT_D1, 29 },
    { "D1.30",      UNIT_D1, 30 },
    { "D1.31",      UNIT_D1, 31 },

    { "A0StP",      UNIT_A0,  0 },
    { "A0.0",       UNIT_A0,  0 },
    { "A0FrP",      UNIT_A0,  1 },
    { "A0.1",       UNIT_A0,  1 },
    { "A0.2",       UNIT_A0,  2 },
    { "A0.3",       UNIT_A0,  3 },
    { "A0.4",       UNIT_A0,  4 },
    { "A0.5",       UNIT_A0,  5 },
    { "A0.6",       UNIT_A0,  6 },
    { "A0.7",       UNIT_A0,  7 },
    { "A0.8",       UNIT_A0,  8 },
    { "A0.9",       UNIT_A0,  9 },
    { "A0.10",      UNIT_A0, 10 },
    { "A0.11",      UNIT_A0, 11 },
    { "A0.12",      UNIT_A0, 12 },
    { "A0.13",      UNIT_A0, 13 },
    { "A0.14",      UNIT_A0, 14 },
    { "A0.15",      UNIT_A0, 15 },
    { "CPC0",       UNIT_A0, 16 },

    { "A1GbP",      UNIT_A1,  0 },
    { "A1.0",       UNIT_A1,  0 },
    { "A1LbP",      UNIT_A1,  1 },
    { "A1.1",       UNIT_A1,  1 },
    { "A1.2",       UNIT_A1,  2 },
    { "A1.3",       UNIT_A1,  3 },
    { "A1.4",       UNIT_A1,  4 },
    { "A1.5",       UNIT_A1,  5 },
    { "A1.6",       UNIT_A1,  6 },
    { "A1.7",       UNIT_A1,  7 },
    { "A1.8",       UNIT_A1,  8 },
    { "A1.9",       UNIT_A1,  9 },
    { "A1.10",      UNIT_A1, 10 },
    { "A1.11",      UNIT_A1, 11 },
    { "A1.12",      UNIT_A1, 12 },
    { "A1.13",      UNIT_A1, 13 },
    { "A1.14",      UNIT_A1, 14 },
    { "A1.15",      UNIT_A1, 15 },
    { "CPC1",       UNIT_A1, 16 },

    { "PC",         UNIT_PC,  0 },
    { "PCX",        UNIT_PC,  1 },

    { "RD",         UNIT_RD,  0 },
    { "RA",         UNIT_RD, 16 },
    { "RD",         UNIT_RD, 16 },
    { "RAPF",       UNIT_RD, 17 },
    { "RAM8X32",    UNIT_RD, 22 },
    { "RAM8X",      UNIT_RD, 23 },
    { "RABZ",       UNIT_RD, 24 },
    { "RAWZ",       UNIT_RD, 25 },
    { "RADZ",       UNIT_RD, 26 },
    { "RABX",       UNIT_RD, 28 },
    { "RAWX",       UNIT_RD, 29 },
    { "RADX",       UNIT_RD, 30 },
    { "RAMX",       UNIT_RD, 31 },
    { "RAM16X",     UNIT_RD, 31 },

    { "TXSTAT",     UNIT_TR,  0 },
    { "TR.0",       UNIT_TR,  0 },
    { "TXMASK",     UNIT_TR,  1 },
    { "TR.1",       UNIT_TR,  1 },
    { "TXSTATI",    UNIT_TR,  2 },
    { "TR.2",       UNIT_TR,  2 },
    { "TXMASKI",    UNIT_TR,  3 },
    { "TR.3",       UNIT_TR,  3 },
    { "TXPOLL",     UNIT_TR,  4 },
    { "TR.4",       UNIT_TR,  4 },
    { "TXGPIOI",    UNIT_TR,  5 },
    { "TR.5",       UNIT_TR,  5 },
    { "TXPOLLI",    UNIT_TR,  6 },
    { "TR.6",       UNIT_TR,  6 },
    { "TXGPIOO",    UNIT_TR,  7 },
    { "TR.7",       UNIT_TR,  7 },

    { "TTEXEC",     UNIT_TT,  0 },
    { "TT.0",       UNIT_TT,  0 },
    { "TTCTRL",     UNIT_TT,  1 },
    { "TT.1",       UNIT_TT,  1 },
    { "TTMARK",     UNIT_TT,  2 },
    { "TT.2",       UNIT_TT,  2 },
    { "TTREC",      UNIT_TT,  3 },
    { "TT.3",       UNIT_TT,  3 },
    { "GTEXEC",     UNIT_TT,  4 },
    { "TT.4",       UNIT_TT,  4 },

    { "FX.0",       UNIT_FX,  0 },
    { "FX.1",       UNIT_FX,  1 },
    { "FX.2",       UNIT_FX,  2 },
    { "FX.3",       UNIT_FX,  3 },
    { "FX.4",       UNIT_FX,  4 },
    { "FX.5",       UNIT_FX,  5 },
    { "FX.6",       UNIT_FX,  6 },
    { "FX.7",       UNIT_FX,  7 },
    { "FX.8",       UNIT_FX,  8 },
    { "FX.9",       UNIT_FX,  9 },
    { "FX.10",      UNIT_FX, 10 },
    { "FX.11",      UNIT_FX, 11 },
    { "FX.12",      UNIT_FX, 12 },
    { "FX.13",      UNIT_FX, 13 },
    { "FX.14",      UNIT_FX, 14 },
    { "FX.15",      UNIT_FX, 15 },
  };

static const metag_reg metag_dsp_regtab[] =
  {
    { "D0AR.0",   UNIT_RAM_D0,  0 },
    { "D0AR.1",   UNIT_RAM_D0,  1 },
    { "D0AW.0",   UNIT_RAM_D0,  2 },
    { "D0AW.1",   UNIT_RAM_D0,  3 },
    { "D0BR.0",   UNIT_RAM_D0,  4 },
    { "D0BR.1",   UNIT_RAM_D0,  5 },
    { "D0BW.0",   UNIT_RAM_D0,  6 },
    { "D0BW.1",   UNIT_RAM_D0,  7 },
    { "D0ARI.0",  UNIT_RAM_D0,  8 },
    { "D0ARI.1",  UNIT_RAM_D0,  9 },
    { "D0AWI.0",  UNIT_RAM_D0, 10 },
    { "D0AWI.1",  UNIT_RAM_D0, 11 },
    { "D0BRI.0",  UNIT_RAM_D0, 12 },
    { "D0BRI.1",  UNIT_RAM_D0, 13 },
    { "D0BWI.0",  UNIT_RAM_D0, 14 },
    { "D0BWI.1",  UNIT_RAM_D0, 15 },

    { "AC0.0",    UNIT_ACC_D0, 16 },
    { "AC0.1",    UNIT_ACC_D0, 17 },
    { "AC0.2",    UNIT_ACC_D0, 18 },
    { "AC0.3",    UNIT_ACC_D0, 19 },

    { "D1AR.0",   UNIT_RAM_D1,  0 },
    { "D1AR.1",   UNIT_RAM_D1,  1 },
    { "D1AW.0",   UNIT_RAM_D1,  2 },
    { "D1AW.1",   UNIT_RAM_D1,  3 },
    { "D1BR.0",   UNIT_RAM_D1,  4 },
    { "D1BR.1",   UNIT_RAM_D1,  5 },
    { "D1BW.0",   UNIT_RAM_D1,  6 },
    { "D1BW.1",   UNIT_RAM_D1,  7 },
    { "D1ARI.0",  UNIT_RAM_D1,  8 },
    { "D1ARI.1",  UNIT_RAM_D1,  9 },
    { "D1AWI.0",  UNIT_RAM_D1, 10 },
    { "D1AWI.1",  UNIT_RAM_D1, 11 },
    { "D1BRI.0",  UNIT_RAM_D1, 12 },
    { "D1BRI.1",  UNIT_RAM_D1, 13 },
    { "D1BWI.0",  UNIT_RAM_D1, 14 },
    { "D1BWI.1",  UNIT_RAM_D1, 15 },

    { "AC1.0",    UNIT_ACC_D1, 16 },
    { "AC1.1",    UNIT_ACC_D1, 17 },
    { "AC1.2",    UNIT_ACC_D1, 18 },
    { "AC1.3",    UNIT_ACC_D1, 19 },

    { "T0",       UNIT_DT,  0 },
    { "T1",       UNIT_DT,  1 },
    { "T2",       UNIT_DT,  2 },
    { "T3",       UNIT_DT,  3 },
    { "T4",       UNIT_DT,  4 },
    { "T5",       UNIT_DT,  5 },
    { "T6",       UNIT_DT,  6 },
    { "T7",       UNIT_DT,  7 },
    { "T8",       UNIT_DT,  8 },
    { "T9",       UNIT_DT,  9 },
    { "TA",       UNIT_DT, 10 },
    { "TB",       UNIT_DT, 11 },
    { "TC",       UNIT_DT, 12 },
    { "TD",       UNIT_DT, 13 },
    { "TE",       UNIT_DT, 14 },
    { "TF",       UNIT_DT, 15 },
  };

/* This table differs from 'metag_dsp_regtab' in that the number
   fields in this table are suitable for insertion into DSPRAM
   template definition instruction encodings.

   The table is indexed by "load". The main benefit of this is that we
   can implicitly check that the correct DSPRAM register has been used
   when parsing, e.g. the read pointer only appears in the load table
   and the write pointer only exists in the store table.

   The ordering of the table entries might look a bit weird but it is
   based on matching the longest register string. */
static const metag_reg metag_dsp_tmpl_regtab[2][56] =
  {
    {
      { "D0AW.0+D0AWI.0++", UNIT_RAM_D0, 18 },
      { "D0AW.0+D0AWI.0",   UNIT_RAM_D0, 18 },
      { "D0AW.0+D0AWI.1++", UNIT_RAM_D0, 19 },
      { "D0AW.0+D0AWI.1",   UNIT_RAM_D0, 19 },
      { "D0AW.0++",         UNIT_RAM_D0, 17 },
      { "D0AW.0",           UNIT_RAM_D0, 16 },
      { "D0AWI.0",          UNIT_RAM_D0, 18 },
      { "D0AWI.1",          UNIT_RAM_D0, 19 },
      { "D0AW.1+D0AWI.0++", UNIT_RAM_D0, 22 },
      { "D0AW.1+D0AWI.0",   UNIT_RAM_D0, 22 },
      { "D0AW.1+D0AWI.1++", UNIT_RAM_D0, 23 },
      { "D0AW.1+D0AWI.1",   UNIT_RAM_D0, 23 },
      { "D0AW.1++",         UNIT_RAM_D0, 21 },
      { "D0AW.1",           UNIT_RAM_D0, 20 },
      { "D0BW.0+D0BWI.0++", UNIT_RAM_D0, 26 },
      { "D0BW.0+D0BWI.0",   UNIT_RAM_D0, 26 },
      { "D0BW.0+D0BWI.1++", UNIT_RAM_D0, 27 },
      { "D0BW.0+D0BWI.1",   UNIT_RAM_D0, 27 },
      { "D0BW.0++",         UNIT_RAM_D0, 25 },
      { "D0BW.0",           UNIT_RAM_D0, 24 },
      { "D0BWI.0",          UNIT_RAM_D0, 18 },
      { "D0BWI.1",          UNIT_RAM_D0, 19 },
      { "D0BW.1+D0BWI.0++", UNIT_RAM_D0, 30 },
      { "D0BW.1+D0BWI.0",   UNIT_RAM_D0, 30 },
      { "D0BW.1+D0BWI.1++", UNIT_RAM_D0, 31 },
      { "D0BW.1+D0BWI.1",   UNIT_RAM_D0, 31 },
      { "D0BW.1++",         UNIT_RAM_D0, 29 },
      { "D0BW.1",           UNIT_RAM_D0, 28 },

      { "D1AW.0+D1AWI.0++", UNIT_RAM_D1, 18 },
      { "D1AW.0+D1AWI.0",   UNIT_RAM_D1, 18 },
      { "D1AW.0+D1AWI.1++", UNIT_RAM_D1, 19 },
      { "D1AW.0+D1AWI.1",   UNIT_RAM_D1, 19 },
      { "D1AW.0++",         UNIT_RAM_D1, 17 },
      { "D1AW.0",           UNIT_RAM_D1, 16 },
      { "D1AWI.0",          UNIT_RAM_D1, 18 },
      { "D1AWI.1",          UNIT_RAM_D1, 19 },
      { "D1AW.1+D1AWI.0++", UNIT_RAM_D1, 22 },
      { "D1AW.1+D1AWI.0",   UNIT_RAM_D1, 22 },
      { "D1AW.1+D1AWI.1++", UNIT_RAM_D1, 23 },
      { "D1AW.1+D1AWI.1",   UNIT_RAM_D1, 23 },
      { "D1AW.1++",         UNIT_RAM_D1, 21 },
      { "D1AW.1",           UNIT_RAM_D1, 20 },
      { "D1BW.0+D1BWI.0++", UNIT_RAM_D1, 26 },
      { "D1BW.0+D1BWI.0",   UNIT_RAM_D1, 26 },
      { "D1BW.0+D1BWI.1++", UNIT_RAM_D1, 27 },
      { "D1BW.0+D1BWI.1",   UNIT_RAM_D1, 27 },
      { "D1BW.0++",         UNIT_RAM_D1, 25 },
      { "D1BW.0",           UNIT_RAM_D1, 24 },
      { "D1BWI.0",          UNIT_RAM_D1, 18 },
      { "D1BWI.1",          UNIT_RAM_D1, 19 },
      { "D1BW.1+D1BWI.0++", UNIT_RAM_D1, 30 },
      { "D1BW.1+D1BWI.0",   UNIT_RAM_D1, 30 },
      { "D1BW.1+D1BWI.1++", UNIT_RAM_D1, 31 },
      { "D1BW.1+D1BWI.1",   UNIT_RAM_D1, 31 },
      { "D1BW.1++",         UNIT_RAM_D1, 29 },
      { "D1BW.1",           UNIT_RAM_D1, 28 },
    },

    {
      { "D0AR.0+D0ARI.0++", UNIT_RAM_D0, 18 },
      { "D0AR.0+D0ARI.0",   UNIT_RAM_D0, 18 },
      { "D0AR.0+D0ARI.1++", UNIT_RAM_D0, 19 },
      { "D0AR.0+D0ARI.1",   UNIT_RAM_D0, 19 },
      { "D0AR.0++",         UNIT_RAM_D0, 17 },
      { "D0AR.0",           UNIT_RAM_D0, 16 },
      { "D0ARI.0",          UNIT_RAM_D0, 18 },
      { "D0ARI.1",          UNIT_RAM_D0, 19 },
      { "D0AR.1+D0ARI.0++", UNIT_RAM_D0, 22 },
      { "D0AR.1+D0ARI.0",   UNIT_RAM_D0, 22 },
      { "D0AR.1+D0ARI.1++", UNIT_RAM_D0, 23 },
      { "D0AR.1+D0ARI.1",   UNIT_RAM_D0, 23 },
      { "D0AR.1++",         UNIT_RAM_D0, 21 },
      { "D0AR.1",           UNIT_RAM_D0, 20 },
      { "D0BR.0+D0BRI.0++", UNIT_RAM_D0, 26 },
      { "D0BR.0+D0BRI.0",   UNIT_RAM_D0, 26 },
      { "D0BR.0+D0BRI.1++", UNIT_RAM_D0, 27 },
      { "D0BR.0+D0BRI.1",   UNIT_RAM_D0, 27 },
      { "D0BR.0++",         UNIT_RAM_D0, 25 },
      { "D0BR.0",           UNIT_RAM_D0, 24 },
      { "D0BRI.0",          UNIT_RAM_D0, 18 },
      { "D0BRI.1",          UNIT_RAM_D0, 19 },
      { "D0BR.1+D0BRI.0++", UNIT_RAM_D0, 30 },
      { "D0BR.1+D0BRI.0",   UNIT_RAM_D0, 30 },
      { "D0BR.1+D0BRI.1++", UNIT_RAM_D0, 31 },
      { "D0BR.1+D0BRI.1",   UNIT_RAM_D0, 31 },
      { "D0BR.1++",         UNIT_RAM_D0, 29 },
      { "D0BR.1",           UNIT_RAM_D0, 28 },

      { "D1AR.0+D1ARI.0++", UNIT_RAM_D1, 18 },
      { "D1AR.0+D1ARI.0",   UNIT_RAM_D1, 18 },
      { "D1AR.0+D1ARI.1++", UNIT_RAM_D1, 19 },
      { "D1AR.0+D1ARI.1",   UNIT_RAM_D1, 19 },
      { "D1AR.0++",         UNIT_RAM_D1, 17 },
      { "D1AR.0",           UNIT_RAM_D1, 16 },
      { "D1ARI.0",          UNIT_RAM_D1, 18 },
      { "D1ARI.1",          UNIT_RAM_D1, 19 },
      { "D1AR.1+D1ARI.0++", UNIT_RAM_D1, 22 },
      { "D1AR.1+D1ARI.0",   UNIT_RAM_D1, 22 },
      { "D1AR.1+D1ARI.1++", UNIT_RAM_D1, 23 },
      { "D1AR.1+D1ARI.1",   UNIT_RAM_D1, 23 },
      { "D1AR.1++",         UNIT_RAM_D1, 21 },
      { "D1AR.1",           UNIT_RAM_D1, 20 },
      { "D1BR.0+D1BRI.0++", UNIT_RAM_D1, 26 },
      { "D1BR.0+D1BRI.0",   UNIT_RAM_D1, 26 },
      { "D1BR.0+D1BRI.1++", UNIT_RAM_D1, 27 },
      { "D1BR.0+D1BRI.1",   UNIT_RAM_D1, 27 },
      { "D1BR.0++",         UNIT_RAM_D1, 25 },
      { "D1BR.0",           UNIT_RAM_D1, 24 },
      { "D1BR.1+D1BRI.0++", UNIT_RAM_D1, 30 },
      { "D1BR.1+D1BRI.0",   UNIT_RAM_D1, 30 },
      { "D1BR.1+D1BRI.1++", UNIT_RAM_D1, 31 },
      { "D1BR.1+D1BRI.1",   UNIT_RAM_D1, 31 },
      { "D1BR.1++",         UNIT_RAM_D1, 29 },
      { "D1BR.1",           UNIT_RAM_D1, 28 },
      { "D1BRI.0",          UNIT_RAM_D1, 18 },
      { "D1BRI.1",          UNIT_RAM_D1, 19 },
    },
  };

typedef struct
{
  const char *  name;
  unsigned int  part;
} metag_acf;

static const metag_acf metag_acftab[] =
  {
    { "ACF.0", 0},
    { "ACF.1", 1},
    { "ACF.2", 2},
    { "ACF.3", 3},
  };

enum insn_encoding
{
  ENC_NONE,
  ENC_MOV_U2U,
  ENC_MOV_PORT,
  ENC_MMOV,
  ENC_MDRD,
  ENC_MOVL_TTREC,
  ENC_GET_SET,
  ENC_GET_SET_EXT,
  ENC_MGET_MSET,
  ENC_COND_SET,
  ENC_XFR,
  ENC_MOV_CT,
  ENC_SWAP,
  ENC_JUMP,
  ENC_CALLR,
  ENC_ALU,
  ENC_SHIFT,
  ENC_MIN_MAX,
  ENC_BITOP,
  ENC_CMP,
  ENC_BRANCH,
  ENC_KICK,
  ENC_SWITCH,
  ENC_CACHER,
  ENC_CACHEW,
  ENC_ICACHE,
  ENC_LNKGET,
  ENC_FMOV,
  ENC_FMMOV,
  ENC_FMOV_DATA,
  ENC_FMOV_I,
  ENC_FPACK,
  ENC_FSWAP,
  ENC_FCMP,
  ENC_FMINMAX,
  ENC_FCONV,
  ENC_FCONVX,
  ENC_FBARITH,
  ENC_FEARITH,
  ENC_FREC,
  ENC_FSIMD,
  ENC_FGET_SET_ACF,
  ENC_DGET_SET,
  ENC_DTEMPLATE,
  ENC_DALU,
  ENC_MAX,
};

enum insn_type
{
  INSN_GP,
  INSN_FPU,
  INSN_DSP,
  INSN_DSP_FPU,
};

typedef struct
{
  const char *name;

  unsigned int core_flags;
#define CoreMeta11             0x1 /* The earliest Meta core we support */
#define CoreMeta12             0x2
#define CoreMeta21             0x4

#define FpuMeta21             0x21

#define DspMeta21             0x100

  unsigned int meta_opcode;
  unsigned int meta_mask;

  enum insn_type insn_type;

  enum insn_encoding encoding;

#define DSP_ARGS_1    0x0000001 /* De.r,Dx.r,De.r (3 register operands) */
#define DSP_ARGS_ACC2 0x0000002 /* Accumulator source operand 2 */
#define DSP_ARGS_QR   0x0000004 /* QUICKRoT */
#define DSP_ARGS_XACC 0x0000008 /* Cross-unit accumulator op */
#define DSP_ARGS_DACC 0x0000010 /* Target accumulator as destination */
#define DSP_ARGS_SRD  0x0000020 /* Source the RD port */
#define DSP_ARGS_2    0x0000040 /* De.r,Dx.r (2 register operands) */
#define DSP_ARGS_DSP_SRC1   0x0000080 /* Source a DSP register */
#define DSP_ARGS_DSP_SRC2   0x0000100 /* Source a DSP register */
#define DSP_ARGS_IMM 0x0000200 /* Immediate value for src 2 */
#define DSP_ARGS_SPLIT8  0x0000400 /* Data unit split 8 operations */
#define DSP_ARGS_12  0x0000800 /* De.r,Dx.r */
#define DSP_ARGS_13  0x0001000 /* Dx.r,Rx.r */
#define DSP_ARGS_14  0x0002000 /* DSPe.r,Dx.r */
#define DSP_ARGS_15  0x0004000 /* DSPx.r,#I16 */
#define DSP_ARGS_16  0x0008000 /* De.r,DSPx.r */
#define DSP_ARGS_17  0x0010000 /* De.r|ACe.r,Dx.r,Rx.r|RD */
#define DSP_ARGS_18  0x0020000 /* De.r,Dx.r|ACx.r */
#define DSP_ARGS_20  0x0080000 /* De.r,Dx.r|ACx.r,De.r */
#define DSP_ARGS_21  0x0100000 /* De.r,Dx.r|ACx.r,#I5 */
#define DSP_ARGS_22  0x0200000 /* De.r,Dx.r|ACx.r,De.r|#I5 */
#define DSP_ARGS_23  0x0400000 /* Ux.r,Dx.r|ACx.r,De.r|#I5 */
#define GP_ARGS_QR   0x0000001 /* QUICKRoT */
  unsigned int arg_type;
} insn_template;

enum major_opcode
{
  OPC_ADD,
  OPC_SUB,
  OPC_AND,
  OPC_OR,
  OPC_XOR,
  OPC_SHIFT,
  OPC_MUL,
  OPC_CMP,
  OPC_ADDR,
  OPC_9,
  OPC_MISC,
  OPC_SET,
  OPC_GET,
  OPC_XFR,
  OPC_CPR,
  OPC_FPU,
};

#define GET_EXT_MINOR        0x7
#define MOV_EXT_MINOR        0x6
#define MOVL_MINOR           0x2

#define MAJOR_OPCODE(opcode) (((opcode) >> 28) & 0xf)
#define MINOR_OPCODE(opcode) (((opcode) >> 24) & 0xf)

enum cond_code
{
  COND_A,
  COND_EQ,
  COND_NE,
  COND_CS,
  COND_CC,
  COND_MI,
  COND_PL,
  COND_VS,
  COND_VC,
  COND_HI,
  COND_LS,
  COND_GE,
  COND_LT,
  COND_GT,
  COND_LE,
  COND_NV,
};

enum scond_code
{
  SCOND_A,
  SCOND_LEQ,
  SCOND_LNE,
  SCOND_LLO,
  SCOND_LHS,
  SCOND_HEQ,
  SCOND_HNE,
  SCOND_HLO,
  SCOND_HHS,
  SCOND_LGR,
  SCOND_LLE,
  SCOND_HGR,
  SCOND_HLE,
  SCOND_EEQ,
  SCOND_ELO,
  SCOND_NV,
};

typedef struct
{
  const char *name;
  enum scond_code code;
} split_condition;

static const split_condition metag_scondtab[] ATTRIBUTE_UNUSED =
  {
    { "LEQ",   SCOND_LEQ },
    { "LEZ",   SCOND_LEQ },
    { "LNE",   SCOND_LNE },
    { "LNZ",   SCOND_LNE },
    { "LLO",   SCOND_LLO },
    { "LCS",   SCOND_LLO },
    { "LHS",   SCOND_LHS },
    { "LCC",   SCOND_LHS },
    { "HEQ",   SCOND_HEQ },
    { "HEZ",   SCOND_HEQ },
    { "HNE",   SCOND_HNE },
    { "HNZ",   SCOND_HNE },
    { "HLO",   SCOND_HLO },
    { "HCS",   SCOND_HLO },
    { "HHS",   SCOND_HHS },
    { "HCC",   SCOND_HHS },
    { "LGR",   SCOND_LGR },
    { "LHI",   SCOND_LGR },
    { "LLE",   SCOND_LLE },
    { "LLS",   SCOND_LLE },
    { "HGR",   SCOND_HGR },
    { "HHI",   SCOND_HGR },
    { "HLE",   SCOND_HLE },
    { "HLS",   SCOND_HLE },
    { "EEQ",   SCOND_EEQ },
    { "EEZ",   SCOND_EEQ },
    { "ELO",   SCOND_ELO },
    { "ECS",   SCOND_ELO },
  };

static const split_condition metag_dsp_scondtab[] =
  {
    { "LEQ",   SCOND_LEQ },
    { "LEZ",   SCOND_LEQ },
    { "LNE",   SCOND_LNE },
    { "LNZ",   SCOND_LNE },
    { "LCS",   SCOND_LLO },
    { "LLO",   SCOND_LLO },
    { "LCC",   SCOND_LHS },
    { "LHS",   SCOND_LHS },
    { "HEQ",   SCOND_HEQ },
    { "HEZ",   SCOND_HEQ },
    { "HNE",   SCOND_HNE },
    { "HNZ",   SCOND_HNE },
    { "HCS",   SCOND_HLO },
    { "HLO",   SCOND_HLO },
    { "HCC",   SCOND_HHS },
    { "HHS",   SCOND_HHS },
    { "LHI",   SCOND_LGR },
    { "LGR",   SCOND_LGR },
    { "LLS",   SCOND_LLE },
    { "LLE",   SCOND_LLE },
    { "HHI",   SCOND_HGR },
    { "HGR",   SCOND_HGR },
    { "HLS",   SCOND_HLE },
    { "HLE",   SCOND_HLE },
    { "EEQ",   SCOND_EEQ },
    { "EEZ",   SCOND_EEQ },
    { "ECS",   SCOND_ELO },
    { "ELO",   SCOND_ELO },
  };

static const split_condition metag_fpu_scondtab[] =
  {
    { "LEQ",   SCOND_LEQ },
    { "LEZ",   SCOND_LEQ },
    { "LNE",   SCOND_LNE },
    { "LNZ",   SCOND_LNE },
    { "LLO",   SCOND_LLO },
    { "LCS",   SCOND_LLO },
    { "LHS",   SCOND_LHS },
    { "LCC",   SCOND_LHS },
    { "HEQ",   SCOND_HEQ },
    { "HEZ",   SCOND_HEQ },
    { "HNE",   SCOND_HNE },
    { "HNZ",   SCOND_HNE },
    { "HLO",   SCOND_HLO },
    { "HCS",   SCOND_HLO },
    { "HHS",   SCOND_HHS },
    { "HCC",   SCOND_HHS },
    { "LGR",   SCOND_LGR },
    { "LHI",   SCOND_LGR },
    { "LLE",   SCOND_LLE },
    { "LLS",   SCOND_LLE },
    { "HGR",   SCOND_HGR },
    { "HHI",   SCOND_HGR },
    { "HLE",   SCOND_HLE },
    { "HLS",   SCOND_HLE },
    { "EEQ",   SCOND_EEQ },
    { "EEZ",   SCOND_EEQ },
    { "ELO",   SCOND_ELO },
    { "ECS",   SCOND_ELO },
  };

enum fcond_code
{
  FCOND_A,
  FCOND_FEQ,
  FCOND_UNE,
  FCOND_FLT,
  FCOND_UGE,

  FCOND_UVS = 7,
  FCOND_FVC,
  FCOND_UGT,
  FCOND_FLE,
  FCOND_FGE,
  FCOND_ULT,
  FCOND_FGT,
  FCOND_ULE,
  FCOND_NV,
};

#define COND_INSN(mnemonic, suffix, field_shift, flags, meta_opcode,	\
		  meta_mask, insn_type,	encoding, args)			\
  { mnemonic suffix, flags, meta_opcode, meta_mask,			\
      insn_type, encoding, args },					\
  { mnemonic "A" suffix, flags, meta_opcode, meta_mask,			\
      insn_type, encoding, args },					\
  { mnemonic "EQ" suffix, flags, meta_opcode | (COND_EQ << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "Z" suffix, flags, meta_opcode | (COND_EQ << field_shift),	\
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "NE" suffix, flags, meta_opcode | (COND_NE << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "NZ" suffix, flags, meta_opcode | (COND_NE << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "CS" suffix, flags, meta_opcode | (COND_CS << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "LO" suffix, flags, meta_opcode | (COND_CS << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "CC" suffix, flags, meta_opcode | (COND_CC << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "HS" suffix, flags, meta_opcode | (COND_CC << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "MI" suffix, flags, meta_opcode | (COND_MI << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "N" suffix, flags, meta_opcode | (COND_MI << field_shift),	\
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "PL" suffix, flags, meta_opcode | (COND_PL << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "NC" suffix, flags, meta_opcode | (COND_PL << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "VS" suffix, flags, meta_opcode | (COND_VS << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "VC" suffix, flags, meta_opcode | (COND_VC << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "HI" suffix, flags, meta_opcode | (COND_HI << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "LS" suffix, flags, meta_opcode | (COND_LS << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "GE" suffix, flags, meta_opcode | (COND_GE << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "LT" suffix, flags, meta_opcode | (COND_LT << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "GT" suffix, flags, meta_opcode | (COND_GT << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "LE" suffix, flags, meta_opcode | (COND_LE << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "NV" suffix, flags, meta_opcode | (COND_NV << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "FEQ" suffix, flags, meta_opcode |				\
      (FCOND_FEQ << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FZ" suffix, flags, meta_opcode |				\
      (FCOND_FEQ << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "UNE" suffix, flags, meta_opcode |				\
      (FCOND_UNE << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "UNZ" suffix, flags, meta_opcode |				\
      (FCOND_UNE << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FLT" suffix, flags, meta_opcode |				\
      (FCOND_FLT << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FLO" suffix, flags, meta_opcode |				\
      (FCOND_FLT << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "UGE" suffix, flags, meta_opcode |				\
      (FCOND_UGE << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "UHS" suffix, flags, meta_opcode |				\
      (FCOND_UGE << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "UVS" suffix, flags, meta_opcode |				\
      (FCOND_UVS << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FVC" suffix, flags, meta_opcode |				\
      (FCOND_FVC << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "UGT" suffix, flags, meta_opcode |				\
      (FCOND_UGT << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "UHI" suffix, flags, meta_opcode |				\
      (FCOND_UGT << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FLE" suffix, flags, meta_opcode |				\
      (FCOND_FLE << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FGE" suffix, flags, meta_opcode |				\
      (FCOND_FGE << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FHS" suffix, flags, meta_opcode |				\
      (FCOND_FGE << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "ULT" suffix, flags, meta_opcode |				\
      (FCOND_ULT << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "ULO" suffix, flags, meta_opcode |				\
      (FCOND_ULT << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FGT" suffix, flags, meta_opcode |				\
      (FCOND_FGT << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FHI" suffix, flags, meta_opcode |				\
      (FCOND_FGT << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "ULE" suffix, flags, meta_opcode |				\
      (FCOND_ULE << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "NV" suffix, flags, meta_opcode |				\
      (FCOND_NV << field_shift), meta_mask, INSN_FPU, encoding, args }

#define FCOND_INSN(mnemonic, suffix, field_shift, flags, meta_opcode,	\
		  meta_mask, insn_type,	encoding, args)			\
  { mnemonic suffix, flags, meta_opcode, meta_mask,			\
      insn_type, encoding, args },					\
  { mnemonic "A" suffix, flags, meta_opcode, meta_mask,			\
      insn_type, encoding, args },					\
  { mnemonic "FEQ" suffix, flags, meta_opcode |				\
      (FCOND_FEQ << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FZ" suffix, flags, meta_opcode |				\
      (FCOND_FEQ << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "UNE" suffix, flags, meta_opcode |				\
      (FCOND_UNE << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "UNZ" suffix, flags, meta_opcode |				\
      (FCOND_UNE << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FLO" suffix, flags, meta_opcode |				\
      (FCOND_FLT << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FLT" suffix, flags, meta_opcode |				\
      (FCOND_FLT << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "UHS" suffix, flags, meta_opcode |				\
      (FCOND_UGE << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "UGE" suffix, flags, meta_opcode |				\
      (FCOND_UGE << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "UVS" suffix, flags, meta_opcode |				\
      (FCOND_UVS << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FVC" suffix, flags, meta_opcode |				\
      (FCOND_FVC << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "UHI" suffix, flags, meta_opcode |				\
      (FCOND_UGT << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "UGT" suffix, flags, meta_opcode |				\
      (FCOND_UGT << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FLE" suffix, flags, meta_opcode |				\
      (FCOND_FLE << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FGE" suffix, flags, meta_opcode |				\
      (FCOND_FGE << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FHS" suffix, flags, meta_opcode |				\
      (FCOND_FGE << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "ULT" suffix, flags, meta_opcode |				\
      (FCOND_ULT << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "ULO" suffix, flags, meta_opcode |				\
      (FCOND_ULT << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FGT" suffix, flags, meta_opcode |				\
      (FCOND_FGT << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "FHI" suffix, flags, meta_opcode |				\
      (FCOND_FGT << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "ULE" suffix, flags, meta_opcode |				\
      (FCOND_ULE << field_shift), meta_mask, INSN_FPU, encoding, args }, \
  { mnemonic "NV" suffix, flags, meta_opcode |				\
      (FCOND_NV << field_shift), meta_mask, INSN_FPU, encoding, args },	\
  { mnemonic "EQ" suffix, flags, meta_opcode | (COND_EQ << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "Z" suffix, flags, meta_opcode | (COND_EQ << field_shift),	\
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "NE" suffix, flags, meta_opcode | (COND_NE << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "NZ" suffix, flags, meta_opcode | (COND_NE << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "CS" suffix, flags, meta_opcode | (COND_CS << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "LO" suffix, flags, meta_opcode | (COND_CS << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "CC" suffix, flags, meta_opcode | (COND_CC << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "HS" suffix, flags, meta_opcode | (COND_CC << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "MI" suffix, flags, meta_opcode | (COND_MI << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "N" suffix, flags, meta_opcode | (COND_MI << field_shift),	\
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "PL" suffix, flags, meta_opcode | (COND_PL << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "NC" suffix, flags, meta_opcode | (COND_PL << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "VS" suffix, flags, meta_opcode | (COND_VS << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "VC" suffix, flags, meta_opcode | (COND_VC << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "HI" suffix, flags, meta_opcode | (COND_HI << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "LS" suffix, flags, meta_opcode | (COND_LS << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "GE" suffix, flags, meta_opcode | (COND_GE << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "LT" suffix, flags, meta_opcode | (COND_LT << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "GT" suffix, flags, meta_opcode | (COND_GT << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "LE" suffix, flags, meta_opcode | (COND_LE << field_shift), \
      meta_mask, insn_type, encoding, args },				\
  { mnemonic "NV" suffix, flags, meta_opcode | (COND_NV << field_shift), \
      meta_mask, insn_type, encoding, args }

#define TEMPLATE_INSN(flags, meta_opcode, meta_mask, insn_type)		\
  { "T0", flags, meta_opcode | 0x0, meta_mask, insn_type, ENC_DTEMPLATE, 0 }, \
  { "T1", flags, meta_opcode | 0x1, meta_mask, insn_type, ENC_DTEMPLATE, 0 }, \
  { "T2", flags, meta_opcode | 0x2, meta_mask, insn_type, ENC_DTEMPLATE, 0 }, \
  { "T3", flags, meta_opcode | 0x3, meta_mask, insn_type, ENC_DTEMPLATE, 0 }, \
  { "T4", flags, meta_opcode | 0x4, meta_mask, insn_type, ENC_DTEMPLATE, 0 }, \
  { "T5", flags, meta_opcode | 0x5, meta_mask, insn_type, ENC_DTEMPLATE, 0 }, \
  { "T6", flags, meta_opcode | 0x6, meta_mask, insn_type, ENC_DTEMPLATE, 0 }, \
  { "T7", flags, meta_opcode | 0x7, meta_mask, insn_type, ENC_DTEMPLATE, 0 }, \
  { "T8", flags, meta_opcode | 0x8, meta_mask, insn_type, ENC_DTEMPLATE, 0 }, \
  { "T9", flags, meta_opcode | 0x9, meta_mask, insn_type, ENC_DTEMPLATE, 0 }, \
  { "TA", flags, meta_opcode | 0xa, meta_mask, insn_type, ENC_DTEMPLATE, 0 }, \
  { "TB", flags, meta_opcode | 0xb, meta_mask, insn_type, ENC_DTEMPLATE, 0 }, \
  { "TC", flags, meta_opcode | 0xc, meta_mask, insn_type, ENC_DTEMPLATE, 0 }, \
  { "TD", flags, meta_opcode | 0xd, meta_mask, insn_type, ENC_DTEMPLATE, 0 }, \
  { "TE", flags, meta_opcode | 0xe, meta_mask, insn_type, ENC_DTEMPLATE, 0 }, \
  { "TF", flags, meta_opcode | 0xf, meta_mask, insn_type, ENC_DTEMPLATE, 0 }


/* Unimplemented GP instructions:
     CPR - coprocessor read
     CPW - coprocessor write
     MORT - morton order operation
     VPACK, VADD, VSUB - vector instructions
  
   The order of the entries in this table is extremely important. DO
   NOT modify it unless you know what you're doing. If you do modify
   it, be sure to run the entire testsuite to make sure you haven't
   caused a regression.  */

static const insn_template metag_optab[] =
  {
    /* Port-to-unit MOV */
    COND_INSN ("MOVB", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0xa1800000, 0xfff83e1f, INSN_GP, ENC_MOV_PORT, 0),
    COND_INSN ("MOVW", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0xa1800001, 0xfff83e1f, INSN_GP, ENC_MOV_PORT, 0),
    COND_INSN ("MOVD", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0xa1800200, 0xfff83e1f, INSN_GP, ENC_MOV_PORT, 0),
    COND_INSN ("MOVL", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0xa2800000, 0xfff8019f, INSN_GP, ENC_MOV_PORT, 0),

    /* Read pipeline prime/drain */
    { "MMOVD", CoreMeta11|CoreMeta12|CoreMeta21,
      0xca000000, 0xff00001f, INSN_GP, ENC_MMOV, 0 },
    { "MMOVL", CoreMeta11|CoreMeta12|CoreMeta21,
      0xcb000000, 0xff00001f, INSN_GP, ENC_MMOV, 0 },
    { "MMOVD", CoreMeta11|CoreMeta12|CoreMeta21,
      0xcc000000, 0xff07c067, INSN_GP, ENC_MMOV, 0 },
    { "MMOVL", CoreMeta11|CoreMeta12|CoreMeta21,
      0xcd000000, 0xff07c067, INSN_GP, ENC_MMOV, 0 },

    /* Read pipeline flush */
    { "MDRD", CoreMeta11|CoreMeta12|CoreMeta21,
      0xcc000002, 0xffffc07f, INSN_GP, ENC_MDRD, 0 },

    /* Unit-to-TTREC MOVL */
    COND_INSN ("MOVL", "", 1, CoreMeta12|CoreMeta21,
	       0xa2002001, 0xff003e7f, INSN_GP, ENC_MOVL_TTREC, 0),

    /* MOV to RA (extended) */
    { "MOVB", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa6000000, 0xff00001e, INSN_GP, ENC_GET_SET_EXT, 0 },
    { "MOVW", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa6000002, 0xff00001e, INSN_GP, ENC_GET_SET_EXT, 0 },
    { "MOVD", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa6000004, 0xff00001e, INSN_GP, ENC_GET_SET_EXT, 0 },
    { "MOVL", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa6000006, 0xff00001e, INSN_GP, ENC_GET_SET_EXT, 0 },

    /* Extended GET */
    { "GETB", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa7000000, 0xff000006, INSN_GP, ENC_GET_SET_EXT, 0 },
    { "GETW", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa7000002, 0xff000006, INSN_GP, ENC_GET_SET_EXT, 0 },
    { "GETD", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa7000004, 0xff000006, INSN_GP, ENC_GET_SET_EXT, 0 },
    { "GETL", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa7000006, 0xff000006, INSN_GP, ENC_GET_SET_EXT, 0 },

    /* Extended SET */
    { "SETB", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa5000000, 0xff000006, INSN_GP, ENC_GET_SET_EXT, 0 },
    { "SETW", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa5000002, 0xff000006, INSN_GP, ENC_GET_SET_EXT, 0 },
    { "SETD", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa5000004, 0xff000006, INSN_GP, ENC_GET_SET_EXT, 0 },
    { "SETL", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa5000006, 0xff000006, INSN_GP, ENC_GET_SET_EXT, 0 },

    /* MOV to RA */
    { "MOVB", CoreMeta11|CoreMeta12|CoreMeta21,
      0xc000000c, 0xfd00001e, INSN_GP, ENC_GET_SET, 0 },
    { "MOVW", CoreMeta11|CoreMeta12|CoreMeta21,
      0xc100000c, 0xfd00001e, INSN_GP, ENC_GET_SET, 0 },
    { "MOVD", CoreMeta11|CoreMeta12|CoreMeta21,
      0xc400000c, 0xfd00001e, INSN_GP, ENC_GET_SET, 0 },
    { "MOVL", CoreMeta11|CoreMeta12|CoreMeta21,
      0xc500000c, 0xfd00001e, INSN_GP, ENC_GET_SET, 0 },

    /* Standard GET */
    { "GETB", CoreMeta11|CoreMeta12|CoreMeta21,
      0xc0000000, 0xfd000000, INSN_GP, ENC_GET_SET, 0 },
    { "GETW", CoreMeta11|CoreMeta12|CoreMeta21,
      0xc1000000, 0xfd000000, INSN_GP, ENC_GET_SET, 0 },
    { "GETD", CoreMeta11|CoreMeta12|CoreMeta21,
      0xc4000000, 0xfd000000, INSN_GP, ENC_GET_SET, 0 },
    /* GET is a synonym for GETD. */
    { "GET", CoreMeta11|CoreMeta12|CoreMeta21,
      0xc4000000, 0xfd000000, INSN_GP, ENC_GET_SET, 0 },
    { "GETL", CoreMeta11|CoreMeta12|CoreMeta21,
      0xc5000000, 0xfd000000, INSN_GP, ENC_GET_SET, 0 },

    /* Standard SET */
    { "SETB", CoreMeta11|CoreMeta12|CoreMeta21,
      0xb0000000, 0xfd000000, INSN_GP, ENC_GET_SET, 0 },
    { "SETW", CoreMeta11|CoreMeta12|CoreMeta21,
      0xb1000000, 0xfd000000, INSN_GP, ENC_GET_SET, 0 },
    { "SETD", CoreMeta11|CoreMeta12|CoreMeta21,
      0xb4000000, 0xfd000000, INSN_GP, ENC_GET_SET, 0 },
    /* SET is a synonym for SETD. */
    { "SET", CoreMeta11|CoreMeta12|CoreMeta21,
      0xb4000000, 0xfd000000, INSN_GP, ENC_GET_SET, 0 },
    { "SETL", CoreMeta11|CoreMeta12|CoreMeta21,
      0xb5000000, 0xfd000000, INSN_GP, ENC_GET_SET, 0 },

    /* Multiple GET */
    { "MGETD", CoreMeta11|CoreMeta12|CoreMeta21,
      0xc8000000, 0xff000007, INSN_GP, ENC_MGET_MSET, 0 },
    { "MGETL", CoreMeta11|CoreMeta12|CoreMeta21,
      0xc9000000, 0xff000007, INSN_GP, ENC_MGET_MSET, 0 },

    /* Multiple SET */
    { "MSETD", CoreMeta11|CoreMeta12|CoreMeta21,
      0xb8000000, 0xff000007, INSN_GP, ENC_MGET_MSET, 0 },
    { "MSETL", CoreMeta11|CoreMeta12|CoreMeta21,
      0xb9000000, 0xff000007, INSN_GP, ENC_MGET_MSET, 0 },

    /* Conditional SET */
    COND_INSN ("SETB", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0xa4000000, 0xff00039f, INSN_GP, ENC_COND_SET, 0),
    COND_INSN ("SETW", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0xa4000001, 0xff00039f, INSN_GP, ENC_COND_SET, 0),
    COND_INSN ("SETD", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0xa4000200, 0xff00039f, INSN_GP, ENC_COND_SET, 0),
    COND_INSN ("SETL", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0xa4000201, 0xff00039f, INSN_GP, ENC_COND_SET, 0),
    { "XFRD", CoreMeta11|CoreMeta12|CoreMeta21,
      0xd0000000, 0xf2000000, INSN_GP, ENC_XFR, 0 },
    { "XFRL", CoreMeta11|CoreMeta12|CoreMeta21,
      0xd2000000, 0xf2000000, INSN_GP, ENC_XFR, 0 },

    /* Fast control register setup */
    { "MOV", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa9000000, 0xff000005, INSN_GP, ENC_MOV_CT, 0 },
    { "MOVT", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa9000001, 0xff000005, INSN_GP, ENC_MOV_CT, 0 },
    { "MOV", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa9000004, 0xff000005, INSN_GP, ENC_MOV_CT, 0 },
    { "MOVT", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa9000005, 0xff000005, INSN_GP, ENC_MOV_CT, 0 },

    /* Internal transfer operations */
    { "JUMP", CoreMeta11|CoreMeta12|CoreMeta21,
      0xac000000, 0xff000004, INSN_GP, ENC_JUMP, 0 },
    { "CALL", CoreMeta11|CoreMeta12|CoreMeta21,
      0xac000004, 0xff000004, INSN_GP, ENC_JUMP, 0 },
    { "CALLR", CoreMeta11|CoreMeta12|CoreMeta21,
      0xab000000, 0xff000000, INSN_GP, ENC_CALLR, 0 },

    /* Address unit ALU operations */
    { "MOV", CoreMeta11|CoreMeta12|CoreMeta21,
      0x80000004, 0xfe0001fc, INSN_GP, ENC_ALU, 0 },
    { "MOV", CoreMeta11|CoreMeta12|CoreMeta21,
      0x82000004, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "MOVT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x82000005, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "ADD", CoreMeta11|CoreMeta12|CoreMeta21,
      0x80000000, 0xfe0001fc, INSN_GP, ENC_ALU, 0 },
    { "ADD", CoreMeta11|CoreMeta12|CoreMeta21,
      0x82000000, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "ADDT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x82000001, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "ADD", CoreMeta11|CoreMeta12|CoreMeta21,
      0x86000020, 0xfe000021, INSN_GP, ENC_ALU, 0 },
    COND_INSN ("ADD", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x84000000, 0xfe00001e, INSN_GP, ENC_ALU, 0),
    COND_INSN ("ADD", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x86000000, 0xfe00003f, INSN_GP, ENC_ALU, 0),
    { "NEG", CoreMeta11|CoreMeta12|CoreMeta21,
      0x88000004, 0xfe0001fc, INSN_GP, ENC_ALU, 0 },
    { "NEG", CoreMeta11|CoreMeta12|CoreMeta21,
      0x8a000004, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "NEGT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x8a000005, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "SUB", CoreMeta11|CoreMeta12|CoreMeta21,
      0x88000000, 0xfe0001fc, INSN_GP, ENC_ALU, 0 },
    { "SUB", CoreMeta11|CoreMeta12|CoreMeta21,
      0x8a000000, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "SUBT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x8a000001, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "SUB", CoreMeta11|CoreMeta12|CoreMeta21,
      0x8e000020, 0xfe000021, INSN_GP, ENC_ALU, 0 },
    COND_INSN ("SUB", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x8c000000, 0xfe00001e, INSN_GP, ENC_ALU, 0),
    COND_INSN ("SUB", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x8e000000, 0xfe00003f, INSN_GP, ENC_ALU, 0),

    /* Data unit ALU operations */
    { "MOV", CoreMeta11|CoreMeta12|CoreMeta21,
      0x00000004, 0xfe0001fc, INSN_GP, ENC_ALU, 0 },
    { "MOVS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x08000004, 0xfe0001fc, INSN_GP, ENC_ALU, 0 },
    { "MOV", CoreMeta11|CoreMeta12|CoreMeta21,
      0x02000004, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "MOVS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x0a000004, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "MOVT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x02000005, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "MOVST", CoreMeta11|CoreMeta12|CoreMeta21,
      0x0a000005, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "ADD", DspMeta21,
      0x00000100, 0xfe000104, INSN_DSP, ENC_DALU,
      DSP_ARGS_1|DSP_ARGS_ACC2|DSP_ARGS_XACC|DSP_ARGS_IMM },
    { "ADD", DspMeta21,
      0x02000003, 0xfe000003, INSN_DSP, ENC_DALU,
      DSP_ARGS_1|DSP_ARGS_IMM },
    COND_INSN ("ADD", "", 1, DspMeta21,
	       0x040001e0, 0xfe0001fe, INSN_DSP, ENC_DALU, DSP_ARGS_1),
    { "ADD", CoreMeta11|CoreMeta12|CoreMeta21,
      0x00000000, 0xfe0001fc, INSN_GP, ENC_ALU, 0 },
    { "ADDS", DspMeta21,
      0x08000100, 0xfe000104, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_ACC2 },
    { "ADDS", DspMeta21,
      0x0a000003, 0xfe000003, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_IMM },
    { "ADDS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x08000000, 0xfe0001fc, INSN_GP, ENC_ALU, 0 },
    { "ADD", CoreMeta11|CoreMeta12|CoreMeta21,
      0x02000000, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "ADDS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x0a000000, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "ADDT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x02000001, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "ADDST", CoreMeta11|CoreMeta12|CoreMeta21,
      0x0a000001, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "ADD", CoreMeta11|CoreMeta12|CoreMeta21,
      0x06000020, 0xfe000021, INSN_GP, ENC_ALU, 0 },
    COND_INSN ("ADDS", "", 1, DspMeta21,
	       0x0c0001e0, 0xfe0001fe, INSN_DSP, ENC_DALU, DSP_ARGS_1),
    { "ADDS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x0e000020, 0xfe000021, INSN_GP, ENC_ALU, 0 },
    COND_INSN ("ADD", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x04000000, 0xfe00001e, INSN_GP, ENC_ALU, 0),
    COND_INSN ("ADDS", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x0c000000, 0xfe00001e, INSN_GP, ENC_ALU, 0),
    COND_INSN ("ADD", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x06000000, 0xfe00003f, INSN_GP, ENC_ALU, 0),
    COND_INSN ("ADDS", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x0e000000, 0xfe00003f, INSN_GP, ENC_ALU, 0),
    { "NEG", CoreMeta11|CoreMeta12|CoreMeta21,
      0x10000004, 0xfe0001fc, INSN_GP, ENC_ALU, 0 },
    { "NEGS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x18000004, 0xfe0001fc, INSN_GP, ENC_ALU, 0 },
    { "NEG", CoreMeta11|CoreMeta12|CoreMeta21,
      0x12000004, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "NEGS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x1a000004, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "NEGT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x12000005, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "NEGST", CoreMeta11|CoreMeta12|CoreMeta21,
      0x1a000005, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "SUB", DspMeta21,
      0x10000100, 0xfe000104, INSN_DSP, ENC_DALU,
      DSP_ARGS_1|DSP_ARGS_ACC2|DSP_ARGS_XACC },
    { "SUB", DspMeta21,
      0x12000003, 0xfe000003, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_IMM },
    { "SUB", CoreMeta11|CoreMeta12|CoreMeta21,
      0x10000000, 0xfe0001fc, INSN_GP, ENC_ALU, 0 },
    { "SUBS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x18000000, 0xfe0001fc, INSN_GP, ENC_ALU, 0 },
    { "SUB", CoreMeta11|CoreMeta12|CoreMeta21,
      0x12000000, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "SUBS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x1a000000, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "SUBT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x12000001, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "SUBS", DspMeta21,
      0x18000100, 0xfe000104, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_ACC2 },
    { "SUBS", DspMeta21,
      0x1a000003, 0xfe000003, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_IMM },
    { "SUBST", CoreMeta11|CoreMeta12|CoreMeta21,
      0x1a000001, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "SUB", CoreMeta11|CoreMeta12|CoreMeta21,
      0x16000020, 0xfe000021, INSN_GP, ENC_ALU, 0 },
    { "SUBS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x1e000020, 0xfe000021, INSN_GP, ENC_ALU, 0 },
    COND_INSN ("SUBS", "", 1, DspMeta21,
	       0x1c0001e0, 0xfe0001fe, INSN_DSP, ENC_DALU, DSP_ARGS_1),
    COND_INSN ("SUB", "", 1, DspMeta21,
	       0x140001e0, 0xfe0001fe, INSN_DSP, ENC_DALU, DSP_ARGS_1),
    COND_INSN ("SUB", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x14000000, 0xfe00001e, INSN_GP, ENC_ALU, 0),
    COND_INSN ("SUBS", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x1c000000, 0xfe00001e, INSN_GP, ENC_ALU, 0),
    COND_INSN ("SUB", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x16000000, 0xfe00003f, INSN_GP, ENC_ALU, 0),
    COND_INSN ("SUBS", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x1e000000, 0xfe00003f, INSN_GP, ENC_ALU, 0),
    { "AND", CoreMeta11|CoreMeta12|CoreMeta21,
      0x20000000, 0xfe0001fe, INSN_GP, ENC_ALU, 0 },
    { "ANDS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x28000000, 0xfe0001fe, INSN_GP, ENC_ALU, 0 },
    { "ANDQ", CoreMeta11|CoreMeta12|CoreMeta21,
      0x20000040, 0xfe00017e, INSN_GP, ENC_ALU, GP_ARGS_QR },
    { "ANDSQ", CoreMeta11|CoreMeta12|CoreMeta21,
      0x28000040, 0xfe00017e, INSN_GP, ENC_ALU, GP_ARGS_QR },
    { "AND", CoreMeta11|CoreMeta12|CoreMeta21,
      0x22000000, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "ANDMB", CoreMeta11|CoreMeta12|CoreMeta21,
      0x22000004, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "ANDS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x2a000000, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "ANDSMB", CoreMeta11|CoreMeta12|CoreMeta21,
      0x2a000004, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "ANDT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x22000001, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "ANDMT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x22000005, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "ANDST", CoreMeta11|CoreMeta12|CoreMeta21,
      0x2a000001, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "ANDSMT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x2a000005, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "AND", DspMeta21,
      0x20000100, 0xfe00014c, INSN_DSP, ENC_DALU, DSP_ARGS_1 },
    { "AND", CoreMeta11|CoreMeta12|CoreMeta21,
      0x26000020, 0xfe000021, INSN_GP, ENC_ALU, 0 },
    { "ANDS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x2e000020, 0xfe000021, INSN_GP, ENC_ALU, 0 },
    { "ANDQ", CoreMeta11|CoreMeta12|CoreMeta21,
      0x26000021, 0xfe000021, INSN_GP, ENC_ALU, GP_ARGS_QR },
    { "ANDSQ", CoreMeta11|CoreMeta12|CoreMeta21,
      0x2e000021, 0xfe000021, INSN_GP, ENC_ALU, GP_ARGS_QR },
    { "ANDQ", DspMeta21,
      0x20000140, 0xfe00014c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_QR },
    COND_INSN ("ANDQ", "", 1, DspMeta21,
	       0x240001c0, 0xfe0001de, INSN_DSP, ENC_DALU,
	       DSP_ARGS_1|DSP_ARGS_QR),
    COND_INSN ("AND", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x24000000, 0xfe00001e, INSN_GP, ENC_ALU, 0),
    { "ANDSQ", DspMeta21,
      0x28000140, 0xfe00014c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_QR },
    COND_INSN ("ANDSQ", "", 1, DspMeta21,
	       0x2c0001c0, 0xfe0001de, INSN_DSP, ENC_DALU,
	       DSP_ARGS_1|DSP_ARGS_QR),
    COND_INSN ("ANDS", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x2c000000, 0xfe00001e, INSN_GP, ENC_ALU, 0),
    COND_INSN ("AND", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x26000000, 0xfe00003f, INSN_GP, ENC_ALU, 0),
    COND_INSN ("ANDS", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x2e000000, 0xfe00003f, INSN_GP, ENC_ALU, 0),
    COND_INSN ("ANDQ", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x26000001, 0xfe00003f, INSN_GP, ENC_ALU, GP_ARGS_QR),
    COND_INSN ("ANDSQ", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x2e000001, 0xfe00003f, INSN_GP, ENC_ALU, GP_ARGS_QR),
    { "OR", CoreMeta11|CoreMeta12|CoreMeta21,
      0x30000000, 0xfe0001fe, INSN_GP, ENC_ALU, 0 },
    { "ORS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x38000000, 0xfe0001fe, INSN_GP, ENC_ALU, 0 },
    { "ORQ", CoreMeta11|CoreMeta12|CoreMeta21,
      0x30000040, 0xfe00017e, INSN_GP, ENC_ALU, GP_ARGS_QR },
    { "ORSQ", CoreMeta11|CoreMeta12|CoreMeta21,
      0x38000040, 0xfe00017e, INSN_GP, ENC_ALU, GP_ARGS_QR },
    { "OR", CoreMeta11|CoreMeta12|CoreMeta21,
      0x32000000, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "ORMB", CoreMeta11|CoreMeta12|CoreMeta21,
      0x32000004, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "ORS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x3a000000, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "ORSMB", CoreMeta11|CoreMeta12|CoreMeta21,
      0x3a000004, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "ORT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x32000001, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "ORMT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x32000005, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "ORST", CoreMeta11|CoreMeta12|CoreMeta21,
      0x3a000001, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "ORSMT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x3a000005, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "OR", CoreMeta11|CoreMeta12|CoreMeta21,
      0x36000020, 0xfe000021, INSN_GP, ENC_ALU, 0 },
    { "ORS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x3e000020, 0xfe000021, INSN_GP, ENC_ALU, 0 },
    { "ORQ", CoreMeta11|CoreMeta12|CoreMeta21,
      0x36000021, 0xfe000021, INSN_GP, ENC_ALU, GP_ARGS_QR },
    { "ORSQ", CoreMeta11|CoreMeta12|CoreMeta21,
      0x3e000021, 0xfe000021, INSN_GP, ENC_ALU, GP_ARGS_QR },
    { "ORQ", DspMeta21,
      0x30000140, 0xfe00014c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_QR },
    COND_INSN ("ORQ", "", 1, DspMeta21,
	       0x340001c0, 0xfe0001de, INSN_DSP, ENC_DALU,
	       DSP_ARGS_1|DSP_ARGS_QR),
    COND_INSN ("OR", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x34000000, 0xfe00001e, INSN_GP, ENC_ALU, 0),
    { "ORSQ", DspMeta21,
      0x38000140, 0xfe00014c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_QR },
    COND_INSN ("ORSQ", "", 1, DspMeta21,
	       0x3c0001c0, 0xfe0001de, INSN_DSP, ENC_DALU,
	       DSP_ARGS_1|DSP_ARGS_QR),
    COND_INSN ("ORS", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x3c000000, 0xfe00001e, INSN_GP, ENC_ALU, 0),
    COND_INSN ("OR", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x36000000, 0xfe00003f, INSN_GP, ENC_ALU, 0),
    COND_INSN ("ORS", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x3e000000, 0xfe00003f, INSN_GP, ENC_ALU, 0),
    COND_INSN ("ORQ", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x36000001, 0xfe00003f, INSN_GP, ENC_ALU, GP_ARGS_QR),
    COND_INSN ("ORSQ", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x3e000001, 0xfe00003f, INSN_GP, ENC_ALU, GP_ARGS_QR),
    { "XOR", CoreMeta11|CoreMeta12|CoreMeta21,
      0x40000000, 0xfe0001fe, INSN_GP, ENC_ALU, 0 },
    { "XORS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x48000000, 0xfe0001fe, INSN_GP, ENC_ALU, 0 },
    { "XORQ", CoreMeta11|CoreMeta12|CoreMeta21,
      0x40000040, 0xfe00017e, INSN_GP, ENC_ALU, GP_ARGS_QR },
    { "XORSQ", CoreMeta11|CoreMeta12|CoreMeta21,
      0x48000040, 0xfe00017e, INSN_GP, ENC_ALU, GP_ARGS_QR },
    { "XOR", CoreMeta11|CoreMeta12|CoreMeta21,
      0x42000000, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "XORMB", CoreMeta11|CoreMeta12|CoreMeta21,
      0x42000004, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "XORS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x4a000000, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "XORSMB", CoreMeta11|CoreMeta12|CoreMeta21,
      0x4a000004, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "XORT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x42000001, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "XORMT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x42000005, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "XORST", CoreMeta11|CoreMeta12|CoreMeta21,
      0x4a000001, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "XORSMT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x4a000005, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "XOR", CoreMeta11|CoreMeta12|CoreMeta21,
      0x46000020, 0xfe000021, INSN_GP, ENC_ALU, 0 },
    { "XORS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x4e000020, 0xfe000021, INSN_GP, ENC_ALU, 0 },
    { "XORQ", CoreMeta11|CoreMeta12|CoreMeta21,
      0x46000021, 0xfe000021, INSN_GP, ENC_ALU, GP_ARGS_QR },
    { "XORSQ", CoreMeta11|CoreMeta12|CoreMeta21,
      0x4e000021, 0xfe000021, INSN_GP, ENC_ALU, GP_ARGS_QR },
    { "XORQ", DspMeta21,
      0x40000140, 0xfe00014c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_QR },
    COND_INSN ("XORQ", "", 1, DspMeta21,
	       0x440001c0, 0xfe0001de, INSN_DSP, ENC_DALU,
	       DSP_ARGS_1|DSP_ARGS_QR),
    COND_INSN ("XOR", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x44000000, 0xfe00001e, INSN_GP, ENC_ALU, 0),
    { "XORSQ", DspMeta21,
      0x48000140, 0xfe000140, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_QR },
    COND_INSN ("XORSQ", "", 1, DspMeta21,
	       0x4c0001c0, 0xfe0001de, INSN_DSP, ENC_DALU,
	       DSP_ARGS_1|DSP_ARGS_QR),
    COND_INSN ("XORS", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x4c000000, 0xfe00001e, INSN_GP, ENC_ALU, 0),
    COND_INSN ("XOR", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x46000000, 0xfe00003f, INSN_GP, ENC_ALU, 0),
    COND_INSN ("XORS", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x4e000000, 0xfe00003f, INSN_GP, ENC_ALU, 0),
    COND_INSN ("XORQ", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x46000001, 0xfe00003f, INSN_GP, ENC_ALU, GP_ARGS_QR),
    COND_INSN ("XORSQ", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x4e000001, 0xfe00003f, INSN_GP, ENC_ALU, GP_ARGS_QR),
    { "LSL", CoreMeta11|CoreMeta12|CoreMeta21,
      0x50000000, 0xfc0001ff, INSN_GP, ENC_SHIFT, 0 },
    { "LSL", CoreMeta11|CoreMeta12|CoreMeta21,
      0x54000020, 0xfc0001e0, INSN_GP, ENC_SHIFT, 0 },
    COND_INSN ("LSL", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x54000000, 0xfc0001ff, INSN_GP, ENC_SHIFT, 0),
    { "LSLS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x58000000, 0xfc0001ff, INSN_GP, ENC_SHIFT, 0 },
    { "LSLS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x5c000020, 0xfc0001e0, INSN_GP, ENC_SHIFT, 0 },
    COND_INSN ("LSLS", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x5c000000, 0xfc0001ff, INSN_GP, ENC_SHIFT, 0),
    { "LSR", CoreMeta11|CoreMeta12|CoreMeta21,
      0x50000040, 0xfc0001ff, INSN_GP, ENC_SHIFT, 0 },
    { "LSR", CoreMeta11|CoreMeta12|CoreMeta21,
      0x54000060, 0xfc0001e0, INSN_GP, ENC_SHIFT, 0 },
    COND_INSN ("LSR", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x54000040, 0xfc0001ff, INSN_GP, ENC_SHIFT, 0),
    { "LSRS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x58000040, 0xfc0001ff, INSN_GP, ENC_SHIFT, 0 },
    { "LSRS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x5c000060, 0xfc0001e0, INSN_GP, ENC_SHIFT, 0 },
    COND_INSN ("LSRS", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x5c000040, 0xfc0001ff, INSN_GP, ENC_SHIFT, 0),
    { "ASL", CoreMeta11|CoreMeta12|CoreMeta21,
      0x50000080, 0xfc0001ff, INSN_GP, ENC_SHIFT, 0 },
    { "ASL", CoreMeta11|CoreMeta12|CoreMeta21,
      0x540000a0, 0xfc0001e0, INSN_GP, ENC_SHIFT, 0 },
    COND_INSN ("ASL", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x54000080, 0xfc0001ff, INSN_GP, ENC_SHIFT, 0),
    { "ASLS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x58000080, 0xfc0001ff, INSN_GP, ENC_SHIFT, 0 },
    { "ASLS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x5c0000a0, 0xfc0001e0, INSN_GP, ENC_SHIFT, 0 },
    COND_INSN ("ASLS", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x5c000080, 0xfc0001ff, INSN_GP, ENC_SHIFT, 0),
    { "ASR", CoreMeta11|CoreMeta12|CoreMeta21,
      0x500000c0, 0xfc0001ff, INSN_GP, ENC_SHIFT, 0 },
    { "ASR", CoreMeta11|CoreMeta12|CoreMeta21,
      0x540000e0, 0xfc0001e0, INSN_GP, ENC_SHIFT, 0 },
    COND_INSN ("ASR", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x540000c0, 0xfc0001ff, INSN_GP, ENC_SHIFT, 0),
    { "ASRS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x580000c0, 0xfc0001ff, INSN_GP, ENC_SHIFT, 0 },
    { "ASRS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x5c0000e0, 0xfc0001e0, INSN_GP, ENC_SHIFT, 0 },
    COND_INSN ("ASRS", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x5c0000c0, 0xfc0001ff, INSN_GP, ENC_SHIFT, 0),
    { "MULW", CoreMeta11|CoreMeta12|CoreMeta21,
      0x60000000, 0xfe0001fe, INSN_GP, ENC_ALU, 0 },
    { "MULD", CoreMeta11|CoreMeta12|CoreMeta21,
      0x60000040, 0xfe0001fe, INSN_GP, ENC_ALU, 0 },
    /* MUL is a synonym from MULD. */
    { "MUL", CoreMeta11|CoreMeta12|CoreMeta21,
      0x60000040, 0xfe0001fe, INSN_GP, ENC_ALU, 0 },
    { "MULW", CoreMeta11|CoreMeta12|CoreMeta21,
      0x62000000, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "MULD", CoreMeta11|CoreMeta12|CoreMeta21,
      0x62000004, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "MUL", CoreMeta11|CoreMeta12|CoreMeta21,
      0x62000004, 0xfe000005, INSN_GP, ENC_ALU, 0 },
    { "MULWT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x62000001, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "MULDT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x62000005, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "MULT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x62000005, 0xfe000007, INSN_GP, ENC_ALU, 0 },
    { "MULW", CoreMeta11|CoreMeta12|CoreMeta21,
      0x64000020, 0xfe0001e0, INSN_GP, ENC_ALU, 0 },
    { "MULD", CoreMeta11|CoreMeta12|CoreMeta21,
      0x64000060, 0xfe0001e0, INSN_GP, ENC_ALU, 0 },
    { "MUL", CoreMeta11|CoreMeta12|CoreMeta21,
      0x64000060, 0xfe0001e0, INSN_GP, ENC_ALU, 0 },
    { "MULW", CoreMeta11|CoreMeta12|CoreMeta21,
      0x66000020, 0xfe000021, INSN_GP, ENC_ALU, 0 },
    { "MULD", CoreMeta11|CoreMeta12|CoreMeta21,
      0x66000021, 0xfe000021, INSN_GP, ENC_ALU, 0 },
    { "MUL", CoreMeta11|CoreMeta12|CoreMeta21,
      0x66000021, 0xfe000021, INSN_GP, ENC_ALU, 0 },
    COND_INSN ("MULW", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x64000000, 0xfe0001fe, INSN_GP, ENC_ALU, 0),
    COND_INSN ("MULD", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x64000040, 0xfe0001fe, INSN_GP, ENC_ALU, 0),
    COND_INSN ("MUL", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x64000040, 0xfe0001fe, INSN_GP, ENC_ALU, 0),
    COND_INSN ("MULW", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x66000000, 0xfe00003f, INSN_GP, ENC_ALU, 0),
    COND_INSN ("MULD", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x66000001, 0xfe00003f, INSN_GP, ENC_ALU, 0),
    COND_INSN ("MUL", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x66000001, 0xfe00003f, INSN_GP, ENC_ALU, 0),
    { "MIN", CoreMeta11|CoreMeta12|CoreMeta21,
      0x70000020, 0xfe0001ff, INSN_GP, ENC_MIN_MAX, 0 },
    { "MAX", CoreMeta11|CoreMeta12|CoreMeta21,
      0x70000024, 0xfe0001ff, INSN_GP, ENC_MIN_MAX, 0 },
    { "FFB", CoreMeta11|CoreMeta12|CoreMeta21,
      0x70000004, 0xfe003fff, INSN_GP, ENC_BITOP, 0 },
    { "NORM", CoreMeta11|CoreMeta12|CoreMeta21,
      0x70000008, 0xfe003fff, INSN_GP, ENC_BITOP, 0 },
    { "ABS", CoreMeta11|CoreMeta12|CoreMeta21,
      0x70000028, 0xfe003fff, INSN_GP, ENC_BITOP, 0 },
    { "XSDB", CoreMeta11|CoreMeta12|CoreMeta21,
      0xaa000000, 0xff003ffe, INSN_GP, ENC_BITOP, 0 },
    { "XSDSB", CoreMeta11|CoreMeta12|CoreMeta21,
      0xaa000008, 0xff003ffe, INSN_GP, ENC_BITOP, 0 },
    { "XSDW", CoreMeta11|CoreMeta12|CoreMeta21,
      0xaa000002, 0xff003ffe, INSN_GP, ENC_BITOP, 0 },
    { "XSDSW", CoreMeta11|CoreMeta12|CoreMeta21,
      0xaa00000a, 0xff003ffe, INSN_GP, ENC_BITOP, 0 },
    { "RTDW", CoreMeta11|CoreMeta12|CoreMeta21,
      0xaa000006, 0xff003ffe, INSN_GP, ENC_BITOP, 0 },
    { "RTDSW", CoreMeta11|CoreMeta12|CoreMeta21,
      0xaa00000e, 0xff003ffe, INSN_GP, ENC_BITOP, 0 },
    { "NMIN", CoreMeta11|CoreMeta12|CoreMeta21,
      0x7000002c, 0xfe0001ff, INSN_GP, ENC_MIN_MAX, 0 },

    /* Condition setting operations */
    { "CMP", CoreMeta11|CoreMeta12|CoreMeta21,
      0x70000000, 0xfef801fe, INSN_GP, ENC_CMP, 0 },
    { "TST", CoreMeta11|CoreMeta12|CoreMeta21,
      0x78000000, 0xfef801fe, INSN_GP, ENC_CMP, 0 },
    { "CMP", CoreMeta11|CoreMeta12|CoreMeta21,
      0x72000000, 0xfe000005, INSN_GP, ENC_CMP, 0 },
    { "CMPMB", CoreMeta11|CoreMeta12|CoreMeta21,
      0x72000004, 0xfe000005, INSN_GP, ENC_CMP, 0 },
    { "TST", CoreMeta11|CoreMeta12|CoreMeta21,
      0x7a000000, 0xfe000005, INSN_GP, ENC_CMP, 0 },
    { "TSTMB", CoreMeta11|CoreMeta12|CoreMeta21,
      0x7a000004, 0xfe000005, INSN_GP, ENC_CMP, 0 },
    { "CMPT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x72000001, 0xfe000007, INSN_GP, ENC_CMP, 0 },
    { "CMPMT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x72000005, 0xfe000007, INSN_GP, ENC_CMP, 0 },
    { "TSTT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x7a000001, 0xfe000007, INSN_GP, ENC_CMP, 0 },
    { "TSTMT", CoreMeta11|CoreMeta12|CoreMeta21,
      0x7a000005, 0xfe000007, INSN_GP, ENC_CMP, 0 },
    COND_INSN ("CMP", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x74000000, 0xfef801fe, INSN_GP, ENC_CMP, 0),
    COND_INSN ("TST", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x7c000000, 0xfef801fe, INSN_GP, ENC_CMP, 0),
    COND_INSN ("CMP", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x76000000, 0xfef8003e, INSN_GP, ENC_CMP, 0),
    COND_INSN ("TST", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0x7e000000, 0xfef8003e, INSN_GP, ENC_CMP, 0),

    /* No-op (BNV) */
    { "NOP", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa0fffffe, 0xffffffff, INSN_GP, ENC_NONE, 0 },

    /* Branch */
    COND_INSN ("B", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0xa0000000, 0xff00001f, INSN_GP, ENC_BRANCH, 0),
    COND_INSN ("B", "R", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0xa0000001, 0xff00001f, INSN_GP, ENC_BRANCH, 0),

    /* System operations */
    { "LOCK0", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa8000000, 0xffffffff, INSN_GP, ENC_NONE, 0 },
    { "LOCK1", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa8000001, 0xffffffff, INSN_GP, ENC_NONE, 0 },
    { "LOCK2", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa8000003, 0xffffffff, INSN_GP, ENC_NONE, 0 },
    { "RTI", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa3ffffff, 0xffffffff, INSN_GP, ENC_NONE, 0 },
    { "RTH", CoreMeta11|CoreMeta12|CoreMeta21,
      0xa37fffff, 0xffffffff, INSN_GP, ENC_NONE, 0 },
    COND_INSN ("KICK", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0xa3000001, 0xff003e1f, INSN_GP, ENC_KICK, 0),
    { "SWITCH", CoreMeta11|CoreMeta12|CoreMeta21,
      0xaf000000, 0xff000000, INSN_GP, ENC_SWITCH, 0 },
    { "DCACHE", CoreMeta11|CoreMeta12|CoreMeta21,
      0xad000000, 0xff000087, INSN_GP, ENC_CACHEW, 0 },
    { "ICACHEEXIT", CoreMeta12|CoreMeta21,
      0xae000000, 0xffffffff, INSN_GP, ENC_NONE, 0 },
    { "ICACHEEXITR", CoreMeta12|CoreMeta21,
      0xae000001, 0xffffffff, INSN_GP, ENC_NONE, 0 },
    { "ICACHE", CoreMeta12|CoreMeta21,
      0xae000000, 0xff0001e1, INSN_GP, ENC_ICACHE, 0 },
    { "ICACHER", CoreMeta12|CoreMeta21,
      0xae000001, 0xff0001e1, INSN_GP, ENC_ICACHE, 0 },

    /* Meta 2 instructions */
    { "CACHERD", CoreMeta21,
      0xad000081, 0xff000087, INSN_GP, ENC_CACHER, 0 },
    { "CACHERL", CoreMeta21,
      0xad000083, 0xff000087, INSN_GP, ENC_CACHER, 0 },
    { "CACHEWD", CoreMeta21,
      0xad000001, 0xff000087, INSN_GP, ENC_CACHEW, 0 },
    { "CACHEWL", CoreMeta21,
      0xad000003, 0xff000087, INSN_GP, ENC_CACHEW, 0 },
    COND_INSN ("DEFR", "", 1, CoreMeta21,
	       0xa3002001, 0xff003e1f, INSN_GP, ENC_KICK, 0),
    { "BEXD", CoreMeta21,
      0xaa000004, 0xff003ffe, INSN_GP, ENC_BITOP, 0 },
    { "BEXSD", CoreMeta21,
      0xaa00000c, 0xff003ffe, INSN_GP, ENC_BITOP, 0 },
    { "BEXL", CoreMeta21,
      0xaa000014, 0xff003ffe, INSN_GP, ENC_BITOP, 0 },
    { "BEXSL", CoreMeta21,
      0xaa00001c, 0xff003ffe, INSN_GP, ENC_BITOP, 0 },
    { "LNKGETB", CoreMeta21,
      0xad000080, 0xff000087, INSN_GP, ENC_LNKGET, 0 },
    { "LNKGETW", CoreMeta21,
      0xad000082, 0xff000087, INSN_GP, ENC_LNKGET, 0 },
    { "LNKGETD", CoreMeta21,
      0xad000084, 0xff000087, INSN_GP, ENC_LNKGET, 0 },
    { "LNKGETL", CoreMeta21,
      0xad000086, 0xff000087, INSN_GP, ENC_LNKGET, 0 },
    COND_INSN ("LNKSETB", "", 1, CoreMeta21,
	       0xa4000080, 0xff00039f, INSN_GP, ENC_COND_SET, 0),
    COND_INSN ("LNKSETW", "", 1, CoreMeta21,
	       0xa4000081, 0xff00039f, INSN_GP, ENC_COND_SET, 0),
    COND_INSN ("LNKSETD", "", 1, CoreMeta21,
	       0xa4000280, 0xff00039f, INSN_GP, ENC_COND_SET, 0),
    COND_INSN ("LNKSETL", "", 1, CoreMeta21,
	       0xa4000281, 0xff00039f, INSN_GP, ENC_COND_SET, 0),

    /* Meta 2 FPU instructions */

    /* Port-to-unit MOV */
    COND_INSN ("MOVL", "", 1, FpuMeta21,
	       0xa1800201, 0xfff83e1f, INSN_FPU, ENC_MOV_PORT, 0),

    /* Read pipeline drain */
    { "MMOVD", FpuMeta21,
      0xce000006, 0xfffc007f, INSN_FPU, ENC_MMOV, 0 },
    { "MMOVL", FpuMeta21,
      0xcf000006, 0xfffc007f, INSN_FPU, ENC_MMOV, 0 },

    /* FP data movement instructions */
    FCOND_INSN ("ABS", "", 1, FpuMeta21,
		0xf0000080, 0xff843f9f, INSN_FPU, ENC_FMOV, 0),
    { "MMOVD", FpuMeta21,
      0xbe000002, 0xff84007e, INSN_FPU, ENC_FMMOV, 0 },
    { "MMOVL", FpuMeta21,
      0xbf000002, 0xff84007e, INSN_FPU, ENC_FMMOV, 0 },
    { "MMOVD", FpuMeta21,
      0xce000002, 0xff84007e, INSN_FPU, ENC_FMMOV, 0 },
    { "MMOVL", FpuMeta21,
      0xcf000002, 0xff84007e, INSN_FPU, ENC_FMMOV, 0 },
    { "MOVD", FpuMeta21,
      0x08000144, 0xfe03e1ff, INSN_FPU, ENC_FMOV_DATA, 0 },
    { "MOVD", FpuMeta21,
      0x080001c4, 0xfe83c1ff, INSN_FPU, ENC_FMOV_DATA, 0 },
    { "MOVL", FpuMeta21,
      0x08000154, 0xfe03e1ff, INSN_FPU, ENC_FMOV_DATA, 0 },
    { "MOVL", FpuMeta21,
      0x080001d4, 0xfe83c1ff, INSN_FPU, ENC_FMOV_DATA, 0 },
    FCOND_INSN ("MOV", "", 1, FpuMeta21,
		0xf0000000, 0xff843f9f, INSN_FPU, ENC_FMOV, 0),
    { "MOV", FpuMeta21,
      0xf0000001, 0xff800001, INSN_FPU, ENC_FMOV_I, 0 },
    FCOND_INSN ("NEG", "", 1, FpuMeta21,
		0xf0000100, 0xff843f9f, INSN_FPU, ENC_FMOV, 0),
    { "PACK", FpuMeta21,
      0xf0000180, 0xff8c21ff, INSN_FPU, ENC_FPACK, 0 },
    { "SWAP", FpuMeta21,
      0xf00001c0, 0xff8c7fff, INSN_FPU, ENC_FSWAP, 0 },

    /* FP comparison instructions */
    FCOND_INSN ("CMP", "", 1, FpuMeta21,
		0xf3000000, 0xfff4201f, INSN_FPU, ENC_FCMP, 0),
    FCOND_INSN ("MAX", "", 1, FpuMeta21,
		0xf3000081, 0xff84219f, INSN_FPU, ENC_FMINMAX, 0),
    FCOND_INSN ("MIN", "", 1, FpuMeta21,
		0xf3000001, 0xff84219f, INSN_FPU, ENC_FMINMAX, 0),

    /* FP data conversion instructions */
    FCOND_INSN ("DTOF", "", 1, FpuMeta21,
		0xf2000121, 0xff843fff, INSN_FPU, ENC_FCONV, 0),
    FCOND_INSN ("FTOD", "", 1, FpuMeta21,
		0xf2000101, 0xff843fff, INSN_FPU, ENC_FCONV, 0),
    FCOND_INSN ("DTOH", "", 1, FpuMeta21,
		0xf2000320, 0xff843fff, INSN_FPU, ENC_FCONV, 0),
    FCOND_INSN ("FTOH", "", 1, FpuMeta21,
		0xf2000300, 0xff843fbf, INSN_FPU, ENC_FCONV, 0),
    FCOND_INSN ("DTOI", "", 1, FpuMeta21,
		0xf2002120, 0xff842fff, INSN_FPU, ENC_FCONV, 0),
    FCOND_INSN ("FTOI", "", 1, FpuMeta21,
		0xf2002100, 0xff842fbf, INSN_FPU, ENC_FCONV, 0),
    FCOND_INSN ("DTOL", "", 1, FpuMeta21,
		0xf2002320, 0xff8c6fff, INSN_FPU, ENC_FCONV, 0),

    FCOND_INSN ("DTOX", "", 1, FpuMeta21,
		0xf2000020, 0xff8401bf, INSN_FPU, ENC_FCONVX, 0),
    FCOND_INSN ("FTOX", "", 1, FpuMeta21,
		0xf2000000, 0xff8401bf, INSN_FPU, ENC_FCONVX, 0),
    FCOND_INSN ("DTOXL", "", 1, FpuMeta21,
		0xf20000a0, 0xff8c40ff, INSN_FPU, ENC_FCONVX, 0),

    FCOND_INSN ("HTOD", "", 1, FpuMeta21,
		0xf2000321, 0xff843fff, INSN_FPU, ENC_FCONV, 0),
    FCOND_INSN ("HTOF", "", 1, FpuMeta21,
		0xf2000301, 0xff843fbf, INSN_FPU, ENC_FCONV, 0),
    FCOND_INSN ("ITOD", "", 1, FpuMeta21,
		0xf2002121, 0xff843fff, INSN_FPU, ENC_FCONV, 0),
    FCOND_INSN ("ITOF", "", 1, FpuMeta21,
		0xf2002101, 0xff843fbf, INSN_FPU, ENC_FCONV, 0),
    FCOND_INSN ("LTOD", "", 1, FpuMeta21,
		0xf2002321, 0xff8c7fff, INSN_FPU, ENC_FCONV, 0),

    FCOND_INSN ("XTOD", "", 1, FpuMeta21,
		0xf2000021, 0xff8401bf, INSN_FPU, ENC_FCONVX, 0),
    FCOND_INSN ("XTOF", "", 1, FpuMeta21,
		0xf2000001, 0xff8401bf, INSN_FPU, ENC_FCONVX, 0),
    FCOND_INSN ("XLTOD", "", 1, FpuMeta21,
		0xf20000a1, 0xff8c40ff, INSN_FPU, ENC_FCONVX, 0),

    /* FP basic arithmetic instructions */
    FCOND_INSN ("ADD", "", 1, FpuMeta21,
		0xf1000001, 0xff84211f, INSN_FPU, ENC_FBARITH, 0),
    FCOND_INSN ("MUL", "", 1, FpuMeta21,
		0xf1000100, 0xff84211f, INSN_FPU, ENC_FBARITH, 0),
    FCOND_INSN ("SUB", "", 1, FpuMeta21,
		0xf1000101, 0xff84211f, INSN_FPU, ENC_FBARITH, 0),

    /* FP extended arithmetic instructions */
    { "MAC", FpuMeta21,
      0xf6000000, 0xfffc219f, INSN_FPU, ENC_FEARITH, 0 },
    { "MACS", FpuMeta21,
      0xf6000100, 0xfffc219f, INSN_FPU, ENC_FEARITH, 0 },

    { "MAR", FpuMeta21,
      0xf6000004, 0xff84211f, INSN_FPU, ENC_FEARITH, 0 },
    { "MARS", FpuMeta21,
      0xf6000104, 0xff84211f, INSN_FPU, ENC_FEARITH, 0 },

    { "MAW", FpuMeta21,
      0xf6000008, 0xff84219f, INSN_FPU, ENC_FEARITH, 0 },
    { "MAWS", FpuMeta21,
      0xf6000108, 0xff84219f, INSN_FPU, ENC_FEARITH, 0 },
    { "MAW1", FpuMeta21,
      0xf6000009, 0xff84219f, INSN_FPU, ENC_FEARITH, 0 },
    { "MAWS1", FpuMeta21,
      0xf6000109, 0xff84219f, INSN_FPU, ENC_FEARITH, 0 },

    FCOND_INSN ("MXA", "", 1, FpuMeta21,
		0xf5000000, 0xff84211f, INSN_FPU, ENC_FEARITH, 0),
    FCOND_INSN ("MXAS", "", 1, FpuMeta21,
		0xf5000100, 0xff84211f, INSN_FPU, ENC_FEARITH, 0),
    FCOND_INSN ("MXA1", "", 1, FpuMeta21,
		0xf5000001, 0xff84211f, INSN_FPU, ENC_FEARITH, 0),
    FCOND_INSN ("MXAS1", "", 1, FpuMeta21,
		0xf5000101, 0xff84211f, INSN_FPU, ENC_FEARITH, 0),

    { "MUZ", FpuMeta21,
      0xf6000010, 0xff84211d, INSN_FPU, ENC_FEARITH, 0 },
    { "MUZS", FpuMeta21,
      0xf6000110, 0xff84211d, INSN_FPU, ENC_FEARITH, 0 },
    { "MUZ1", FpuMeta21,
      0xf6000011, 0xff84211d, INSN_FPU, ENC_FEARITH, 0 },
    { "MUZS1", FpuMeta21,
      0xf6000111, 0xff84211d, INSN_FPU, ENC_FEARITH, 0 },

    { "RCP", FpuMeta21,
      0xf7000000, 0xff84391f, INSN_FPU, ENC_FREC, 0 },
    { "RSQ", FpuMeta21,
      0xf7000100, 0xff84391f, INSN_FPU, ENC_FREC, 0 },

    /* FP SIMD arithmetic instructions */
    { "ADDRE", FpuMeta21,
      0xf4000000, 0xff8c637f, INSN_FPU, ENC_FSIMD, 0 },
    { "MULRE", FpuMeta21,
      0xf4000001, 0xff8c637f, INSN_FPU, ENC_FSIMD, 0 },
    { "SUBRE", FpuMeta21,
      0xf4000100, 0xff8c637f, INSN_FPU, ENC_FSIMD, 0 },

    /* FP memory instructions */
    { "MGETD", FpuMeta21,
      0xce000000, 0xff00001f, INSN_FPU, ENC_MGET_MSET, 0 },
    { "MGET", FpuMeta21,
      0xce000000, 0xff00001f, INSN_FPU, ENC_MGET_MSET, 0 },
    { "MGETL", FpuMeta21,
      0xcf000000, 0xff00001f, INSN_FPU, ENC_MGET_MSET, 0 },

    { "MSETD", FpuMeta21,
      0xbe000000, 0xff00001f, INSN_FPU, ENC_MGET_MSET, 0 },
    { "MSET", FpuMeta21,
      0xbe000000, 0xff00001f, INSN_FPU, ENC_MGET_MSET, 0 },
    { "MSETL", FpuMeta21,
      0xbf000000, 0xff00001f, INSN_FPU, ENC_MGET_MSET, 0 },

    /* FP accumulator memory instructions */
    { "GETL", FpuMeta21,
      0xcf000004, 0xffe03f9f, INSN_FPU, ENC_FGET_SET_ACF, 0 },
    { "SETL", FpuMeta21,
      0xbf000004, 0xffe03f9f, INSN_FPU, ENC_FGET_SET_ACF, 0 },

    /* DSP FPU data movement */
    { "MOV", DspMeta21|FpuMeta21,
      0x08000146, 0xfe0001ee, INSN_DSP_FPU, ENC_DALU,
      DSP_ARGS_2|DSP_ARGS_DSP_SRC1 },
    { "MOV", DspMeta21|FpuMeta21,
      0x080001c6, 0xfe0001ee, INSN_DSP_FPU, ENC_DALU,
      DSP_ARGS_2|DSP_ARGS_DSP_SRC2 },

    /* Unit-to-unit MOV */
    COND_INSN ("MOV", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0xa3000000, 0xff00021f, INSN_GP, ENC_MOV_U2U, 0),
    COND_INSN ("TTMOV", "", 1, CoreMeta12|CoreMeta21,
	       0xa3000201, 0xff00021f, INSN_GP, ENC_MOV_U2U, 0),
    COND_INSN ("SWAP", "", 1, CoreMeta11|CoreMeta12|CoreMeta21,
	       0xa3000200, 0xff00021f, INSN_GP, ENC_SWAP, 0),

    /* DSP memory instructions */
    { "GETD", DspMeta21,
      0x94000100, 0xff0001fc, INSN_DSP, ENC_DGET_SET, 0 },
    { "SETD", DspMeta21,
      0x94000000, 0xff0001fc, INSN_DSP, ENC_DGET_SET, 0 },
    { "GETL", DspMeta21,
      0x94000104, 0xff0001fc, INSN_DSP, ENC_DGET_SET, 0 },
    { "SETL", DspMeta21,
      0x94000004, 0xff0001fc, INSN_DSP, ENC_DGET_SET, 0 },

    /* DSP read pipeline prime/drain */
    { "MMOVD", DspMeta21,
      0xca000001, 0xff00001f, INSN_DSP, ENC_MMOV, 0 },
    { "MMOVL", DspMeta21,
      0xcb000001, 0xff00001f, INSN_DSP, ENC_MMOV, 0 },
    { "MMOVD", DspMeta21,
      0xcc000001, 0xff07c067, INSN_DSP, ENC_MMOV, 0 },
    { "MMOVL", DspMeta21,
      0xcd000001, 0xff07c067, INSN_DSP, ENC_MMOV, 0 },

    /* DSP Template instantiation */
    TEMPLATE_INSN (DspMeta21, 0x90000000, 0xff00000f, INSN_DSP),
    TEMPLATE_INSN (DspMeta21, 0x93000000, 0xff0001ff, INSN_DSP),
    TEMPLATE_INSN (DspMeta21, 0x95000000, 0xff00000f, INSN_DSP),

    { "AND", DspMeta21,
      0x22000003, 0xfe000003, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_IMM },
    { "ANDS", DspMeta21,
      0x28000100, 0xfe00014c, INSN_DSP, ENC_DALU, DSP_ARGS_1 },
    { "ANDS", DspMeta21,
      0x2a000003, 0xfe000003, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_IMM },
    { "MAX", DspMeta21,
      0x70000124, 0xfe0001ec, INSN_DSP, ENC_DALU, DSP_ARGS_1 },
    { "MIN", DspMeta21,
      0x70000120, 0xfe0001ec, INSN_DSP, ENC_DALU, DSP_ARGS_1 },
    { "NMIN", DspMeta21,
      0x7000012c, 0xfe0001ec, INSN_DSP, ENC_DALU, DSP_ARGS_1 },
    { "OR", DspMeta21,
      0x30000100, 0xfe00014c, INSN_DSP, ENC_DALU, DSP_ARGS_1 },
    { "OR", DspMeta21,
      0x32000003, 0xfe000003, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_IMM },
    { "ORS", DspMeta21,
      0x38000100, 0xfe00014c, INSN_DSP, ENC_DALU, DSP_ARGS_1 },
    { "ORS", DspMeta21,
      0x3a000003, 0xfe000003, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_IMM },
    { "XOR", DspMeta21,
      0x40000100, 0xfe00014c, INSN_DSP, ENC_DALU, DSP_ARGS_1 },
    { "XOR", DspMeta21,
      0x42000003, 0xfe000003, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_IMM },
    { "XORS", DspMeta21,
      0x48000100, 0xfe00014c, INSN_DSP, ENC_DALU, DSP_ARGS_1 },
    { "XORS", DspMeta21,
      0x4a000003, 0xfe000003, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_IMM },
    { "ADDB8", DspMeta21,
      0x20000108, 0xfe00010c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_SPLIT8 },
    { "ADDT8", DspMeta21,
      0x2000010c, 0xfe00010c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_SPLIT8 },
    { "ADDSB8", DspMeta21,
      0x28000108, 0xfe00014c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_SPLIT8 },
    { "ADDST8", DspMeta21,
      0x2800010c, 0xfe00014c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_SPLIT8 },
    { "MULB8", DspMeta21,
      0x40000108, 0xfe00012c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_SPLIT8 },
    { "MULT8", DspMeta21,
      0x4000010c, 0xfe00012c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_SPLIT8 },
    { "MULSB8", DspMeta21,
      0x48000108, 0xfe00012c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_SPLIT8 },
    { "MULST8", DspMeta21,
      0x4800010c, 0xfe00012c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_SPLIT8 },
    { "SUBB8", DspMeta21,
      0x30000108, 0xfe00010c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_SPLIT8 },
    { "SUBT8", DspMeta21,
      0x3000010c, 0xfe00014c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_SPLIT8 },
    { "SUBSB8", DspMeta21,
      0x38000108, 0xfe00014c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_SPLIT8 },
    { "SUBST8", DspMeta21,
      0x3800010c, 0xfe00014c, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_SPLIT8 },
    { "MUL", DspMeta21,
      0x60000100, 0xfe000100, INSN_DSP, ENC_DALU,
      DSP_ARGS_1|DSP_ARGS_DACC },
    { "MUL", DspMeta21,
      0x62000003, 0xfe000003, INSN_DSP, ENC_DALU, DSP_ARGS_1|DSP_ARGS_IMM },
    { "ABS", DspMeta21,
      0x70000128, 0xfe0001ec, INSN_DSP, ENC_DALU, DSP_ARGS_2 },
    { "FFB", DspMeta21,
      0x70000104, 0xfe0001ec, INSN_DSP, ENC_DALU, DSP_ARGS_2 },
    { "NORM", DspMeta21,
      0x70000108, 0xfe0001ec, INSN_DSP, ENC_DALU, DSP_ARGS_2 },
    { "CMP", DspMeta21,
      0x70000000, 0xfe0000ec, INSN_DSP, ENC_DALU, DSP_ARGS_2|DSP_ARGS_IMM },
    { "CMP", DspMeta21,
      0x72000003, 0xfe000003, INSN_DSP, ENC_DALU, DSP_ARGS_2|DSP_ARGS_IMM },
    { "TST", DspMeta21,
      0x78000100, 0xfe0001ec, INSN_DSP, ENC_DALU, DSP_ARGS_2|DSP_ARGS_IMM },
    { "TST", DspMeta21,
      0x7a000003, 0xfe000003, INSN_DSP, ENC_DALU, DSP_ARGS_2|DSP_ARGS_IMM },
    { "MOV", DspMeta21,
      0x00000104, 0xfe078146, INSN_DSP, ENC_DALU,
      DSP_ARGS_2|DSP_ARGS_DSP_SRC1|DSP_ARGS_DSP_SRC2|DSP_ARGS_IMM },
    { "MOVS", DspMeta21,
      0x08000104, 0xfe000146, INSN_DSP, ENC_DALU, DSP_ARGS_2|DSP_ARGS_DSP_SRC2 },
    { "MOV", DspMeta21,
      0x91000000, 0xff000000, INSN_DSP, ENC_DALU,
      DSP_ARGS_2|DSP_ARGS_DSP_SRC1|DSP_ARGS_IMM },
    { "MOV", DspMeta21,
      0x92000000, 0xff000000, INSN_DSP, ENC_DALU, DSP_ARGS_2|DSP_ARGS_DSP_SRC2 },
    { "NEG", DspMeta21,
      0x10000104, 0xfe000146, INSN_DSP, ENC_DALU, DSP_ARGS_2|DSP_ARGS_DSP_SRC2 },
    { "NEGS", DspMeta21,
      0x18000104, 0xfe000146, INSN_DSP, ENC_DALU, DSP_ARGS_2|DSP_ARGS_DSP_SRC2 },
    { "XSDB", DspMeta21,
      0xaa000100, 0xff0001ee, INSN_DSP, ENC_DALU, DSP_ARGS_2 },
    { "XSD", DspMeta21,
      0xaa000100, 0xff0001ee, INSN_DSP, ENC_DALU, DSP_ARGS_2 },
    { "XSDW", DspMeta21,
      0xaa000102, 0xff0001ee, INSN_DSP, ENC_DALU, DSP_ARGS_2 },
    { "XSDSB", DspMeta21,
      0xaa000108, 0xff0001ee, INSN_DSP, ENC_DALU, DSP_ARGS_2 },
    { "XSDS", DspMeta21,
      0xaa000108, 0xff0001ee, INSN_DSP, ENC_DALU, DSP_ARGS_2 },
    { "XSDSW", DspMeta21,
      0xaa00010a, 0xff0001ee, INSN_DSP, ENC_DALU, DSP_ARGS_2 },
    { "LSL", DspMeta21,
      0x50000100, 0xfc0001c0, INSN_DSP, ENC_DALU, 0 },
    { "LSR", DspMeta21,
      0x50000140, 0xfc0001c0, INSN_DSP, ENC_DALU, 0 },
    { "ASL", DspMeta21,
      0x50000180, 0xfc0001c0, INSN_DSP, ENC_DALU, 0 },
    { "ASR", DspMeta21,
      0x500001c0, 0xfc0001c0, INSN_DSP, ENC_DALU, 0 },
    { "LSL", DspMeta21,
      0x54000120, 0xfc0001e0, INSN_DSP, ENC_DALU, DSP_ARGS_IMM },
    { "LSR", DspMeta21,
      0x54000160, 0xfc0001e0, INSN_DSP, ENC_DALU, DSP_ARGS_IMM },
    { "ASL", DspMeta21,
      0x540001a0, 0xfc0001e0, INSN_DSP, ENC_DALU, DSP_ARGS_IMM },
    { "ASR", DspMeta21,
      0x540001e0, 0xfc0001e0, INSN_DSP, ENC_DALU, DSP_ARGS_IMM },
    COND_INSN ("LSL", "", 1, DspMeta21,
	       0x54000100, 0xfc0001fe, INSN_DSP, ENC_DALU, 0),
    COND_INSN ("LSR", "", 1, DspMeta21,
	       0x54000140, 0xfc0001fe, INSN_DSP, ENC_DALU, 0),
    COND_INSN ("ASL", "", 1, DspMeta21,
	       0x54000180, 0xfc0001fe, INSN_DSP, ENC_DALU, 0),
    COND_INSN ("ASR", "", 1, DspMeta21,
	       0x540001c0, 0xfc0001fe, INSN_DSP, ENC_DALU, 0),
    { "LSLS", DspMeta21,
      0x58000100, 0xfc0001c0, INSN_DSP, ENC_DALU, 0 },
    { "LSRS", DspMeta21,
      0x58000140, 0xfc0001c0, INSN_DSP, ENC_DALU, 0 },
    { "ASLS", DspMeta21,
      0x58000180, 0xfc0001c0, INSN_DSP, ENC_DALU, 0 },
    { "ASRS", DspMeta21,
      0x580001c0, 0xfc0001c0, INSN_DSP, ENC_DALU, 0 },
    COND_INSN ("LSLS", "", 1, DspMeta21,
	       0x5c000100, 0xfc0001fe, INSN_DSP, ENC_DALU, 0),
    COND_INSN ("LSRS", "", 1, DspMeta21,
	       0x5c000140, 0xfc0001fe, INSN_DSP, ENC_DALU, 0),
    COND_INSN ("ASLS", "", 1, DspMeta21,
	       0x5c000180, 0xfc0001fe, INSN_DSP, ENC_DALU, 0),
    COND_INSN ("ASRS", "", 1, DspMeta21,
	       0x5c0001c0, 0xfc0001fe, INSN_DSP, ENC_DALU, 0),
    { "LSLS", DspMeta21,
      0x5c000120, 0xfc0001e0, INSN_DSP, ENC_DALU, 0 },
    { "LSRS", DspMeta21,
      0x5c000160, 0xfc0001e0, INSN_DSP, ENC_DALU, 0 },
    { "ASLS", DspMeta21,
      0x5c0001a0, 0xfc0001e0, INSN_DSP, ENC_DALU, 0 },
    { "ASRS", DspMeta21,
      0x5c0001e0, 0xfc0001e0, INSN_DSP, ENC_DALU, 0 },
    { "RTDW", DspMeta21,
      0xaa000106, 0xff00010e, INSN_DSP, ENC_DALU, DSP_ARGS_2 },
    { "RTDSW", DspMeta21,
      0xaa00010e, 0xff00010e, INSN_DSP, ENC_DALU, DSP_ARGS_2 },
  };

#define UNIT_MASK                  0xf
#define SHORT_UNIT_MASK            0x3
#define EXT_BASE_REG_MASK          0x1
#define REG_MASK                  0x1f
#define CC_MASK                    0xf
#define RMASK_MASK                0x7f
#define GET_SET_IMM_MASK          0x3f
#define GET_SET_IMM_BITS             6
#define GET_SET_EXT_IMM_MASK     0xfff
#define GET_SET_EXT_IMM_BITS        12
#define DGET_SET_IMM_MASK          0x3
#define DGET_SET_IMM_BITS            2
#define MGET_MSET_MAX_REGS           8
#define MMOV_MAX_REGS                8
#define IMM16_MASK              0xffff
#define IMM16_BITS                  16
#define IMM19_MASK             0x7ffff
#define IMM19_BITS                  19
#define IMM8_MASK                 0xff
#define IMM8_BITS                    8
#define IMM24_MASK            0xffffff
#define IMM24_BITS                  24
#define IMM5_MASK                 0x1f
#define IMM5_BITS                    5
#define IMM6_MASK                 0x3f
#define IMM6_BITS                    6
#define IMM15_MASK              0x7fff
#define IMM15_BITS                  15
#define IMM4_MASK                 0x1f
#define IMM4_BITS                    4
#define CALLR_REG_MASK             0x7
#define CPC_REG_MASK               0xf
#define O2R_REG_MASK               0x7
#define ACF_PART_MASK              0x3
#define DSP_REG_MASK               0xf
#define DSP_PART_MASK             0x17
#define TEMPLATE_NUM_REGS            4
#define TEMPLATE_REGS_MASK         0xf

#define IS_TEMPLATE_DEF(insn) (insn->dsp_daoppame_flags & DSP_DAOPPAME_TEMP)

unsigned int metag_get_set_size_bytes (unsigned int opcode);
unsigned int metag_get_set_ext_size_bytes (unsigned int opcode);
unsigned int metag_cond_set_size_bytes (unsigned int opcode);

#ifdef __cplusplus
}
#endif
