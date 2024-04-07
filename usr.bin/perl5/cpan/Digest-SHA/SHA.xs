#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#ifdef SvPVbyte
	#if PERL_REVISION == 5 && PERL_VERSION < 8
		#undef SvPVbyte
		#define SvPVbyte(sv, lp) \
			(sv_utf8_downgrade((sv), 0), SvPV((sv), (lp)))
	#endif
#else
	#define SvPVbyte SvPV
#endif

#ifndef dTHX
	#define pTHX_
	#define aTHX_
#endif

#ifndef PerlIO
	#define PerlIO				FILE
	#define PerlIO_read(f, buf, count)	fread(buf, 1, count, f)
#endif

#ifndef sv_derived_from
	#include "src/sdf.c"
#endif

#ifndef Newx
	#define Newx(ptr, num, type)	New(0, ptr, num, type)
	#define Newxz(ptr, num, type)	Newz(0, ptr, num, type)
#endif

#include "src/sha.c"

static const int ix2alg[] =
	{1,1,1,224,224,224,256,256,256,384,384,384,512,512,512,
	512224,512224,512224,512256,512256,512256};

#ifndef INT2PTR
#define INT2PTR(p, i) (p) (i)
#endif

#define MAX_WRITE_SIZE 16384
#define IO_BUFFER_SIZE 4096

static SHA *getSHA(pTHX_ SV *self)
{
	if (!sv_isobject(self) || !sv_derived_from(self, "Digest::SHA"))
		return(NULL);
	return INT2PTR(SHA *, SvIV(SvRV(self)));
}

MODULE = Digest::SHA		PACKAGE = Digest::SHA

PROTOTYPES: ENABLE

int
shainit(s, alg)
	SHA *	s
	int	alg

void
sharewind(s)
	SHA *	s

unsigned long
shawrite(bitstr, bitcnt, s)
	unsigned char *	bitstr
	unsigned long	bitcnt
	SHA *	s

SV *
newSHA(classname, alg)
	char *	classname
	int 	alg
PREINIT:
	SHA *state;
CODE:
	Newxz(state, 1, SHA);
	if (!shainit(state, alg)) {
		Safefree(state);
		XSRETURN_UNDEF;
	}
	RETVAL = newSV(0);
	sv_setref_pv(RETVAL, classname, (void *) state);
	SvREADONLY_on(SvRV(RETVAL));
OUTPUT:
	RETVAL

SV *
clone(self)
	SV *	self
PREINIT:
	SHA *state;
	SHA *clone;
CODE:
	if ((state = getSHA(aTHX_ self)) == NULL)
		XSRETURN_UNDEF;
	Newx(clone, 1, SHA);
	RETVAL = newSV(0);
	sv_setref_pv(RETVAL, sv_reftype(SvRV(self), 1), (void *) clone);
	SvREADONLY_on(SvRV(RETVAL));
	Copy(state, clone, 1, SHA);
OUTPUT:
	RETVAL

void
DESTROY(s)
	SHA *	s
CODE:
	Safefree(s);

SV *
sha1(...)
ALIAS:
	Digest::SHA::sha1 = 0
	Digest::SHA::sha1_hex = 1
	Digest::SHA::sha1_base64 = 2
	Digest::SHA::sha224 = 3
	Digest::SHA::sha224_hex = 4
	Digest::SHA::sha224_base64 = 5
	Digest::SHA::sha256 = 6
	Digest::SHA::sha256_hex = 7
	Digest::SHA::sha256_base64 = 8
	Digest::SHA::sha384 = 9
	Digest::SHA::sha384_hex = 10
	Digest::SHA::sha384_base64 = 11
	Digest::SHA::sha512 = 12
	Digest::SHA::sha512_hex = 13
	Digest::SHA::sha512_base64 = 14
	Digest::SHA::sha512224 = 15
	Digest::SHA::sha512224_hex = 16
	Digest::SHA::sha512224_base64 = 17
	Digest::SHA::sha512256 = 18
	Digest::SHA::sha512256_hex = 19
	Digest::SHA::sha512256_base64 = 20
PREINIT:
	int i;
	UCHR *data;
	STRLEN len;
	SHA sha;
	char *result;
CODE:
	if (!shainit(&sha, ix2alg[ix]))
		XSRETURN_UNDEF;
	for (i = 0; i < items; i++) {
		data = (UCHR *) (SvPVbyte(ST(i), len));
		while (len > MAX_WRITE_SIZE) {
			shawrite(data, MAX_WRITE_SIZE << 3, &sha);
			data += MAX_WRITE_SIZE;
			len  -= MAX_WRITE_SIZE;
		}
		shawrite(data, (ULNG) len << 3, &sha);
	}
	shafinish(&sha);
	len = 0;
	if (ix % 3 == 0) {
		result = (char *) shadigest(&sha);
		len = sha.digestlen;
	}
	else if (ix % 3 == 1)
		result = shahex(&sha);
	else
		result = shabase64(&sha);
	RETVAL = newSVpv(result, len);
OUTPUT:
	RETVAL

SV *
hmac_sha1(...)
ALIAS:
	Digest::SHA::hmac_sha1 = 0
	Digest::SHA::hmac_sha1_hex = 1
	Digest::SHA::hmac_sha1_base64 = 2
	Digest::SHA::hmac_sha224 = 3
	Digest::SHA::hmac_sha224_hex = 4
	Digest::SHA::hmac_sha224_base64 = 5
	Digest::SHA::hmac_sha256 = 6
	Digest::SHA::hmac_sha256_hex = 7
	Digest::SHA::hmac_sha256_base64 = 8
	Digest::SHA::hmac_sha384 = 9
	Digest::SHA::hmac_sha384_hex = 10
	Digest::SHA::hmac_sha384_base64 = 11
	Digest::SHA::hmac_sha512 = 12
	Digest::SHA::hmac_sha512_hex = 13
	Digest::SHA::hmac_sha512_base64 = 14
	Digest::SHA::hmac_sha512224 = 15
	Digest::SHA::hmac_sha512224_hex = 16
	Digest::SHA::hmac_sha512224_base64 = 17
	Digest::SHA::hmac_sha512256 = 18
	Digest::SHA::hmac_sha512256_hex = 19
	Digest::SHA::hmac_sha512256_base64 = 20
PREINIT:
	int i;
	UCHR *key = (UCHR *) "";
	UCHR *data;
	STRLEN len = 0;
	HMAC hmac;
	char *result;
CODE:
	if (items > 0) {
		key = (UCHR *) (SvPVbyte(ST(items-1), len));
	}
	if (hmacinit(&hmac, ix2alg[ix], key, (UINT) len) == NULL)
		XSRETURN_UNDEF;
	for (i = 0; i < items - 1; i++) {
		data = (UCHR *) (SvPVbyte(ST(i), len));
		while (len > MAX_WRITE_SIZE) {
			hmacwrite(data, MAX_WRITE_SIZE << 3, &hmac);
			data += MAX_WRITE_SIZE;
			len  -= MAX_WRITE_SIZE;
		}
		hmacwrite(data, (ULNG) len << 3, &hmac);
	}
	hmacfinish(&hmac);
	len = 0;
	if (ix % 3 == 0) {
		result = (char *) hmacdigest(&hmac);
		len = hmac.digestlen;
	}
	else if (ix % 3 == 1)
		result = hmachex(&hmac);
	else
		result = hmacbase64(&hmac);
	RETVAL = newSVpv(result, len);
OUTPUT:
	RETVAL

int
hashsize(self)
	SV *	self
ALIAS:
	Digest::SHA::hashsize = 0
	Digest::SHA::algorithm = 1
PREINIT:
	SHA *state;
CODE:
	if ((state = getSHA(aTHX_ self)) == NULL)
		XSRETURN_UNDEF;
	RETVAL = ix ? state->alg : (int) (state->digestlen << 3);
OUTPUT:
	RETVAL

void
add(self, ...)
	SV *	self
PREINIT:
	int i;
	UCHR *data;
	STRLEN len;
	SHA *state;
PPCODE:
	if ((state = getSHA(aTHX_ self)) == NULL)
		XSRETURN_UNDEF;
	for (i = 1; i < items; i++) {
		data = (UCHR *) (SvPVbyte(ST(i), len));
		while (len > MAX_WRITE_SIZE) {
			shawrite(data, MAX_WRITE_SIZE << 3, state);
			data += MAX_WRITE_SIZE;
			len  -= MAX_WRITE_SIZE;
		}
		shawrite(data, (ULNG) len << 3, state);
	}
	XSRETURN(1);

SV *
digest(self)
	SV *	self
ALIAS:
	Digest::SHA::digest = 0
	Digest::SHA::hexdigest = 1
	Digest::SHA::b64digest = 2
PREINIT:
	STRLEN len;
	SHA *state;
	char *result;
CODE:
	if ((state = getSHA(aTHX_ self)) == NULL)
		XSRETURN_UNDEF;
	shafinish(state);
	len = 0;
	if (ix == 0) {
		result = (char *) shadigest(state);
		len = state->digestlen;
	}
	else if (ix == 1)
		result = shahex(state);
	else
		result = shabase64(state);
	RETVAL = newSVpv(result, len);
	sharewind(state);
OUTPUT:
	RETVAL

SV *
_getstate(self)
	SV *	self
PREINIT:
	SHA *state;
	UCHR buf[256];
	UCHR *ptr = buf;
CODE:
	if ((state = getSHA(aTHX_ self)) == NULL)
		XSRETURN_UNDEF;
	Copy(digcpy(state), ptr, state->alg <= SHA256 ? 32 : 64, UCHR);
	ptr += state->alg <= SHA256 ? 32 : 64;
	Copy(state->block, ptr, state->alg <= SHA256 ? 64 : 128, UCHR);
	ptr += state->alg <= SHA256 ? 64 : 128;
	ptr = w32mem(ptr, state->blockcnt);
	ptr = w32mem(ptr, state->lenhh);
	ptr = w32mem(ptr, state->lenhl);
	ptr = w32mem(ptr, state->lenlh);
	ptr = w32mem(ptr, state->lenll);
	RETVAL = newSVpv((char *) buf, (STRLEN) (ptr - buf));
OUTPUT:
	RETVAL

void
_putstate(self, packed_state)
	SV *	self
	SV *	packed_state
PREINIT:
	UINT bc;
	STRLEN len;
	SHA *state;
	UCHR *data;
PPCODE:
	if ((state = getSHA(aTHX_ self)) == NULL)
		XSRETURN_UNDEF;
	data = (UCHR *) SvPV(packed_state, len);
	if (len != (state->alg <= SHA256 ? 116U : 212U))
		XSRETURN_UNDEF;
	data = statecpy(state, data);
	Copy(data, state->block, state->blocksize >> 3, UCHR);
	data += (state->blocksize >> 3);
	bc = memw32(data), data += 4;
	if (bc >= (state->alg <= SHA256 ? 512U : 1024U))
		XSRETURN_UNDEF;
	state->blockcnt = bc;
	state->lenhh = memw32(data), data += 4;
	state->lenhl = memw32(data), data += 4;
	state->lenlh = memw32(data), data += 4;
	state->lenll = memw32(data);
	XSRETURN(1);

void
_addfilebin(self, f)
	SV *		self
	PerlIO *	f
PREINIT:
	SHA *state;
	int n;
	UCHR in[IO_BUFFER_SIZE];
PPCODE:
	if (!f || (state = getSHA(aTHX_ self)) == NULL)
		XSRETURN_UNDEF;
	while ((n = (int) PerlIO_read(f, in, sizeof(in))) > 0)
		shawrite(in, (ULNG) n << 3, state);
	XSRETURN(1);

void
_addfileuniv(self, f)
	SV *		self
	PerlIO *	f
PREINIT:
	UCHR c;
	int n;
	int cr = 0;
	UCHR *src, *dst;
	UCHR in[IO_BUFFER_SIZE+1];
	SHA *state;
PPCODE:
	if (!f || (state = getSHA(aTHX_ self)) == NULL)
		XSRETURN_UNDEF;
	while ((n = (int) PerlIO_read(f, in+1, IO_BUFFER_SIZE)) > 0) {
		for (dst = in, src = in + 1; n; n--) {
			c = *src++;
			if (!cr) {
				if (c == '\015')
					cr = 1;
				else
					*dst++ = c;
			}
			else {
				if (c == '\015')
					*dst++ = '\012';
				else if (c == '\012') {
					*dst++ = '\012';
					cr = 0;
				}
				else {
					*dst++ = '\012';
					*dst++ = c;
					cr = 0;
				}
			}
		}
		shawrite(in, (ULNG) (dst - in) << 3, state);
	}
	if (cr) {
		in[0] = '\012';
		shawrite(in, 1UL << 3, state);
	}
	XSRETURN(1);
