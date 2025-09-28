/*
 * testOOM.c: Test out-of-memory handling
 *
 * See Copyright for the status of this software.
 *
 * hp@redhat.com
 */

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include <libxml/xmlreader.h>

#include "testOOMlib.h"

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#define EXIT_OOM 2

int error = FALSE;
int errcount = 0;
int noent = 0;
int count = 0;
int valid = 0;
int showErrs = 0;

/*
 * Since we are using the xmlTextReader functions, we set up
 * strings for the element types to help in debugging any error
 * output
 */
const char *elementNames[] = {
    "XML_READER_TYPE_NONE",
    "XML_READER_TYPE_ELEMENT",
    "XML_READER_TYPE_ATTRIBUTE",
    "XML_READER_TYPE_TEXT",
    "XML_READER_TYPE_CDATA",
    "XML_READER_TYPE_ENTITY_REFERENCE",
    "XML_READER_TYPE_ENTITY",
    "XML_READER_TYPE_PROCESSING_INSTRUCTION",
    "XML_READER_TYPE_COMMENT",
    "XML_READER_TYPE_DOCUMENT",
    "XML_READER_TYPE_DOCUMENT_TYPE",
    "XML_READER_TYPE_DOCUMENT_FRAGMENT",
    "XML_READER_TYPE_NOTATION",
    "XML_READER_TYPE_WHITESPACE",
    "XML_READER_TYPE_SIGNIFICANT_WHITESPACE",
    "XML_READER_TYPE_END_ELEMENT",
    "XML_READER_TYPE_END_ENTITY",
    "XML_READER_TYPE_XML_DECLARATION"};

/* not using xmlBuff here because I don't want those
 * mallocs to interfere */
struct buffer {
    char *str;
    size_t len;
    size_t max;
};

static struct buffer *buffer_create (size_t init_len)
{
    struct buffer *b;
    b = malloc (sizeof *b);
    if (b == NULL)
	exit (EXIT_OOM);
    if (init_len) {
	b->str = malloc (init_len);
	if (b->str == NULL)
	    exit (EXIT_OOM);
    }
    else
	b->str = NULL;
    b->len = 0;
    b->max = init_len;
    return b;
}

static void buffer_free (struct buffer *b)
{
    free (b->str);
    free (b);
}

static size_t buffer_get_length (struct buffer *b)
{
    return b->len;
}

static void buffer_expand (struct buffer *b, size_t min)
{
    void *new_str;
    size_t new_size = b->max ? b->max : 512;
    while (new_size < b->len + min)
	new_size *= 2;
    if (new_size > b->max) {
	new_str = realloc (b->str, new_size);
	if (new_str == NULL)
	    exit (EXIT_OOM);
	b->str = new_str;
	b->max = new_size;
    }
}

static void buffer_add_char (struct buffer *b, char c)
{
    buffer_expand (b, 1);
    b->str[b->len] = c;
    b->len += 1;
}

static void buffer_add_string (struct buffer *b, const char *s)
{
    size_t size = strlen(s) + 1;
    unsigned int ix;
    for (ix=0; ix<size-1; ix++) {
        if (s[ix] < 0x20)
	    printf ("binary data [0x%02x]?\n", (unsigned char)s[ix]);
    }
    buffer_expand (b, size);
    strcpy (b->str + b->len, s);
    b->str[b->len+size-1] = '\n';	/* replace string term with newline */
    b->len += size;
}

static int buffer_equal (struct buffer *b1, struct buffer *b2)
{
    return (b1->len == b2->len &&
	    (b1->len == 0 || (memcmp (b1->str, b2->str, b1->len) == 0)));
}

static void buffer_dump (struct buffer *b, const char *fname)
{
    FILE *f = fopen (fname, "wb");
    if (f != NULL) {
	fwrite (b->str, 1, b->len, f);
	fclose (f);
    }
}


static void usage(const char *progname) {
    printf("Usage : %s [options] XMLfiles ...\n", progname);
    printf("\tParse the XML files using the xmlTextReader API\n");
    printf("\t --count: count the number of attribute and elements\n");
    printf("\t --valid: validate the document\n");
    printf("\t --show:  display the error messages encountered\n");
    exit(1);
}
static unsigned int elem, attrs, chars;

static int processNode (xmlTextReaderPtr reader, void *data)
{
    struct buffer *buff = data;
    int type;

    type = xmlTextReaderNodeType(reader);
    if (count) {
	if (type == 1) {
	    elem++;
	    attrs += xmlTextReaderAttributeCount(reader);
	} else if (type == 3) {
	  const xmlChar *txt;
	  txt = xmlTextReaderConstValue(reader);
	  if (txt != NULL)
	    chars += xmlStrlen (txt);
	  else
	    return FALSE;
	}
    }

    if (buff != NULL) {
	int ret;
	const char *s;

	buffer_add_string (buff, elementNames[type]);

	if (type == 1) {
	    s = (const char *)xmlTextReaderConstName (reader);
	    if (s == NULL) return FALSE;
	    buffer_add_string (buff, s);
	    while ((ret = xmlTextReaderMoveToNextAttribute (reader)) == 1) {
		s = (const char *)xmlTextReaderConstName (reader);
		if (s == NULL) return FALSE;
		buffer_add_string (buff, s);
		buffer_add_char (buff, '=');
		s = (const char *)xmlTextReaderConstValue (reader);
		if (s == NULL) return FALSE;
		buffer_add_string (buff, s);
	    }
	    if (ret == -1) return FALSE;
	}
	else if (type == 3) {
	    s = (const char *)xmlTextReaderConstValue (reader);
	    if (s == NULL) return FALSE;
	    buffer_add_string (buff, s);
	}
    }

    return TRUE;
}


struct file_params {
    const char *filename;
    struct buffer *verif_buff;
};

static void
error_func (void *data ATTRIBUTE_UNUSED, xmlErrorPtr err)
{

    errcount++;
    if (err->level == XML_ERR_ERROR ||
        err->level == XML_ERR_FATAL)
        error = TRUE;
    if (showErrs) {
        printf("%3d line %d: %s\n", error, err->line, err->message);
    }
}

static int
check_load_file_memory_func (void *data)
{
     struct file_params *p = data;
     struct buffer *b;
     xmlTextReaderPtr reader;
     int ret, status, first_run;

     if (count) {
         elem = 0;
         attrs = 0;
         chars = 0;
     }

     first_run = p->verif_buff == NULL;
     status = TRUE;
     error = FALSE;
     if (first_run)
	 b = buffer_create (0);
     else
	 b = buffer_create (buffer_get_length (p->verif_buff));

     reader = xmlNewTextReaderFilename (p->filename);
     if (reader == NULL)
       goto out;

     xmlTextReaderSetStructuredErrorHandler (reader, error_func, NULL);
     xmlSetStructuredErrorFunc(NULL, error_func);

     if (valid) {
       if (xmlTextReaderSetParserProp(reader, XML_PARSER_VALIDATE, 1) == -1)
	 goto out;
     }

     /*
      * Process all nodes in sequence
      */
     while ((ret = xmlTextReaderRead(reader)) == 1) {
	 if (!processNode(reader, b))
	 goto out;
     }
     if (ret == -1)
       goto out;

     if (error) {
	 fprintf (stdout, "error handler was called but parse completed successfully (last error #%d)\n", errcount);
	 return FALSE;
     }

     /*
      * Done, cleanup and status
      */
     if (! first_run) {
	 status = buffer_equal (p->verif_buff, b);
	 if (! status) {
	     buffer_dump (p->verif_buff, ".OOM.verif_buff");
	     buffer_dump (b, ".OOM.buff");
	 }
     }

     if (count)
       {
	   fprintf (stdout, "# %s: %u elems, %u attrs, %u chars %s\n",
		    p->filename, elem, attrs, chars,
		    status ? "ok" : "wrong");
       }

 out:
     if (first_run)
	 p->verif_buff = b;
     else
	 buffer_free (b);
     if (reader)
	 xmlFreeTextReader (reader);
     return status;
}

int main(int argc, char **argv) {
    int i;
    int files = 0;

    if (argc <= 1) {
	usage(argv[0]);
	return(1);
    }
    LIBXML_TEST_VERSION;

    xmlMemSetup (test_free,
                 test_malloc,
                 test_realloc,
                 test_strdup);

    xmlInitParser();

    for (i = 1; i < argc ; i++) {
        if ((!strcmp(argv[i], "-count")) || (!strcmp(argv[i], "--count")))
	    count++;
	else if ((!strcmp(argv[i], "-valid")) || (!strcmp(argv[i], "--valid")))
	    valid++;
	else if ((!strcmp(argv[i], "-noent")) ||
	         (!strcmp(argv[i], "--noent")))
	    noent++;
	else if ((!strcmp(argv[i], "-show")) ||
		 (!strcmp(argv[i], "--show")))
	    showErrs++;
    }
    if (noent != 0)
      xmlSubstituteEntitiesDefault(1);
    for (i = 1; i < argc ; i++) {
	if (argv[i][0] != '-') {
             struct file_params p;
	     p.filename = argv[i];
	     p.verif_buff = NULL;

             if (!test_oom_handling (check_load_file_memory_func,
                                     &p)) {
                  fprintf (stdout, "Failed!\n");
                  return 1;
             }

	     buffer_free (p.verif_buff);
             xmlCleanupParser();

             if (test_get_malloc_blocks_outstanding () > 0) {
                  fprintf (stdout, "%d blocks leaked\n",
                           test_get_malloc_blocks_outstanding ());
                  return 1;
             }

	    files ++;
	}
    }

    return 0;
}
