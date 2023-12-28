/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/time.h>
#include "jsynprog.h"

typedef long long       hrtime_t;
extern "C" {
    hrtime_t gethrtime();
    hrtime_t gethrvtime();
}
static jdouble testtime = 3.0 * 1e9;

int cfunc(int);

JNIEXPORT jdouble JNICALL
Java_jsynprog_Timer (JNIEnv *env, jclass obj)
{
    jdouble      jd;
    hrtime_t     start;

    start = gethrtime();
    jd = (double)(start);
    return jd;
}

JNIEXPORT jdouble JNICALL
Java_jsynprog_cTimer (JNIEnv *env, jclass obj)
{
    jdouble      jd;
    hrtime_t     vstart;

    vstart = gethrvtime();
    jd = (double)(vstart);
    return jd;
}

JNIEXPORT jdouble JNICALL
Java_jsynprog_computeSet (JNIEnv *env, jclass obj)
{
    char *s;

    testtime = 3.0;
    s = getenv("SP_COLLECTOR_TEST_TIMER");
    if( s ) {
        testtime = atof(s);
        if (testtime < 1.0)
            testtime = 1.0;
    }
    testtime *= 1e9;
    return testtime;
}

JNIEXPORT jint JNICALL
Java_jsynprog_JavaJavaC (JNIEnv *env, jclass obj, jint n, int scale )
{
    // fprintf(stderr, "Entering Java_jsynprog_JavaJavaC, scale = %d\n", scale);
    int imax = 100000;
    n = 0;
    for (int i =0; i<imax; i++) {
        n=n+((i%2==0)?1:2);
    }
    return n;
}

JNIEXPORT void JNICALL
Java_jsynprog_JavaCC (JNIEnv *env, jclass obj, int scale)
{
    fprintf(stderr, "Entering Java_jsynprog_JavaCC, scale = %d\n", scale);
    int n =0;
    if (scale == 1) {
        scale *= 1000;
    }
    int imax = 4*scale;
    double tEnd = gethrtime() + testtime;
    do { n = 0;
    for (int i =0; i<imax; i++) {
	n = cfunc(n);
    }
    } while (gethrtime() < tEnd);
}

int cfunc (int n) {
    for (int j =0; j<100000;j++) {
	n=n+1;
    }
    return n;
}

JNIEXPORT void JNICALL
Java_jsynprog_JavaCJava (JNIEnv *env, jclass obj, int scale)
{
    fprintf(stderr, "Entering Java_jsynprog_JavaCJava, scale = %d\n", scale);
    int pnum = 0;
    jmethodID mid = (env)->GetStaticMethodID(obj, "javafunc", "(I)I");
    if (mid == 0) {
	fprintf(stderr, "Can't get jmethodID for \"javafunc\", \"(I)I\"\n");
	return;
    }
    fprintf(stderr, "Calling CallStaticIntMethod, scale = %d\n", scale);
    pnum = (env)->CallStaticIntMethod(obj, mid, scale);
}

JNIEXPORT jint JNICALL
Java_jsynprog_isJVMPI (JNIEnv *env, jclass obj)
{
    char *jvmpi = getenv("SP_COLLECTOR_USE_JVMPI");

    return jvmpi ? 1 : 0;
}
