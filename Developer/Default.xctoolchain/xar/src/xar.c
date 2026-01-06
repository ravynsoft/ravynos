/*
 * Copyright (c) 2005 Rob Braun
 * Portions Copyright (c) 2012 Kyle J. McKay.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Rob Braun nor the names of his contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * 03-Apr-2005
 * DRI: Rob Braun <bbraun@synack.net>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlstring.h>
#include <limits.h>
#include <getopt.h>
#include <regex.h>
#include <errno.h>
#include <time.h>
#include "xar.h"
#include "../lib/filetree.h"
#include "../lib/config.h"

#define MIN_XAR_NEW_OPTIONS 0x01060180

#define SYMBOLIC 1
#define NUMERIC  2

/* error codes for B&I */
#define E_NOSIG     60
#define E_SIGEXISTS 61

struct HashType {
  const char *name; /* "none", "sha1", etc. */
  size_t hashlen;
  const unsigned char *diprefix;
  size_t diprefixlen;
};

static const unsigned char Md5DigestInfoPrefix[18] = {
	0x30, 0x20, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86,
	0x48, 0x86, 0xf7, 0x0d, 0x02, 0x05, 0x05, 0x00,
        0x04, 0x10
};

static const unsigned char Sha1DigestInfoPrefix[15] = {
	0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e,
	0x03, 0x02, 0x1a, 0x05, 0x00, 0x04, 0x14
};

static const unsigned char Sha224DigestInfoPrefix[19] = {
	0x30, 0x2d, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86,
	0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04, 0x05,
        0x00, 0x04, 0x1c
};

static const unsigned char Sha256DigestInfoPrefix[19] = {
	0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86,
	0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05,
        0x00, 0x04, 0x20
};

static const unsigned char Sha384DigestInfoPrefix[19] = {
	0x30, 0x41, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86,
	0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02, 0x05,
        0x00, 0x04, 0x30
};

static const unsigned char Sha512DigestInfoPrefix[19] = {
	0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86,
	0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x05,
        0x00, 0x04, 0x40
};

static const struct HashType HashTypes[] = {
  { XAR_OPT_VAL_NONE,   0,  NULL, 0 },
  { XAR_OPT_VAL_MD5,    16, Md5DigestInfoPrefix,    sizeof(Md5DigestInfoPrefix) },
  { XAR_OPT_VAL_SHA1,   20, Sha1DigestInfoPrefix,   sizeof(Sha1DigestInfoPrefix) },
  { XAR_OPT_VAL_SHA224, 28, Sha224DigestInfoPrefix, sizeof(Sha224DigestInfoPrefix) },
  { XAR_OPT_VAL_SHA256, 32, Sha256DigestInfoPrefix, sizeof(Sha256DigestInfoPrefix) },
  { XAR_OPT_VAL_SHA384, 48, Sha384DigestInfoPrefix, sizeof(Sha384DigestInfoPrefix) },
  { XAR_OPT_VAL_SHA512, 64, Sha512DigestInfoPrefix, sizeof(Sha512DigestInfoPrefix) }
};

#define SHA1_HASH_INDEX 2

static unsigned long xar_lib_version = 0;
static int xar_lib_version_fetched = 0;

static struct HashType CustomTocHash, CustomFileHash;

static int Perms = 0;
static int Local = 0;
static char *Subdoc = NULL;
static char *SubdocName = NULL;
static const struct HashType *Toccksum = NULL;
static const struct HashType *Filecksum = NULL;
static char *Compression = NULL;
static char *Rsize = NULL;
static char *CompressionArg = NULL;
static char *Chdir = NULL;
static char *DataToSignDumpPath = NULL;
static char *SigOffsetDumpPath = NULL;
static char *SignatureDumpPath = NULL;
static char *StripComponents = NULL;

static int Err = 0;
static int List = 0;
static int Verbose = 0;
static int Coalesce = 0;
static int LinkSame = 0;
static int NoOverwrite = 0;
static int SaveSuid = 0;
static int DoSign = 0;
static int DumpDigestInfo = 0;
static int Recompress = 0;
static int ToStdout = 0;
static int RFC6713 = 0;

static long SigSize = 0;
static int SigSizePresent = 0;

struct lnode {
	char *str;
	regex_t reg;
	struct lnode *next;
};
struct cnode {
	char *cert_path;
	struct cnode *next;
};
struct __stack_element {
	struct __stack_element *prev;
	struct __stack_element *next;
	void *data;
};
struct __stack {
	struct __stack_element *bottom;
	struct __stack_element *top;
};

typedef struct __stack_element *stack_element;
typedef struct __stack *stack;

static struct lnode *Exclude = NULL;
static struct lnode *Exclude_Tail = NULL;
static struct lnode *NoCompress = NULL;
static struct lnode *NoCompress_Tail = NULL;
static struct lnode *PropInclude = NULL;
static struct lnode *PropInclude_Tail = NULL;
static struct lnode *PropExclude = NULL;
static struct lnode *PropExclude_Tail = NULL;
static struct cnode *CertPath = NULL;
static struct cnode *CertPath_Tail = NULL;

static char *unlink_temp_file = NULL;

static int32_t err_callback(int32_t sev, int32_t err, xar_errctx_t ctx, void *usrctx);
static int32_t signingCallback(xar_signature_t sig, void *context, uint8_t *data, uint32_t length, uint8_t **signed_data, uint32_t *signed_len);
static void insert_cert(xar_signature_t sig, const char *cert_path);
static const struct HashType *get_hash_alg(const char *str);

static void print_file(xar_t x, xar_file_t f, FILE *out) {
	if( List && Verbose ) {
		char *size = xar_get_size(x, f);
		char *path = xar_get_path(f);
		char *type = xar_get_type(x, f);
		char *mode = xar_get_mode(x, f);
		char *user = xar_get_owner(x, f);
		char *group = xar_get_group(x, f);
		char *mtime = xar_get_mtime(x, f);
		fprintf(out, "%s %8s/%-8s %10s %s %s\n", mode, user, group, size, mtime, path);
		free(size);
		free(type);
		free(path);
		free(mode);
		free(user);
		free(group);
		free(mtime);
	} else if( List || Verbose ) {
		char *path = xar_get_path(f);
		fprintf(out, "%s\n", path);
		free(path);
	}
}

static void add_subdoc(xar_t x) {
	xar_subdoc_t s;
	int fd;
	unsigned char *buf;
	unsigned int len;
	struct stat sb;
	ssize_t rcnt;

	if( SubdocName == NULL ) SubdocName = "subdoc";

	fd = open(Subdoc, O_RDONLY);
	if( fd < 0 ) {
		fprintf(stderr, "ERROR: subdoc file %s doesn't exist -- ignoring\n", Subdoc);
		return;
	}
	s = xar_subdoc_new(x, (const char *)SubdocName);
	fstat(fd, &sb);
	len = (unsigned)sb.st_size;
	buf = malloc(len+1);
	if( buf == NULL ) {
		close(fd);
		return;
	}
	memset(buf, 0, len+1);
	rcnt = read(fd, buf, len);
	close(fd);
	if (rcnt != (ssize_t)len) {
		fprintf(stderr, "Error reading subdoc %s\n", Subdoc);
		exit(1);
	}

	xar_subdoc_copyin(s, buf, len);


	return;
}

static void extract_subdoc(xar_t x, const char *name) {
	ssize_t wcnt;
	xar_subdoc_t i;

	for( i = xar_subdoc_first(x); i; i = xar_subdoc_next(i) ) {
		const char *sname = xar_subdoc_name(i);
		unsigned char *sdoc;
		int fd, size;
		if( name && strcmp(name, sname) != 0 )
			continue;
		xar_subdoc_copyout(i, &sdoc, (unsigned int *)&size);
		fd = open(Subdoc, O_WRONLY|O_CREAT|O_TRUNC, 0644);
		if( fd < 0 ) return;
		wcnt = write(fd, sdoc, size);
		if (wcnt != (ssize_t)size) {
			fprintf(stderr, "Error writing subdoc %s\n", Subdoc);
			exit(1);
		}
		close(fd);
		free(sdoc);
	}

	return;
}

static void extract_data_to_sign(const char *filename) {
	xar_signature_t sig;
	uint64_t signatureOffset;
	FILE *file;
	xar_t x;
	int i;
	uint64_t dataToSignOffset = 0;
	uint32_t dataToSignSize = 0;
	char *buffer = NULL;
	const char *value;
	const struct HashType *hash = NULL;

	/* find signature stub */
	x = xar_open(filename, READ);
	if ( x == NULL ) {
		fprintf(stderr, "Could not open %s to extract data to sign\n", filename);
		exit(1);
	}
	sig = xar_signature_first(x);
	if ( !sig ) {
		fprintf(stderr, "No signatures found to extract data from\n");
		exit(E_NOSIG);
	}

	/* locate data to sign */
	if (DumpDigestInfo) {
		const char *hash_name = xar_attr_get((xar_file_t)x, "checksum", "style");
		if (hash_name)
			hash = get_hash_alg(hash_name);
		if (!hash_name || !hash) {
			fprintf(stderr, "--digestinfo-to-sign does not support hash type \"%s\"\n", hash_name ? hash_name : XAR_OPT_VAL_NONE);
			exit(1);
		}
	}
	dataToSignOffset = xar_get_heap_offset(x);
	dataToSignOffset += strtoull(value, (char **)NULL, 10);
	if( 0 != xar_prop_get((xar_file_t)x, "checksum/size" ,&value) ){
		fprintf(stderr, "Could not locate checksum/size in archive\n");
		exit(1);
	}
	dataToSignSize = strtoull(value, (char **)NULL, 10);

	/* get signature offset (inject signature here) */
	xar_signature_copy_signed_data(sig, NULL, NULL, NULL, NULL, &signatureOffset);
	signatureOffset += xar_get_heap_offset(x);
	xar_close(x);

	/* now get data to be signed, using offset and size */
	file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "Could not open %s for reading data to sign\n", filename);
		exit(1);
	}
	fseek(file, (long)dataToSignOffset, SEEK_SET);
	buffer = malloc(dataToSignSize);
	i = (int)fread(buffer, dataToSignSize, 1, file);
	if (i != 1) {
		fprintf(stderr, "Failed to read data to sign from %s\n", filename);
		exit(1);
	}
	fclose(file);

	/* save data to sign */
	file = fopen(DataToSignDumpPath, "wb");
	if (!file) {
		fprintf(stderr, "Could not open %s for saving data to sign\n", DataToSignDumpPath);
		exit(1);
	}
	if (DumpDigestInfo) {
		i = (int)fwrite(hash->diprefix, hash->diprefixlen, 1, file);
		if (i != 1) {
			fprintf(stderr, "Failed to write DigestInfo data to sign prefix to %s (fwrite() returned %i)\n", DataToSignDumpPath, i);
			exit(1);
		}
	}
	i = (int)fwrite(buffer, dataToSignSize, 1, file);
	if (i != 1) {
		fprintf(stderr, "Failed to write %sdata to sign to %s (fwrite() returned %i)\n", DumpDigestInfo?"DigestInfo ":"", DataToSignDumpPath, i);
		exit(1);
	}
	fclose(file);

	if (SigOffsetDumpPath) {
		/* save signature offset */
		file = fopen(SigOffsetDumpPath, "wb");
		if (!file) {
			fprintf(stderr, "Could not open %s for saving signature offset\n", SigOffsetDumpPath);
			exit(1);
		}
		i = fprintf(file, "%llu\n", (unsigned long long)signatureOffset);
		if (i < 0) {
			fprintf(stderr, "Failed to write signature offset to %s (fprintf() returned %i)\n", SigOffsetDumpPath, i);
			exit(1);
		}
		fclose(file);
	}

	free(buffer);
}

static const unsigned char b64_table[64] = {
'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

static char *xar_to_base64(const void *input, unsigned int len, unsigned *ocnt)
{
#define INP(x) ((const unsigned char *)(x))
    unsigned char b6;
    unsigned int count = 0;
    unsigned i=0;
    unsigned char *output;
    int outsize = (((float)len)*4/3)+5;

    if (ocnt) *ocnt = 0;
    output = (unsigned char *)malloc(outsize);
    if( !output )
        return NULL;

    for(;;) {
        if( i >= len ) {
            output[count++] = '\0';
            if (ocnt) *ocnt = count;
            return (char *)output;
        }
        b6 = INP(input)[i];
        b6 >>= 2;
        output[count++] = b64_table[b6];

        b6 = INP(input)[i++];
        b6 &= 0x03;
        b6 <<= 4;
        if( i >= len ) {
            output[count++] = b64_table[b6];
            output[count++] = '=';
            output[count++] = '=';
            output[count++] = '\0';
            if (ocnt) *ocnt = count;
            return (char *)output;
        }
        b6 |= INP(input)[i] >> 4;
        output[count++] = b64_table[b6];

        b6 = INP(input)[i++] & 0x0F;
        b6 <<= 2;
        if( i >= len ) {
            output[count++] = b64_table[b6];
            output[count++] = '=';
            output[count++] = '\0';
            if (ocnt) *ocnt = count;
            return (char *)output;
        }
        b6 |= INP(input)[i] >> 6;
        output[count++] = b64_table[b6];

        b6 = INP(input)[i++] & 0x3F;
        output[count++] = b64_table[b6];
    }
#undef INP
}

static void extract_certs(char *filename, char *cert_base_path, char *CApath) {
	xar_signature_t sig;
	xar_t x;
	int32_t count;
	int i, n;
	const uint8_t *cert_data;
	uint32_t cert_len;
	FILE *file, *CAfile = NULL;
	char *cert_path;

	/* open xar, get signature */
	x = xar_open(filename, READ);
	if ( x == NULL ) {
		fprintf(stderr, "Could not open %s to extract certificates\n", filename);
		exit(1);
	}
	sig = xar_signature_first(x);
	if ( !sig ) {
		fprintf(stderr, "No signatures found to extract data from\n");
		exit(E_NOSIG);
	}

	/* iterate through all certificates associated with that signature, write them to disk */
	count = xar_signature_get_x509certificate_count(sig);
	if (!count) {
		fprintf(stderr, "Signature bears no certificates -- how odd\n");
		exit(1);
	}
	if (CApath) {
		CAfile = fopen(CApath, "wb");
		if (!CAfile) {
			fprintf(stderr, "Could not save certificates to %s\n", CApath);
			exit(1);
		}
	}
	for (i=0; i<count; i++) {
		xar_signature_get_x509certificate_data(sig, i, &cert_data, &cert_len);
		if (cert_base_path) {
			if (asprintf(&cert_path, "%s/cert%02i", cert_base_path, i) == -1) {
				fprintf(stderr, "Could not save certificate %i to %s\n", i, cert_path);
				exit(1);
			}
			file = fopen(cert_path, "wb");
			if (!file) {
				fprintf(stderr, "Could not save certificate %i to %s\n", i, cert_path);
				exit(1);
			}
			n = (int)fwrite(cert_data, cert_len, 1, file);
			if (n < 0) {
				fprintf(stderr, "Failed to write certificate to %s (fwrite() returned %i)\n", cert_path, n);
				exit(1);
			}
			fclose(file);
			free(cert_path);
		}
		if (CAfile) {
			unsigned cnt;
			char *p;
			char *b64 = xar_to_base64(cert_data, cert_len, &cnt);
			if (!b64 || !cnt) {
				fprintf(stderr, "Could not save certificates to %s\n", CApath);
				exit(1);
			}
			if (fputs("-----BEGIN CERTIFICATE-----", CAfile) == -1) {
				fprintf(stderr, "Failed to write certificates to %s\n", CApath);
				exit(1);
			}
			--cnt; /* skip trailing nul */
			for (p = b64; cnt > 64; p += 64, cnt -= 64) {
				if (fputs("\n", CAfile) == -1 || (n = (int)fwrite(p, 64, 1, CAfile)) < 0) {
					fprintf(stderr, "Failed to write certificates to %s\n", CApath);
					exit(1);
				}
			}
			if (cnt) {
				if (fputs("\n", CAfile) == -1 || (n = (int)fwrite(p, cnt, 1, CAfile)) < 0) {
					fprintf(stderr, "Failed to write certificates to %s\n", CApath);
					exit(1);
				}
			}
			if (fputs("\n-----END CERTIFICATE-----\n", CAfile) == -1) {
				fprintf(stderr, "Failed to write certificates to %s\n", CApath);
				exit(1);
			}
			free(b64);
		}
	}
	if (CAfile) {
		fclose(CAfile);
	}

	/* clean up */
	xar_close(x);
}

static int32_t get_sig_info(xar_t x, uint64_t *sig_off, uint32_t *sig_len, uint8_t **sig_data) {
	int32_t err;
	uint64_t signedDataOffset = 0;

	xar_signature_t sig = xar_signature_first(x);
	err = xar_signature_copy_signed_data(sig,NULL,NULL,sig_data,sig_len,&signedDataOffset);
	if (!err && sig_off) {
		signedDataOffset += xar_get_heap_offset(x);
		*sig_off = signedDataOffset;
	}

	return err;
}

static void write_sig_offset(const char *filename, uint64_t signedDataOffset) {
	int i;
	FILE *file = fopen(filename, "wb");
	if (!file) {
		fprintf(stderr, "Could not open %s for saving signature offset\n", filename);
		exit(1);
	}
	i = (int)fprintf(file, "%llu\n", (unsigned long long)signedDataOffset);
	if (i < 0) {
		fprintf(stderr, "Failed to write signature offset to %s (fprintf() returned %i)\n", filename, i);
		exit(1);
	}
	fclose(file);
}

static void extract_sig_offset(xar_t x, const char *filename) {
	uint64_t signedDataOffset;

	/* get offset */
	if (get_sig_info(x, &signedDataOffset, NULL, NULL) != 0) {
		fprintf(stderr, "Could not read signature offset from %s\n", filename);
		exit(1);
	}

	/* and save it to file */
	write_sig_offset(filename, signedDataOffset);
}

static void extract_signature(const char *filename, const char *sigfile) {
	xar_t x;
	xar_signature_t sig;
	uint64_t signedDataOffset;
	uint32_t signedDataLength;
	uint8_t *signedData;
	FILE *file;
	int i;

	/* open xar, get signature */
	x = xar_open(filename, READ);
	if ( x == NULL ) {
		fprintf(stderr, "Could not open %s to extract signature data\n", filename);
		exit(1);
	}
	sig = xar_signature_first(x);
	if ( !sig ) {
		fprintf(stderr, "No signatures found to extract data from\n");
		exit(E_NOSIG);
	}

	/* get the signature data */
	if (get_sig_info(x, &signedDataOffset, &signedDataLength, &signedData) != 0
	    || !signedDataLength || !signedData) {
		fprintf(stderr, "Could not read signature data from %s!\n", filename);
		exit(1);
	}
	xar_close(x);

	/* and save it to file */
	file = fopen(sigfile, "wb");
	if (!file) {
		fprintf(stderr, "Could not open %s for saving signature data\n", sigfile);
		exit(1);
	}
	i = (int)fwrite(signedData, signedDataLength, 1, file);
	if (i < 0) {
		fprintf(stderr, "Failed to write signature data to %s (fwrite() returned %i)\n", filename, i);
		exit(1);
	}
	fclose(file);

	if( SigOffsetDumpPath )
		/* also save sig offset if --sig-offset present */
		write_sig_offset(SigOffsetDumpPath, signedDataOffset);

}

stack stack_new() {
	stack s = (stack)malloc(sizeof(struct __stack));
	s->bottom = s->top = NULL;
	return s;
}
void stack_free(stack s) {
	free(s);
}
void stack_push(stack s, void *data) {
	stack_element e = malloc(sizeof(struct __stack_element));
	e->data = data;
	if (s->top) {
		s->top->next = e;
		e->prev = s->top;
	} else {
		s->top = s->bottom = e;
		e->prev = NULL;
	}
	e->next = NULL;
	s->top = e;
}
void *stack_pop(stack s) {
	void *ret;
	stack_element temp;
	if (!s->top)
		return NULL;
	ret = s->top->data;
	temp = s->top;
	s->top = s->top->prev;
	free(temp);
	if (s->top)
		s->top->next = NULL;
	else 
		s->bottom = NULL;
	return ret;
}

/* is_valid_dir: check for a valid directory.
                 Returns 0 if invalid, non-zero if valid
                 NULL means check the current directory
*/
static int is_valid_dir(const char *dirpath) {
	int err, result, curdir = open(".", O_RDONLY);
	if (curdir == -1)
		return 0;
	if (dirpath)
		result = chdir(dirpath);
	else
		result = chdir(".");
	err = fchdir(curdir);
	close(curdir);
	return result == 0;
}

/* get_umask: return the current umask without changing it
*/
static mode_t get_umask(void) {
	mode_t curmask = umask(077);
	umask(curmask);
	return curmask;
}

/* remove_temp: remove the temporary file
*/
static void remove_temp(void) {
	if (unlink_temp_file) {
		char *remove_file = unlink_temp_file;
		unlink_temp_file = NULL;
		unlink(remove_file);
		free(remove_file);
	}
}

/* replace_sign: rip out all current signatures and certs and insert a new pair
		Since libxar is currently not capable of doing this directly, we have to create a new xar archive,
		copy all the files and options from the current archive, and sign the new archive
*/
static void replace_sign(const char *filename) {

	static const char *const opts[5] = {XAR_OPT_COMPRESSION, XAR_OPT_COALESCE, XAR_OPT_LINKSAME, XAR_OPT_RSIZE, XAR_OPT_OWNERSHIP};
	xar_t old_xar, new_xar;
	char *new_xar_path;
	char *systemcall;
	const char *temp_dir;
	size_t new_xar_path_len;
	struct cnode *c;
	int err, tempfd;
	const char *hash_name = NULL;
	/* copy options -- this may not be effective since these do not appear to be stored in the TOC */
	int i;
	const char *opt;
	xar_iter_t iter;
	xar_file_t f;
	const char *name;
	stack s_new;
	stack s_old;
	xar_file_t last_copied, last_added;
	xar_iter_t loopIter;
	xar_file_t current_xar_file;

	/* open both archives */
	old_xar = xar_open(filename, READ);
	if ( old_xar == NULL ) {
		fprintf(stderr, "Could not open archive %s\n", filename);
		exit(1);
	}

	if (!Toccksum || strcmp(Toccksum->name, XAR_OPT_VAL_NONE) == 0) {
		hash_name = xar_attr_get((xar_file_t)old_xar, "checksum", "style");
		if (!hash_name || strcmp(hash_name, XAR_OPT_VAL_NONE) == 0) {
			fprintf(stderr, "A TOC checksum style value other than \"%s\" is required for signatures\n", XAR_OPT_VAL_NONE);
			exit(1);
		}
	}
	else
		hash_name = Toccksum->name;

	if (DumpDigestInfo) {
		const struct HashType *hash = NULL;
		if (hash_name)
			hash = get_hash_alg(hash_name);
		if (!hash_name || !hash) {
			fprintf(stderr, "--digestinfo-to-sign does not support hash type \"%s\"\n", hash_name ? hash_name : XAR_OPT_VAL_NONE);
			exit(1);
		}
		Toccksum = hash;
	}

	/* create the temporary archive file */
	temp_dir = getenv("TMPDIR");
	if (!temp_dir || !is_valid_dir(temp_dir)) {
		temp_dir = getenv("TMP");
		if (!temp_dir || !is_valid_dir(temp_dir)) {
			temp_dir = "/tmp";
			if (!is_valid_dir(temp_dir))
				temp_dir = NULL;
		}
	}
	if (!temp_dir) {
		fprintf(stderr, "No temporary directory available (none of $TMPDIR $TMP or /tmp are directories)\n");
		exit(1);
	}
	new_xar_path_len = strlen(temp_dir) + 12;
	new_xar_path = (char *)malloc(new_xar_path_len);
	if (!new_xar_path) {
		fprintf(stderr, "Could not allocate memory for temporary file path\n");
		exit(1);
	}
	strncpy(new_xar_path, temp_dir, new_xar_path_len);
	strncpy(new_xar_path + (new_xar_path_len - 12), "/xar-XXXXXX", 12);
	tempfd = mkstemp(new_xar_path);
	if( tempfd == -1 ) {
		free(new_xar_path);
		fprintf(stderr, "Error creating new archive %s\n", new_xar_path);
		exit(1);
	}
	fchmod(tempfd, 0666 & ~get_umask());
	close(tempfd);
	unlink_temp_file = new_xar_path;
	atexit(remove_temp);

	new_xar = xar_open(new_xar_path, WRITE);
	if( !new_xar ) {
		fprintf(stderr, "Error creating new archive %s\n", new_xar_path);
		exit(1);
	}

	if (SigSize > 0) {
		/* install new signature and new certs in new_xar */
		xar_signature_t sig;
		sig = xar_signature_new(new_xar, "RSA", (int32_t)SigSize, &signingCallback, NULL);
		for( c = CertPath; c; c=c->next ) {
			insert_cert(sig, c->cert_path);
		}
	}

	for (i=0; i<(int)(sizeof(opts)/sizeof(opts[0])); i++) {
		opt = xar_opt_get(old_xar, opts[i]);
		if (opt)
			xar_opt_set(new_xar, opts[i], opt);
	}
	if (xar_opt_set(new_xar, XAR_OPT_TOCCKSUM, hash_name) != 0) {
		fprintf(stderr, "Unsupported TOC checksum type %s\n", hash_name);
		exit(1);
	}

	/* skip copy subdocs for now since we don't use them yet */

	/* copy files */
	iter = xar_iter_new();
	f = xar_file_first(old_xar, iter);
	/* xar_file_next iterates the archive depth-first, i.e. all children are enumerated before the siblings. */
	s_new = stack_new();
	s_old = stack_new();
	last_copied = last_added = NULL;
	loopIter = xar_iter_new();
	for (current_xar_file = xar_file_first(old_xar, loopIter); current_xar_file; current_xar_file = xar_file_next(loopIter))
	{
		if (Verbose)
			printf("old_xar -> %s (parent: %s)\n",xar_get_path(current_xar_file),XAR_FILE(current_xar_file)->parent?xar_get_path(XAR_FILE(current_xar_file)->parent):"(nil)");
	}
	xar_iter_free(loopIter);

	do {
		/* parent is the parent in the new archive! */
		/* 3 cases:
		 *  1. the file has no parent. Happens for every file at the top level of the archive.
		 *  2. the file's parent is the last file we added. Happens while descending down a path
		 *  3. the file's parent is one of the ancestors of the last file (and not NULL, that would be case 1)
		 *		that means we either go back up the tree and add a sibling of one of the ancestors, or we add a
		 *		sibling on the same level
		 */
		xar_prop_get(f, "name", &name);	/* filename, without any path info */
		if (!XAR_FILE(f)->parent) {	/* case 1 */
			if (Verbose)
				printf("root: %s\n",xar_get_path(f));
			last_added = xar_add_from_archive(new_xar, NULL, name, old_xar, f);
			last_copied = f;
			stack_push(s_new, (void *)last_added);
			stack_push(s_old, (void *)last_copied);
		} else if (f->parent == last_copied) {	/* case 2 */
			if (Verbose)
				printf("child: %s -> %s\n",xar_get_path(f->parent),xar_get_path(f));
			last_added = xar_add_from_archive(new_xar, last_added, name, old_xar, f);
			last_copied = f;
			stack_push(s_new, (void *)last_added);
			stack_push(s_old, (void *)last_copied);
		} else {	/* case 3 */
			if (Verbose)
				printf("searching for parent: %s ?\n",xar_get_path(f));
			while (XAR_FILE(f)->parent != XAR_FILE(s_old->top->data)->parent) {
				if (Verbose)
					printf("popping: %s\n",xar_get_path(XAR_FILE(s_old->top->data)));
				stack_pop(s_new);
				stack_pop(s_old);
			}
			if (Verbose)
				printf("found: %s -> %s\n",xar_get_path(XAR_FILE(s_new->top->data)),xar_get_path(f));
			stack_pop(s_new);
			stack_pop(s_old);
			last_added = xar_add_from_archive(new_xar, (xar_file_t)(s_new->top->data), name, old_xar, f);
			last_copied = f;
			stack_push(s_new, (void *)last_added);
			stack_push(s_old, (void *)last_copied);
		}
	} while ((f = xar_file_next(iter)));

	loopIter = xar_iter_new();
	for (current_xar_file = xar_file_first(new_xar, loopIter); current_xar_file; current_xar_file = xar_file_next(loopIter))
	{
		if (Verbose) {
			char * current_path = xar_get_path(current_xar_file);
			printf("new_xar -> %s\n",current_path);
		}
	}
	xar_iter_free(loopIter);

	xar_iter_free(iter);
	stack_free(s_new);
	stack_free(s_old);
	if( xar_close(new_xar) != 0 ) {
		fprintf(stderr, "Error creating the archive\n");
		if( !Err )
			Err = 42;
	}
	xar_close(old_xar);

	/* write signature offset to file (have to re-open so xar_close can figure out the correct offset) */
	new_xar = xar_open(new_xar_path, READ);
	if( !new_xar ) {
		fprintf(stderr, "Error re-opening new archive %s\n", new_xar_path);
		exit(1);
	}
	if( SigOffsetDumpPath )
		extract_sig_offset(new_xar, SigOffsetDumpPath);
	xar_close(new_xar);

	/* delete old archive, move new in its place */
	unlink(filename);
	err = asprintf(&systemcall, "cp \"%s\" \"%s\"", new_xar_path, filename);
	if (err == -1) {
		fprintf(stderr, "Could not copy new archive to final location (asprintf() error %i)\n", errno);
		exit(1);
	}
	err = system(systemcall);
	if (err) {
		fprintf(stderr, "Could not copy new archive to final location (system() returned %i)\n", err);
		exit(1);
	}
	free(systemcall);
	/* delete temporary archive */
	remove_temp();
}

/*	belated_sign
	Prepare a previously unsigned archive for signing by creating a signature placeholder and inserting the certificates.
	Since libxar is currently not capable of doing this directly, we have to create a new xar archive,
	copy all the files and options from the current archive, and sign the new archive
*/
static void belated_sign(const char *filename) {
	xar_signature_t sig;
	xar_t x = xar_open(filename, READ);
	if ( x == NULL ) {
		fprintf(stderr, "Could not open archive %s\n", filename);
		exit(1);
	}
	sig = xar_signature_first(x);
	if ( sig ) {
		fprintf(stderr, "Archive has already been signed. Use --replace-sign instead\n");
		exit(E_SIGEXISTS);
	}
	xar_close(x);
	replace_sign(filename);
}

static int32_t signingCallback(xar_signature_t sig, void *context, uint8_t *data, uint32_t length, uint8_t **signed_data, uint32_t *signed_len) {

	/* save data to file for later signature */
	(void)sig; (void)context;
	if (DataToSignDumpPath) {
		FILE *file = fopen(DataToSignDumpPath, "wb");
		int i;
		if (!file) {
			fprintf(stderr, "Could not open %s for saving data to sign\n", DataToSignDumpPath);
			exit(1);
		}
		if (DumpDigestInfo) {
			i = (int)fwrite(Toccksum->diprefix, Toccksum->diprefixlen, 1, file);
			if (i != 1) {
				fprintf(stderr, "Failed to write DigestInfo data to sign prefix to %s (fwrite() returned %i)\n", DataToSignDumpPath, i);
				exit(1);
			}
		}
		i = (int)fwrite(data, length, 1, file);
		if (i != 1) {
			fprintf(stderr, "Failed to write %sdata to sign to %s (fwrite() returned %i)\n", DumpDigestInfo?"DigestInfo ":"", DataToSignDumpPath, i);
			exit(1);
		}
		fclose(file);
	}

	/* now return blank placeholder data */
	*signed_data = (uint8_t *)malloc(SigSize);
	memset(*signed_data, 0, SigSize);
	strncpy((char *)*signed_data, "helloworld", 10);	/* debug */
	*signed_len = (uint32_t)SigSize;
	return 0;	/* no error */
}

static void insert_cert(xar_signature_t sig, const char *cert_path) {
	struct stat *s = malloc(sizeof(struct stat));
	void *cert;
	FILE *file;
	int i;
	if (stat(cert_path, s) == -1) {
		fprintf(stderr, "Could not stat() certificate file %s (errno == %i)\n", cert_path, errno);
		exit(1);
	}
	cert = malloc((size_t)s->st_size);
	file = fopen(cert_path, "rb");
	if (!file) {
		fprintf(stderr, "Could not open %s for reading certificate\n", cert_path);
		exit(1);
	}
	i = (int)fread(cert, (size_t)s->st_size, 1, file);
	if (i != 1) {
		fprintf(stderr, "Failed to read certificate from %s\n", cert_path);
		exit(1);
	}
	fclose(file);
	xar_signature_add_x509certificate(sig, cert, (uint32_t)s->st_size);
	free(s);
	free(cert);
}

static void inject_signature(const char *xar_path, const char *sig_path) {
	/* since there is no API to insert a signature other than during signingCallback time, we have to */
	/* inject it by editing the raw file */
	int buffer_size = 1024;
	void *buffer = malloc(buffer_size);
	FILE *sig, *xar;
	uint64_t signedDataOffset;
        uint32_t signedDataLength, sig_data_len;
	xar_t x;
	int i;

	if (Verbose)
		printf("inject_signature(%s, %s)\n",xar_path,sig_path);

	/* open xar via the API first to determine the signature offset */
	x = xar_open(xar_path, READ);
	if ( x == NULL ) {
		fprintf(stderr, "Could not open xar archive %s to get signature offset\n", xar_path);
		exit(1);
	}
	if (get_sig_info(x, &signedDataOffset, &signedDataLength, NULL) != 0) {
		fprintf(stderr, "Could not read xar archive %s to get signature offset\n", xar_path);
		exit(1);
	}
	xar_close(x);

	/* then re-open xar and signature files raw... */
	sig = fopen(sig_path, "rb");
	if (!sig) {
		fprintf(stderr, "Could not open %s for reading signature\n", sig_path);
		exit(1);
	}
	if (fseek(sig, 0, SEEK_END) == -1) {
		fprintf(stderr, "Could not get length of %s\n", sig_path);
		exit(1);
	}
	sig_data_len = (uint32_t)ftell(sig);
	if (fseek(sig, 0, SEEK_SET) == -1) {
		fprintf(stderr, "Could not rewind %s\n", sig_path);
		exit(1);
	}
	if (!signedDataLength || !sig_data_len || signedDataLength != sig_data_len) {
		fprintf(stderr, "Bad signature length\n");
		exit(1);
	}

	xar = fopen(xar_path, "r+b");
	if (!xar) {
		fprintf(stderr, "Could not open xar archive %s for injecting signature\n", xar_path);
		exit(1);
	}
	/* ...and inject the signature */
	fseek(xar, (long)signedDataOffset, SEEK_SET);
	do {
		i = (int)fread(buffer, 1, buffer_size, sig);
		if (ferror(sig)) {
			fprintf(stderr, "Failed to read signature from %s\n", sig_path);
			exit(1);
		}
		fwrite(buffer, 1, i, xar);
	} while (!feof(sig));
	fclose(sig);
	fclose(xar);

	free(buffer);
}

static int archive(const char *filename, int arglen, char *args[]) {
	xar_t x;
	FTS *fts;
	FTSENT *ent;
	int flags;
	struct lnode *i;
	const char *default_compression;
	int curdir = open(".", O_RDONLY);

	(void)arglen;
	x = xar_open(filename, WRITE);
	if( !x ) {
		fprintf(stderr, "Error creating archive %s\n", filename);
		exit(1);
	}

	if ( SigSize ) {
		struct cnode *c;
		xar_signature_t sig = xar_signature_new(x, "RSA", (int32_t)SigSize, &signingCallback, NULL);
		for( c = CertPath; c; c=c->next ) {
			insert_cert(sig, c->cert_path);
		}
	}

	if( Toccksum )
		if (xar_opt_set(x, XAR_OPT_TOCCKSUM, Toccksum->name) != 0) {
			fprintf(stderr, "Unsupported TOC checksum type %s\n", Toccksum->name);
			exit(1);
		}

	if( Filecksum )
		if (xar_opt_set(x, XAR_OPT_FILECKSUM, Filecksum->name) != 0) {
			fprintf(stderr, "Unsupported file checksum type %s\n", Filecksum->name);
			exit(1);
		}

	if( Compression )
		xar_opt_set(x, XAR_OPT_COMPRESSION, Compression);

	if( CompressionArg )
		xar_opt_set(x, XAR_OPT_COMPRESSIONARG, CompressionArg);

	if( Coalesce )
		xar_opt_set(x, XAR_OPT_COALESCE, "true");

	if( LinkSame )
		xar_opt_set(x, XAR_OPT_LINKSAME, "true");

	if ( Rsize != NULL )
		xar_opt_set(x, XAR_OPT_RSIZE, Rsize);

	if( Recompress )
		xar_opt_set(x, XAR_OPT_RECOMPRESS, XAR_OPT_VAL_TRUE);

	if( RFC6713 )
		xar_opt_set(x, XAR_OPT_RFC6713FORMAT, XAR_OPT_VAL_TRUE);

	xar_register_errhandler(x, err_callback, NULL);

	for( i = PropInclude; i; i=i->next ) {
		xar_opt_set(x, XAR_OPT_PROPINCLUDE, i->str);
	}
	for( i = PropExclude; i; i=i->next ) {
		xar_opt_set(x, XAR_OPT_PROPEXCLUDE, i->str);
	}

	if( Subdoc )
		add_subdoc(x);

	if( Perms == SYMBOLIC ) {
		xar_opt_set(x, XAR_OPT_OWNERSHIP, XAR_OPT_VAL_SYMBOLIC);
	}
	if( Perms == NUMERIC ) {
		xar_opt_set(x, XAR_OPT_OWNERSHIP, XAR_OPT_VAL_NUMERIC);
	}

	default_compression = strdup(xar_opt_get(x, XAR_OPT_COMPRESSION));
	if( !default_compression )
		default_compression = strdup(XAR_OPT_VAL_GZIP);

	flags = FTS_PHYSICAL|FTS_NOSTAT|FTS_NOCHDIR;
	if( Local )
		flags |= FTS_XDEV;
	if(Chdir) {
		if (curdir < 0) {
			fprintf(stderr, "Unable to get current directory\n");
			exit(1);
		}
		if( chdir(Chdir) != 0 ) {
			fprintf(stderr, "Unable to chdir to %s\n", Chdir);
			exit(1);
		}
	}
	fts = fts_open(args, flags, NULL);
	if( !fts ) {
		fprintf(stderr, "Error traversing file tree\n");
		exit(1);
	}

	while( (ent = fts_read(fts)) ) {
		xar_file_t f;
		int exclude_match = 1;
		int nocompress_match = 1;
		if( ent->fts_info == FTS_DP )
			continue;

		if( strcmp(ent->fts_path, "/") == 0 )
			continue;
		if( strcmp(ent->fts_path, ".") == 0 )
			continue;
		
		for( i = Exclude; i; i=i->next ) {
			exclude_match = regexec(&i->reg, ent->fts_path, 0, NULL, 0);
			if( !exclude_match )
				break;
		}
		if( !exclude_match ) {
			if( Verbose )
				printf("Excluding %s\n", ent->fts_path);
			continue;
		}

		for( i = NoCompress; i; i=i->next ) {
			nocompress_match = regexec(&i->reg, ent->fts_path, 0, NULL, 0);
			if( !nocompress_match ) {
				xar_opt_set(x, XAR_OPT_COMPRESSION, XAR_OPT_VAL_NONE);
				break;
			}
		}
		f = xar_add(x, ent->fts_path);
		if( !f ) {
			fprintf(stderr, "Error adding file %s\n", ent->fts_path);
		} else {
			print_file(x, f, stdout);
		}
		if( !nocompress_match )
			xar_opt_set(x, XAR_OPT_COMPRESSION, default_compression);
	}
	fts_close(fts);
	if(Chdir) {
		int err;
		err = fchdir(curdir);
	}
	if( xar_close(x) != 0 ) {
		fprintf(stderr, "Error creating the archive\n");
		if( !Err )
			Err = 42;
	}

	free((char *)default_compression);
	for( i = Exclude; i; ) {
		struct lnode *tmp;
		regfree(&i->reg);
		tmp = i;
		i = i->next;
		free(tmp);
	}
	for( i = NoCompress; i; ) {
		struct lnode *tmp;
		regfree(&i->reg);
		tmp = i;
		i = i->next;
		free(tmp);
	}

	if ( SigOffsetDumpPath ) {
		x = xar_open(filename, READ);
		if( !x ) {
			fprintf(stderr, "Error re-opening archive %s\n", filename);
			exit(1);
		}
		extract_sig_offset(x, SigOffsetDumpPath);
		if( xar_close(x) != 0 ) {
			fprintf(stderr, "Error re-closing the archive\n");
			if( !Err )
				Err = 42;
		}
	}

	return Err;
}

static int extract(const char *filename, int arglen, char *args[]) {
	xar_t x;
	xar_iter_t i;
	xar_file_t f;
	int files_extracted = 0;
	int argi;
	struct lnode *extract_files = NULL;
	struct lnode *extract_tail = NULL;
	struct lnode *lnodei = NULL;
	struct lnode *dirs = NULL;

	(void)arglen;
	for(argi = 0; args[argi]; argi++) {
		struct lnode *tmp;
		int err;
		tmp = malloc(sizeof(struct lnode));
		tmp->str = strdup(args[argi]);
		tmp->next = NULL;
		err = regcomp(&tmp->reg, tmp->str, REG_NOSUB);
		if( err ) {
			char errstr[1024];
			regerror(err, &tmp->reg, errstr, sizeof(errstr));
			fprintf(stderr, "Error with regular expression %s: %s\n", tmp->str, errstr);
			exit(1);
		}
		if( extract_files == NULL ) {
			extract_files = tmp;
			extract_tail = tmp;
		} else {
			extract_tail->next = tmp;
			extract_tail = tmp;
		}
		
		/* Add a clause for recursive extraction */
		tmp = malloc(sizeof(struct lnode));
		if (asprintf(&tmp->str, "%s/.*", args[argi]) == -1) {
			fprintf(stderr, "Error with asprintf()\n");
			exit(1);
		}
		tmp->next = NULL;
		err = regcomp(&tmp->reg, tmp->str, REG_NOSUB);
		if( err ) {
			char errstr[1024];
			regerror(err, &tmp->reg, errstr, sizeof(errstr));
			fprintf(stderr, "Error with regular expression %s: %s\n", tmp->str, errstr);
			exit(1);
		}
		if( extract_files == NULL ) {
			extract_files = tmp;
			extract_tail = tmp;
		} else {
			extract_tail->next = tmp;
			extract_tail = tmp;
		}
	}

	x = xar_open(filename, READ);
	if( !x ) {
		fprintf(stderr, "Error opening xar archive: %s\n", filename);
		exit(1);
	}

	if(Chdir) {
		if( chdir(Chdir) != 0 ) {
			fprintf(stderr, "Unable to chdir to %s\n", Chdir);
			exit(1);
		}
	}

	xar_register_errhandler(x, err_callback, NULL);

	if( Perms == SYMBOLIC ) {
		xar_opt_set(x, XAR_OPT_OWNERSHIP, XAR_OPT_VAL_SYMBOLIC);
	}
	if( Perms == NUMERIC ) {
		xar_opt_set(x, XAR_OPT_OWNERSHIP, XAR_OPT_VAL_NUMERIC);
	}
	if ( Rsize != NULL ) {
		xar_opt_set(x, XAR_OPT_RSIZE, Rsize);
	}
	if( SaveSuid ) {
		xar_opt_set(x, XAR_OPT_SAVESUID, XAR_OPT_VAL_TRUE);
	}
	if (StripComponents) {
		xar_opt_set(x, XAR_OPT_STRIPCOMPONENTS, StripComponents);
	}
	if (ToStdout) {
		xar_opt_set(x, XAR_OPT_EXTRACTSTDOUT, XAR_OPT_VAL_TRUE);
	}

	i = xar_iter_new();
	if( !i ) {
		fprintf(stderr, "Error creating xar iterator\n");
		exit(1);
	}

	for(f = xar_file_first(x, i); f; f = xar_file_next(i)) {
		int matched = 0;
		int exclude_match = 1;
		struct lnode *i;

		char *path = xar_get_path(f);

		if( args[0] ) {
			for(i = extract_files; i != NULL; i = i->next) {
				int extract_match = 1;

				extract_match = regexec(&i->reg, path, 0, NULL, 0);
				if( !extract_match ) {
					matched = 1;
					break;
				}
			}
		} else {
			matched = 1;
		}

		for( i = Exclude; i; i=i->next ) {
			exclude_match = regexec(&i->reg, path, 0, NULL, 0);
			if( !exclude_match )
				break;
		}
		if( !exclude_match ) {
			if( Verbose )
				printf("Excluding %s\n", path);
			free(path);
			continue;
		}
		
		if( matched ) {
			struct stat sb;
			if( !ToStdout && NoOverwrite && (lstat(path, &sb) == 0) ) {
				fprintf(stderr, "%s already exists, not overwriting\n", path);
			} else {
				const char *prop = NULL;
				int deferred = 0;
				if( xar_prop_get(f, "type", &prop) == 0 ) {
					if( strcmp(prop, "directory") == 0 ) {
						if (!ToStdout) {
							struct lnode *tmpl = calloc(sizeof(struct lnode),1);
							tmpl->str = (char *)f;
							tmpl->next = dirs;
							dirs = tmpl;
						}
						deferred = 1;
					}
				}
				if( ! deferred ) {
					print_file(x, f, stdout);
					if (xar_extract(x, f) == 0)
						files_extracted++;
					else if (!ToStdout)
						fprintf(stderr, "Unable to extract file %s\n", path);
				}
			}
		}
		free(path);
	}
	for(lnodei = dirs; lnodei; lnodei = lnodei->next) {
		files_extracted++;
		print_file(x,(xar_file_t)lnodei->str, stdout);
		xar_extract(x, (xar_file_t)lnodei->str);
	}
	if( args[0] && (files_extracted == 0) ) {
		fprintf(stderr, "No files matched extraction criteria\n");
		Err = 3;
	}

	if( Subdoc )
		extract_subdoc(x, NULL);

	xar_iter_free(i);
	if( xar_close(x) != 0 ) {
		fprintf(stderr, "Error extracting the archive\n");
		if( !Err )
			Err = 42;
	}

	for(lnodei = extract_files; lnodei != NULL; ) {
		struct lnode *tmp;
		free(lnodei->str);
		regfree(&lnodei->reg);
		tmp = lnodei;
		lnodei = lnodei->next;
		free(tmp);
	}
	return Err;
}

static int list_subdocs(const char *filename) {
	xar_t x;
	xar_subdoc_t s;

	x = xar_open(filename, READ);
	if( !x ) {
		fprintf(stderr, "Error opening xar archive: %s\n", filename);
		exit(1);
	}

	for(s = xar_subdoc_first(x); s; s = xar_subdoc_next(s)) {
		printf("%s\n", xar_subdoc_name(s));
	}
	xar_close(x);

	return Err;
}

static int list(const char *filename, int arglen, char *args[]) {
	xar_t x;
	xar_iter_t i;
	xar_file_t f;
	int argi = 0;
	struct lnode *list_files = NULL;
	struct lnode *list_tail = NULL;
	struct lnode *lnodei = NULL;

	(void)arglen;
	for(argi = 0; args[argi]; argi++) {
		struct lnode *tmp;
		int err;
		tmp = malloc(sizeof(struct lnode));
		tmp->str = strdup(args[argi]);
		tmp->next = NULL;
		err = regcomp(&tmp->reg, tmp->str, REG_NOSUB);
		if( err ) {
			char errstr[1024];
			regerror(err, &tmp->reg, errstr, sizeof(errstr));
			fprintf(stderr, "Error with regular expression %s: %s\n", tmp->str, errstr);
			exit(1);
		}
		if( list_files == NULL ) {
			list_files = tmp;
			list_tail = tmp;
		} else {
			list_tail->next = tmp;
			list_tail = tmp;
		}
	}

	x = xar_open(filename, READ);
	if( !x ) {
		fprintf(stderr, "Error opening xar archive: %s\n", filename);
		exit(1);
	}

	i = xar_iter_new();
	if( !i ) {
		fprintf(stderr, "Error creating xar iterator\n");
		exit(1);
	}

	for(f = xar_file_first(x, i); f; f = xar_file_next(i)) {
		int matched = 0;

		if( args[0] ) {
			char *path = xar_get_path(f);
			for(lnodei = list_files; lnodei != NULL; lnodei = lnodei->next) {
				int list_match = 1;

				list_match = regexec(&lnodei->reg, path, 0, NULL, 0);
				if( !list_match ) {
					matched = 1;
					break;
				}
			}
			free(path);
		} else {
			matched = 1;
		}

		if( matched )
			print_file(x, f, stdout);
	}

	xar_iter_free(i);
	xar_close(x);

	for(lnodei = list_files; lnodei != NULL; ) {
		struct lnode *tmp;
		free(lnodei->str);
		regfree(&lnodei->reg);
		tmp = lnodei;
		lnodei = lnodei->next;
		free(tmp);
	}

	return Err;
}

static int dumptoc(const char *filename, const char* tocfile) {
	xar_t x;
	x = xar_open(filename, READ);
	if( !x ) {
		fprintf(stderr, "Error opening xar archive: %s\n", filename);
		exit(1);
	}

	xar_serialize(x, tocfile);
	xar_close(x);
	return Err;
}

static int dump_header(const char *filename) {
	int fd;
	xar_header_ex_t xh;

	if(filename == NULL)
		fd = 0;
	else {
		fd = open(filename, O_RDONLY);
		if( fd < 0 ) {
			perror("open");
			exit(1);
		}
	}

	if( read(fd, &xh, sizeof(xh)) < (int)sizeof(xar_header_t) ) {
		fprintf(stderr, "error reading header\n");
		exit(1);
	}

	printf("magic:                   0x%x ", ntohl(xh.magic));
	if( ntohl(xh.magic) != XAR_HEADER_MAGIC )
		printf("(BAD)\n");
	else
		printf("(OK)\n");
	printf("size:                    %d\n", ntohs(xh.size));
	printf("version:                 %d\n", ntohs(xh.version));
	printf("Compressed TOC length:   %" PRId64 "\n", xar_ntoh64(xh.toc_length_compressed));
	printf("Uncompressed TOC length: %" PRId64 "\n", xar_ntoh64(xh.toc_length_uncompressed));
	printf("TOC Checksum algorithm:  %d ", ntohl(xh.cksum_alg));
	switch( ntohl(xh.cksum_alg) ) {
	case XAR_CKSUM_NONE:	printf("(%s)\n", XAR_OPT_VAL_NONE);
				break;
	case XAR_CKSUM_MD5:	printf("(%s)\n", XAR_OPT_VAL_MD5);
				break;
	case XAR_CKSUM_SHA1:	printf("(%s)\n", XAR_OPT_VAL_SHA1);
				break;
	case XAR_CKSUM_OTHER:
	{
				uint16_t hsiz = ntohs(xh.size);
				if (hsiz < sizeof(xar_header_t) + 4 || (hsiz & 0x3) != 0) {
					printf("(OTHER + invalid header length)\n");
					break;
				}
				if (hsiz > sizeof(xar_header_ex_t))
					hsiz = sizeof(xar_header_ex_t);
				if (!memchr(xh.toc_cksum_name, 0, hsiz - sizeof(xar_header_t))) {
					printf("(OTHER + invalid non-nul terminated name)\n");
					break;
				}
				if (!xh.toc_cksum_name[0]) {
					printf("(OTHER + invalid empty name)\n");
					break;
				}
				printf("(OTHER + %s)\n", xh.toc_cksum_name);
				break;
	}
	default: printf("(unknown)\n");
	         break;
	};

	return 0;
}

static int dumptoc_raw(const char *filename, const char* tocfile) {
	int fd, toc;
	xar_header_ex_t xh;
	uint64_t clen;
	uint16_t hlen;
	unsigned buffer_size = 4096;
	void *buffer = malloc(buffer_size);

	if (!buffer)
		return -1;
	if(filename == NULL)
		fd = 0;
	else {
		fd = open(filename, O_RDONLY);
		if( fd < 0 ) {
			perror("open");
			exit(1);
		}
	}

	if( read(fd, &xh, sizeof(xh)) < (int)sizeof(xar_header_t) ) {
		fprintf(stderr, "error reading header\n");
		exit(1);
	}

	if( ntohl(xh.magic) != XAR_HEADER_MAGIC ) {
		fprintf(stderr, "error reading header (bad magic number)\n");
		exit(1);
	}

	hlen = ntohs(xh.size);
	clen = xar_ntoh64(xh.toc_length_compressed);

	if( hlen < sizeof(xar_header_t) ) {
		fprintf(stderr, "error reading header (header size field value too small)\n");
		exit(1);
	}
	if( hlen > sizeof(xh) && (hlen - sizeof(xh) > buffer_size) ) {
		fprintf(stderr, "error reading header (bad header size field -- greater than %u)\n", buffer_size + (unsigned)sizeof(xh));
		exit(1);
	}
	if( hlen > sizeof(xh) &&
		read(fd, buffer, hlen - sizeof(xh)) < (int)(hlen - sizeof(xh)) ) {
		fprintf(stderr, "error reading header (premature EOF)\n");
		exit(1);
	}

	toc = open(tocfile, O_WRONLY|O_CREAT|O_TRUNC, 0644);
	if( toc < 0 ) {
		perror("open dump file for writing");
		exit(1);
	}

	while (clen) {
		ssize_t bytes;
		if (buffer_size > clen)
			buffer_size = (unsigned)clen;
		bytes = read(fd, buffer, buffer_size);
		if (bytes != (ssize_t)buffer_size) {
			fprintf(stderr, "error reading toc (premature EOF)\n");
			exit(1);
		}
		bytes = write(toc, buffer, buffer_size);
		if (bytes != (ssize_t)buffer_size) {
			fprintf(stderr, "error writing raw toc dump file\n");
			exit(1);
		}
		clen -= (uint64_t)bytes;
	}

	close(fd);
	close(toc);
	free(buffer);
	return 0;
}

static int32_t err_callback(int32_t sev, int32_t err, xar_errctx_t ctx, void *usrctx) {
	xar_file_t f;
	xar_t x;
	const char *str;
	int e;

	(void)usrctx;
	x = xar_err_get_archive(ctx);
	f = xar_err_get_file(ctx);
	str = xar_err_get_string(ctx);
	e = xar_err_get_errno(ctx);

	switch(sev) {
	case XAR_SEVERITY_DEBUG:
	case XAR_SEVERITY_INFO:
		break;
	case XAR_SEVERITY_WARNING:
		fprintf(stderr, "%s\n", str);
		break;
	case XAR_SEVERITY_NORMAL:
		if( (err = XAR_ERR_ARCHIVE_CREATION) && f )
			print_file(x, f, stderr);
		break;
	case XAR_SEVERITY_NONFATAL:
	case XAR_SEVERITY_FATAL:
		Err = 2;
		fprintf(stderr, "Error while ");
		if( err == XAR_ERR_ARCHIVE_CREATION ) fprintf(stderr, "creating");
		if( err == XAR_ERR_ARCHIVE_EXTRACTION ) fprintf(stderr, "extracting");
		fprintf(stderr, " archive");
		if( f ) {
			const char *file = xar_get_path(f);
			if( file ) fprintf(stderr, ":(%s)", file);
			free((char *)file);
		}
		if( str ) fprintf(stderr, ": %s", str);
		if( err ) fprintf(stderr, " (%s)", strerror(e));
		if( sev == XAR_SEVERITY_NONFATAL ) {
			fprintf(stderr, " - ignored");
			fprintf(stderr, "\n");
		} else {
			fprintf(stderr, "\n");
			exit(1);
		}
		break;
	}
	return 0;
}

static void _usagehint(const char *prog, FILE *helpout) {
	fprintf(helpout, "Usage: %s -[ctx][v] -f <archive> ...\n", prog);
	fprintf(helpout, "(Use %s --help for extended help)\n", prog);
}

static void _usage(const char *prog, FILE *helpout) {
	fprintf(helpout, "Usage: %s -[ctx][v] -f <archive> ...\n", prog);
	fprintf(helpout, "\t-c               Creates an archive\n");
	fprintf(helpout, "\t--create         Synonym for \"-c\"\n");
	fprintf(helpout, "\t-x               Extracts an archive\n");
	fprintf(helpout, "\t--extract        Synonym for \"-x\"\n");
	fprintf(helpout, "\t-t               Lists an archive\n");
	fprintf(helpout, "\t--list           Synonym for \"-t\"\n");
	fprintf(helpout, "\t--sign           Creates a placeholder signature and saves\n");
	fprintf(helpout, "\t                 the data to sign to disk. Works with -c or -f, requires\n");
	fprintf(helpout, "\t                 --sig-size and one or more --cert-loc to be set.\n");
	fprintf(helpout, "\t                 Setting --data-to-sign and --sig-offset is optional.\n");
	fprintf(helpout, "\t                 Fails with error code %i if the archive has already\n", E_SIGEXISTS);
	fprintf(helpout, "\t                 been signed.\n");
	fprintf(helpout, "\t--replace-sign   Rips out existing signature(s) and makes a new one.\n");
	fprintf(helpout, "\t                 Same required parameter set as --sign, \n");
	fprintf(helpout, "\t                 but -f instead of -c.\n");
	fprintf(helpout, "\t--extract-data-to-sign Extracts data to be signed from an\n");
	fprintf(helpout, "\t                 existing archive. Requires --data-to-sign (and -f)\n");
	fprintf(helpout, "\t                 to be set.  Setting --sig-offset is optional.\n");
	fprintf(helpout, "\t--extract-certs <dir> Extracts all certificates in DER (binary) format\n");
	fprintf(helpout, "\t                 into the specified pre-existing directory, naming them\n");
	fprintf(helpout, "\t                  'cert00', 'cert01', 'cert02' etc. where 'cert00' is\n");
	fprintf(helpout, "\t                 the leaf/signing cert.  Requires -f.\n");
	fprintf(helpout, "\t--extract-CAfile <filename> Extracts all certificates in PEM format,\n");
	fprintf(helpout, "\t                 concatenates them together and stores the result in the\n");
	fprintf(helpout, "\t                 specified file.  Requires -f.\n");
	fprintf(helpout, "\t--extract-sig <filename> Extracts the signature data and stores it into\n");
	fprintf(helpout, "\t                 the specified file.  Requires -f.  Setting --sig-offset\n");
	fprintf(helpout, "\t                 Setting --sig-offset is optional.\n");
	fprintf(helpout, "\t--inject-sig <filename> After extracting the data to be signed and\n");
	fprintf(helpout, "\t                 doing the signing externally, injects the\n");
	fprintf(helpout, "\t                 signature. Requires -f.\n");
	fprintf(helpout, "\t-f <filename>    Specifies an archive to operate on [REQUIRED!]\n");
	fprintf(helpout, "\t--file=<filename> Synonym for \"-f <filename>\"\n");
	fprintf(helpout, "\t-v               Print filenames as they are archived\n");
	fprintf(helpout, "\t--verbose        Synonym for \"-v\"\n");
	fprintf(helpout, "\t-C <path>        Change directory to this location before doing anything\n");
	fprintf(helpout, "\t--directory=<path> Synonym for \"-C <path>\"\n");
	fprintf(helpout, "\t-n name          Provides a name for a subdocument\n");
	fprintf(helpout, "\t-s <filename>    On extract, specifies the file to extract\n");
	fprintf(helpout, "\t                      subdocuments to.\n");
	fprintf(helpout, "\t                 On archival, specifies an xml file to add\n");
	fprintf(helpout, "\t                      as a subdocument.\n");
	fprintf(helpout, "\t-l               On archival, stay on the local device.\n");
	fprintf(helpout, "\t--one-file-system Synonym for \"-l\"\n");
	fprintf(helpout, "\t-p               On extract, set ownership based on symbolic\n");
	fprintf(helpout, "\t                      names, if possible.\n");
	fprintf(helpout, "\t-P               On extract, set ownership based on uid/gid.\n");
	fprintf(helpout, "\t--toc-cksum      Specifies the hashing algorithm to use for\n");
	fprintf(helpout, "\t                      xml header verification.\n");
	fprintf(helpout, "\t                      Valid values: none, sha1, and md5\n");
	fprintf(helpout, "\t                      Default: sha1\n");
	fprintf(helpout, "\t                      If the linked library supports them, sha224\n");
	fprintf(helpout, "\t                      sha256, sha384 and sha512 may also be used.\n");
	fprintf(helpout, "\t                      Setting a stronger toc hash than the default will\n");
	fprintf(helpout, "\t                      also set the file hash to the same value if it's\n");
	fprintf(helpout, "\t                      not been explictly set to something else.\n");
	fprintf(helpout, "\t--file-cksum     Specifies the hashing algorithm to use for\n");
	fprintf(helpout, "\t                      file verification.\n");
	fprintf(helpout, "\t                      Same values and defaults as --toc-cksum.\n");
	fprintf(helpout, "\t                      Setting a stronger file hash than the default will\n");
	fprintf(helpout, "\t                      also set the toc hash to the same value if it's\n");
	fprintf(helpout, "\t                      not been explictly set to something else.\n");
	fprintf(helpout, "\t--dump-toc=<filename> Has xar dump the xml header into the\n");
	fprintf(helpout, "\t                      specified file.\n");
	fprintf(helpout, "\t-d <filename>    Synonym for \"--dump-toc=<filename>\"\n");
	fprintf(helpout, "\t--dump-toc-raw=<filename> Has xar dump the raw, compressed xml\n");
	fprintf(helpout, "\t                      header data into the specified file.\n");
	fprintf(helpout, "\t--dump-header    Prints out the xar binary header information\n");
	fprintf(helpout, "\t--compression    Specifies the compression type to use.\n");
	fprintf(helpout, "\t                      Valid values: none, gzip, bzip2, lzma, xz\n");
	fprintf(helpout, "\t                      Default: gzip\n");
	fprintf(helpout, "\t-a               Synonym for \"--compression=lzma\"\n");
	fprintf(helpout, "\t-j               Synonym for \"--compression=bzip2\"\n");
	fprintf(helpout, "\t-z               Synonym for \"--compression=gzip\"\n");
	fprintf(helpout, "\t--compression-args=arg Specifies arguments to be passed\n");
	fprintf(helpout, "\t                       to the compression engine.\n");
	fprintf(helpout, "\t--rfc6713        Always use application/zlib for gzip encoding style\n");
	fprintf(helpout, "\t--list-subdocs   List the subdocuments in the xml header\n");
	fprintf(helpout, "\t--extract-subdoc=name Extracts the specified subdocument\n");
	fprintf(helpout, "\t                      to a document in cwd named <name>.xml\n");
	fprintf(helpout, "\t--exclude=<regexp> POSIX basic regular expression of files to \n");
	fprintf(helpout, "\t                      ignore while archiving.\n");
	fprintf(helpout, "\t--strip-components=n Number of path components to strip when extracting\n");
	fprintf(helpout, "\t--to-stdout      Write file contents to standard out when extracting\n");
	fprintf(helpout, "\t-O               Synonym for \"--to-stdout\"\n");
	fprintf(helpout, "\t--rsize          Specifies the size of the buffer used\n");
	fprintf(helpout, "\t                      for read IO operations in bytes.\n");
	fprintf(helpout, "\t--coalesce-heap  When archived files are identical, only store one copy\n");
	fprintf(helpout, "\t                      This option creates an archive which\n");
	fprintf(helpout, "\t                      is not streamable\n");
	fprintf(helpout, "\t--link-same      Hardlink identical files\n");
	fprintf(helpout, "\t--recompress     Allow recompressing already compressed files\n");
	fprintf(helpout, "\t--no-compress    POSIX regular expression of files\n");
	fprintf(helpout, "\t                      to archive, but not compress.\n");
	fprintf(helpout, "\t--prop-include=<p> File properties to include in archive\n");
	fprintf(helpout, "\t--prop-exclude=<p> File properties to exclude in archive\n");
	fprintf(helpout, "\t--distribution   Only includes a subset of file properties\n");
	fprintf(helpout, "\t                      appropriate for archive distribution\n");
	fprintf(helpout, "\t--keep-existing  Do not overwrite existing files while extracting\n");
	fprintf(helpout, "\t-k               Synonym for --keep-existing\n");
	fprintf(helpout, "\t--keep-setuid    Preserve the suid/sgid bits when extracting\n");
	fprintf(helpout, "\t--sig-size=n     Size (in bytes) of the signature placeholder\n");
	fprintf(helpout, "\t                      to generate.\n");
	fprintf(helpout, "\t--sig-len=n      Synonym for \"--sig-size=n\"\n");
	fprintf(helpout, "\t--data-to-sign=file   Path where to dump the data to be signed.\n");
	fprintf(helpout, "\t                      Requires --toc-cksum type other than none.\n");
	fprintf(helpout, "\t--digestinfo-to-sign=file Path where to dump the DigestInfo data to be\n");
	fprintf(helpout, "\t                 signed.  This option requires the --toc-cksum type be\n");
	fprintf(helpout, "\t                 set to sha1 (the default), md5, sha224, sha256, sha384\n");
	fprintf(helpout, "\t                 or sha512.  It produces the same output data as the\n");
	fprintf(helpout, "\t                 --data-to-sign option does except that the output has\n");
	fprintf(helpout, "\t                 the appropriate DigestInfo prefix value prepended.\n");
	fprintf(helpout, "\t                 May only be used in place of the --data-to-sign option.\n");
	fprintf(helpout, "\t--sig-offset=file     Path where to dump the signature's offset\n");
	fprintf(helpout, "\t                      within the xar.  Never required.\n");
	fprintf(helpout, "\t--cert-loc=file  Location of a signing certificate to include in the\n");
	fprintf(helpout, "\t                      archive.  May be repeated to include a\n");
	fprintf(helpout, "\t                      certificate chain.  The first cert-loc option\n");
	fprintf(helpout, "\t                      should specify the leaf certificate, the next its\n");
	fprintf(helpout, "\t                      issuer and so on so that the last cert-loc option\n");
	fprintf(helpout, "\t                      specifies the root CA for the chain.\n");
	fprintf(helpout, "\t                      Certificate files must be in DER (binary) format.\n");
	fprintf(helpout, "\t                      --leaf-cert-loc= and --intermediate-cert-loc=\n");
	fprintf(helpout, "\t                      are accepted as synonyms for --cert-loc= for\n");
	fprintf(helpout, "\t                      historical reasons.\n");
	fprintf(helpout, "\t--help           Show this help on stdout\n");
	fprintf(helpout, "\t-h               Synonym for \"--help\"\n");
	fprintf(helpout, "\t-V               Synonym for \"--version\"\n");
	fprintf(helpout, "\t--version        Print xar's version number to stdout\n");
}

/*
static void usage(const char *prog) {
	_usage(prog, stderr);
}
*/

static void usagehint(const char *prog) {
	_usagehint(prog, stderr);
}

static void get_libxar_version() {
	if (!xar_lib_version_fetched) {
		const char *libver = NULL;
		xar_t x = xar_open("", WRITE);
		if (x) {
			libver = xar_opt_get(x, XAR_OPT_XARLIBVERSION);
			xar_close(x);
		}
		if (libver)
			xar_lib_version = (unsigned long)strtol(libver, NULL, 0);
		else
			xar_lib_version = 0;
		xar_lib_version_fetched = 1;
	}
}

static void print_version() {
	printf("xar %s\n", XAR_VERSION);
	if (Verbose) {
#ifdef XAR_COMMIT_ID
		puts(XAR_COMMIT_ID);
#endif
		get_libxar_version();
		if (xar_lib_version)
			printf("xar library version 0x%08lX\n", xar_lib_version);
		else
			printf("xar library version unknown\n");
	}
}

static const struct HashType *get_hash_alg(const char *str) {
	unsigned i, count = (unsigned)(sizeof(HashTypes) / sizeof(HashTypes[0]));
	for (i = 0; i < count; ++i) {
		if (strcmp(str, HashTypes[i].name) == 0)
			return &HashTypes[i];
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	int ret;
	char *filename = NULL;
	char *sig_path = NULL;
	char *cert_path = NULL;
	char *cert_CAfile = NULL;
	char command = 0, c;
	char **args;
	const char *argv0;
	const char *tocfile = NULL;
	int arglen, i, err;
	xar_t x;
	int loptind = 0;
	int required_dash_f = 0;  /* This release requires us to use -f */
	struct lnode *tmp;
	struct cnode *ctmp;
	long int longtmp;
	struct stat stat_struct;
	struct option o[] = { 
		{"create", 0, 0, 'c'},
		{"extract", 0, 0, 'x'},
		{"list", 0, 0, 't'},
		{"file", 1, 0, 'f'},
		{"directory", 1, 0, 'C'},
		{"verbose", 0, 0, 'v'},
		{"one-file-system", 0, 0, 'l'},
		{"to-stdout", 0, 0, 'O'},
		{"toc-cksum", 1, 0, 1},
		{"dump-toc", 1, 0, 'd'},
		{"compression", 1, 0, 2},
		{"list-subdocs", 0, 0, 3},
		{"help", 0, 0, 'h'},
		{"version", 0, 0, 4},
		{"dump-header", 0, 0, 5},
		{"extract-subdoc", 1, 0, 6},
		{"exclude", 1, 0, 7},
		{"rsize", 1, 0, 8},
		{"coalesce-heap", 0, 0, 9},
		{"link-same", 0, 0, 10},
		{"no-compress", 1, 0, 11},
		{"prop-include", 1, 0, 12},
		{"prop-exclude", 1, 0, 13},
		{"distribution", 0, 0, 14},
		{"keep-existing", 0, 0, 15},
		{"keep-setuid", 0, 0, 16},
		{"compression-args", 1, 0, 17},
		{"file-cksum", 1, 0, 18},
		{"sig-size", 1, 0, 19},
		{"sig-len", 1, 0, 19},
		{"data-to-sign", 1, 0, 20},
		{"sig-offset", 1, 0, 21},
		{"cert-loc", 1, 0, 22},
		{"intermediate-cert-loc", 1, 0, 22}, /* historical compatibility */
		{"leaf-cert-loc", 1, 0, 23}, /* historical compatibility */
		{"extract-data-to-sign", 0, 0, 24},
		{"sign", 0, 0, 25},
		{"replace-sign", 0, 0, 26},
		{"inject-sig", 1, 0, 27},
		{"extract-certs", 1, 0, 28},
		{"extract-CAfile", 1, 0, 29},
		{"extract-sig", 1, 0, 30},
		{"dump-toc-raw", 1, 0, 31},
		{"digestinfo-to-sign", 1, 0, 32},
		{"recompress", 0, 0, 33},
		{"strip-components", 1, 0, 34},
		{"rfc6713", 0, 0, 35},
		{ 0, 0, 0, 0}
	};

	if (!(argv0=strrchr(argv[0], '/')))
		argv0 = argv[0];
	else
		++argv0;

	if( argc < 2 ) {
		usagehint(argv0);
		exit(1);
	}

	get_libxar_version();
	if (xar_lib_version < XAR_VERSION_NUM)
		fprintf(stderr, "%s: warning: linked xar library version older than %s executable\n", argv0, argv0);

	while( (c = getopt_long(argc, argv, "axcVOC:vtjzf:hpPln:s:d:k", o, &loptind)) != -1 ) {
		switch(c) {
		case  1 :
		{
		          const struct HashType *opthash;
		          size_t optlen;
		          int custom = 0;
		          if( !optarg ) {
				usagehint(argv0);
		          	fprintf(stderr, "\n--toc-cksum requires an argument\n");
		          	exit(1);
		          }
		          optlen = strlen(optarg);
		          if (optlen >= 2 && optarg[optlen-1] == '!') {
				optarg[optlen-1] = '\0';
				custom = 1;
		          }
		          if( (opthash = get_hash_alg(optarg)) == NULL && !custom ) {
				usagehint(argv0);
		          	fprintf(stderr, "\n--toc-cksum unrecognized hash type %s\n", optarg);
				exit(1);
		          }
		          if (!opthash) {
				CustomTocHash.name = optarg;
				opthash = &CustomTocHash;
		          }
		          Toccksum = opthash;
		          break;
		}
		case  2 : if( !optarg ) {
				usagehint(argv0);
				fprintf(stderr, "\n--compression requires an argument\n");
		          	exit(1);
		          }
		          if( (strcmp(optarg, XAR_OPT_VAL_NONE) != 0) &&
		              (strcmp(optarg, XAR_OPT_VAL_GZIP) != 0)
#ifdef HAVE_LIBBZ2
		              && (strcmp(optarg, XAR_OPT_VAL_BZIP) != 0)
#endif
#ifdef HAVE_LIBLZMA
		              && (strcmp(optarg, XAR_OPT_VAL_LZMA) != 0)
		              && (strcmp(optarg, XAR_OPT_VAL_XZ) != 0)
#endif
		          ) {
				usagehint(argv0);
				fprintf(stderr, "\nThis instance of xar doesn't understand compression type %s\n", optarg);
				exit(1);
		          }
		          Compression = optarg;
		          break;
		case  3 : if( command && (command != 'L') ) {
				usagehint(argv0);
				fprintf(stderr, "\nConflicting commands: --list-subdocs and -%c specified\n", command);
				exit(1);
		          }
			  command = 'L';
			  break;
		case 'V':
		case  4 : print_version();
		          exit(0);
		case 'd':
			if( !optarg ) {
				usagehint(argv0);
				fprintf(stderr, "\n--dump-toc requires an argument\n");
				exit(1);
			}
			if( command && (command != 'd') ) {
				usagehint(argv0);
				fprintf(stderr, "\nConflicting commands: -%c and -%c specified\n", c, command);
				exit(1);
			}
			tocfile = optarg;
			command = 'd';
			break;
		case  5 :
			if( command && (command != 'H') ) {
				usagehint(argv0);
				fprintf(stderr, "\nConflicting commands: --dump-header and -%c specified\n", command);
				exit(1);
			}
			command = 'H';
			break;
		case  6 :
			SubdocName = optarg;
			err = asprintf(&Subdoc, "%s.xml", SubdocName);
			if( err == -1 ) {
				fprintf(stderr, "%s: Error with asprintf()\n", argv0);
				exit(1);
			}
			if( !command )
				command = 'S';
			break;
		case  7 :
			tmp = malloc(sizeof(struct lnode));
			tmp->str = optarg;
			tmp->next = NULL;
			err = regcomp(&tmp->reg, tmp->str, REG_NOSUB);
			if( err ) {
				char errstr[1024];
				regerror(err, &tmp->reg, errstr, sizeof(errstr));
				fprintf(stderr, "%s: Error with regular expression %s: %s\n", argv0, tmp->str, errstr);
				exit(1);
			}
			if( Exclude == NULL ) {
				Exclude = tmp;
				Exclude_Tail = tmp;
			} else {
				Exclude_Tail->next = tmp;
				Exclude_Tail = tmp;
			}
			break;
		case  8 :
			if ( !optarg ) {
				usagehint(argv0);
				fprintf(stderr, "\n--rsize requires an argument\n");
				exit(1);
			}
			longtmp = strtol(optarg, NULL, 10);
			if( (((longtmp == LONG_MIN) || (longtmp == LONG_MAX)) && (errno == ERANGE)) || (longtmp < 16) ) {
				usagehint(argv0);
				fprintf(stderr, "\nInvalid rsize value: %s\n", optarg);
				exit(5);
			}
			Rsize = optarg;
			break;
		case  9 : Coalesce = 1; break;
		case 10 : LinkSame = 1; break;
		case 11 :
			tmp = malloc(sizeof(struct lnode));
			tmp->str = optarg;
			tmp->next = NULL;
			err = regcomp(&tmp->reg, tmp->str, REG_NOSUB);
			if( err ) {
				char errstr[1024];
				regerror(err, &tmp->reg, errstr, sizeof(errstr));
				printf("Error with regular expression %s: %s\n", tmp->str, errstr);
				exit(1);
			}
			if( NoCompress == NULL ) {
				NoCompress = tmp;
				NoCompress_Tail = tmp;
			} else {
				NoCompress_Tail->next = tmp;
				NoCompress_Tail = tmp;
			}
			break;
		case 12 :
			tmp = malloc(sizeof(struct lnode));
			tmp->str = optarg;
			tmp->next = NULL;
			if( PropInclude == NULL ) {
				PropInclude = tmp;
				PropInclude_Tail = tmp;
			} else {
				PropInclude_Tail->next = tmp;
				PropInclude_Tail = tmp;
			}
			break;
		case 13 :
			tmp = malloc(sizeof(struct lnode));
			tmp->str = optarg;
			tmp->next = NULL;
			if( PropExclude == NULL ) {
				PropExclude = tmp;
				PropExclude_Tail = tmp;
			} else {
				PropExclude_Tail->next = tmp;
				PropExclude_Tail = tmp;
			}
			break;
		case 14 :
		{
			char *props[] = { "type", "data", "mode", "name" };
			int i;
			for( i = 0; i < 4; i++ ) {
				tmp = malloc(sizeof(struct lnode));
				tmp->str = strdup(props[i]);
				tmp->next = NULL;
				if( PropInclude == NULL ) {
					PropInclude = tmp;
					PropInclude_Tail = tmp;
				} else {
					PropInclude_Tail->next = tmp;
					PropInclude_Tail = tmp;
				}
			}
		}
			break;
		case 'k':
		case 15 :
			NoOverwrite++;
			break;
		case 16 :
			SaveSuid++;
			break;
		case 17 :
			CompressionArg = optarg;
			break;
		case 18 :
		{
		          const struct HashType *opthash;
		          size_t optlen;
		          int custom = 0;
		          if( !optarg ) {
				usagehint(argv0);
		          	fprintf(stderr, "\n--file-cksum requires an argument\n");
				exit(1);
		          }
		          optlen = strlen(optarg);
		          if (optlen >= 2 && optarg[optlen-1] == '!') {
				optarg[optlen-1] = '\0';
				custom = 1;
		          }
		          if( (opthash = get_hash_alg(optarg)) == NULL && !custom ) {
				usagehint(argv0);
		          	fprintf(stderr, "\n--file-cksum unrecognized hash type %s\n", optarg);
				exit(1);
		          }
		          if (!opthash) {
				CustomFileHash.name = optarg;
				opthash = &CustomFileHash;
		          }
		          Filecksum = opthash;
		          break;
		}
		case 19 :
			if( !optarg ) {
				usagehint(argv0);
		          	fprintf(stderr, "\n--sig-size requires an argument\n");
				exit(1);
			}
			SigSize = strtol(optarg, (char **)NULL, 10);
			SigSizePresent = 1;
			break;
		case 20 :
			if( !optarg ) {
				usagehint(argv0);
		          	fprintf(stderr, "\n--data-to-sign requires an argument\n");
				exit(1);
			}
			if (DumpDigestInfo) {
				usagehint(argv0);
				fprintf(stderr, "\n--data-to-sign may not be used with --digestinfo-to-sign\n");
				exit(1);
			}
			if (DataToSignDumpPath) {
				usagehint(argv0);
				fprintf(stderr, "\n--data-to-sign may only be used once\n");
				exit(1);
			}
			DataToSignDumpPath = optarg;
			break;
		case 21 :
			if( !optarg ) {
				usagehint(argv0);
		          	fprintf(stderr, "\n--sig-offset requires an argument\n");
				exit(1);
			}
			SigOffsetDumpPath = optarg;
			break;
		case 22 :	/* cert-loc & intermediate-cert-loc */
		case 23 :	/* leaf-cert-loc */
			if( !optarg ) {
				usagehint(argv0);
		          	fprintf(stderr, "\n--cert-loc requires an argument\n");
				exit(1);
			}
			ctmp = malloc(sizeof(struct cnode));
			ctmp->cert_path = optarg;
			ctmp->next = NULL;
			if( CertPath == NULL ) {
				CertPath = ctmp;
				CertPath_Tail = ctmp;
			} else {
				if( c == 22 ) {
					CertPath_Tail->next = ctmp;
					CertPath_Tail = ctmp;
				} else {
					ctmp->next = CertPath;
					CertPath = ctmp;
				}
			}
			break;
		case 24 :	/* extract-data-to-sign */
			if (command && (command != 'e')) {
				usagehint(argv0);
				fprintf(stderr, "\nConflicting commands: --extract-data-to-sign and -%c specified\n", command);
				exit(1);
			}
			command = 'e';
			break;
		case 25 :	/* sign */
			DoSign = 1;
			break;
		case 26 :	/* replace-sign */
			if (command && (command != 'r')) {
				usagehint(argv0);
				fprintf(stderr, "\nConflicting commands: --replace-sign and -%c specified\n", command);
				exit(1);
			}
			command = 'r';
			DoSign = 1;
			break;
		case 27 :	/* inject signature */
			if (!optarg) {
				usagehint(argv0);
		          	fprintf(stderr, "\n--inject-sig requires an argument\n");
				exit(1);
			}
			if (command && (command != 'i')) {
				usagehint(argv0);
				fprintf(stderr, "\nConflicting commands: --inject-sig and -%c specified\n", command);
				exit(1);
			}
			sig_path = optarg;
			command = 'i';
			break;
		case 28 :	/* extract-certs */
			if (!optarg) {
				usagehint(argv0);
		          	fprintf(stderr, "\n--extract-certs requires an argument\n");
				exit(1);
			}
			if (command && (command != 'j' || cert_CAfile)) {
				usagehint(argv0);
				fprintf(stderr, "\nConflicting commands: --extract-certs and -%c specified\n", command);
				exit(1);
			}
			cert_path = optarg;
			err = stat(cert_path, &stat_struct);
			if (err || !(stat_struct.st_mode & S_IFDIR)) {
				usagehint(argv0);
				fprintf(stderr, "\n%s is not a directory\n", cert_path);
				exit(1);
			}
			command = 'j';
			break;
		case 29 :	/* extract-CAfile */
			if (!optarg) {
				usagehint(argv0);
		          	fprintf(stderr, "\n--extract-CAfile requires an argument\n");
				exit(1);
			}
			if (command && (command != 'j' || cert_path)) {
				usagehint(argv0);
				fprintf(stderr, "\nConflicting commands: --extract-CAfile and -%c specified\n", command);
				exit(1);
			}
			cert_CAfile = optarg;
			err = stat(cert_CAfile, &stat_struct);
			if (!err && (stat_struct.st_mode & S_IFDIR)) {
				usagehint(argv0);
				fprintf(stderr, "\n%s is a directory\n", cert_CAfile);
				exit(1);
			}
			command = 'j';
			break;
		case 30 :	/* extract-sig */
			if( !optarg ) {
				usagehint(argv0);
		          	fprintf(stderr, "\n--extract-sig requires an argument\n");
				exit(1);
			}
			if (command && (command != 'g')) {
				usagehint(argv0);
				fprintf(stderr, "\nConflicting commands: --extract-sig and -%c specified\n", command);
				exit(1);
			}
			SignatureDumpPath = optarg;
			err = stat(SignatureDumpPath, &stat_struct);
			if (!err && (stat_struct.st_mode & S_IFDIR)) {
				usagehint(argv0);
				fprintf(stderr, "\n%s is a directory\n", SignatureDumpPath);
				exit(1);
			}
			command = 'g';
			break;
		case 31:
			if( !optarg ) {
				usagehint(argv0);
		          	fprintf(stderr, "\n--dump-toc-raw requires an argument\n");
				exit(1);
			}
			if (command && (command != 'w')) {
				usagehint(argv0);
				fprintf(stderr, "\nConflicting commands: --dump-toc-raw and -%c specified\n", command);
				exit(1);
			}
			tocfile = optarg;
			command = 'w';
			break;
		case 32 :
			if( !optarg ) {
				usagehint(argv0);
		          	fprintf(stderr, "\n--digestinfo-to-sign requires an argument\n");
				exit(1);
			}
			if (DataToSignDumpPath && !DumpDigestInfo) {
				usagehint(argv0);
				fprintf(stderr, "\n--digestinfo-to-sign may not be used with --data-to-sign\n");
				exit(1);
			}
			if (DataToSignDumpPath) {
				usagehint(argv0);
				fprintf(stderr, "\n--digestinfo-to-sign may only be used once\n");
				exit(1);
			}
			DataToSignDumpPath = optarg;
			DumpDigestInfo = 1;
			break;
		case 33 :
			Recompress++;
			break;
		case 34 :
		{
			long comps;
			char *endptr;
			if( !optarg ) {
				usagehint(argv0);
				fprintf(stderr, "\n--strip-components requires an argument\n");
				exit(1);
			}
			if (xar_lib_version < MIN_XAR_NEW_OPTIONS) {
				usagehint(argv0);
				fprintf(stderr, "\n--strip-components requires a newer xar library\n");
				exit(1);
			}
			comps = strtol(optarg, &endptr, 0);
			if (!*optarg || *endptr || comps < 0) {
				usagehint(argv0);
				fprintf(stderr, "\n--strip-components requires a non-negative number argument\n");
				exit(1);
			}
			StripComponents = optarg;
			break;
		}
		case 35 :
			RFC6713++;
			break;
		case 'C': if( !optarg ) {
				usagehint(argv0);
				fprintf(stderr, "\n-C requires an argument\n");
		          	exit(1);
		          }
		          Chdir = optarg;
		          break;
		case 'c':
		case 'x':
		case 't':
			if( command && (command != 's') ) {
				usagehint(argv0);
				fprintf(stderr, "\nConflicting commands: -%c and -%c specified\n", c, command);
				exit(1);
			}
			if( c == 't' )
				List = 1;
			command = c;
			break;
		case 'a':
			Compression = "lzma";
			break;
		case 'j':
			Compression = "bzip2";
			break;
		case 'z':
			Compression = "gzip";
			break;
		case 'f':
		        required_dash_f = 1;
			filename = optarg;
			break;
		case 'p':
			Perms = SYMBOLIC;
			break;
		case 'P':
			Perms = NUMERIC;
			break;
		case 'l':
			Local = 1;
			break;
		case 'O':
			if (xar_lib_version < MIN_XAR_NEW_OPTIONS) {
				fprintf(stderr, "%s: --to-stdout requires a newer xar library\n", argv0);
				exit(1);
			}
			ToStdout = 1;
			break;
		case 'n':
			SubdocName = optarg;
			break;
		case 's':
			Subdoc = optarg;
			if( !command )
				command = 's';
			break;
		case 'v':
			Verbose++;
			break;
		case 'h':
			print_version();
			_usage(argv0, stdout);
			exit(0);
		default:
			usagehint(argv0);
			exit(1);
		}
	}

	if (Toccksum && !Filecksum && Toccksum->hashlen > HashTypes[SHA1_HASH_INDEX].hashlen)
		Filecksum = Toccksum;
	if (Filecksum && !Toccksum && Filecksum->hashlen > HashTypes[SHA1_HASH_INDEX].hashlen)
		Toccksum = Filecksum;
	if (command == 'c') {
		if (!Toccksum)
			Toccksum = &HashTypes[SHA1_HASH_INDEX];
		if (!Filecksum)
			Filecksum = &HashTypes[SHA1_HASH_INDEX];
		if (DoSign && strcmp(Toccksum->name, XAR_OPT_VAL_NONE) == 0) {
			usagehint(argv0);
			fprintf(stderr, "\n--sign requires a --toc-cksum type value other than \"%s\"\n", XAR_OPT_VAL_NONE);
			exit(1);
		}
	}

	if (!DoSign && (SigSizePresent || CertPath)) {
		usagehint(argv0);
		fprintf(stderr, "\nNeither --sig-size nor --cert-loc may be used without either --sign or --replace-sign\n");
		exit(1);
	}

	if ((Toccksum && strcmp(Toccksum->name, XAR_OPT_VAL_MD5) == 0) || (Filecksum && strcmp(Filecksum->name, XAR_OPT_VAL_MD5) == 0)) {
		fprintf(stderr, "%s: warning: The md5 hash is obsolete and should not be used anymore -- continuing anyway\n", argv0);
	}

	if (! required_dash_f)	{
		usagehint(argv0);
		fprintf(stderr, "\n-f option is REQUIRED\n");
		exit(1);
	}

	/* extract-data-to-sign */
	if ( (command == 'e') && ((!filename) || (!DataToSignDumpPath)) ) {
		usagehint(argv0);
		fprintf(stderr, "\n--extract-data-to-sign also requires either --data-to-sign or --digestinfo-to-sign\n");
		exit(1);
	}

	if (command == 'c' && DataToSignDumpPath && !DumpDigestInfo && strcmp(Toccksum->name, XAR_OPT_VAL_NONE) == 0) {
		usagehint(argv0);
		fprintf(stderr, "\n--data-to-sign requires a --toc-cksum type value other than \"%s\"\n", XAR_OPT_VAL_NONE);
		exit(1);
	}

	if (command == 'c' && DataToSignDumpPath && DumpDigestInfo && (!Toccksum->diprefix || !Toccksum->diprefixlen)) {
		usagehint(argv0);
		fprintf(stderr, "\n--digestinfo-to-sign requires --toc-cksum of \"%s\", \"%s\", \"%s\", \"%s\", \"%s\" or \"%s\"\n",
			XAR_OPT_VAL_MD5, XAR_OPT_VAL_SHA1, XAR_OPT_VAL_SHA224, XAR_OPT_VAL_SHA256, XAR_OPT_VAL_SHA384, XAR_OPT_VAL_SHA512);
		exit(1);
	}


	if ( DoSign && command == 'r' && SigSizePresent && !SigSize ) {
		if (CertPath || SigOffsetDumpPath) {
			usagehint(argv0);
			fprintf(stderr, "\nNeither --cert-loc nor --sig-offset may be used when removing signatures with --sig-size=0\n");
			exit(1);
		}
	}
	else if ( DoSign ) {
		if (  ( SigSize <= 0 || !CertPath ) 
			  || ((command != 'c') && (!filename)) ) {
			usagehint(argv0);
			fprintf(stderr, "\n--sig-size > 0 and at least one --cert-loc option are required to sign\n");
			exit(1);
		}
		if (!command)
			command = 'n';
	}

	if (command == 'r') {
		/*if ( !SigSize || !CertPath || !filename) {
			usage(argv0);
			exit(1);
		}
		xar_t x = xar_open(filename, READ);
		if ( x == NULL ) {
			fprintf(stderr, "%s: Could not open archive %s!\n", argv0, filename);
			exit(1);
		}
		xar_signature_t sig = xar_signature_first(x);
		if ( !sig ) {
			fprintf(stderr, "%s: No signature found to replace\n", argv0);
			exit(E_NOSIG);
		}
		xar_close(x);*/
	}

	if ((command == 'i') && ((!filename) || (!sig_path))) {
		usagehint(argv0);
		fprintf(stderr, "\n--inject-sig requires an argument and also -f\n");
		exit(1);
	}

	if ((command == 'j' || command == 'g') && (!filename)) {
		usagehint(argv0);
		fprintf(stderr, "\nmissing required -f\n");
		exit(1);
	}

	switch(command) {
		case 'H': 
		        return dump_header(filename);
		case 'L': 
			return list_subdocs(filename);
		case 'c':
			if( optind == argc ) {
				usagehint(argv0);
				fprintf(stderr, "\nNo files to operate on\n");
				exit(1);
			}
			arglen = argc - optind;
			args = malloc(sizeof(char*) * (arglen+1));
			memset(args, 0, sizeof(char*) * (arglen+1));
			for( i = 0; i < arglen; i++ )
				args[i] = strdup(argv[optind + i]);

			return archive(filename, arglen, args);
		case 'd':
			if( !tocfile ) {
				usagehint(argv0);
				fprintf(stderr, "\nmissing --dump-toc argument\n");
				exit(1);
			}
			return dumptoc(filename, tocfile);
		case 'w':
			if( !tocfile ) {
				usagehint(argv0);
				fprintf(stderr, "\nmissing --dump-toc-raw argument\n");
				exit(1);
			}
			return dumptoc_raw(filename, tocfile);
		case 'x':
			arglen = argc - optind;
			args = malloc(sizeof(char*) * (arglen+1));
			for( i = 0; i < arglen; i++ )
				args[i] = strdup(argv[optind + i]);
			args[i] = NULL;
			return extract(filename, arglen, args);
		case 't':
			arglen = argc - optind;
			args = calloc(sizeof(char*) * (arglen+1),1);
			for( i = 0; i < arglen; i++ )
				args[i] = strdup(argv[optind + i]);
			ret = list(filename, arglen, args);
			for( i = 0; i < arglen; i++ )
				free(args[i]);
		case 'S':
		case 's':
			x = xar_open(filename, READ);
			if( !x ) {
				fprintf(stderr, "%s: Error opening xar archive: %s\n", argv0, filename);
				exit(1);
			}
			xar_register_errhandler(x, err_callback, NULL);
			extract_subdoc(x, SubdocName);
			xar_close(x);
			exit(Err);
			break;
		case 'e':
			extract_data_to_sign(filename);
			exit(Err);
		case 'g':
			extract_signature(filename, SignatureDumpPath);
			exit(Err);
		case 'r':
			replace_sign(filename);
			exit(Err);
		case 'i':
			inject_signature(filename, sig_path);
			exit(Err);
		case 'n':
			belated_sign(filename);
			exit(Err);
		case 'j':
			extract_certs(filename, cert_path, cert_CAfile);
			exit(Err);
		default:
			usagehint(argv0);
			fprintf(stderr, "\nUnrecognized command\n");
			exit(1);
	}

	/* unreached */
	exit(0);
}
