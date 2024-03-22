/*
 * Copyright © 2014 Intel Corporation
 * Copyright © 2016 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * This program reads and compiles multiple GLSL shaders from one source file.
 * Each shader must begin with the #shader directive. Syntax:
 *     #shader [vs|tcs|tes|gs|ps|cs] [name]
 *
 * The shader name is printed, followed by the stderr output from
 * glCreateShaderProgramv. (radeonsi prints the shader disassembly there)
 *
 * The optional parameter -mcpu=[processor] forces radeonsi to compile for
 * the specified GPU processor. (e.g. tahiti, bonaire, tonga)
 *
 * The program doesn't check if the underlying driver is really radeonsi.
 * OpenGL 4.3 Core profile is required.
 */

/* for asprintf() */
#define _GNU_SOURCE

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include <epoxy/gl.h>
#include <epoxy/egl.h>
#include <gbm.h>

#define unlikely(x) __builtin_expect(!!(x), 0)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static int fd;
static EGLDisplay egl_dpy;
static EGLContext ctx;

static void
create_gl_core_context()
{
    const char *client_extensions = eglQueryString(EGL_NO_DISPLAY,
                                                   EGL_EXTENSIONS);
    if (!client_extensions) {
        fprintf(stderr, "ERROR: Missing EGL_EXT_client_extensions\n");
        exit(1);
    }

    if (!strstr(client_extensions, "EGL_MESA_platform_gbm")) {
        fprintf(stderr, "ERROR: Missing EGL_MESA_platform_gbm\n");
        exit(1);
    }

    fd = open("/dev/dri/renderD128", O_RDWR);
    if (unlikely(fd < 0)) {
        fprintf(stderr, "ERROR: Couldn't open /dev/dri/renderD128\n");
        exit(1);
    }

    struct gbm_device *gbm = gbm_create_device(fd);
    if (unlikely(gbm == NULL)) {
        fprintf(stderr, "ERROR: Couldn't create gbm device\n");
        exit(1);
    }

    egl_dpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_MESA,
                                       gbm, NULL);
    if (unlikely(egl_dpy == EGL_NO_DISPLAY)) {
        fprintf(stderr, "ERROR: eglGetDisplay() failed\n");
        exit(1);
    }

    if (unlikely(!eglInitialize(egl_dpy, NULL, NULL))) {
        fprintf(stderr, "ERROR: eglInitialize() failed\n");
        exit(1);
    }

    static const char *egl_extension[] = {
            "EGL_KHR_create_context",
            "EGL_KHR_surfaceless_context"
    };
    const char *extension_string = eglQueryString(egl_dpy, EGL_EXTENSIONS);
    for (int i = 0; i < ARRAY_SIZE(egl_extension); i++) {
        if (strstr(extension_string, egl_extension[i]) == NULL) {
            fprintf(stderr, "ERROR: Missing %s\n", egl_extension[i]);
            exit(1);
        }
    }

    static const EGLint config_attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };
    EGLConfig cfg;
    EGLint count;

    if (!eglChooseConfig(egl_dpy, config_attribs, &cfg, 1, &count) ||
        count == 0) {
        fprintf(stderr, "ERROR: eglChooseConfig() failed\n");
        exit(1);
    }
    eglBindAPI(EGL_OPENGL_API);

    static const EGLint attribs[] = {
        EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR,
        EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
        EGL_CONTEXT_MAJOR_VERSION_KHR, 4,
        EGL_CONTEXT_MINOR_VERSION_KHR, 3,
        EGL_NONE
    };
    ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, attribs);
    if (ctx == EGL_NO_CONTEXT) {
        fprintf(stderr, "eglCreateContext(GL 3.2) failed.\n");
        exit(1);
    }

    if (!eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
        fprintf(stderr, "eglMakeCurrent failed.\n");
        exit(1);
    }
}

static char *
read_file(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Can't open file: %s\n", filename);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    int filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *input = (char*)malloc(filesize + 1);
    if (!input) {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    if (fread(input, filesize, 1, f) != 1) {
        fprintf(stderr, "fread failed\n");
        exit(1);
    }
    fclose(f);

    input[filesize] = 0;
    return input;
}

static void addenv(const char *name, const char *value)
{
    const char *orig = getenv(name);
    if (orig) {
        char *newval;
        (void)!asprintf(&newval, "%s,%s", orig, value);
        setenv(name, newval, 1);
        free(newval);
    } else {
        setenv(name, value, 1);
    }
}

int
main(int argc, char **argv)
{
    const char *filename = NULL;

    for (int i = 1; i < argc; i++) {
        if (strstr(argv[i], "-mcpu=") == argv[i]) {
            setenv("AMD_FORCE_FAMILY", argv[i] + 6, 1);
        } else if (filename == NULL) {
            filename = argv[i];
        } else {
            if (strcmp(argv[i], "--help") != 0 && strcmp(argv[i], "-h") != 0)
                fprintf(stderr, "Unknown option: %s\n\n", argv[i]);

            fprintf(stderr, "Usage: amdgcn_glslc -mcpu=[chip name] [glsl filename]\n");
            return 1;
        }
    }

    if (filename == NULL) {
        fprintf(stderr, "No filename specified.\n");
        return 1;
    }

    addenv("R600_DEBUG", "precompile,vs,tcs,tes,gs,ps,cs,noir,notgsi");

    create_gl_core_context();

    /* Read the source. */
    char *input = read_file(filename);

    /* Comment out lines beginning with ; (FileCheck prefix). */
    if (input[0] == ';')
        memcpy(input, "//", 2);

    char *s = input;
    while (s = strstr(s, "\n;"))
        memcpy(s + 1, "//", 2);

    s = input;
    while (s && (s = strstr(s, "#shader "))) {
        char type_str[16], name[128];
        GLenum type;

        /* If #shader is not at the beginning of the line. */
        if (s != input && s[-1] != '\n' && s[-1] != 0) {
            s = strstr(s, "\n");
            continue;
        }

        /* Parse the #shader directive. */
        if (sscanf(s + 8, "%s %s", type_str, name) != 2) {
            fprintf(stderr, "Cannot parse #shader directive.\n");
            continue;
        }

        if (!strcmp(type_str, "vs"))
            type = GL_VERTEX_SHADER;
        else if (!strcmp(type_str, "tcs"))
            type = GL_TESS_CONTROL_SHADER;
        else if (!strcmp(type_str, "tes"))
            type = GL_TESS_EVALUATION_SHADER;
        else if (!strcmp(type_str, "gs"))
            type = GL_GEOMETRY_SHADER;
        else if (!strcmp(type_str, "fs"))
            type = GL_FRAGMENT_SHADER;
        else if (!strcmp(type_str, "cs"))
            type = GL_COMPUTE_SHADER;

        /* Go the next line. */
        s = strstr(s, "\n");
        if (!s)
            break;
        s++;

        const char *source = s;

        /* Cut the shader source at the end. */
        s = strstr(s, "#shader");
        if (s && s[-1] == '\n')
            s[-1] = 0;

        /* Compile the shader. */
        printf("@%s:\n", name);

        /* Redirect stderr to stdout for the compiler. */
        FILE *stderr_original = stderr;
        stderr = stdout;
        GLuint prog = glCreateShaderProgramv(type, 1, &source);
        stderr = stderr_original;

        GLint linked;
        glGetProgramiv(prog, GL_LINK_STATUS, &linked);

        if (!linked) {
            char log[4096];
            GLsizei length;

            glGetProgramInfoLog(prog, sizeof(log), &length, log);
            fprintf(stderr, "ERROR: Compile failure:\n\n%s\n%s\n",
                    source, log);
            return 1;
        }

        glDeleteProgram(prog);
    }
    return 0;
}
