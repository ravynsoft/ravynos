/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2022 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#define SUDO_ERROR_WRAP 0

#include <sudo_compat.h>
#include <sudo_json.h>
#include <sudo_util.h>
#include <sudo_fatal.h>

sudo_dso_public int main(int argc, char *argv[]);

/* Expected JSON output */
const char outbuf[] = "\n"
    "    \"test1\": {\n"
    "        \"string1\": \"test\\\\\\b\\f\\n\\r\\t string1\",\n"
    "        \"id1\": 4294967295,\n"
    "        \"number1\": -1,\n"
    "        \"bool1\": true,\n"
    "        \"bool2\": false,\n"
    "        \"null1\": null,\n"
    "        \"array1\": [\n"
    "            \"string2\": \"test\\f\\u0011string2\",\n"
    "            \"number2\": -9223372036854775808,\n"
    "            \"number3\": 9223372036854775807\n"
    "        ]\n"
    "    }";

/*
 * Simple tests for sudo json functions()
 */
int
main(int argc, char *argv[])
{
    struct json_container jsonc;
    struct json_value value;
    int ch, errors = 0, ntests = 0;

    initprogname(argc > 0 ? argv[0] : "json_test");

    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	case 'v':
	    /* ignore */
	    break;
	default:
	    fprintf(stderr, "usage: %s [-v]\n", getprogname());
	    return EXIT_FAILURE;
	}
    }
    argc -= optind;
    argv += optind;

    ntests++;
    if (!sudo_json_init(&jsonc, 4, false, true, true)) {
	sudo_warnx("unable to initialize json");
	errors++;
	goto done;
    }

    /* Open main JSON object. */
    ntests++;
    if (!sudo_json_open_object(&jsonc, "test1")) {
	sudo_warnx("unable to open json object");
	errors++;
	goto done;
    }

    /* Verify invalid value is detected. */
    value.type = -1;
    value.u.string = NULL;
    ntests++;
    if (sudo_json_add_value(&jsonc, "bogus1", &value)) {
	/* should have failed, not a fatal error */
	sudo_warnx("should not be able to add bogus type value");
	errors++;
    }

    /* Verify that adding an array is not allowed. */
    value.type = JSON_ARRAY;
    value.u.string = NULL;
    ntests++;
    if (sudo_json_add_value(&jsonc, "bogus2", &value)) {
	/* should have failed, not a fatal error */
	sudo_warnx("should not be able to add array type value");
	errors++;
    }

    /* Verify that adding an object is not allowed. */
    value.type = JSON_OBJECT;
    value.u.string = NULL;
    ntests++;
    if (sudo_json_add_value(&jsonc, "bogus3", &value)) {
	/* should have failed, not a fatal error */
	sudo_warnx("should not be able to add object type value");
	errors++;
    }

    value.type = JSON_STRING;
    value.u.string = "test\\\b\f\n\r\t string1";
    ntests++;
    if (!sudo_json_add_value(&jsonc, "string1", &value)) {
	/* not a fatal error */
	sudo_warnx("unable to add string value (string1)");
	errors++;
    }

    value.type = JSON_ID;
    value.u.id = 0xffffffff;
    ntests++;
    if (!sudo_json_add_value(&jsonc, "id1", &value)) {
	/* not a fatal error */
	sudo_warnx("unable to add ID value (0xffffffff)");
	errors++;
    }

    value.type = JSON_NUMBER;
    value.u.number = -1;
    ntests++;
    if (!sudo_json_add_value(&jsonc, "number1", &value)) {
	/* not a fatal error */
	sudo_warnx("unable to add number value (-1)");
	errors++;
    }

    value.type = JSON_BOOL;
    value.u.boolean = true;
    ntests++;
    if (!sudo_json_add_value(&jsonc, "bool1", &value)) {
	/* not a fatal error */
	sudo_warnx("unable to add bool value (true)");
	errors++;
    }
    value.u.boolean = false;
    ntests++;
    if (!sudo_json_add_value(&jsonc, "bool2", &value)) {
	/* not a fatal error */
	sudo_warnx("unable to add bool value (false)");
	errors++;
    }

    value.type = JSON_NULL;
    ntests++;
    if (!sudo_json_add_value(&jsonc, "null1", &value)) {
	/* not a fatal error */
	sudo_warnx("unable to add null value");
	errors++;
    }

    /* Open JSON array. */
    ntests++;
    if (!sudo_json_open_array(&jsonc, "array1")) {
	sudo_warnx("unable to open json array");
	errors++;
	goto done;
    }

    value.type = JSON_STRING;
    value.u.string = "test\x0c\x11string2";
    ntests++;
    if (!sudo_json_add_value(&jsonc, "string2", &value)) {
	/* not a fatal error */
	sudo_warnx("unable to add string value (string2)");
	errors++;
    }

    value.type = JSON_NUMBER;
    value.u.number = LLONG_MIN;
    ntests++;
    if (!sudo_json_add_value(&jsonc, "number2", &value)) {
	/* not a fatal error */
	sudo_warnx("unable to add number value (LLONG_MIN)");
	errors++;
    }
    value.u.number = LLONG_MAX;
    ntests++;
    if (!sudo_json_add_value(&jsonc, "number3", &value)) {
	/* not a fatal error */
	sudo_warnx("unable to add number value (LLONG_MAX)");
	errors++;
    }

    /* Close JSON array. */
    if (!sudo_json_close_array(&jsonc)) {
	sudo_warnx("unable to close json array");
	errors++;
	goto done;
    }

    /* Close main JSON object. */
    if (!sudo_json_close_object(&jsonc)) {
	sudo_warnx("unable to close json object");
	errors++;
	goto done;
    }

    if (strcmp(outbuf, jsonc.buf) != 0) {
	fprintf(stderr, "Expected:\n%s\n", outbuf);
	fprintf(stderr, "Received:\n%s\n", jsonc.buf);
    }

done:
    sudo_json_free(&jsonc);

    if (ntests != 0) {
	printf("%s: %d tests run, %d errors, %d%% success rate\n",
	    getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }

    return errors;
}
