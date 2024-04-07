/*******************************************************************************
*
*  Perl/Pollution/Portability
*
********************************************************************************
*
*  Version 3.x, Copyright (C) 2004-2013, Marcus Holland-Moritz.
*  Version 2.x, Copyright (C) 2001, Paul Marquess.
*  Version 1.x, Copyright (C) 1999, Kenneth Albanowski.
*
*  This program is free software; you can redistribute it and/or
*  modify it under the same terms as Perl itself.
*
*******************************************************************************/

#include "EXTERN.h"
#include "perl.h"

#define NEED_PL_parser
#define NO_XSLOCKS
#include "XSUB.h"

#include "ppport.h"

static void throws_exception(int throw_e)
{
  if (throw_e)
    croak("boo\n");
}

int exception(int throw_e)
{
  dTHR;
  dXCPT;
  SV *caught = get_sv("Devel::PPPort::exception_caught", 0);

  XCPT_TRY_START {
    throws_exception(throw_e);
  } XCPT_TRY_END

  XCPT_CATCH
  {
    sv_setiv(caught, 1);
    XCPT_RETHROW;
  }

  sv_setiv(caught, 0);

  return 42;
}

void call_newCONSTSUB_3(void)
{
  newCONSTSUB(gv_stashpv("Devel::PPPort", FALSE), "test_value_3", newSViv(3));
}

U32 get_PL_signals_3(void)
{
  return PL_signals;
}

int dummy_parser_warning(void)
{
  char * volatile my_bufptr;
  char * volatile *my_p_bufptr;
  my_bufptr = PL_bufptr;
  my_p_bufptr = &PL_bufptr;
  PL_bufptr = my_bufptr;
  PL_bufptr = *my_p_bufptr;
  return my_p_bufptr != NULL;
}
