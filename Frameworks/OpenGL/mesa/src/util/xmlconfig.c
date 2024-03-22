/*
 * XML DRI client-side driver configuration
 * Copyright (C) 2003 Felix Kuehling
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * FELIX KUEHLING, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
/**
 * \file xmlconfig.c
 * \brief Driver-independent client-side part of the XML configuration
 * \author Felix Kuehling
 */

#include "xmlconfig.h"
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if WITH_XMLCONFIG
#include <expat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#endif
#ifdef NO_REGEX
typedef int regex_t;
#define REG_EXTENDED 0
#define REG_NOSUB 0
#define REG_NOMATCH 1
static inline int regcomp(regex_t *r, const char *s, int f) { return 0; }
static inline int regexec(regex_t *r, const char *s, int n, void *p, int f) { return REG_NOMATCH; }
static inline void regfree(regex_t* r) {}
#else
#include <regex.h>
#endif
#include <fcntl.h>
#include <math.h>
#include "strndup.h"
#include "u_process.h"
#include "os_file.h"
#include "os_misc.h"

/* For systems like Hurd */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static bool
be_verbose(void)
{
   const char *s = getenv("MESA_DEBUG");
   if (!s)
      return true;

   return strstr(s, "silent") == NULL;
}

/** \brief Locale-independent integer parser.
 *
 * Works similar to strtol. Leading space is NOT skipped. The input
 * number may have an optional sign. Radix is specified by base. If
 * base is 0 then decimal is assumed unless the input number is
 * prefixed by 0x or 0X for hexadecimal or 0 for octal. After
 * returning tail points to the first character that is not part of
 * the integer number. If no number was found then tail points to the
 * start of the input string. */
static int
strToI(const char *string, const char **tail, int base)
{
   int radix = base == 0 ? 10 : base;
   int result = 0;
   int sign = 1;
   bool numberFound = false;
   const char *start = string;

   assert(radix >= 2 && radix <= 36);

   if (*string == '-') {
      sign = -1;
      string++;
   } else if (*string == '+')
      string++;
   if (base == 0 && *string == '0') {
      numberFound = true;
      if (*(string+1) == 'x' || *(string+1) == 'X') {
         radix = 16;
         string += 2;
      } else {
         radix = 8;
         string++;
      }
   }
   do {
      int digit = -1;
      if (radix <= 10) {
         if (*string >= '0' && *string < '0' + radix)
            digit = *string - '0';
      } else {
         if (*string >= '0' && *string <= '9')
            digit = *string - '0';
         else if (*string >= 'a' && *string < 'a' + radix - 10)
            digit = *string - 'a' + 10;
         else if (*string >= 'A' && *string < 'A' + radix - 10)
            digit = *string - 'A' + 10;
      }
      if (digit != -1) {
         numberFound = true;
         result = radix*result + digit;
         string++;
      } else
         break;
   } while (true);
   *tail = numberFound ? string : start;
   return sign * result;
}

/** \brief Locale-independent floating-point parser.
 *
 * Works similar to strtod. Leading space is NOT skipped. The input
 * number may have an optional sign. '.' is interpreted as decimal
 * point and may occur at most once. Optionally the number may end in
 * [eE]<exponent>, where <exponent> is an integer as recognized by
 * strToI. In that case the result is number * 10^exponent. After
 * returning tail points to the first character that is not part of
 * the floating point number. If no number was found then tail points
 * to the start of the input string.
 *
 * Uses two passes for maximum accuracy. */
static float
strToF(const char *string, const char **tail)
{
   int nDigits = 0, pointPos, exponent;
   float sign = 1.0f, result = 0.0f, scale;
   const char *start = string, *numStart;

   /* sign */
   if (*string == '-') {
      sign = -1.0f;
      string++;
   } else if (*string == '+')
      string++;

   /* first pass: determine position of decimal point, number of
    * digits, exponent and the end of the number. */
   numStart = string;
   while (*string >= '0' && *string <= '9') {
      string++;
      nDigits++;
   }
   pointPos = nDigits;
   if (*string == '.') {
      string++;
      while (*string >= '0' && *string <= '9') {
         string++;
         nDigits++;
      }
   }
   if (nDigits == 0) {
      /* no digits, no number */
      *tail = start;
      return 0.0f;
   }
   *tail = string;
   if (*string == 'e' || *string == 'E') {
      const char *expTail;
      exponent = strToI(string+1, &expTail, 10);
      if (expTail == string+1)
         exponent = 0;
      else
         *tail = expTail;
   } else
      exponent = 0;
   string = numStart;

   /* scale of the first digit */
   scale = sign * (float)pow(10.0, (double)(pointPos-1 + exponent));

   /* second pass: parse digits */
   do {
      if (*string != '.') {
         assert(*string >= '0' && *string <= '9');
         result += scale * (float)(*string - '0');
         scale *= 0.1f;
         nDigits--;
      }
      string++;
   } while (nDigits > 0);

   return result;
}

/** \brief Parse a value of a given type. */
static unsigned char
parseValue(driOptionValue *v, driOptionType type, const char *string)
{
   const char *tail = NULL;
   /* skip leading white-space */
   string += strspn(string, " \f\n\r\t\v");
   switch (type) {
   case DRI_BOOL:
      if (!strcmp(string, "false")) {
         v->_bool = false;
         tail = string + 5;
      } else if (!strcmp(string, "true")) {
         v->_bool = true;
         tail = string + 4;
      }
      else
         return false;
      break;
   case DRI_ENUM: /* enum is just a special integer */
   case DRI_INT:
      v->_int = strToI(string, &tail, 0);
      break;
   case DRI_FLOAT:
      v->_float = strToF(string, &tail);
      break;
   case DRI_STRING:
      free(v->_string);
      v->_string = strndup(string, STRING_CONF_MAXLEN);
      return true;
   case DRI_SECTION:
      unreachable("shouldn't be parsing values in section declarations");
   }

   if (tail == string)
      return false; /* empty string (or containing only white-space) */
   /* skip trailing white space */
   if (*tail)
      tail += strspn(tail, " \f\n\r\t\v");
   if (*tail)
      return false; /* something left over that is not part of value */

   return true;
}

/** \brief Find an option in an option cache with the name as key */
static uint32_t
findOption(const driOptionCache *cache, const char *name)
{
   uint32_t len = strlen(name);
   uint32_t size = 1 << cache->tableSize, mask = size - 1;
   uint32_t hash = 0;
   uint32_t i, shift;

   /* compute a hash from the variable length name */
   for (i = 0, shift = 0; i < len; ++i, shift = (shift+8) & 31)
      hash += (uint32_t)name[i] << shift;
   hash *= hash;
   hash = (hash >> (16-cache->tableSize/2)) & mask;

   /* this is just the starting point of the linear search for the option */
   for (i = 0; i < size; ++i, hash = (hash+1) & mask) {
      /* if we hit an empty entry then the option is not defined (yet) */
      if (cache->info[hash].name == NULL)
         break;
      else if (!strcmp(name, cache->info[hash].name))
         break;
   }
   /* this assertion fails if the hash table is full */
   assert (i < size);

   return hash;
}

/** \brief Like strdup with error checking. */
#define XSTRDUP(dest,source) do {                                       \
      if (!(dest = strdup(source))) {                                   \
         fprintf(stderr, "%s: %d: out of memory.\n", __FILE__, __LINE__); \
         abort();                                                       \
      }                                                                 \
   } while (0)

/** \brief Check if a value is in info->range. */
UNUSED static bool
checkValue(const driOptionValue *v, const driOptionInfo *info)
{
   switch (info->type) {
   case DRI_ENUM: /* enum is just a special integer */
   case DRI_INT:
      return (info->range.start._int == info->range.end._int ||
              (v->_int >= info->range.start._int &&
               v->_int <= info->range.end._int));

   case DRI_FLOAT:
      return (info->range.start._float == info->range.end._float ||
              (v->_float >= info->range.start._float &&
               v->_float <= info->range.end._float));

   default:
      return true;
   }
}

void
driParseOptionInfo(driOptionCache *info,
                   const driOptionDescription *configOptions,
                   unsigned numOptions)
{
   /* Make the hash table big enough to fit more than the maximum number of
    * config options we've ever seen in a driver.
    */
   info->tableSize = 7;
   info->info = calloc((size_t)1 << info->tableSize, sizeof(driOptionInfo));
   info->values = calloc((size_t)1 << info->tableSize, sizeof(driOptionValue));
   if (info->info == NULL || info->values == NULL) {
      fprintf(stderr, "%s: %d: out of memory.\n", __FILE__, __LINE__);
      abort();
   }

   UNUSED bool in_section = false;
   for (int o = 0; o < numOptions; o++) {
      const driOptionDescription *opt = &configOptions[o];

      if (opt->info.type == DRI_SECTION) {
         in_section = true;
         continue;
      }

      /* for driconf xml generation, options must always be preceded by a
       * DRI_CONF_SECTION
       */
      assert(in_section);

      const char *name = opt->info.name;
      int i = findOption(info, name);
      driOptionInfo *optinfo = &info->info[i];
      driOptionValue *optval = &info->values[i];

      if (optinfo->name) {
         /* Duplicate options override the value, but the type must match. */
         assert(optinfo->type == opt->info.type);
      } else {
         XSTRDUP(optinfo->name, name);
      }

      optinfo->type = opt->info.type;
      optinfo->range = opt->info.range;

      switch (opt->info.type) {
      case DRI_BOOL:
         optval->_bool = opt->value._bool;
         break;

      case DRI_INT:
      case DRI_ENUM:
         optval->_int = opt->value._int;
         break;

      case DRI_FLOAT:
         optval->_float = opt->value._float;
         break;

      case DRI_STRING:
         XSTRDUP(optval->_string, opt->value._string);
         break;

      case DRI_SECTION:
         unreachable("handled above");
      }

      /* Built-in default values should always be valid. */
      assert(checkValue(optval, optinfo));

      const char *envVal = os_get_option(name);
      if (envVal != NULL) {
         driOptionValue v;

         /* make sure the value is initialized to something sensible */
         v._string = NULL;

         if (parseValue(&v, opt->info.type, envVal) &&
             checkValue(&v, optinfo)) {
            /* don't use XML_WARNING, we want the user to see this! */
            if (be_verbose()) {
               fprintf(stderr,
                       "ATTENTION: default value of option %s overridden by environment.\n",
                       name);
            }
            *optval = v;
         } else {
            fprintf(stderr, "illegal environment value for %s: \"%s\".  Ignoring.\n",
                    name, envVal);
         }
      }
   }
}

char *
driGetOptionsXml(const driOptionDescription *configOptions, unsigned numOptions)
{
   char *str = ralloc_strdup(NULL,
      "<?xml version=\"1.0\" standalone=\"yes\"?>\n" \
      "<!DOCTYPE driinfo [\n" \
      "   <!ELEMENT driinfo      (section*)>\n" \
      "   <!ELEMENT section      (description+, option+)>\n" \
      "   <!ELEMENT description  (enum*)>\n" \
      "   <!ATTLIST description  lang CDATA #FIXED \"en\"\n" \
      "                          text CDATA #REQUIRED>\n" \
      "   <!ELEMENT option       (description+)>\n" \
      "   <!ATTLIST option       name CDATA #REQUIRED\n" \
      "                          type (bool|enum|int|float) #REQUIRED\n" \
      "                          default CDATA #REQUIRED\n" \
      "                          valid CDATA #IMPLIED>\n" \
      "   <!ELEMENT enum         EMPTY>\n" \
      "   <!ATTLIST enum         value CDATA #REQUIRED\n" \
      "                          text CDATA #REQUIRED>\n" \
      "]>" \
      "<driinfo>\n");

   bool in_section = false;
   for (int o = 0; o < numOptions; o++) {
      const driOptionDescription *opt = &configOptions[o];

      const char *name = opt->info.name;
      const char *types[] = {
         [DRI_BOOL] = "bool",
         [DRI_INT] = "int",
         [DRI_FLOAT] = "float",
         [DRI_ENUM] = "enum",
         [DRI_STRING] = "string",
      };

      if (opt->info.type == DRI_SECTION) {
         if (in_section)
            ralloc_asprintf_append(&str, "  </section>\n");

         ralloc_asprintf_append(&str,
                                "  <section>\n"
                                "    <description lang=\"en\" text=\"%s\"/>\n",
                                opt->desc);

         in_section = true;
         continue;
      }

      ralloc_asprintf_append(&str,
                             "      <option name=\"%s\" type=\"%s\" default=\"",
                             name,
                             types[opt->info.type]);

      switch (opt->info.type) {
      case DRI_BOOL:
         ralloc_asprintf_append(&str, opt->value._bool ? "true" : "false");
         break;

      case DRI_INT:
      case DRI_ENUM:
         ralloc_asprintf_append(&str, "%d", opt->value._int);
         break;

      case DRI_FLOAT:
         ralloc_asprintf_append(&str, "%f", opt->value._float);
         break;

      case DRI_STRING:
         ralloc_asprintf_append(&str, "%s", opt->value._string);
         break;

      case DRI_SECTION:
         unreachable("handled above");
         break;
      }
      ralloc_asprintf_append(&str, "\"");


      switch (opt->info.type) {
      case DRI_INT:
      case DRI_ENUM:
         if (opt->info.range.start._int < opt->info.range.end._int) {
            ralloc_asprintf_append(&str, " valid=\"%d:%d\"",
                                   opt->info.range.start._int,
                                   opt->info.range.end._int);
         }
         break;

      case DRI_FLOAT:
         if (opt->info.range.start._float < opt->info.range.end._float) {
            ralloc_asprintf_append(&str, " valid=\"%f:%f\"",
                                   opt->info.range.start._float,
                                   opt->info.range.end._float);
         }
         break;

      default:
         break;
      }

      ralloc_asprintf_append(&str, ">\n"); /* end of <option> */


      ralloc_asprintf_append(&str, "        <description lang=\"en\" text=\"%s\"%s>\n",
                             opt->desc, opt->info.type != DRI_ENUM ? "/" : "");

      if (opt->info.type == DRI_ENUM) {
         for (int i = 0; i < ARRAY_SIZE(opt->enums) && opt->enums[i].desc; i++) {
            ralloc_asprintf_append(&str, "          <enum value=\"%d\" text=\"%s\"/>\n",
                                   opt->enums[i].value, opt->enums[i].desc);
         }
         ralloc_asprintf_append(&str, "        </description>\n");
      }

      ralloc_asprintf_append(&str, "      </option>\n");
   }

   assert(in_section);
   ralloc_asprintf_append(&str, "  </section>\n");

   ralloc_asprintf_append(&str, "</driinfo>\n");

   char *output = strdup(str);
   ralloc_free(str);

   return output;
}

/**
 * Print message to \c stderr if the \c LIBGL_DEBUG environment variable
 * is set.
 *
 * Is called from the drivers.
 *
 * \param f \c printf like format string.
 */
static void
__driUtilMessage(const char *f, ...)
{
   va_list args;
   const char *libgl_debug;

   libgl_debug=getenv("LIBGL_DEBUG");
   if (libgl_debug && !strstr(libgl_debug, "quiet")) {
      fprintf(stderr, "libGL: ");
      va_start(args, f);
      vfprintf(stderr, f, args);
      va_end(args);
      fprintf(stderr, "\n");
   }
}

/* We don't have real line/column # info in static-config case: */
#if !WITH_XML_CONFIG
#  define XML_GetCurrentLineNumber(p) -1
#  define XML_GetCurrentColumnNumber(p) -1
#endif

/** \brief Output a warning message. */
#define XML_WARNING1(msg) do {                                          \
      __driUtilMessage("Warning in %s line %d, column %d: "msg, data->name, \
                        (int) XML_GetCurrentLineNumber(data->parser),   \
                        (int) XML_GetCurrentColumnNumber(data->parser)); \
   } while (0)
#define XML_WARNING(msg, ...) do {                                      \
      __driUtilMessage("Warning in %s line %d, column %d: "msg, data->name, \
                        (int) XML_GetCurrentLineNumber(data->parser),   \
                        (int) XML_GetCurrentColumnNumber(data->parser), \
                        ##__VA_ARGS__);                                 \
   } while (0)
/** \brief Output an error message. */
#define XML_ERROR1(msg) do {                                            \
      __driUtilMessage("Error in %s line %d, column %d: "msg, data->name, \
                        (int) XML_GetCurrentLineNumber(data->parser),   \
                        (int) XML_GetCurrentColumnNumber(data->parser)); \
   } while (0)
#define XML_ERROR(msg, ...) do {                                        \
      __driUtilMessage("Error in %s line %d, column %d: "msg, data->name, \
                        (int) XML_GetCurrentLineNumber(data->parser),   \
                        (int) XML_GetCurrentColumnNumber(data->parser), \
                        ##__VA_ARGS__);                                 \
   } while (0)

/** \brief Parser context for configuration files. */
struct OptConfData {
   const char *name;
#if WITH_XMLCONFIG
   XML_Parser parser;
#endif
   driOptionCache *cache;
   int screenNum;
   const char *driverName, *execName;
   const char *kernelDriverName;
   const char *deviceName;
   const char *engineName;
   const char *applicationName;
   uint32_t engineVersion;
   uint32_t applicationVersion;
   uint32_t ignoringDevice;
   uint32_t ignoringApp;
   uint32_t inDriConf;
   uint32_t inDevice;
   uint32_t inApp;
   uint32_t inOption;
};

/** \brief Parse a list of ranges of type info->type. */
static unsigned char
parseRange(driOptionInfo *info, const char *string)
{
   char *cp;

   XSTRDUP(cp, string);

   char *sep;
   sep = strchr(cp, ':');
   if (!sep) {
      free(cp);
      return false;
   }

   *sep = '\0';
   if (!parseValue(&info->range.start, info->type, cp) ||
       !parseValue(&info->range.end, info->type, sep+1)) {
      free(cp);
      return false;
   }
   if (info->type == DRI_INT &&
       info->range.start._int >= info->range.end._int) {
      free(cp);
      return false;
   }
   if (info->type == DRI_FLOAT &&
       info->range.start._float >= info->range.end._float) {
      free(cp);
      return false;
   }

   free(cp);
   return true;
}

/** \brief Parse attributes of a device element. */
static void
parseDeviceAttr(struct OptConfData *data, const char **attr)
{
   uint32_t i;
   const char *driver = NULL, *screen = NULL, *kernel = NULL, *device = NULL;
   for (i = 0; attr[i]; i += 2) {
      if (!strcmp(attr[i], "driver")) driver = attr[i+1];
      else if (!strcmp(attr[i], "screen")) screen = attr[i+1];
      else if (!strcmp(attr[i], "kernel_driver")) kernel = attr[i+1];
      else if (!strcmp(attr[i], "device")) device = attr[i+1];
      else XML_WARNING("unknown device attribute: %s.", attr[i]);
   }
   if (driver && strcmp(driver, data->driverName))
      data->ignoringDevice = data->inDevice;
   else if (kernel && (!data->kernelDriverName ||
                       strcmp(kernel, data->kernelDriverName)))
      data->ignoringDevice = data->inDevice;
   else if (device && (!data->deviceName ||
                       strcmp(device, data->deviceName)))
      data->ignoringDevice = data->inDevice;
   else if (screen) {
      driOptionValue screenNum;
      if (!parseValue(&screenNum, DRI_INT, screen))
         XML_WARNING("illegal screen number: %s.", screen);
      else if (screenNum._int != data->screenNum)
         data->ignoringDevice = data->inDevice;
   }
}

/** \brief Parse attributes of an application element. */
static void
parseAppAttr(struct OptConfData *data, const char **attr)
{
   uint32_t i;
   const char *exec = NULL;
   const char *sha1 = NULL;
   const char *exec_regexp = NULL;
   const char *application_name_match = NULL;
   const char *application_versions = NULL;
   driOptionInfo version_range = {
      .type = DRI_INT,
   };

   for (i = 0; attr[i]; i += 2) {
      if (!strcmp(attr[i], "name")) /* not needed here */;
      else if (!strcmp(attr[i], "executable")) exec = attr[i+1];
      else if (!strcmp(attr[i], "executable_regexp")) exec_regexp = attr[i+1];
      else if (!strcmp(attr[i], "sha1")) sha1 = attr[i+1];
      else if (!strcmp(attr[i], "application_name_match"))
         application_name_match = attr[i+1];
      else if (!strcmp(attr[i], "application_versions"))
         application_versions = attr[i+1];
      else XML_WARNING("unknown application attribute: %s.", attr[i]);
   }
   if (exec && strcmp(exec, data->execName)) {
      data->ignoringApp = data->inApp;
   } else if (exec_regexp) {
      regex_t re;

      if (regcomp(&re, exec_regexp, REG_EXTENDED|REG_NOSUB) == 0) {
         if (regexec(&re, data->execName, 0, NULL, 0) == REG_NOMATCH)
            data->ignoringApp = data->inApp;
         regfree(&re);
      } else
         XML_WARNING("Invalid executable_regexp=\"%s\".", exec_regexp);
   } else if (sha1) {
      /* SHA1_DIGEST_STRING_LENGTH includes terminating null byte */
      if (strlen(sha1) != (SHA1_DIGEST_STRING_LENGTH - 1)) {
         XML_WARNING("Incorrect sha1 application attribute");
         data->ignoringApp = data->inApp;
      } else {
         size_t len;
         char* content;
         char path[PATH_MAX];
         if (util_get_process_exec_path(path, ARRAY_SIZE(path)) > 0 &&
             (content = os_read_file(path, &len))) {
            uint8_t sha1x[SHA1_DIGEST_LENGTH];
            char sha1s[SHA1_DIGEST_STRING_LENGTH];
            _mesa_sha1_compute(content, len, sha1x);
            _mesa_sha1_format((char*) sha1s, sha1x);
            free(content);

            if (strcmp(sha1, sha1s)) {
               data->ignoringApp = data->inApp;
            }
         } else {
            data->ignoringApp = data->inApp;
         }
      }
   } else if (application_name_match) {
      regex_t re;

      if (regcomp(&re, application_name_match, REG_EXTENDED|REG_NOSUB) == 0) {
         if (regexec(&re, data->applicationName, 0, NULL, 0) == REG_NOMATCH)
            data->ignoringApp = data->inApp;
         regfree(&re);
      } else
         XML_WARNING("Invalid application_name_match=\"%s\".", application_name_match);
   }
   if (application_versions) {
      driOptionValue v = { ._int = data->applicationVersion };
      if (parseRange(&version_range, application_versions)) {
         if (!checkValue(&v, &version_range))
            data->ignoringApp = data->inApp;
      } else {
         XML_WARNING("Failed to parse application_versions range=\"%s\".",
                     application_versions);
      }
   }
}

/** \brief Parse attributes of an application element. */
static void
parseEngineAttr(struct OptConfData *data, const char **attr)
{
   uint32_t i;
   const char *engine_name_match = NULL, *engine_versions = NULL;
   driOptionInfo version_range = {
      .type = DRI_INT,
   };
   for (i = 0; attr[i]; i += 2) {
      if (!strcmp(attr[i], "name")) /* not needed here */;
      else if (!strcmp(attr[i], "engine_name_match")) engine_name_match = attr[i+1];
      else if (!strcmp(attr[i], "engine_versions")) engine_versions = attr[i+1];
      else XML_WARNING("unknown application attribute: %s.", attr[i]);
   }
   if (engine_name_match) {
      regex_t re;

      if (regcomp(&re, engine_name_match, REG_EXTENDED|REG_NOSUB) == 0) {
         if (regexec(&re, data->engineName, 0, NULL, 0) == REG_NOMATCH)
            data->ignoringApp = data->inApp;
         regfree(&re);
      } else
         XML_WARNING("Invalid engine_name_match=\"%s\".", engine_name_match);
   }
   if (engine_versions) {
      driOptionValue v = { ._int = data->engineVersion };
      if (parseRange(&version_range, engine_versions)) {
         if (!checkValue(&v, &version_range))
            data->ignoringApp = data->inApp;
      } else {
         XML_WARNING("Failed to parse engine_versions range=\"%s\".",
                     engine_versions);
      }
   }
}

/** \brief Parse attributes of an option element. */
static void
parseOptConfAttr(struct OptConfData *data, const char **attr)
{
   uint32_t i;
   const char *name = NULL, *value = NULL;
   for (i = 0; attr[i]; i += 2) {
      if (!strcmp(attr[i], "name")) name = attr[i+1];
      else if (!strcmp(attr[i], "value")) value = attr[i+1];
      else XML_WARNING("unknown option attribute: %s.", attr[i]);
   }
   if (!name) XML_WARNING1("name attribute missing in option.");
   if (!value) XML_WARNING1("value attribute missing in option.");
   if (name && value) {
      driOptionCache *cache = data->cache;
      uint32_t opt = findOption(cache, name);
      if (cache->info[opt].name == NULL)
         /* don't use XML_WARNING, drirc defines options for all drivers,
          * but not all drivers support them */
         return;
      else if (getenv(cache->info[opt].name)) {
         /* don't use XML_WARNING, we want the user to see this! */
         if (be_verbose()) {
            fprintf(stderr,
                    "ATTENTION: option value of option %s ignored.\n",
                    cache->info[opt].name);
         }
      } else if (!parseValue(&cache->values[opt], cache->info[opt].type, value))
         XML_WARNING("illegal option value: %s.", value);
   }
}

#if WITH_XMLCONFIG

/** \brief Elements in configuration files. */
enum OptConfElem {
   OC_APPLICATION = 0, OC_DEVICE, OC_DRICONF, OC_ENGINE, OC_OPTION, OC_COUNT
};
static const char *OptConfElems[] = {
   [OC_APPLICATION]  = "application",
   [OC_DEVICE] = "device",
   [OC_DRICONF] = "driconf",
   [OC_ENGINE]  = "engine",
   [OC_OPTION] = "option",
};

static int compare(const void *a, const void *b) {
   return strcmp(*(char *const*)a, *(char *const*)b);
}
/** \brief Binary search in a string array. */
static uint32_t
bsearchStr(const char *name, const char *elems[], uint32_t count)
{
   const char **found;
   found = bsearch(&name, elems, count, sizeof(char *), compare);
   if (found)
      return found - elems;
   else
      return count;
}

/** \brief Handler for start element events. */
static void
optConfStartElem(void *userData, const char *name,
                 const char **attr)
{
   struct OptConfData *data = (struct OptConfData *)userData;
   enum OptConfElem elem = bsearchStr(name, OptConfElems, OC_COUNT);
   switch (elem) {
   case OC_DRICONF:
      if (data->inDriConf)
         XML_WARNING1("nested <driconf> elements.");
      if (attr[0])
         XML_WARNING1("attributes specified on <driconf> element.");
      data->inDriConf++;
      break;
   case OC_DEVICE:
      if (!data->inDriConf)
         XML_WARNING1("<device> should be inside <driconf>.");
      if (data->inDevice)
         XML_WARNING1("nested <device> elements.");
      data->inDevice++;
      if (!data->ignoringDevice && !data->ignoringApp)
         parseDeviceAttr(data, attr);
      break;
   case OC_APPLICATION:
      if (!data->inDevice)
         XML_WARNING1("<application> should be inside <device>.");
      if (data->inApp)
         XML_WARNING1("nested <application> or <engine> elements.");
      data->inApp++;
      if (!data->ignoringDevice && !data->ignoringApp)
         parseAppAttr(data, attr);
      break;
   case OC_ENGINE:
      if (!data->inDevice)
         XML_WARNING1("<engine> should be inside <device>.");
      if (data->inApp)
         XML_WARNING1("nested <application> or <engine> elements.");
      data->inApp++;
      if (!data->ignoringDevice && !data->ignoringApp)
         parseEngineAttr(data, attr);
      break;
   case OC_OPTION:
      if (!data->inApp)
         XML_WARNING1("<option> should be inside <application>.");
      if (data->inOption)
         XML_WARNING1("nested <option> elements.");
      data->inOption++;
      if (!data->ignoringDevice && !data->ignoringApp)
         parseOptConfAttr(data, attr);
      break;
   default:
      XML_WARNING("unknown element: %s.", name);
   }
}

/** \brief Handler for end element events. */
static void
optConfEndElem(void *userData, const char *name)
{
   struct OptConfData *data = (struct OptConfData *)userData;
   enum OptConfElem elem = bsearchStr(name, OptConfElems, OC_COUNT);
   switch (elem) {
   case OC_DRICONF:
      data->inDriConf--;
      break;
   case OC_DEVICE:
      if (data->inDevice-- == data->ignoringDevice)
         data->ignoringDevice = 0;
      break;
   case OC_APPLICATION:
   case OC_ENGINE:
      if (data->inApp-- == data->ignoringApp)
         data->ignoringApp = 0;
      break;
   case OC_OPTION:
      data->inOption--;
      break;
   default:
      /* unknown element, warning was produced on start tag */;
   }
}

static void
_parseOneConfigFile(XML_Parser p)
{
#define BUF_SIZE 0x1000
   struct OptConfData *data = (struct OptConfData *)XML_GetUserData(p);
   int status;
   int fd;

   if ((fd = open(data->name, O_RDONLY)) == -1) {
      __driUtilMessage("Can't open configuration file %s: %s.",
                       data->name, strerror(errno));
      return;
   }

   while (1) {
      int bytesRead;
      void *buffer = XML_GetBuffer(p, BUF_SIZE);
      if (!buffer) {
         __driUtilMessage("Can't allocate parser buffer.");
         break;
      }
      bytesRead = read(fd, buffer, BUF_SIZE);
      if (bytesRead == -1) {
         __driUtilMessage("Error reading from configuration file %s: %s.",
                          data->name, strerror(errno));
         break;
      }
      status = XML_ParseBuffer(p, bytesRead, bytesRead == 0);
      if (!status) {
         XML_ERROR("%s.", XML_ErrorString(XML_GetErrorCode(p)));
         break;
      }
      if (bytesRead == 0)
         break;
   }

   close(fd);
#undef BUF_SIZE
}

/** \brief Parse the named configuration file */
static void
parseOneConfigFile(struct OptConfData *data, const char *filename)
{
   XML_Parser p;

   p = XML_ParserCreate(NULL); /* use encoding specified by file */
   XML_SetElementHandler(p, optConfStartElem, optConfEndElem);
   XML_SetUserData(p, data);
   data->parser = p;
   data->name = filename;
   data->ignoringDevice = 0;
   data->ignoringApp = 0;
   data->inDriConf = 0;
   data->inDevice = 0;
   data->inApp = 0;
   data->inOption = 0;

   _parseOneConfigFile(p);
   XML_ParserFree(p);
}

static int
scandir_filter(const struct dirent *ent)
{
#ifndef DT_REG /* systems without d_type in dirent results */
   struct stat st;

   if ((lstat(ent->d_name, &st) != 0) ||
       (!S_ISREG(st.st_mode) && !S_ISLNK(st.st_mode)))
      return 0;
#else
   /* Allow through unknown file types for filesystems that don't support d_type
    * The full filepath isn't available here to stat the file
    */
   if (ent->d_type != DT_REG && ent->d_type != DT_LNK && ent->d_type != DT_UNKNOWN)
      return 0;
#endif

   int len = strlen(ent->d_name);
   if (len <= 5 || strcmp(ent->d_name + len - 5, ".conf"))
      return 0;

   return 1;
}

/** \brief Parse configuration files in a directory */
static void
parseConfigDir(struct OptConfData *data, const char *dirname)
{
   int i, count;
   struct dirent **entries = NULL;

   count = scandir(dirname, &entries, scandir_filter, alphasort);
   if (count < 0)
      return;

   for (i = 0; i < count; i++) {
      char filename[PATH_MAX];
#ifdef DT_REG
      unsigned char d_type = entries[i]->d_type;
#endif

      snprintf(filename, PATH_MAX, "%s/%s", dirname, entries[i]->d_name);
      free(entries[i]);

#ifdef DT_REG
      /* In the case of unknown d_type, ensure it is a regular file
       * This can be accomplished with stat on the full filepath
       */
      if (d_type == DT_UNKNOWN) {
         struct stat st;
         if (stat(filename, &st) != 0 ||
             !S_ISREG(st.st_mode)) {
            continue;
         }
      }
#endif

      parseOneConfigFile(data, filename);
   }

   free(entries);
}
#else
#  include "driconf_static.h"

static void
parseStaticOptions(struct OptConfData *data, const struct driconf_option *options,
                   unsigned num_options)
{
   if (data->ignoringDevice || data->ignoringApp)
      return;
   for (unsigned i = 0; i < num_options; i++) {
      const char *optattr[] = {
         "name", options[i].name,
         "value", options[i].value,
         NULL
      };
      parseOptConfAttr(data, optattr);
   }
}

static void
parseStaticConfig(struct OptConfData *data)
{
   data->ignoringDevice = 0;
   data->ignoringApp = 0;
   data->inDriConf = 0;
   data->inDevice = 0;
   data->inApp = 0;
   data->inOption = 0;

   for (unsigned i = 0; i < ARRAY_SIZE(driconf); i++) {
      const struct driconf_device *d = driconf[i];
      const char *devattr[] = {
         "driver", d->driver,
         "device", d->device,
         NULL
      };

      data->ignoringDevice = 0;
      data->inDevice++;
      parseDeviceAttr(data, devattr);
      data->inDevice--;

      data->inApp++;

      for (unsigned j = 0; j < d->num_engines; j++) {
         const struct driconf_engine *e = &d->engines[j];
         const char *engattr[] = {
            "engine_name_match", e->engine_name_match,
            "engine_versions", e->engine_versions,
            NULL
         };

         data->ignoringApp = 0;
         parseEngineAttr(data, engattr);
         parseStaticOptions(data, e->options, e->num_options);
      }

      for (unsigned j = 0; j < d->num_applications; j++) {
         const struct driconf_application *a = &d->applications[j];
         const char *appattr[] = {
            "name", a->name,
            "executable", a->executable,
            "executable_regexp", a->executable_regexp,
            "sha1", a->sha1,
            "application_name_match", a->application_name_match,
            "application_versions", a->application_versions,
            NULL
         };

         data->ignoringApp = 0;
         parseAppAttr(data, appattr);
         parseStaticOptions(data, a->options, a->num_options);
      }

      data->inApp--;
   }
}
#endif /* WITH_XMLCONFIG */

/** \brief Initialize an option cache based on info */
static void
initOptionCache(driOptionCache *cache, const driOptionCache *info)
{
   unsigned i, size = 1 << info->tableSize;
   cache->info = info->info;
   cache->tableSize = info->tableSize;
   cache->values = malloc(((size_t)1 << info->tableSize) * sizeof(driOptionValue));
   if (cache->values == NULL) {
      fprintf(stderr, "%s: %d: out of memory.\n", __FILE__, __LINE__);
      abort();
   }
   memcpy(cache->values, info->values,
           ((size_t)1 << info->tableSize) * sizeof(driOptionValue));
   for (i = 0; i < size; ++i) {
      if (cache->info[i].type == DRI_STRING)
         XSTRDUP(cache->values[i]._string, info->values[i]._string);
   }
}

static const char *execname;

void
driInjectExecName(const char *exec)
{
   execname = exec;
}

void
driParseConfigFiles(driOptionCache *cache, const driOptionCache *info,
                    int screenNum, const char *driverName,
                    const char *kernelDriverName,
                    const char *deviceName,
                    const char *applicationName, uint32_t applicationVersion,
                    const char *engineName, uint32_t engineVersion)
{
   initOptionCache(cache, info);
   struct OptConfData userData = {0};

   if (!execname)
      execname = os_get_option("MESA_DRICONF_EXECUTABLE_OVERRIDE");
   if (!execname)
      execname = util_get_process_name();

   userData.cache = cache;
   userData.screenNum = screenNum;
   userData.driverName = driverName;
   userData.kernelDriverName = kernelDriverName;
   userData.deviceName = deviceName;
   userData.applicationName = applicationName ? applicationName : "";
   userData.applicationVersion = applicationVersion;
   userData.engineName = engineName ? engineName : "";
   userData.engineVersion = engineVersion;
   userData.execName = execname;

#if WITH_XMLCONFIG
   char *home, *configdir;

   /* parse from either $DRIRC_CONFIGDIR or $datadir/drirc.d */
   if ((configdir = getenv("DRIRC_CONFIGDIR")))
      parseConfigDir(&userData, configdir);
   else {
      parseConfigDir(&userData, DATADIR "/drirc.d");
      parseOneConfigFile(&userData, SYSCONFDIR "/drirc");
   }

   if ((home = getenv("HOME"))) {
      char filename[PATH_MAX];

      snprintf(filename, PATH_MAX, "%s/.drirc", home);
      parseOneConfigFile(&userData, filename);
   }
#else
   parseStaticConfig(&userData);
#endif /* WITH_XMLCONFIG */
}

void
driDestroyOptionInfo(driOptionCache *info)
{
   driDestroyOptionCache(info);
   if (info->info) {
      uint32_t i, size = 1 << info->tableSize;
      for (i = 0; i < size; ++i) {
         if (info->info[i].name) {
            free(info->info[i].name);
         }
      }
      free(info->info);
   }
}

void
driDestroyOptionCache(driOptionCache *cache)
{
   if (cache->info) {
      unsigned i, size = 1 << cache->tableSize;
      for (i = 0; i < size; ++i) {
         if (cache->info[i].type == DRI_STRING)
            free(cache->values[i]._string);
      }
   }
   free(cache->values);
}

unsigned char
driCheckOption(const driOptionCache *cache, const char *name,
               driOptionType type)
{
   uint32_t i = findOption(cache, name);
   return cache->info[i].name != NULL && cache->info[i].type == type;
}

unsigned char
driQueryOptionb(const driOptionCache *cache, const char *name)
{
   uint32_t i = findOption(cache, name);
   /* make sure the option is defined and has the correct type */
   assert(cache->info[i].name != NULL);
   assert(cache->info[i].type == DRI_BOOL);
   return cache->values[i]._bool;
}

int
driQueryOptioni(const driOptionCache *cache, const char *name)
{
   uint32_t i = findOption(cache, name);
   /* make sure the option is defined and has the correct type */
   assert(cache->info[i].name != NULL);
   assert(cache->info[i].type == DRI_INT || cache->info[i].type == DRI_ENUM);
   return cache->values[i]._int;
}

float
driQueryOptionf(const driOptionCache *cache, const char *name)
{
   uint32_t i = findOption(cache, name);
   /* make sure the option is defined and has the correct type */
   assert(cache->info[i].name != NULL);
   assert(cache->info[i].type == DRI_FLOAT);
   return cache->values[i]._float;
}

char *
driQueryOptionstr(const driOptionCache *cache, const char *name)
{
   uint32_t i = findOption(cache, name);
   /* make sure the option is defined and has the correct type */
   assert(cache->info[i].name != NULL);
   assert(cache->info[i].type == DRI_STRING);
   return cache->values[i]._string;
}
