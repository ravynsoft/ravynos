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

import java.lang.reflect.*;

public class Launcher {
// Byte array for dynamically loaded class:            //
//public class DynLoadedClass {                        //
//   public int DynamicFunction(int x) {               //
//      float f = 0;                                   //
//      for (int k=0 ; k<20000; k++) {                 //
//         f =  ((float)k) / x;                        //
//      }                                              //
//      return (int)f;                                 //
//   }                                                 //
//                                                     //
//    public static void main(String[] args){          //
//        DynLoadedClass dcls = new DynLoadedClass();  //
//        for (int k=0 ; k<10; k++) {                  //
//            dcls.DynamicFunction(k);                 //
//        }                                            //
//    }                                                //
//}                                                    //
static final byte [] bClassGenerated = {
        -54, -2, -70, -66, 0, 0, 0, 46, 0, 20, 10, 0, 5, 0, 16, 7, 0, 17, 10, 0, 2, 0, 16, 10, 0, 2, 
        0, 18, 7, 0, 19, 1, 0, 6, 60, 105, 110, 105, 116, 62, 1, 0, 3, 40, 41, 86, 1, 0, 4, 67, 111, 
        100, 101, 1, 0, 15, 76, 105, 110, 101, 78, 117, 109, 98, 101, 114, 84, 97, 98, 108, 101, 1, 0, 15, 68, 121, 
        110, 97, 109, 105, 99, 70, 117, 110, 99, 116, 105, 111, 110, 1, 0, 4, 40, 73, 41, 73, 1, 0, 4, 109, 97, 
        105, 110, 1, 0, 22, 40, 91, 76, 106, 97, 118, 97, 47, 108, 97, 110, 103, 47, 83, 116, 114, 105, 110, 103, 59, 
        41, 86, 1, 0, 10, 83, 111, 117, 114, 99, 101, 70, 105, 108, 101, 1, 0, 19, 68, 121, 110, 76, 111, 97, 100, 
        101, 100, 67, 108, 97, 115, 115, 46, 106, 97, 118, 97, 12, 0, 6, 0, 7, 1, 0, 14, 68, 121, 110, 76, 111, 
        97, 100, 101, 100, 67, 108, 97, 115, 115, 12, 0, 10, 0, 11, 1, 0, 16, 106, 97, 118, 97, 47, 108, 97, 110, 
        103, 47, 79, 98, 106, 101, 99, 116, 0, 33, 0, 2, 0, 5, 0, 0, 0, 0, 0, 3, 0, 1, 0, 6, 0, 
        7, 0, 1, 0, 8, 0, 0, 0, 29, 0, 1, 0, 1, 0, 0, 0, 5, 42, -73, 0, 1, -79, 0, 0, 0, 
        1, 0, 9, 0, 0, 0, 6, 0, 1, 0, 0, 0, 1, 0, 1, 0, 10, 0, 11, 0, 1, 0, 8, 0, 0, 
        0, 66, 0, 2, 0, 4, 0, 0, 0, 26, 11, 69, 3, 62, 29, 17, 78, 32, -94, 0, 15, 29, -122, 27, -122, 
        110, 69, -124, 3, 1, -89, -1, -16, 36, -117, -84, 0, 0, 0, 1, 0, 9, 0, 0, 0, 22, 0, 5, 0, 0, 
        0, 3, 0, 2, 0, 4, 0, 11, 0, 5, 0, 17, 0, 4, 0, 23, 0, 7, 0, 9, 0, 12, 0, 13, 0, 
        1, 0, 8, 0, 0, 0, 69, 0, 2, 0, 3, 0, 0, 0, 29, -69, 0, 2, 89, -73, 0, 3, 76, 3, 61, 
        28, 16, 10, -94, 0, 15, 43, 28, -74, 0, 4, 87, -124, 2, 1, -89, -1, -15, -79, 0, 0, 0, 1, 0, 9, 
        0, 0, 0, 22, 0, 5, 0, 0, 0, 11, 0, 8, 0, 12, 0, 16, 0, 13, 0, 22, 0, 12, 0, 28, 0, 
        15, 0, 1, 0, 14, 0, 0, 0, 2, 0, 15
    };
    
    private static DynClassLoader persistentInstance;
    
    public static DynClassLoader getPersistentInstance()
    {
        if (persistentInstance == null)
            persistentInstance = new DynClassLoader();
        return persistentInstance;
    }	

    public static void main(String args []) {
        if (args.length != 1) {
	    System.err.println("Usage: Launcher DynLoadedClass");
	    return;
	}

        String className = args[0]; // Dynamic class name

        try {
            Class    genClass  = getPersistentInstance().getClassFromByteArray(className, bClassGenerated);
            Method[] methods_g = genClass.getDeclaredMethods();

            for (int i = 0; i < methods_g.length; i++) {
                Method m = methods_g[i];
                String methodName = m.getName();
		String progArgs[] = new String[1];
		//System.out.println("Invoking method " + className + "." + methodName);
		if (methodName.equals("main"))
		    m.invoke( null, (Object[]) progArgs );
            }
        } catch (InvocationTargetException iex) {
            System.err.println("InvocationTargetException");
        } catch (IllegalAccessException aex) {
            System.err.println("IllegalAccessException");
        }
    }

    // Class loader to generate dynamic class on the fly from the byte array
    private static class DynClassLoader extends ClassLoader {
        public DynClassLoader() { }
        public Class getClassFromByteArray(String name, byte[] b) {
            return super.defineClass(name, b, 0, b.length);
        }
    }


}
