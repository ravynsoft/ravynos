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

import java.util.*;
import java.io.*;
import java.text.*;

class jsynprog 
{
    private static String	dir_home;
    private static PrintWriter	log;
    private static double 	pstart, cstart;

    /* JNI calls */
    public  static native double Timer();
    private static native double cTimer();
    private static native double computeSet();
    private static native int JavaJavaC(int np, int scale);
    private static native void JavaCC(int scale);
    private static native void JavaCJava(int scale);
    private static native int isJVMPI();

    public static double testtime = 3.0 * 1e9;

    public static void main (String [] args)
    {
	jsynprog jsyn_obj = new jsynprog();
	Integer ni;
	int scale = 1000;
	String commands = "memalloc.add_int.add_double.has_inner_class" + 
	  ".recurse.recursedeep.bounce.array_op.vector_op.sys_op" +
	  ".jni_JavaJavaC.JavaCC.JavaCJava.Launcher";

	createAcct();
	LoadJNILibrary(args);
        testtime = computeSet();

	/* check for invocation parameter */
	for (int i = 0; i < args.length; i++) {
	    if (args[i].equals("-fast")) {
		scale = 10000;
	    } else if (args[i].equals("-slow")) {
		scale = 1;
	    } else if (args[i].equals("-j") && i + 1 < args.length) {
		commands = args[++i];
	    } else {
		System.err.println("fatal: unexpected argument: " + args[0] );
		System.exit(1);
	    }
	}

	/* large memory allocations, trigger gc */
	Routine rtn = new Routine();
	Sub_Routine sbrt = new Sub_Routine();

	if (commands.indexOf("memalloc") >= 0) {
	    recTime(); 
	    rtn.memalloc(10000, scale);
	    printValue("Routine.memalloc", false);
	}

	/* add integers */
	if (commands.indexOf("add_int") >= 0) {
	    recTime(); 
	    ni = new Integer (rtn.add_int(scale));
	    printValue("Routine.add_int", true);
	}

	/* add double */
	if (commands.indexOf("add_double") >= 0) {
	    recTime(); 
	    Double nd = new Double(rtn.add_double(scale)); 
	    printValue("Routine.add_double", true);
	}

	/* call method in derived class */ 
	if (commands.indexOf("add_int") >= 0) {
	    recTime(); 
	    ni = new Integer (sbrt.add_int(scale));
	    printValue("Sub_Routine.add_int", true);
	}

	/* call method that defines an inner class */ 
	if (commands.indexOf("has_inner_class") >= 0) {
	    recTime(); 
	    Integer[] na = rtn.has_inner_class(scale);
	    printValue("Routine.has_inner_class", true);
	}

	/* recursion */ 
	if (commands.indexOf("recurse") >= 0) {
	    recTime(); 
	    rtn.recurse(0,80, scale);
	    printValue("Routine.recurse", true);
	}

	/* deep recursion */ 
	if (commands.indexOf("recursedeep") >= 0) {
	    recTime(); 
	    rtn.recursedeep(0,500, scale);
	    printValue("<Truncated-stack>", true);
	}

	/* indirect recursion */ 
	if (commands.indexOf("bounce") >= 0) {
	    recTime(); 
	    rtn.bounce(0,20, scale);
	    printValue("Routine.bounce", true);
	}

	/* array operations */ 
	if (commands.indexOf("array_op") >= 0) {
	    recTime(); 
	    rtn.array_op(scale);
	    printValue("Routine.array_op", false);
	}

	/* Vector operations */ 
	if (commands.indexOf("vector_op") >= 0) {
	    recTime(); 
	    rtn.vector_op(scale);
	    printValue("Routine.vector_op", false);
	}

	/* spend time in system calls */ 
	if (commands.indexOf("sys_op") >= 0) {
	    recTime(); 
	    rtn.sys_op(scale);
	    printValue("Routine.sys_op", false);
	}

	/* java->java->c */
	if (commands.indexOf("jni_JavaJavaC") >= 0) {
	    recTime(); 
	    int np = 0;
	    jni_JavaJavaC(np, scale);
	    printValue("jsynprog.jni_JavaJavaC", true);
	}

	/* java->c->c */
	if (commands.indexOf("JavaCC") >= 0) {
	    recTime(); 
	    JavaCC(scale);
	    printValue("jsynprog.JavaCC", true);
	}

	/* java->c->java */
	if (commands.indexOf("JavaCJava") >= 0) {
	    recTime(); 
	    JavaCJava(scale);
	    printValue("jsynprog.JavaCJava", true);
	}
     
     
	/* dynamically loaded classes */
	if (commands.indexOf("Launcher") >= 0) {
	    String java_ver = System.getProperty("java.version");
	    Launcher lnch = new Launcher();
	    String[] params = new String[]{"DynLoadedClass"};
	    recTime();
	    lnch.main(params);
	    printValue("Launcher.main", true);
	}

	System.gc();
   }

   /* 
    ** Create accounting file 
    */
   private static void createAcct() {
	System.out.println ("Directing output to acct file...");
	try {
	   log = new PrintWriter (new FileWriter("jsynprog.acct"), true);
	} catch (IOException ioe) {
	   ioe.printStackTrace();
	   System.err.println("fatal: Cannot create accounting file ");
	   System.exit(1);
	}

	log.println("X\tLWPTime\tCPUTime\tFunction");
   }

   /* 
    ** Print output in acct file 
    */
   private static void printValue (String fname, boolean noignore) {
	double	p_end = Timer();	 // Global.Timer();
	double	c_end = cTimer();	// Global.cTimer();
	double	prog_elapsed = p_end - pstart;
	double	cpu_elapsed = c_end - cstart;
	DecimalFormat     format_decimal = new DecimalFormat("0.000");

	System.out.println("Running " + fname + "; T = " + format_decimal.format(prog_elapsed * 0.000000001)
		+" UCPU = " + format_decimal.format(cpu_elapsed * 0.000000001));
	log.print( (noignore == true?  "X" : "Y")
		+ "\t" + format_decimal.format(prog_elapsed * 0.000000001) + "\t"
		+ format_decimal.format(cpu_elapsed * 0.000000001) + "\t");
	log.println(fname);
   }

   /*
    ** Record intial times
    */
   private static void recTime() {
	pstart = Timer();	 // Global.Timer();
	cstart = cTimer();	// Global.cTimer();
   }

   /*
    ** Load dynamic shared library for JNI
    */
   private static void LoadJNILibrary(String[] args) {

	try {
	   dir_home = (new File(".")).getCanonicalPath();
	} catch (IOException e) {
	   dir_home = "..";
	}
	System.out.println("libpath:"+dir_home);

	// Find which JVM was invoked
	String jvm_format = System.getProperty("java.vm.name"); 
	System.out.println("jvm "+ jvm_format);
 
	try {
	    System.out.println("Loading library.... " + dir_home + "/libcloop.so");
	    System.load(dir_home + "/libcloop.so");
	} catch (UnsatisfiedLinkError e) {
	   System.err.println("fatal: Cannot load shared library " + e);
	   System.exit(1);
	}
   }

   /*
    ** Makes a lot of JNI calls
    */ 
   private static void jni_JavaJavaC(int np, int scale) {
	int ret = 0;
	int jmax = 10000;
	System.out.println("Entering jni_JavaJavaC, scale = " + scale);
	double tEnd = Timer() + testtime;
	do {
	for (int j =0 ; j<jmax; j++) {
	    ret = JavaJavaC(np, scale);
	}
	} while (Timer() < tEnd);
   }

   public static int javafunc (int scale) {
	int jmax = 200*scale;
	int imax = 40;
	int np = 0;
	// System.out.println("Entering javafunc, scale = " + scale);
	double tEnd = Timer() + testtime;
	do { np = 0;
	for (int j =0 ; j<jmax; j++) {
	    for (int i =0 ; i<imax; i++) {
		np = (i%2==0)?np:(np + 1);
	    }
	}
	} while (Timer() < tEnd);
	return np;
   }
}
