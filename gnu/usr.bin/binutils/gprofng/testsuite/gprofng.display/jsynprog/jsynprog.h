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
/* Header for class jsynprog */

#ifndef _Included_jsynprog
#define _Included_jsynprog
#ifdef __cplusplus
extern "C" {
#endif
/* Inaccessible static: dir_home */
/* Inaccessible static: log */
/* Inaccessible static: pstart */
/* Inaccessible static: cstart */
/*
 * Class:     jsynprog
 * Method:    Timer
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_jsynprog_Timer
  (JNIEnv *, jclass);

/*
 * Class:     jsynprog
 * Method:    cTimer
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_jsynprog_cTimer
  (JNIEnv *, jclass);

/*
 * Class:     jsynprog
 * Method:    computeSet
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_jsynprog_computeSet
  (JNIEnv *, jclass);

/*
 * Class:     jsynprog
 * Method:    JavaJavaC
 * Signature: (I, I)I
 */
JNIEXPORT jint JNICALL Java_jsynprog_JavaJavaC
  (JNIEnv *, jclass, jint, int);

/*
 * Class:     jsynprog
 * Method:    JavaCC
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_jsynprog_JavaCC
  (JNIEnv *, jclass, int);

/*
 * Class:     jsynprog
 * Method:    JavaCJava
 * Signature: (I, I)V
 */
JNIEXPORT void JNICALL Java_jsynprog_JavaCJava
  (JNIEnv *, jclass, int);

/*
 * Class:     jsynprog
 * Method:    isJVMPI
 * Signature: (I)V
 */
JNIEXPORT jint JNICALL Java_jsynprog_isJVMPI
  (JNIEnv *, jclass);

#ifdef __cplusplus
}
#endif
#endif
