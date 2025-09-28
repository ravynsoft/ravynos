MODULE = XS::APItest		PACKAGE = XS::APItest::numeric

void
grok_number(number)
	SV *number
    PREINIT:
	STRLEN len;
	const char *pv = SvPV(number, len);
	UV value;
	int result;
    PPCODE:
	EXTEND(SP,2);
	result = grok_number(pv, len, &value);
	PUSHs(sv_2mortal(newSViv(result)));
	if (result & IS_NUMBER_IN_UV)
	    PUSHs(sv_2mortal(newSVuv(value)));

void
grok_number_flags(number, flags)
	SV *number
	U32 flags
    PREINIT:
	STRLEN len;
	const char *pv = SvPV(number, len);
	UV value;
	int result;
    PPCODE:
	EXTEND(SP,2);
	result = grok_number_flags(pv, len, &value, flags);
	PUSHs(sv_2mortal(newSViv(result)));
	if (result & IS_NUMBER_IN_UV)
	    PUSHs(sv_2mortal(newSVuv(value)));

void
grok_atoUV(number, endsv)
	SV *number
	SV *endsv
    PREINIT:
	STRLEN len;
	const char *pv = SvPV(number, len);
	UV value = 0xdeadbeef;
	bool result;
	const char* endptr = pv + len;
    PPCODE:
	EXTEND(SP,2);
	if (endsv == &PL_sv_undef) {
          result = grok_atoUV(pv, &value, NULL);
        } else {
          result = grok_atoUV(pv, &value, &endptr);
        }
	PUSHs(result ? &PL_sv_yes : &PL_sv_no);
	PUSHs(sv_2mortal(newSVuv(value)));
	if (endsv == &PL_sv_undef) {
          PUSHs(sv_2mortal(newSVpvn(NULL, 0)));
	} else {
	  if (endptr) {
	    PUSHs(sv_2mortal(newSViv(endptr - pv)));
	  } else {
	    PUSHs(sv_2mortal(newSViv(0)));
	  }
	}

void
my_strtod(s)
        SV *s
    PREINIT:
        SV *sv = newSVsv(s);
        char *endptr = NULL;
        NV nv;
    PPCODE:
        nv = my_strtod(SvPV_force_nolen(sv), &endptr);
        PUSHs(sv_2mortal(newSVnv(nv)));
        if (endptr) {
            sv_chop(sv, endptr);
            PUSHs(sv_2mortal(sv));
        }
