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

public class Routine implements Intface {

   /* add integers */
   public int add_int (int scale) {
	int 	x = 0;
	int kmax = 100*scale;
	double tEnd = jsynprog.Timer() + jsynprog.testtime;
	do { x = 0;
	for (int k=0; k<kmax;k++) {
	   for (int j=0; j<10000;j++) {
		x = x + 1;
	   }
	}
	} while (jsynprog.Timer() < tEnd);
	return x;
   }

  /* add double */
  public double add_double (int scale) {
	double	y = 0.0;
	int kmax = 1*scale;
	double tEnd = jsynprog.Timer() + jsynprog.testtime;
	do { y = 0.0;
	for (int k=0; k<kmax;k++) {
	   for (int j=0; j<10000;j++) {
		y = y + 1.0;
	   }
	}
	} while (jsynprog.Timer() < tEnd);
	return y;
   }

  /* Use inner class */
  public Integer[] has_inner_class(int scale) {
	class JInner {
	    Integer[] g_int = new Integer[3];
     
	    public Integer[] buildlist(int scale) {
		double tEnd = jsynprog.Timer() + jsynprog.testtime;
		do {
		for (int k=0; k<g_int.length; k++) {
		    int 	x = 0;
		    int imax = 10*scale;
		    for (int i=0; i<imax;i++) {
			for (int j=0; j<10000;j++) {
			    x = x + 1;
			}
		    }
		    g_int[k]=new Integer (x); 
		 } 
		 } while (jsynprog.Timer() < tEnd);
		 return g_int;
	    }
	}
	return ((new JInner()).buildlist(scale));
  }

  public void memalloc (int nsize, int scale) {
	class myobj {
	    int nitem;
	    String shape;
	    String color;

	    myobj() { 
		  nitem = 4;
		  shape = "square";
		  color = "blue";
	    }
	}
	for (int j=0; j<60; j++) {
	   for (int i=0; i<20; i++) {
		myobj[] blueobj = new myobj[1000000]; 
	   }
	}
  } 

  /* routine to do recursion */
  public void recurse(int i, int imax, int scale) {
     if(i == imax) {
	double tEnd = jsynprog.Timer() + jsynprog.testtime;
	do {
	    double x;
	    int j, k;
	    x = 0.0;
	    for(k=0; k<scale; k++) {
		   for(j=0; j<5000000; j++) {
			  x = x + 1.0;
		   }
	    }
	} while (jsynprog.Timer() < tEnd);
     } else {
	    recurse(i+1, imax, scale);
     }
  }

  /* routine to do deep recursion */
  public void recursedeep(int i, int imax, int scale) {
     if(i == imax) {
	  double tEnd = jsynprog.Timer() + jsynprog.testtime;
	  do {
	    double x;
	    int j, k;
	    x = 0.0;
	    for(k=0; k<scale; k++) {
		   for(j=0; j<5000000; j++) {
			  x = x + 1.0;
		   }
	    }
	  } while (jsynprog.Timer() < tEnd);
     } else {
	    recursedeep(i+1, imax, scale);
     }
  }


  /* bounce -- example of indirect recursion */
  public void bounce(int i, int imax, int scale) {
	if(i == imax) {
	  double tEnd = jsynprog.Timer() + jsynprog.testtime;
	  do {
	     double x;
	     int j, k;
		x = 0.0;
		for(k=0; k < scale; k++) {
		    for(j=0; j<5000000; j++) {
			   x = x + 1.0;
		    }
		}
	  } while (jsynprog.Timer() < tEnd);
	} else {
		bounce_b(i, imax, scale);
	}
  }

  private void bounce_b(int i, int imax, int scale) {
	bounce(i+1, imax, scale);
	return;
  }


  /* large array */ 
  public void array_op(int scale) {
	int size = 50000;
	int imax = 1*scale;
	Integer[] y = allocate_array(3*size);
	Integer[] z = allocate_array(size);
	double tEnd = jsynprog.Timer() + jsynprog.testtime;
	do {
	for (int i=0; i<imax; i++) {
	    System.arraycopy(y, 2, z, 0, size);
	}
	} while (jsynprog.Timer() < tEnd);
  }   
  
  /* define large array */ 
  private Integer[] allocate_array(int num) {
	Integer[] x = new Integer[num];
	for (int i=0; i<num;i++) {
	    x[i] = new Integer(i);
	}
	return x;
  }   


  /* large vector */
  public void vector_op(int scale) {
	Vector v = allocate_vector(); 
	int imax = 1*scale;
	int jmax = 1*scale;
	double tEnd = jsynprog.Timer() + jsynprog.testtime;
	do {
	for (int i=0; i<imax; i++) {
	    vrem_last(v);
	}
	for (int j=0; j<jmax; j++) {
	    vrem_first(v);
	}
	} while (jsynprog.Timer() < tEnd);
  }   

  /* define large Vector */ 
  private Vector allocate_vector() {
	Vector<Integer> v1 = new Vector<Integer> (200000);
	for (int i=0; i<1000000;i++) {
	    v1.add(new Integer(i)); 
	}
	return v1;
  }
 
  /* remove last element of vector */ 
  private void vrem_last(Vector v) {
	v.remove(v.size()-1);
  }

  /* remove first element of vector */ 
  private void vrem_first(Vector v) {
	v.remove(0);
  }


  /* Spend time in system calls */
  public void sys_op(int scale) {
	long stime ;
	int jmax = 1000000;
	int imax = 4;
	double tEnd = jsynprog.Timer() + jsynprog.testtime;
	do {
	for (int i = 0; i < imax; i++) {
	    for(int j=0; j<jmax; j++) {
		  stime = System.currentTimeMillis();
	    }
	}
	} while (jsynprog.Timer() < tEnd);
  }

} //end of class
