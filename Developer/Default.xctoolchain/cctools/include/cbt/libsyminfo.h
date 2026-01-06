/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
#include <mach/mach.h>

#ifndef __SymInfoTypes__
typedef void *SymInfoList;
typedef void *SymInfoSymbol;
typedef void *SymInfoDependencies;
#endif // __SymInfoTypes__

/* Creates a SymInfoList structure from a binary */
SymInfoList SymInfoCreate(char *fileName);

/*Access to main structures in the SymInfoList */
SymInfoSymbol *SymInfoGetImports(SymInfoList nmList);
SymInfoSymbol *SymInfoGetExports(SymInfoList nmList);
SymInfoDependencies SymInfoGetLibraryInfo(SymInfoList nmList);
char *SymInfoGetShortName(SymInfoList nmList);

/* Access to data inside the SymInfoSymbol type */
char *SymInfoGetSymbolName(SymInfoSymbol symbol);
const char *SymInfoGetSymbolArch(SymInfoSymbol symbol);
char *SymInfoGetSymbolOrdinal(SymInfoSymbol symbol);
unsigned int SymInfoGetExportCount(SymInfoList nmList);
unsigned int SymInfoGetImportCount(SymInfoList nmList);

/* Access to data inside the SymInfoDependencies type */
char **SymInfoGetSubFrameworks(SymInfoDependencies deps);
char **SymInfoGetSubUmbrellas(SymInfoDependencies deps);
unsigned int SymInfoGetSubUmbrellaCount(SymInfoDependencies deps);
unsigned int SymInfoGetSubFrameworkCount(SymInfoDependencies deps);

/* Functions for freeing the SymInfoList structure */
void SymInfoFree(SymInfoList nmList);
void SymInfoFreeSymbols(SymInfoSymbol symbol);
void SymInfoFreeDependencies(SymInfoDependencies deps);

/* Function for creating SymInfoSymbol */
SymInfoSymbol SymInfoCreateSymbols(char *name,
			  	    char *arch,
			 	    char *ordinal);

/* Function for creating SymInfoDependencies */
SymInfoDependencies SymInfoCreateDependencies(char **subUmbrellas,
				 	      char **subFrameworks,
				    	      int nSubUmbrellas,
				  	      int nSubFrameworks);
