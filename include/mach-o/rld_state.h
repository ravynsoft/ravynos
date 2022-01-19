/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * To allow the debugger to debug programs that used rld on themselves without
 * having the having the programs create a debug file information is maintained
 * in the rld package for the debugger.  This information is two static data
 * items and a static routine.  The information that is maintained is the state
 * of the loaded sets are currently loaded into the program.  The number of
 * object files and their names that make up each set and the resulting address
 * they were loaded at is maintained for each set.  The static data symbol
 * rld_loaded_state points to an array of rld_loaded_state structures that
 * contains the above information.  The static data symbol rld_nloaded_states
 * contains the count of these structures.  When the loaded state is changed
 * the static routine rld_loaded_state_changed() is called.
 *
 * This is the only information in the rld package the debugger is allowed to
 * use.  The debugger sets a break point on the routine rld_loaded_state_changed
 * when it is triped on then it can inspect the rld_loaded_state.  Then using
 * the rld package and the program it is debugging as a base file then it can
 * create the symbols for the loaded sets by doing rld_loads for each set.
 * The debugger uses an undocumented feature of rld_load (intended only for it's
 * use) which is to used the interger value of 1 (RLD_DEBUG_OUTPUT_FILENAME)
 * for the output_filename which causes the symbols to be created and left in
 * memory and not written to a file.
 *
 * When the debugger attaches to a running process there is a window of time
 * where the process could be doing an rld operation and the state in not
 * correct.  The window is shorted to it's minimal time by changing the value
 * of rld_nloaded_states so that that number of states can be safely accessed.
 * There are still small windows where problems can occur.
 */

struct rld_loaded_state {
    char **object_filenames;	/* pointer to an array of file names loaded */
    unsigned long		/*  in this set */
	nobject_filenames;	/* number of file names loaded in this set */
    struct mach_header		/* The address the set was link edited at */
	*header_addr;
};

/* 
 * static unsigned long rld_nloaded_states = 0;
 * static struct rld_loaded_state *rld_loaded_state = NULL;
 *
 * static void rld_loaded_state_changed(void);
 */
#define RLD_NLOADED_STATES	"rld_nloaded_states"
#define RLD_LOADED_STATE	"rld_loaded_state"
#define RLD_LOADED_STATE_CHANGED "rld_loaded_state_changed"

#define RLD_DEBUG_OUTPUT_FILENAME ((char *)1)

/*
 * moninitrld() can be defined in the librld.o library module if it is used or
 * defined as a common in gcrt0.o if the librld.o library module is not used.
 * The library module is passed monaddition() to call when a rld_load() is done
 * and returns a pointer to the routine to get the rld loaded state so it can
 * be written in to the gmon.out file.
 */
extern void (*moninitrld(
    void (* monaddition)(char *lowpc, char *highpc)))
	    (struct rld_loaded_state **rld_loaded_state,
	     unsigned long *rld_nloaded_states);
