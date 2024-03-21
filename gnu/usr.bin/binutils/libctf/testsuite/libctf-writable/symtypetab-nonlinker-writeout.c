/* Make sure that writing out a dict with a symtypetab without going via
   ctf_link_write (as a compiler might do to generate input destined for a
   linker) always writes out a complete indexed, sorted symtypetab, ignoring the
   set of symbols reported (if any).  Also a test of dynamic dict sym
   iteration.  */

#include <ctf-api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int
report_sym (ctf_dict_t *fp, ctf_link_sym_t *sym, const char *name,
	    uint32_t idx, uint32_t st_type)
{
  sym->st_name = name;
  sym->st_symidx = idx;
  sym->st_type = st_type;
  return ctf_link_add_linker_symbol (fp, sym);
}

static void
try_maybe_reporting (int report)
{
  ctf_dict_t *fp;
  ctf_id_t func, func2, func3, base, base2, base3;
  ctf_encoding_t e = { CTF_INT_SIGNED, 0, sizeof (long) };
  ctf_id_t dummy;
  ctf_funcinfo_t fi;
  ctf_next_t *i = NULL;
  ctf_id_t symtype;
  const char *symname;
  unsigned char *buf;
  size_t bufsiz;
  int err;

  if ((fp = ctf_create (&err)) == NULL)
    goto create_err;

  /* Add a couple of sets of types to hang symbols off.  We use multiple
     identical types so we can distinguish between distinct func / data symbols
     later on.  */

  if (((base = ctf_add_integer (fp, CTF_ADD_ROOT, "long int", &e)) == CTF_ERR) ||
      ((base2 = ctf_add_integer (fp, CTF_ADD_ROOT, "long int", &e)) == CTF_ERR) ||
      ((base3 = ctf_add_integer (fp, CTF_ADD_ROOT, "long int", &e)) == CTF_ERR))
      goto create_types_err;

  fi.ctc_return = base;
  fi.ctc_argc = 0;
  fi.ctc_flags = 0;
  if (((func = ctf_add_function (fp, CTF_ADD_ROOT, &fi, &dummy)) == CTF_ERR) ||
      ((func2 = ctf_add_function (fp, CTF_ADD_ROOT, &fi, &dummy)) == CTF_ERR) ||
      ((func3 = ctf_add_function (fp, CTF_ADD_ROOT, &fi, &dummy)) == CTF_ERR))
    goto create_types_err;

  /* Add some function and data symbols.  We intentionally add the symbols in
     near-inverse order by symbol name, so that we can tell whether the
     (necessarily indexed) section was sorted (since the sort is always in
     lexicographical sort ordef by name).  */
  if ((ctf_add_objt_sym (fp, "data_c", base) < 0) ||
      (ctf_add_objt_sym (fp, "data_a", base2) < 0) ||
      (ctf_add_objt_sym (fp, "data_b", base3) < 0))
    goto create_syms_err;

  if ((ctf_add_func_sym (fp, "func_c", func) < 0) ||
      (ctf_add_func_sym (fp, "func_a", func2) < 0) ||
      (ctf_add_func_sym (fp, "func_b", func3) < 0))
    goto create_syms_err;

  /* Make sure we can iterate over them in a dynamic dict and that they have the
     right types.  We don't care about their order at this stage, which makes
     the validation here a bit more verbose than it is below.  */

  while ((symtype = ctf_symbol_next (fp, &i, &symname, 0)) != CTF_ERR)
    {
      if (symtype == base && strcmp (symname, "data_c") == 0)
	continue;
      if (symtype == base2 && strcmp (symname, "data_a") == 0)
	continue;
      if (symtype == base3 && strcmp (symname, "data_b") == 0)
	continue;
      goto iter_compar_err;
    }
  if (ctf_errno (fp) != ECTF_NEXT_END)
    goto iter_err;

  while ((symtype = ctf_symbol_next (fp, &i, &symname, 1)) != CTF_ERR)
    {
      if (symtype == func && strcmp (symname, "func_c") == 0)
	continue;
      if (symtype == func2 && strcmp (symname, "func_a") == 0)
	continue;
      if (symtype == func3 && strcmp (symname, "func_b") == 0)
	continue;
      goto iter_compar_err;
    }
  if (ctf_errno (fp) != ECTF_NEXT_END)
    goto iter_err;

  /* Look up all the symbols by name and make sure that works.  */

  if (ctf_lookup_by_symbol_name (fp, "data_a") != base2)
    goto lookup_syms_err;
  if (ctf_lookup_by_symbol_name (fp, "data_b") != base3)
    goto lookup_syms_err;
  if (ctf_lookup_by_symbol_name (fp, "data_c") != base)
    goto lookup_syms_err;
  if (ctf_lookup_by_symbol_name (fp, "func_a") != func2)
    goto lookup_syms_err;
  if (ctf_lookup_by_symbol_name (fp, "func_b") != func3)
    goto lookup_syms_err;
  if (ctf_lookup_by_symbol_name (fp, "func_c") != func)
    goto lookup_syms_err;

  /* Possibly report some but not all of the symbols, as if we are a linker (no
     real program would do this without using the ctf_link APIs, but it's not
     *prohibited*, just useless, and if they do we don't want things to
     break.  In particular we want all the symbols written out, reported or no,
     ignoring the reported symbol set entirely.)  */
  if (report)
    {
      ctf_link_sym_t sym;
      sym.st_nameidx_set = 0;
      sym.st_nameidx = 0;
      sym.st_shndx = 404; /* Arbitrary, not SHN_UNDEF or SHN_EXTABS.  */
      sym.st_value = 404; /* Arbitrary, nonzero.  */

      /* STT_OBJECT: 1.  Don't rely on the #define being visible: this may be a
	 non-ELF platform!  */
      if (report_sym (fp, &sym, "data_c", 2, 1) < 0 ||
	  report_sym (fp, &sym, "data_a", 3, 1) < 0)
	goto report_err;

      /* STT_FUNC: 2.  */
      if (report_sym (fp, &sym, "func_c", 4, 2) < 0 ||
	  report_sym (fp, &sym, "func_a", 5, 2) < 0)
	goto report_err;

      /* Look up all the symbols by name now we have reported symbols.  */

      if (ctf_lookup_by_symbol_name (fp, "data_a") != base2)
	goto lookup_syms_err;
      if (ctf_lookup_by_symbol_name (fp, "data_b") != base3)
	goto lookup_syms_err;
      if (ctf_lookup_by_symbol_name (fp, "data_c") != base)
	goto lookup_syms_err;
      if (ctf_lookup_by_symbol_name (fp, "func_a") != func2)
	goto lookup_syms_err;
      if (ctf_lookup_by_symbol_name (fp, "func_b") != func3)
	goto lookup_syms_err;
      if (ctf_lookup_by_symbol_name (fp, "func_c") != func)
	goto lookup_syms_err;
    }

  /* Write out, to memory.  */

  if ((buf = ctf_write_mem (fp, &bufsiz, 4096)) == NULL)
    goto write_err;
  ctf_file_close (fp);

  /* Read back in.  */
  if ((fp = ctf_simple_open ((const char *) buf, bufsiz, NULL, 0, 0, NULL,
			     0, &err)) == NULL)
    goto open_err;

  /* Verify symbol order against the order we expect if this dict is sorted and
     indexed.  */

  struct ctf_symtype_expected
  {
    const char *name;
    ctf_id_t id;
  } *expected;
  struct ctf_symtype_expected expected_obj[] = { { "data_a", base2 },
						 { "data_b", base3 },
						 { "data_c", base },
						 { NULL, 0 } };
  struct ctf_symtype_expected expected_func[] = { { "func_a", func2 },
						  { "func_b", func3 },
						  { "func_c", func },
						  { NULL, 0 } };
  expected = expected_obj;

  while ((symtype = ctf_symbol_next (fp, &i, &symname, 0)) != CTF_ERR)
    {
      if (expected == NULL)
	goto expected_overshoot_err;
      if (symtype != expected->id || strcmp (symname, expected->name) != 0)
	goto expected_compar_err;
      printf ("Seen: %s\n", symname);
      expected++;
    }

  expected = expected_func;
  while ((symtype = ctf_symbol_next (fp, &i, &symname, 1)) != CTF_ERR)
    {
      if (expected == NULL)
	goto expected_overshoot_err;
      if (symtype != expected->id || strcmp (symname, expected->name) != 0)
	goto expected_compar_err;
      printf ("Seen: %s\n", symname);
      expected++;
    }

  ctf_file_close (fp);
  free (buf);

  return;

 create_err:
  fprintf (stderr, "Creation failed: %s\n", ctf_errmsg (err));
  exit (1);
 open_err:
  fprintf (stderr, "Reopen failed: %s\n", ctf_errmsg (err));
  exit (1);
 create_types_err:
  fprintf (stderr, "Cannot create types: %s\n", ctf_errmsg (ctf_errno (fp)));
  exit (1);
 create_syms_err:
  fprintf (stderr, "Cannot create syms: %s\n", ctf_errmsg (ctf_errno (fp)));
  exit (1);
 iter_compar_err:
  fprintf (stderr, "Dynamic iteration comparison failure: %s "
	   "(reported type: %lx)\n", symname, symtype);
  exit (1);
 iter_err:
  fprintf (stderr, "Cannot iterate: %s\n", ctf_errmsg (ctf_errno (fp)));
  exit (1);
 report_err:
  fprintf (stderr, "Cannot report symbol: %s\n", ctf_errmsg (ctf_errno (fp)));
  exit (1);
 write_err:
  fprintf (stderr, "Cannot write out: %s\n", ctf_errmsg (ctf_errno (fp)));
  exit (1);
 expected_overshoot_err:
  fprintf (stderr, "Too many symbols in post-writeout comparison\n");
  exit (1);
 lookup_syms_err:
  fprintf (stderr, "Explicit lookup of symbols by name failed: %s\n",
	   ctf_errmsg (ctf_errno (fp)));
  exit (1);
 expected_compar_err:
  fprintf (stderr, "Non-dynamic iteration comparison failure: %s "
	   "(type %lx): expected %s (type %lx)\n", symname, symtype,
	   expected->name, expected->id);
  exit (1);
}

int
main (int argc, char *argv[])
{
  try_maybe_reporting (0);
  try_maybe_reporting (1);
}
