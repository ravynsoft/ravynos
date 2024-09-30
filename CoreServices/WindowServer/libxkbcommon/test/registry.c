/*
 * Copyright © 2020 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include <assert.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "xkbcommon/xkbregistry.h"

#include "utils.h"
#include "test.h"

#define NO_VARIANT NULL

enum {
    MODEL = 78,
    LAYOUT,
    VARIANT,
    OPTION,
};

struct test_model {
    const char *name; /* required */
    const char *vendor;
    const char *description;
};

struct test_layout {
    const char *name; /* required */
    const char *variant;
    const char *brief;
    const char *description;
    const char *iso639[3];  /* language list (iso639 three letter codes), 3 is enough for our test  */
    const char *iso3166[3]; /* country list (iso3166 two letter codes), 3 is enough for our tests */
};

struct test_option {
    const char *name;
    const char *description;
};

struct test_option_group {
    const char *name;
    const char *description;
    bool allow_multiple_selection;

    struct test_option options[10];
};

static void
fprint_config_item(FILE *fp,
                   const char *name,
                   const char *vendor,
                   const char *brief,
                   const char *description,
                   const char * const iso639[3],
                   const char * const iso3166[3])
{
    fprintf(fp, "  <configItem>\n"
                "    <name>%s</name>\n", name);
    if (brief)
        fprintf(fp, "    <shortDescription>%s</shortDescription>\n", brief);
    if (description)
        fprintf(fp, "    <description>%s</description>\n", description);
    if (vendor)
        fprintf(fp, "    <vendor>%s</vendor>\n", vendor);
    if (iso3166 && iso3166[0]) {
        fprintf(fp, "    <countryList>\n");
        for (int i = 0; i < 3; i++) {
            const char *iso = iso3166[i];
            if (!iso)
                break;
            fprintf(fp, "        <iso3166Id>%s</iso3166Id>\n", iso);
        }
        fprintf(fp, "    </countryList>\n");
    }
    if (iso639 && iso639[0]) {
        fprintf(fp, "    <languageList>\n");
        for (int i = 0; i < 3; i++) {
            const char *iso = iso639[i];
            if (!iso)
                break;
            fprintf(fp, "        <iso639Id>%s</iso639Id>\n", iso);
        }
        fprintf(fp, "    </languageList>\n");
    }

    fprintf(fp, "  </configItem>\n");
}

/**
 * Create a directory populated with a rules/<ruleset>.xml that contains the
 * given items.
 *
 * @return the XKB base directory
 */
static char *
test_create_rules(const char *ruleset,
                  const struct test_model *test_models,
                  const struct test_layout *test_layouts,
                  const struct test_option_group *test_groups)
{
    static int iteration;
    char *tmpdir;
    char buf[PATH_MAX];
    int rc;
    FILE *fp;

    char *template = asprintf_safe("%s.%d.XXXXXX", ruleset, iteration++);
    assert(template != NULL);
    tmpdir = test_maketempdir(template);
    free(template);

    free(test_makedir(tmpdir, "rules"));

    rc = snprintf_safe(buf, sizeof(buf), "%s/rules/%s.xml", tmpdir, ruleset);
    assert(rc);

    fp = fopen(buf, "w");
    assert(fp);

    fprintf(fp,
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE xkbConfigRegistry SYSTEM \"xkb.dtd\">\n"
            "<xkbConfigRegistry version=\"1.1\">\n");

    if (test_models) {
        fprintf(fp, "<modelList>\n");

        for (const struct test_model *m = test_models; m->name; m++) {
            fprintf(fp, "<model>\n");
            fprint_config_item(fp, m->name, m->vendor, NULL, m->description, NULL, NULL);
            fprintf(fp, "</model>\n");
        }
        fprintf(fp, "</modelList>\n");
    }

    if (test_layouts) {
        const struct test_layout *l, *next;

        fprintf(fp, "<layoutList>\n");

        l = test_layouts;
        next = l + 1;

        assert(l->variant == NULL);

        while (l->name) {
            fprintf(fp, "<layout>\n");
            fprint_config_item(fp, l->name, NULL, l->brief, l->description, l->iso639, l->iso3166);

            if (next->name && streq(next->name, l->name)) {
                fprintf(fp, "<variantList>\n");
                do {
                    fprintf(fp, "<variant>\n");
                    fprint_config_item(fp, next->variant, NULL, next->brief, next->description, next->iso639, next->iso3166);
                    fprintf(fp, "</variant>\n");
                    l = next;
                    next++;
                } while (next->name && streq(next->name, l->name));
                fprintf(fp, "</variantList>\n");
            }
            fprintf(fp, "</layout>\n");
            l++;
        }
        fprintf(fp, "</layoutList>\n");
    }

    if (test_groups) {
        fprintf(fp, "<optionList>\n");

        for (const struct test_option_group *g = test_groups; g->name; g++) {
            fprintf(fp, "<group allowMultipleSelection=\"%s\">\n",
                    g->allow_multiple_selection ? "true" : "false");
            fprint_config_item(fp, g->name, NULL, NULL, g->description, NULL, NULL);
            for (const struct test_option *o = g->options; o->name; o++) {
                fprintf(fp, "  <option>\n");
                fprint_config_item(fp, o->name, NULL, NULL, o->description, NULL, NULL);
                fprintf(fp, "</option>\n");
            }
            fprintf(fp, "</group>\n");
        }
        fprintf(fp, "</optionList>\n");
    }

    fprintf(fp, "</xkbConfigRegistry>\n");
    fclose(fp);

    return tmpdir;
}

static void
test_remove_rules(char *basedir, const char *ruleset)
{
    char path[PATH_MAX];
    int rc;

    rc = snprintf_safe(path, sizeof(path), "%s/rules/%s.xml", basedir,
                       ruleset);
    assert(rc);
    unlink(path);
    rc = snprintf_safe(path, sizeof(path), "%s/xkb/rules", basedir);
    assert(rc);
    rmdir(path);
    rmdir(basedir);
    free(basedir);
}

static struct rxkb_context *
test_setup_context_for(const char *ruleset,
                       struct test_model *system_models,
                       struct test_model *user_models,
                       struct test_layout *system_layouts,
                       struct test_layout *user_layouts,
                       struct test_option_group *system_groups,
                       struct test_option_group *user_groups)
{
    char *sysdir = NULL, *userdir = NULL;
    struct rxkb_context *ctx;

    sysdir = test_create_rules(ruleset, system_models, system_layouts,
                               system_groups);
    if (user_models || user_layouts || user_groups)
        userdir = test_create_rules(ruleset, user_models, user_layouts,
                                    user_groups);

    ctx = rxkb_context_new(RXKB_CONTEXT_NO_DEFAULT_INCLUDES);
    assert(ctx);
    if (userdir)
        assert(rxkb_context_include_path_append(ctx, userdir));
    assert(rxkb_context_include_path_append(ctx, sysdir));
    assert(rxkb_context_parse(ctx, ruleset));

    test_remove_rules(sysdir, ruleset);
    if (userdir)
        test_remove_rules(userdir, ruleset);

    return ctx;
}

static struct rxkb_context *
test_setup_context(struct test_model *system_models,
                   struct test_model *user_models,
                   struct test_layout *system_layouts,
                   struct test_layout *user_layouts,
                   struct test_option_group *system_groups,
                   struct test_option_group *user_groups)
{
    const char *ruleset = "xkbtests";
    return test_setup_context_for(ruleset, system_models,
                                  user_models, system_layouts,
                                  user_layouts, system_groups,
                                  user_groups);
}

static struct rxkb_model *
fetch_model(struct rxkb_context *ctx, const char *model)
{
    struct rxkb_model *m = rxkb_model_first(ctx);
    while (m) {
        if (streq(rxkb_model_get_name(m), model))
            return rxkb_model_ref(m);
        m = rxkb_model_next(m);
    }
    return NULL;
}

static bool
find_model(struct rxkb_context *ctx, const char *model)
{
    struct rxkb_model *m = fetch_model(ctx, model);
    rxkb_model_unref(m);
    return m != NULL;
}

static bool
find_models(struct rxkb_context *ctx, ...)
{
    va_list args;
    const char *name;
    int idx = 0;
    bool rc = false;

    va_start(args, ctx);
    name = va_arg(args, const char *);
    while(name) {
        assert(++idx < 20); /* safety guard */
        if (!find_model(ctx, name))
            goto out;
        name = va_arg(args, const char *);
    };

    rc = true;
out:
    va_end(args);
    return rc;
}

static struct rxkb_layout *
fetch_layout(struct rxkb_context *ctx, const char *layout, const char *variant)
{
    struct rxkb_layout *l = rxkb_layout_first(ctx);
    while (l) {
        const char *v = rxkb_layout_get_variant(l);

        if (streq(rxkb_layout_get_name(l), layout) &&
            ((v == NULL && variant == NULL) ||
             (v != NULL && variant != NULL && streq(v, variant))))
            return rxkb_layout_ref(l);
        l = rxkb_layout_next(l);
    }
    return NULL;
}

static bool
find_layout(struct rxkb_context *ctx, const char *layout, const char *variant)
{
    struct rxkb_layout *l = fetch_layout(ctx, layout, variant);
    rxkb_layout_unref(l);
    return l != NULL;
}

static bool
find_layouts(struct rxkb_context *ctx, ...)
{
    va_list args;
    const char *name, *variant;
    int idx = 0;
    bool rc = false;

    va_start(args, ctx);
    name = va_arg(args, const char *);
    variant = va_arg(args, const char *);
    while(name) {
        assert(++idx < 20); /* safety guard */
        if (!find_layout(ctx, name, variant))
            goto out;
        name = va_arg(args, const char *);
        if (name)
            variant = va_arg(args, const char *);
    };

    rc = true;
out:
    va_end(args);
    return rc;
}

static struct rxkb_option_group *
fetch_option_group(struct rxkb_context *ctx, const char *grp)
{
    struct rxkb_option_group *g = rxkb_option_group_first(ctx);
    while (g) {
        if (streq(grp, rxkb_option_group_get_name(g)))
            return rxkb_option_group_ref(g);
        g = rxkb_option_group_next(g);
    }
    return NULL;
}

static inline bool
find_option_group(struct rxkb_context *ctx, const char *grp)
{
    struct rxkb_option_group *g = fetch_option_group(ctx, grp);
    rxkb_option_group_unref(g);
    return g != NULL;
}

static struct rxkb_option *
fetch_option(struct rxkb_context *ctx, const char *grp, const char *opt)
{
    struct rxkb_option_group *g = rxkb_option_group_first(ctx);
    while (g) {
        if (streq(grp, rxkb_option_group_get_name(g))) {
            struct rxkb_option *o = rxkb_option_first(g);

            while (o) {
                if (streq(opt, rxkb_option_get_name(o)))
                    return rxkb_option_ref(o);
                o = rxkb_option_next(o);
            }
        }
        g = rxkb_option_group_next(g);
    }
    return NULL;
}

static bool
find_option(struct rxkb_context *ctx, const char *grp, const char *opt)
{
    struct rxkb_option *o = fetch_option(ctx, grp, opt);
    rxkb_option_unref(o);
    return o != NULL;
}

static bool
find_options(struct rxkb_context *ctx, ...)
{
    va_list args;
    const char *grp, *opt;
    int idx = 0;
    bool rc = false;

    va_start(args, ctx);
    grp = va_arg(args, const char *);
    opt = va_arg(args, const char *);
    while(grp) {
        assert(++idx < 20); /* safety guard */
        if (!find_option(ctx, grp, opt))
            goto out;
        grp = va_arg(args, const char *);
        if (grp)
            opt = va_arg(args, const char *);
    };

    rc = true;
out:
    va_end(args);
    return rc;
}

static bool
cmp_models(struct test_model *tm, struct rxkb_model *m)
{
    if (!tm || !m)
        return false;

    if (!streq(tm->name, rxkb_model_get_name(m)))
        return false;

    if (!streq_null(tm->vendor, rxkb_model_get_vendor(m)))
        return false;

    if (!streq_null(tm->description, rxkb_model_get_description(m)))
        return false;

    return true;
}

static bool
cmp_layouts(struct test_layout *tl, struct rxkb_layout *l)
{
    struct rxkb_iso3166_code *iso3166 = NULL;
    struct rxkb_iso639_code *iso639 = NULL;

    if (!tl || !l)
        return false;

    if (!streq(tl->name, rxkb_layout_get_name(l)))
        return false;

    if (!streq_null(tl->variant, rxkb_layout_get_variant(l)))
        return false;

    if (!streq_null(tl->brief, rxkb_layout_get_brief(l)))
        return false;

    if (!streq_null(tl->description, rxkb_layout_get_description(l)))
        return false;

    iso3166 = rxkb_layout_get_iso3166_first(l);
    for (size_t i = 0; i < sizeof(tl->iso3166); i++) {
        const char *iso = tl->iso3166[i];
        if (iso == NULL && iso3166 == NULL)
            break;

        if (!streq_null(iso, rxkb_iso3166_code_get_code(iso3166)))
            return false;

        iso3166 = rxkb_iso3166_code_next(iso3166);
    }

    if (iso3166 != NULL)
        return false;

    iso639 = rxkb_layout_get_iso639_first(l);
    for (size_t i = 0; i < sizeof(tl->iso639); i++) {
        const char *iso = tl->iso639[i];
        if (iso == NULL && iso639 == NULL)
            break;

        if (!streq_null(iso, rxkb_iso639_code_get_code(iso639)))
            return false;

        iso639 = rxkb_iso639_code_next(iso639);
    }

    if (iso639 != NULL)
        return false;

    return true;
}

static bool
cmp_options(struct test_option *to, struct rxkb_option *o)
{
    if (!to || !o)
        return false;

    if (!streq(to->name, rxkb_option_get_name(o)))
        return false;

    if (!streq_null(to->description, rxkb_option_get_description(o)))
        return false;

    return true;
}

enum cmp_type {
    CMP_EXACT,
    CMP_MATCHING_ONLY,
};

static bool
cmp_option_groups(struct test_option_group *tg, struct rxkb_option_group *g,
                  enum cmp_type cmp)
{
    struct rxkb_option *o;
    struct test_option *to;

    if (!tg || !g)
        return false;

    if (!streq(tg->name, rxkb_option_group_get_name(g)))
        return false;

    if (!streq_null(tg->description, rxkb_option_group_get_description(g)))
        return false;

    if (tg->allow_multiple_selection != rxkb_option_group_allows_multiple(g))
        return false;

    to = tg->options;
    o = rxkb_option_first(g);

    while (o && to->name) {
        if (!cmp_options(to, o))
            return false;
        to++;
        o = rxkb_option_next(o);
    }

    if (cmp == CMP_EXACT && (o || to->name))
        return false;

    return true;
}

static void
test_load_basic(void)
{
    struct test_model system_models[] =  {
        {"m1"},
        {"m2"},
        {NULL},
    };
    struct test_layout system_layouts[] =  {
        {"l1"},
        {"l1", "v1"},
        {NULL},
    };
    struct test_option_group system_groups[] = {
        {"grp1", NULL, true,
          { {"grp1:1"}, {"grp1:2"} } },
        {"grp2", NULL, false,
          { {"grp2:1"}, {"grp2:2"} } },
        { NULL },
    };
    struct rxkb_context *ctx;

    ctx = test_setup_context(system_models, NULL,
                             system_layouts, NULL,
                             system_groups, NULL);

    assert(find_models(ctx, "m1", "m2", NULL));
    assert(find_layouts(ctx, "l1", NO_VARIANT,
                             "l1", "v1", NULL));
    assert(find_options(ctx, "grp1", "grp1:1",
                             "grp1", "grp1:2",
                             "grp2", "grp2:1",
                             "grp2", "grp2:2", NULL));
    rxkb_context_unref(ctx);
}

static void
test_load_full(void)
{
    struct test_model system_models[] =  {
        {"m1", "vendor1", "desc1"},
        {"m2", "vendor2", "desc2"},
        {NULL},
    };
    struct test_layout system_layouts[] =  {
        {"l1", NO_VARIANT, "lbrief1", "ldesc1"},
        {"l1", "v1", "vbrief1", "vdesc1"},
        {"l1", "v2", NULL, "vdesc2"},
        {NULL},
    };
    struct test_option_group system_groups[] = {
        {"grp1", "gdesc1", true,
          { {"grp1:1", "odesc11"}, {"grp1:2", "odesc12"} } },
        {"grp2", "gdesc2", false,
          { {"grp2:1", "odesc21"}, {"grp2:2", "odesc22"} } },
        { NULL },
    };
    struct rxkb_context *ctx;
    struct rxkb_model *m;
    struct rxkb_layout *l;
    struct rxkb_option_group *g;

    ctx = test_setup_context(system_models, NULL,
                             system_layouts, NULL,
                             system_groups, NULL);

    m = fetch_model(ctx, "m1");
    assert(cmp_models(&system_models[0], m));
    rxkb_model_unref(m);

    m = fetch_model(ctx, "m2");
    assert(cmp_models(&system_models[1], m));
    rxkb_model_unref(m);

    l = fetch_layout(ctx, "l1", NO_VARIANT);
    assert(cmp_layouts(&system_layouts[0], l));
    rxkb_layout_unref(l);

    l = fetch_layout(ctx, "l1", "v1");
    assert(cmp_layouts(&system_layouts[1], l));
    rxkb_layout_unref(l);

    l = fetch_layout(ctx, "l1", "v2");
    struct test_layout expected = {"l1", "v2", "lbrief1", "vdesc2"};
    assert(cmp_layouts(&expected, l));
    rxkb_layout_unref(l);

    g = fetch_option_group(ctx, "grp1");
    assert(cmp_option_groups(&system_groups[0], g, CMP_EXACT));
    rxkb_option_group_unref(g);

    g = fetch_option_group(ctx, "grp2");
    assert(cmp_option_groups(&system_groups[1], g, CMP_EXACT));
    rxkb_option_group_unref(g);

    rxkb_context_unref(ctx);
}

static void
test_load_languages(void)
{
    struct test_model system_models[] =  {
        {"m1", "vendor1", "desc1"},
        {NULL},
    };
    struct test_layout system_layouts[] =  {
        {"l1", NO_VARIANT, "lbrief1", "ldesc1",
            .iso639 = { "abc", "def" },
            .iso3166 = { "uv", "wx" }},
        {"l1", "v1", "vbrief1", "vdesc1",
            .iso639 = {"efg"},
            .iso3166 = {"yz"}},
        {"l2", NO_VARIANT, "lbrief1", "ldesc1",
            .iso639 = { "hij", "klm" },
            .iso3166 = { "op", "qr" }},
        {"l2", "v2", "lbrief1", "ldesc1",
            .iso639 = { NULL }, /* inherit from parent */
            .iso3166 = { NULL }},  /* inherit from parent */
        {NULL},
    };
    struct test_option_group system_groups[] = {
        {"grp1", "gdesc1", true,
          { {"grp1:1", "odesc11"}, {"grp1:2", "odesc12"} } },
        { NULL },
    };
    struct rxkb_context *ctx;
    struct rxkb_layout *l;
    struct rxkb_iso3166_code *iso3166;
    struct rxkb_iso639_code *iso639;

    ctx = test_setup_context(system_models, NULL,
                             system_layouts, NULL,
                             system_groups, NULL);

    l = fetch_layout(ctx, "l1", NO_VARIANT);
    assert(cmp_layouts(&system_layouts[0], l));
    rxkb_layout_unref(l);

    l = fetch_layout(ctx, "l1", "v1");
    assert(cmp_layouts(&system_layouts[1], l));
    rxkb_layout_unref(l);

    l = fetch_layout(ctx, "l2", "v2");
    iso3166 = rxkb_layout_get_iso3166_first(l);
    assert(streq(rxkb_iso3166_code_get_code(iso3166), "op"));
    iso3166 = rxkb_iso3166_code_next(iso3166);
    assert(streq(rxkb_iso3166_code_get_code(iso3166), "qr"));

    iso639 = rxkb_layout_get_iso639_first(l);
    assert(streq(rxkb_iso639_code_get_code(iso639), "hij"));
    iso639 = rxkb_iso639_code_next(iso639);
    assert(streq(rxkb_iso639_code_get_code(iso639), "klm"));

    rxkb_layout_unref(l);
    rxkb_context_unref(ctx);
}

static void
test_load_invalid_languages(void)
{
    struct test_model system_models[] =  {
        {"m1", "vendor1", "desc1"},
        {NULL},
    };
    struct test_layout system_layouts[] =  {
        {"l1", NO_VARIANT, "lbrief1", "ldesc1",
            .iso639 = { "ab", "def" },
            .iso3166 = { "uvw", "xz" }},
        {NULL},
    };
    struct test_option_group system_groups[] = {
        {"grp1", "gdesc1", true,
          { {"grp1:1", "odesc11"}, {"grp1:2", "odesc12"} } },
        { NULL },
    };
    struct rxkb_context *ctx;
    struct rxkb_layout *l;
    struct rxkb_iso3166_code *iso3166;
    struct rxkb_iso639_code *iso639;

    ctx = test_setup_context(system_models, NULL,
                             system_layouts, NULL,
                             system_groups, NULL);

    l = fetch_layout(ctx, "l1", NO_VARIANT);
    /* uvw is invalid, we expect 2 letters, verify it was ignored */
    iso3166 = rxkb_layout_get_iso3166_first(l);
    assert(streq(rxkb_iso3166_code_get_code(iso3166), "xz"));
    assert(rxkb_iso3166_code_next(iso3166) == NULL);

    /* ab is invalid, we expect 3 letters, verify it was ignored */
    iso639 = rxkb_layout_get_iso639_first(l);
    assert(streq(rxkb_iso639_code_get_code(iso639), "def"));
    assert(rxkb_iso639_code_next(iso639) == NULL);
    rxkb_layout_unref(l);

    rxkb_context_unref(ctx);
}

static void
test_popularity(void)
{
    struct test_layout system_layouts[] =  {
        {"l1", NO_VARIANT },
        {"l1", "v1" },
        {NULL},
    };
    struct rxkb_context *ctx;
    struct rxkb_layout *l;
    const char *ruleset = "xkbtests.extras";
    char *dir = NULL;

    dir = test_create_rules(ruleset, NULL, system_layouts, NULL);
    ctx = rxkb_context_new(RXKB_CONTEXT_NO_DEFAULT_INCLUDES |
                           RXKB_CONTEXT_LOAD_EXOTIC_RULES);
    assert(ctx);
    assert(rxkb_context_include_path_append(ctx, dir));
    /* Hack: rulest above generates xkbtests.extras.xml, loading "xkbtests"
     * means the extras file counts as exotic */
    assert(rxkb_context_parse(ctx, "xkbtests"));

    l = fetch_layout(ctx, "l1", NO_VARIANT);
    assert(rxkb_layout_get_popularity(l) == RXKB_POPULARITY_EXOTIC);
    rxkb_layout_unref(l);

    l = fetch_layout(ctx, "l1", "v1");
    assert(rxkb_layout_get_popularity(l) == RXKB_POPULARITY_EXOTIC);
    rxkb_layout_unref(l);

    test_remove_rules(dir, ruleset);
    rxkb_context_unref(ctx);
}


static void
test_load_merge(void)
{
    struct test_model system_models[] =  {
        {"m1", "vendor1", "desc1"},
        {"m2", "vendor2", "desc2"},
        {NULL},
    };
    struct test_model user_models[] =  {
        {"m3", "vendor3", "desc3"},
        {"m4", "vendor4", "desc4"},
        {NULL},
    };
    struct test_layout system_layouts[] =  {
        {"l1", NO_VARIANT, "lbrief1", "ldesc1"},
        {"l1", "v1", "vbrief1", "vdesc1"},
        {NULL},
    };
    struct test_layout user_layouts[] =  {
        {"l2", NO_VARIANT, "lbrief2", "ldesc2"},
        {"l2", "v2", "vbrief2", "vdesc2"},
        {NULL},
    };
    struct test_option_group system_groups[] = {
        {"grp1", NULL, true,
          { {"grp1:1"}, {"grp1:2"} } },
        {"grp2", NULL, false,
          { {"grp2:1"}, {"grp2:2"} } },
        { NULL },
    };
    struct test_option_group user_groups[] = {
        {"grp3", NULL, true,
          { {"grp3:1"}, {"grp3:2"} } },
        {"grp4", NULL, false,
          { {"grp4:1"}, {"grp4:2"} } },
        { NULL },
    };
    struct rxkb_context *ctx;
    struct rxkb_model *m;
    struct rxkb_layout *l;
    struct rxkb_option_group *g;

    ctx = test_setup_context(system_models, user_models,
                             system_layouts, user_layouts,
                             system_groups, user_groups);

    assert(find_models(ctx, "m1", "m2", "m3", "m4", NULL));
    assert(find_layouts(ctx, "l1", NO_VARIANT,
                             "l1", "v1",
                             "l2", NO_VARIANT,
                             "l2", "v2", NULL));

    m = fetch_model(ctx, "m1");
    assert(cmp_models(&system_models[0], m));
    rxkb_model_unref(m);

    m = fetch_model(ctx, "m2");
    assert(cmp_models(&system_models[1], m));
    rxkb_model_unref(m);

    m = fetch_model(ctx, "m3");
    assert(cmp_models(&user_models[0], m));
    rxkb_model_unref(m);

    m = fetch_model(ctx, "m4");
    assert(cmp_models(&user_models[1], m));
    rxkb_model_unref(m);

    l = fetch_layout(ctx, "l1", NO_VARIANT);
    assert(cmp_layouts(&system_layouts[0], l));
    rxkb_layout_unref(l);

    l = fetch_layout(ctx, "l1", "v1");
    assert(cmp_layouts(&system_layouts[1], l));
    rxkb_layout_unref(l);

    l = fetch_layout(ctx, "l2", NO_VARIANT);
    assert(cmp_layouts(&user_layouts[0], l));
    rxkb_layout_unref(l);

    l = fetch_layout(ctx, "l2", "v2");
    assert(cmp_layouts(&user_layouts[1], l));
    rxkb_layout_unref(l);

    g = fetch_option_group(ctx, "grp1");
    assert(cmp_option_groups(&system_groups[0], g, CMP_EXACT));
    rxkb_option_group_unref(g);

    g = fetch_option_group(ctx, "grp2");
    assert(cmp_option_groups(&system_groups[1], g, CMP_EXACT));
    rxkb_option_group_unref(g);

    g = fetch_option_group(ctx, "grp3");
    assert(cmp_option_groups(&user_groups[0], g, CMP_EXACT));
    rxkb_option_group_unref(g);

    g = fetch_option_group(ctx, "grp4");
    assert(cmp_option_groups(&user_groups[1], g, CMP_EXACT));
    rxkb_option_group_unref(g);

    rxkb_context_unref(ctx);
}

static void
test_load_merge_no_overwrite(void)
{
    struct test_model system_models[] =  {
        {"m1", "vendor1", "desc1"},
        {"m2", "vendor2", "desc2"},
        {NULL},
    };
    struct test_model user_models[] =  {
        {"m1", "vendor3", "desc3"}, /* must not overwrite */
        {"m4", "vendor4", "desc4"},
        {NULL},
    };
    struct test_layout system_layouts[] =  {
        {"l1", NO_VARIANT, "lbrief1", "ldesc1"},
        {"l1", "v1", "vbrief1", "vdesc1"},
        {NULL},
    };
    struct test_layout user_layouts[] =  {
        {"l2", NO_VARIANT, "lbrief2", "ldesc2"},
        {"l2", "v2", "vbrief2", "vdesc2"},
        {"l1", NO_VARIANT, "lbrief3", "ldesc3"}, /* must not overwrite */
        {"l1", "v2", "vbrief3", "vdesc3"}, /* must not overwrite */
        {NULL},
    };
    struct test_option_group system_groups[] = {
        {"grp1", "gdesc1", true,
          { {"grp1:1", "odesc11"}, {"grp1:2", "odesc12"} } },
        {"grp2", "gdesc2", false,
          { {"grp2:1", "odesc21"}, {"grp2:2", "odesc22"} } },
        { NULL },
    };
    struct test_option_group user_groups[] = {
        {"grp1", "XXXXX", false, /* must not overwrite */
          { {"grp1:1", "YYYYYYY"}, /* must not overwrite */
            {"grp1:3", "ZZZZZZ"} } }, /* append */
        {"grp4", "gdesc4", false,
          { {"grp4:1", "odesc41"}, {"grp4:2", "odesc42"} } },
        { NULL },
    };
    struct rxkb_context *ctx;
    struct rxkb_model *m;
    struct rxkb_layout *l;
    struct rxkb_option_group *g;

    ctx = test_setup_context(system_models, user_models,
                             system_layouts, user_layouts,
                             system_groups, user_groups);

    m = fetch_model(ctx, "m1");
    assert(cmp_models(&system_models[0], m));
    rxkb_model_unref(m);

    l = fetch_layout(ctx, "l1", NO_VARIANT);
    assert(cmp_layouts(&system_layouts[0], l));
    rxkb_layout_unref(l);

    l = fetch_layout(ctx, "l1", "v1");
    assert(cmp_layouts(&system_layouts[1], l));
    rxkb_layout_unref(l);

    assert(find_option(ctx, "grp1", "grp1:3"));
    g = fetch_option_group(ctx, "grp1");
    assert(cmp_option_groups(&system_groups[0], g, CMP_MATCHING_ONLY));
    rxkb_option_group_unref(g);

    rxkb_context_unref(ctx);
}

static void
test_no_include_paths(void)
{
    struct rxkb_context *ctx;

    ctx = rxkb_context_new(RXKB_CONTEXT_NO_DEFAULT_INCLUDES);
    assert(ctx);
    assert(!rxkb_context_parse_default_ruleset(ctx));

    rxkb_context_unref(ctx);
}

static void
test_invalid_include(void)
{
    struct rxkb_context *ctx;

    ctx = rxkb_context_new(RXKB_CONTEXT_NO_DEFAULT_INCLUDES);
    assert(ctx);
    assert(!rxkb_context_include_path_append(ctx, "/foo/bar/baz/bat"));
    assert(!rxkb_context_parse_default_ruleset(ctx));

    rxkb_context_unref(ctx);
}

int
main(void)
{
    test_no_include_paths();
    test_invalid_include();
    test_load_basic();
    test_load_full();
    test_load_merge();
    test_load_merge_no_overwrite();
    test_load_languages();
    test_load_invalid_languages();
    test_popularity();

    return 0;
}
