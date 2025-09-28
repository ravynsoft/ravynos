/*-
 * Copyright (c) 2014, Matthew Macy <kmacy@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*
 * Copyright 1991-1998 by Open Software Foundation, Inc.
 *              All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and
 * that both the copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * cmk1.1
 */
/*
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * 92/03/03  16:25:39  jeffreyh
 * 	Changes from TRUNK
 * 	[92/02/26  12:33:02  jeffreyh]
 *
 * 92/01/14  16:47:08  rpd
 * 	Modified WriteTypeDeclIn and WriteTypeDeclOut to disable
 * 	the deallocate flag on Indefinite arguments.
 * 	[92/01/09            rpd]
 *
 * 92/01/03  20:30:51  dbg
 * 	Change argByReferenceUser and argByReferenceServer to fields in
 * 	argument_t.
 * 	[91/08/29            dbg]
 *
 * 91/07/31  18:11:45  dbg
 * 	Accept new dealloc_t argument type in WriteStaticDecl,
 * 	WritePackMsgType.
 *
 * 	Don't need to zero last character of C string.  Mig_strncpy does
 * 	the proper work.
 *
 * 	Add SkipVFPrintf, so that WriteCopyType doesn't print fields in
 * 	comments.
 * 	[91/07/17            dbg]
 *
 * 91/06/25  10:32:36  rpd
 * 	Changed WriteVarDecl to WriteUserVarDecl.
 * 	Added WriteServerVarDecl.
 * 	[91/05/23            rpd]
 *
 * 91/02/05  17:56:28  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:56:39  mrt]
 *
 * 90/06/02  15:06:11  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:14:54  rpd]
 *
 * 07-Apr-89  Richard Draves (rpd) at Carnegie-Mellon University
 *	Extensive revamping.  Added polymorphic arguments.
 *	Allow multiple variable-sized inline arguments in messages.
 *
 * 21-Aug-87  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Added deallocflag to the WritePackMsg routines.
 *
 * 29-Jul-87  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Changed WriteVarDecl to not automatically write
 *	semi-colons between items, so that it can be
 *	used to write C++ argument lists.
 *
 * 27-May-87  Richard Draves (rpd) at Carnegie-Mellon University
 *	Created.
 */
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "type.h"

#include <mach/message.h>
#include "routine.h"
#include "write.h"
#include "global.h"
#include "routine.h"
#include "utils.h"
#include "alloc.h"


void
WriteIdentificationString(FILE * file)
{

    fprintf(file, "/*\n");
    fprintf(file, " * IDENTIFICATION:\n");
    fprintf(file, " * stub generated %s", GenerationDate);
    fprintf(file, " * with a MiG generated %s by %s\n", MigGenerationDate, MigMoreData);
    fprintf(file, " * OPTIONS: \n");
    if (IsKernelUser)
        fprintf(file, " *\tKernelUser\n");
    if (IsKernelServer)
        fprintf(file, " *\tKernelServer\n");
    if (!UseMsgRPC)
        fprintf(file, " *\t-R (no RPC calls)\n");
    fprintf(file, " */\n");
}

void
WriteMigExternal(FILE * file)
{
    fprintf(file, "#ifdef\tmig_external\n");
    fprintf(file, "mig_external\n");
    fprintf(file, "#else\n");
    fprintf(file, "extern\n");
    fprintf(file, "#endif\t/* mig_external */\n");
}

void
WriteMigInternal(FILE * file)
{
    fprintf(file, "#ifdef\tmig_internal\n");
    fprintf(file, "mig_internal\n");
    fprintf(file, "#else\n");
    fprintf(file, "static\n");
    fprintf(file, "#endif\t/* mig_internal */\n");
}

void
WriteImport(FILE * file, string_t filename)
{
    fprintf(file, "#include %s\n", filename);
}

void
WriteImplImports(FILE * file, statement_t * stats, boolean_t isuser)
{
    register statement_t * stat;

    for (stat = stats; stat != stNULL; stat = stat->stNext)
        switch (stat->stKind) {
        case skImport:
        case skIImport:
            WriteImport(file, stat->stFileName);
            break;
        case skSImport:
            if (!isuser)
                WriteImport(file, stat->stFileName);
            break;
        case skUImport:
            if (isuser)
                WriteImport(file, stat->stFileName);
            break;
        case skRoutine:
        case skDImport:
            break;
        default:
            printf("WriteImplImport(): bad statement_kind_t (%d)",
                   (int)stat->stKind);
            abort();
        }
}

void
WriteRCSDecl(FILE * file, identifier_t name, string_t rcs)
{
    fprintf(file, "#ifndef\tlint\n");
    fprintf(file, "#if\tUseExternRCSId\n");
    fprintf(file, "%s char %s_rcsid[] = %s;\n", (BeAnsiC) ? "const" : "", name, rcs);
    fprintf(file, "#else\t/* UseExternRCSId */\n");
    fprintf(file, "static %s char rcsid[] = %s;\n", (BeAnsiC) ? "const" : "", rcs);
    fprintf(file, "#endif\t/* UseExternRCSId */\n");
    fprintf(file, "#endif\t/* lint */\n");
    fprintf(file, "\n");
}

static void
WriteOneApplDefault(FILE * file, const char *word1, const char *word2, const char *word3)
{
    char            buf[50];

    sprintf(buf, "__%s%s%s", word1, word2, word3);
    fprintf(file, "#ifndef\t%s\n", buf);
    fprintf(file, "#define\t%s(_NUM_, _NAME_)\n", buf);
    fprintf(file, "#endif\t/* %s */\n", buf);
    fprintf(file, "\n");
}

void
WriteApplDefaults(FILE * file, const char *dir)
{
    WriteOneApplDefault(file, "Declare", dir, "Rpc");
    WriteOneApplDefault(file, "Before", dir, "Rpc");
    WriteOneApplDefault(file, "After", dir, "Rpc");
    WriteOneApplDefault(file, "Declare", dir, "Simple");
    WriteOneApplDefault(file, "Before", dir, "Simple");
    WriteOneApplDefault(file, "After", dir, "Simple");
}

void
WriteApplMacro(FILE * file, const char *dir, const char *when, routine_t * rt)
{
    const char     *what = (rt->rtOneWay) ? "Simple" : "Rpc";

    fprintf(file, "\t__%s%s%s(%d, \"%s\")\n",
            when, dir, what, SubsystemBase + rt->rtNumber, rt->rtName);
}

void
WriteBogusDefines(FILE * file)
{
    fprintf(file, "#ifndef\tmig_internal\n");
    fprintf(file, "#define\tmig_internal\tstatic __inline__\n");
    fprintf(file, "#endif\t/* mig_internal */\n");
    fprintf(file, "\n");

    fprintf(file, "#ifndef\tmig_external\n");
    fprintf(file, "#define mig_external\n");
    fprintf(file, "#endif\t/* mig_external */\n");
    fprintf(file, "\n");

    fprintf(file, "#if\t!defined(__MigTypeCheck) && defined(TypeCheck)\n");
    fprintf(file, "#define\t__MigTypeCheck\t\tTypeCheck\t/* Legacy setting */\n");
    fprintf(file, "#endif\t/* !defined(__MigTypeCheck) */\n");
    fprintf(file, "\n");

    fprintf(file, "#if\t!defined(__MigKernelSpecificCode) && defined(_MIG_KERNEL_SPECIFIC_CODE_)\n");
    fprintf(file, "#define\t__MigKernelSpecificCode\t_MIG_KERNEL_SPECIFIC_CODE_\t/* Legacy setting */\n");
    fprintf(file, "#endif\t/* !defined(__MigKernelSpecificCode) */\n");
    fprintf(file, "\n");

    fprintf(file, "#ifndef\tLimitCheck\n");
    fprintf(file, "#define\tLimitCheck 0\n");
    fprintf(file, "#endif\t/* LimitCheck */\n");
    fprintf(file, "\n");

    fprintf(file, "#ifndef\tmin\n");
    fprintf(file, "#define\tmin(a,b)  ( ((a) < (b))? (a): (b) )\n");
    fprintf(file, "#endif\t/* min */\n");
    fprintf(file, "\n");

    fprintf(file, "#if !defined(_WALIGN_)\n");
    fprintf(file, "#define _WALIGN_(x) (((x) + %d) & ~%d)\n", (int)(itWordAlign - 1), (int)(itWordAlign - 1));
    fprintf(file, "#endif /* !defined(_WALIGN_) */\n");
    fprintf(file, "\n");

    fprintf(file, "#if !defined(_WALIGNSZ_)\n");
    fprintf(file, "#define _WALIGNSZ_(x) _WALIGN_(sizeof(x))\n");
    fprintf(file, "#endif /* !defined(_WALIGNSZ_) */\n");
    fprintf(file, "\n");

    fprintf(file, "#ifndef\tUseStaticTemplates\n");
    if (BeAnsiC) {
        fprintf(file, "#define\tUseStaticTemplates\t1\n");
    } else {
        fprintf(file, "#if\t%s\n", NewCDecl);
        fprintf(file, "#define\tUseStaticTemplates\t1\n");
        fprintf(file, "#endif\t/* %s */\n", NewCDecl);
    }
    fprintf(file, "#endif\t/* UseStaticTemplates */\n");
    fprintf(file, "\n");

    fprintf(file, "#define _WALIGN_(x) (((x) + %ld) & ~%ld)\n",
            itWordAlign - 1, itWordAlign - 1);
    fprintf(file, "#define _WALIGNSZ_(x) _WALIGN_(sizeof(x))\n");
}

void
                WriteList(FILE * file, argument_t * args, void (*func) (), u_int mask, const char *between, const char *after){
    argument_t     *arg;
    boolean_t       sawone = FALSE;

    for (arg = args; arg != argNULL; arg = arg->argNext) {
        if (akCheckAll(arg->argKind, mask)) {
            if (sawone)
                fprintf(file, "%s", between);
            sawone = TRUE;

            (*func) (file, arg);
        }
    }
    if (sawone)
        fprintf(file, "%s", after);
}

static boolean_t
WriteReverseListPrim(FILE * file, argument_t * arg, void (*func) (), u_int mask, const char *between){
    boolean_t       sawone = FALSE;

    if (arg != argNULL) {
        sawone = WriteReverseListPrim(file, arg->argNext, func, mask, between);

        if (akCheckAll(arg->argKind, mask)) {
            if (sawone)
                fprintf(file, "%s", between);
            sawone = TRUE;

            (*func) (file, arg);
        }
    }

    return sawone;
}

void
                WriteReverseList(FILE * file, argument_t * args, void (*func) (), u_int mask, const char *between, const char *after){
    boolean_t       sawone;

    sawone = WriteReverseListPrim(file, args, func, mask, between);

    if (sawone)
        fprintf(file, "%s", after);
}

void
WriteNameDecl(FILE * file, argument_t * arg)
{
    fprintf(file, "%s", arg->argVarName);
}

void
WriteUserVarDecl(FILE * file, argument_t * arg)
{
    boolean_t       pointer = (arg->argByReferenceUser || arg->argType->itNativePointer);
    const char     *ref = (pointer) ? "*" : "";
    const char     *cnst = ((arg->argFlags & flConst) &&
                            (IS_VARIABLE_SIZED_UNTYPED(arg->argType) ||
     arg->argType->itNoOptArray || arg->argType->itString)) ? "const " : "";

    fprintf(file, "\t%s%s %s%s", cnst, arg->argType->itUserType, ref, arg->argVarName);
}

void
WriteServerVarDecl(FILE * file, argument_t * arg)
{
    const char     *ref = (arg->argByReferenceServer ||
                           arg->argType->itNativePointer) ? "*" : "";
    const char     *cnst = ((arg->argFlags & flConst) &&
                            (IS_VARIABLE_SIZED_UNTYPED(arg->argType) ||
     arg->argType->itNoOptArray || arg->argType->itString)) ? "const " : "";

    fprintf(file, "\t%s%s %s%s", cnst, arg->argType->itTransType, ref, arg->argVarName);
}

const char *
ReturnTypeStr(routine_t * rt)
{
    return rt->rtRetCode->argType->itUserType;
}

const char *
FetchUserType(ipc_type_t * it)
{
    return it->itUserType;
}

const char *
FetchServerType(ipc_type_t * it)
{
    return it->itServerType;
}

const char *
FetchKPDType(ipc_type_t * it)
{
    return it->itKPDType;
}

static void
WriteTrailerDecl(FILE * file, boolean_t trailer)
{
    if (trailer)
        fprintf(file, "\t\tmach_msg_max_trailer_t trailer;\n");
    else
        fprintf(file, "\t\tmach_msg_trailer_t trailer;\n");
}

void
WriteFieldDeclPrim(FILE * file, argument_t * arg, const char *(*tfunc) (ipc_type_t *))
{
    register ipc_type_t * it = arg->argType;

    if (IS_VARIABLE_SIZED_UNTYPED(it) || it->itNoOptArray) {
        register argument_t * count = arg->argCount;
        register ipc_type_t * btype = it->itElement;

        /*
         * Build our own declaration for a varying array: use the element
         * type and maximum size specified. Note arg->argCount->argMultiplier
         * == btype->itNumber.
         */
        /*
         * NDR encoded VarStrings requires the offset field. Since it is not
         * used, it wasn't worthwhile to create an extra parameter
         */
        if (it->itString)
            fprintf(file, "\t\t%s %sOffset; /* MiG doesn't use it */\n",
                    (*tfunc) (count->argType), arg->argName);

        if (!(arg->argFlags & flSameCount) && !it->itNoOptArray)
            /* in these cases we would have a count, which we don't want */
            fprintf(file, "\t\t%s %s;\n", (*tfunc) (count->argType),
                    count->argMsgField);
        fprintf(file, "\t\t%s %s[%d];",
                (*tfunc) (btype),
                arg->argMsgField,
                it->itNumber / btype->itNumber);
    } else if (IS_MULTIPLE_KPD(it))
        fprintf(file, "\t\t%s %s[%d];", (*tfunc) (it), arg->argMsgField,
                it->itKPD_Number);
    else if (IS_OPTIONAL_NATIVE(it)) {
        fprintf(file, "\t\tboolean_t __Present__%s;\n", arg->argMsgField);
        fprintf(file, "\t\tunion {\n");
        fprintf(file, "\t\t    %s __Real__%s;\n",
                (*tfunc) (it), arg->argMsgField);
        fprintf(file, "\t\t    char __Phony__%s[_WALIGNSZ_(%s)];\n",
                arg->argMsgField, (*tfunc) (it));
        fprintf(file, "\t\t} %s;", arg->argMsgField);
    } else {
        /* either simple KPD or simple in-line */
        fprintf(file, "\t\t%s %s;", (*tfunc) (it), arg->argMsgField);
    }

    /* Kernel Processed Data has always PadSize = 0 */
    if (it->itPadSize != 0)
        fprintf(file, "\n\t\tchar %s[%d];", arg->argPadName, it->itPadSize);
}

void
WriteKPDFieldDecl(FILE * file, argument_t * arg)
{
    if (akCheck(arg->argKind, akbSendKPD) ||
        akCheck(arg->argKind, akbReturnKPD))
        WriteFieldDeclPrim(file, arg, FetchKPDType);
    else
        WriteFieldDeclPrim(file, arg, FetchServerType);
}

void
                WriteStructDecl(FILE * file, argument_t * args, void (*func) (), u_int mask,
                      const char *name, boolean_t simple, boolean_t trailer,
                              boolean_t trailer_t, boolean_t template_only){
    fprintf(file, "\n#ifdef  __MigPackStructs\n#pragma pack(%lu)\n#endif\n", sizeof(natural_t));
    fprintf(file, "\ttypedef struct {\n");
    fprintf(file, "\t\tmach_msg_header_t Head;\n");
    if (simple == FALSE) {
        fprintf(file, "\t\t/* start of the kernel processed data */\n");
        fprintf(file, "\t\tmach_msg_body_t msgh_body;\n");
        if (mask == akbRequest)
            WriteList(file, args, func, mask | akbSendKPD, "\n", "\n");
        else
            WriteList(file, args, func, mask | akbReturnKPD, "\n", "\n");
        fprintf(file, "\t\t/* end of the kernel processed data */\n");
    }
    if (!template_only) {
        if (mask == akbRequest)
            WriteList(file, args, func, mask | akbSendBody, "\n", "\n");

        else
            WriteList(file, args, func, mask | akbReturnBody, "\n", "\n");
        if (trailer)
            WriteTrailerDecl(file, trailer_t);
    }
    fprintf(file, "\t} %s;\n", name);
    fprintf(file, "#ifdef  __MigPackStructs\n#pragma pack()\n#endif\n");
}

void
WriteTemplateDeclIn(FILE * file, register argument_t * arg)
{
    (*arg->argKPD_Template) (file, arg, TRUE);
}

void
WriteTemplateDeclOut(FILE * file, register argument_t * arg)
{
    (*arg->argKPD_Template) (file, arg, FALSE);
}

void
WriteTemplateKPD_port(FILE * file, argument_t * arg, boolean_t in)
{
    register ipc_type_t * it = arg->argType;

    fprintf(file, "#if\tUseStaticTemplates\n");
    fprintf(file, "\tconst static %s %s = {\n", it->itKPDType, arg->argTTName);

    fprintf(file, "\t\t.name = MACH_PORT_NULL,\n");
    fprintf(file, "\t\t.disposition = %s,\n", in ? it->itInNameStr : it->itOutNameStr);
    fprintf(file, "\t\t.type = MACH_MSG_PORT_DESCRIPTOR,\n");

    fprintf(file, "\t};\n");
    fprintf(file, "#endif\t/* UseStaticTemplates */\n");
}

void
WriteTemplateKPD_ool(FILE * file, argument_t * arg, boolean_t in __unused)
{
    register ipc_type_t * it = arg->argType;

    fprintf(file, "#if\tUseStaticTemplates\n");
    fprintf(file, "\tconst static %s %s = {\n", it->itKPDType, arg->argTTName);

    if (IS_MULTIPLE_KPD(it))
        it = it->itElement;

    fprintf(file, "\t\t.address = (void *)0,\n");
    if (it->itVarArray)
        fprintf(file, "\t\t.size = 0,\n");
    else
        fprintf(file, "\t\t.size = %d,\n",
                (it->itNumber * it->itSize + 7) / 8);
    fprintf(file, "\t\t.deallocate = %s,\n",
            (arg->argDeallocate == d_YES) ? "TRUE" : "FALSE");
    /* the d_MAYBE case will be fixed runtime */
    fprintf(file, "\t\t.copy = %s,\n",
            (arg->argFlags & flPhysicalCopy) ? "MACH_MSG_PHYSICAL_COPY" : "MACH_MSG_VIRTUAL_COPY");
    /* the PHYSICAL COPY flag has not been established yet */
    fprintf(file, "\t\t.type = MACH_MSG_OOL_DESCRIPTOR,\n");

    fprintf(file, "\t};\n");
    fprintf(file, "#endif\t/* UseStaticTemplates */\n");
}

void
WriteTemplateKPD_oolport(FILE * file, argument_t * arg, boolean_t in)
{
    register ipc_type_t * it = arg->argType;

    fprintf(file, "#if\tUseStaticTemplates\n");
    fprintf(file, "\tconst static %s %s = {\n", it->itKPDType, arg->argTTName);

    if (IS_MULTIPLE_KPD(it))
        it = it->itElement;

    fprintf(file, "\t\t.address = (void *)0,\n");
    if (!it->itVarArray)
        fprintf(file, "\t\t.count = %d,\n",
                it->itNumber);
    else
        fprintf(file, "\t\t.count = 0,\n");
    fprintf(file, "\t\t.deallocate = %s,\n",
            (arg->argDeallocate == d_YES) ? "TRUE" : "FALSE");
    fprintf(file, "\t\t/* copy is meaningful only in overwrite mode */\n");
    fprintf(file, "\t\t.copy = MACH_MSG_PHYSICAL_COPY,\n");
    fprintf(file, "\t\t.disposition = %s,\n",
            in ? it->itInNameStr : it->itOutNameStr);
    fprintf(file, "\t\t.type = MACH_MSG_OOL_PORTS_DESCRIPTOR,\n");

    fprintf(file, "\t};\n");
    fprintf(file, "#endif\t/* UseStaticTemplates */\n");
}

void
WriteReplyTypes(FILE * file, statement_t * stats)
{
    register statement_t * stat;

    fprintf(file, "/* typedefs for all replies */\n\n");
    fprintf(file, "#ifndef __Reply__%s_subsystem__defined\n", SubsystemName);
    fprintf(file, "#define __Reply__%s_subsystem__defined\n", SubsystemName);
    for (stat = stats; stat != stNULL; stat = stat->stNext) {
        if (stat->stKind == skRoutine) {
            register routine_t * rt;
            char            str[MAX_STR_LEN];

            rt = stat->stRoutine;
            sprintf(str, "__Reply__%s_t", rt->rtName);
            WriteStructDecl(file, rt->rtArgs, WriteKPDFieldDecl, akbReply, str, rt->rtSimpleReply, FALSE, FALSE, FALSE);
        }
    }
    fprintf(file, "#endif /* !__Reply__%s_subsystem__defined */\n", SubsystemName);
    fprintf(file, "\n");
}

void
WriteRequestTypes(FILE * file, statement_t * stats)
{
    register statement_t * stat;

    fprintf(file, "/* typedefs for all requests */\n\n");
    fprintf(file, "#ifndef __Request__%s_subsystem__defined\n", SubsystemName);
    fprintf(file, "#define __Request__%s_subsystem__defined\n", SubsystemName);
    for (stat = stats; stat != stNULL; stat = stat->stNext) {
        if (stat->stKind == skRoutine) {
            register routine_t * rt;
            char            str[MAX_STR_LEN];

            rt = stat->stRoutine;
            sprintf(str, "__Request__%s_t", rt->rtName);
            WriteStructDecl(file, rt->rtArgs, WriteKPDFieldDecl, akbRequest, str, rt->rtSimpleRequest, FALSE, FALSE, FALSE);
        }
    }
    fprintf(file, "#endif /* !__Request__%s_subsystem__defined */\n", SubsystemName);
    fprintf(file, "\n");
}

void
WriteNDRConvertArgDecl(FILE * file, argument_t * arg, const char *convert, const char *dir)
{
    argument_t     *count = arg->argCount;
    argument_t     *parent = arg->argParent;
    const char     *carg = (count) ? ", c" : "";
    routine_t      *rt = arg->argRoutine;
    ipc_type_t     *ptype = arg->argType;
    ipc_type_t     *btype;
    int             multi, array;
    char            domain[MAX_STR_LEN];

    fprintf(file, "#ifndef __NDR_convert__%s__%s__%s_t__%s__defined\n#", convert, dir, rt->rtName, arg->argMsgField);

    for (btype = ptype, multi = (!parent) ? arg->argMultiplier : 1, array = 0;
         btype;
      ptype = btype, array += ptype->itVarArray, btype = btype->itElement) {
        const char     *bttype;

        if (btype->itNumber < ptype->itNumber && !ptype->itVarArray && !parent) {
            multi *= ptype->itNumber / btype->itNumber;
            if (!btype->itString)
                continue;
        } else if (array && ptype->itVarArray)
            continue;
        if (btype != ptype)
            fprintf(file, "#el");

        bttype = (multi > 1 && btype->itString) ? "string" : FetchServerType(btype);
        sprintf(domain, "__%s", SubsystemName);
        do {
            fprintf(file, "if\tdefined(__NDR_convert__%s%s__%s__defined)\n", convert, domain, bttype);
            fprintf(file, "#define\t__NDR_convert__%s__%s__%s_t__%s__defined\n", convert, dir, rt->rtName, arg->argMsgField);
            fprintf(file, "#define\t__NDR_convert__%s__%s__%s_t__%s(a, f%s) \\\n\t", convert, dir, rt->rtName, arg->argMsgField, carg);
            if (multi > 1) {
                if (array) {
                    if (btype->itString)
                        fprintf(file, "__NDR_convert__2DARRAY((%s *)(a), f, %d, c, ", bttype, multi);
                    else
                        fprintf(file, "__NDR_convert__ARRAY((%s *)(a), f, %d * (c), ", bttype, multi);
                } else if (!btype->itString)
                    fprintf(file, "__NDR_convert__ARRAY((%s *)(a), f, %d, ", bttype, multi);
            } else if (array)
                fprintf(file, "__NDR_convert__ARRAY((%s *)(a), f, c, ", bttype);
            fprintf(file, "__NDR_convert__%s%s__%s", convert, domain, bttype);
            if (multi > 1) {
                if (!array && btype->itString)
                    fprintf(file, "(a, f, %d", multi);
            } else if (!array)
                fprintf(file, "((%s *)(a), f%s", bttype, carg);
            fprintf(file, ")\n");
        } while (strcmp(domain, "") && (domain[0] = '\0', fprintf(file, "#el")));
    }
    fprintf(file, "#endif /* defined(__NDR_convert__*__defined) */\n");
    fprintf(file, "#endif /* __NDR_convert__%s__%s__%s_t__%s__defined */\n\n", convert, dir, rt->rtName, arg->argMsgField);
}

/*
 * Like vfprintf, but omits a leading comment in the format string
 * and skips the items that would be printed by it.  Only %s, %d,
 * and %f are recognized.
 */
static void
SkipVFPrintf(FILE * file, const char *fmt, va_list pvar)
{
    if (*fmt == 0)
        return;                 /* degenerate case */

    if (fmt[0] == '/' && fmt[1] == '*') {
        /*
         * Format string begins with C comment.  Scan format string until
         * end-comment delimiter, skipping the items in pvar that the
         * enclosed format items would print.
         */

        register int    c;

        fmt += 2;
        for (;;) {
            c = *fmt++;
            if (c == 0)
                return;         /* nothing to format */
            if (c == '*') {
                if (*fmt == '/') {
                    break;
                }
            } else if (c == '%') {
                /* Field to skip */
                c = *fmt++;
                switch (c) {

                case 's':
                    (void)va_arg(pvar, char *);
                    break;

                case 'd':
                    (void)va_arg(pvar, int);
                    break;

                case 'f':
                    (void)va_arg(pvar, double);
                    break;

                case '\0':
                    return;     /* error - fmt ends with '%' */

                default:
                    break;
                }
            }
        }
        /*
         * End of comment.  To be pretty, skip the space that follows.
         */
        fmt++;
        if (*fmt == ' ')
            fmt++;
    }

    /* Now format the string. */
    (void)vfprintf(file, fmt, pvar);
}

static void
vWriteCopyType(FILE * file, ipc_type_t * it, const char *left, const char *right, va_list pvar)
{
    va_list         pv2;
    va_copy(pv2, pvar);
    if (it->itStruct) {
        fprintf(file, "\t");
        (void)SkipVFPrintf(file, left, pvar);
        fprintf(file, " = ");
        (void)SkipVFPrintf(file, right, pv2);
        fprintf(file, ";\n");
    } else if (it->itString) {
        fprintf(file, "\t(void) mig_strncpy(");
        (void)SkipVFPrintf(file, left, pvar);
        fprintf(file, ", ");
        (void)SkipVFPrintf(file, right, pv2);
        fprintf(file, ", %d);\n", it->itTypeSize);
    } else {
        fprintf(file, "\t{   typedef struct { char data[%d]; } *sp;\n",
                it->itTypeSize);
        fprintf(file, "\t    * (sp) ");
        (void)SkipVFPrintf(file, left, pvar);
        fprintf(file, " = * (sp) ");
        (void)SkipVFPrintf(file, right, pv2);
        fprintf(file, ";\n\t}\n");
    }

    va_end(pv2);
}


/*ARGSUSED*/
/*VARARGS4*/
void
WriteCopyType(FILE * file, ipc_type_t * it, const char *left, const char *right,...)
{
    va_list         pvar;
    va_start(pvar, right);

    vWriteCopyType(file, it, left, right, pvar);

    va_end(pvar);
}


/*ARGSUSED*/
/*VARARGS4*/
void
WriteCopyArg(FILE * file, argument_t * arg, const char *left, const char *right,...)
{
    va_list         pvar;
    va_start(pvar, right);

    {
        ipc_type_t     *it = arg->argType;
        if (it->itVarArray && !it->itString) {
            fprintf(file, "\t    (void)memcpy(");
            (void)SkipVFPrintf(file, left, pvar);
            va_end(pvar);
            fprintf(file, ", ");
            va_start(pvar, right);
            (void)SkipVFPrintf(file, right, pvar);
            fprintf(file, ", %s);\n", arg->argCount->argVarName);
        } else
            vWriteCopyType(file, it, left, right, pvar);
    }

    va_end(pvar);
}


/*
 * Global KPD disciplines
 */
void
KPD_error(FILE * file __unused, argument_t * arg)
{
    printf("MiG internal error: argument is %s\n", arg->argVarName);
    exit(1);
}

void
KPD_noop(FILE * file __unused, argument_t * arg __unused)
{
}

static void
WriteStringDynArgs(args, mask, InPOutP, str_oolports, str_ool)
argument_t * args;
    u_int           mask;
    string_t        InPOutP;
    string_t       *str_oolports, *str_ool;
{
    argument_t     *arg;
    char            loc[100], sub[20];
    string_t        tmp_str1 = "";
    string_t        tmp_str2 = "";
    int             cnt, multiplier = 1;
    boolean_t       test, complex = FALSE;

    for (arg = args; arg != argNULL; arg = arg->argNext) {
        ipc_type_t     *it = arg->argType;

        if (IS_MULTIPLE_KPD(it)) {
            test = it->itVarArray || it->itElement->itVarArray;
            if (test) {
                multiplier = it->itKPD_Number;
                it = it->itElement;
                complex = TRUE;
            }
        } else
            test = it->itVarArray;

        cnt = multiplier;
        while (cnt) {
            if (complex)
                sprintf(sub, "[%d]", multiplier - cnt);
            if (akCheck(arg->argKind, mask) &&
                it->itPortType && !it->itInLine && test) {
                sprintf(loc, " + %s->%s%s.count", InPOutP, arg->argMsgField,
                        complex ? sub : "");
                tmp_str1 = strconcat(tmp_str1, loc);
            }
            if (akCheck(arg->argKind, mask) &&
                !it->itInLine && !it->itPortType && test) {
                sprintf(loc, " + %s->%s%s.size", InPOutP, arg->argMsgField,
                        complex ? sub : "");
                tmp_str2 = strconcat(tmp_str2, loc);
            }
            cnt--;
        }
    }
    *str_oolports = tmp_str1;
    *str_ool = tmp_str2;
}

/*
 * Utilities for Logging Events that happen at the stub level
 */
void
WriteLogMsg(FILE * file, routine_t * rt, boolean_t where, boolean_t what)
{
    string_t        ptr_str;
    string_t        StringOolPorts = strNULL;
    string_t        StringOOL = strNULL;
    u_int           ports, oolports, ool;
    string_t        event;

    fprintf(file, "\n#if  MIG_DEBUG\n");
    if (where == LOG_USER)
        fprintf(file, "\tLOG_TRACE(MACH_MSG_LOG_USER,\n");
    else
        fprintf(file, "\tLOG_TRACE(MACH_MSG_LOG_SERVER,\n");
    if (where == LOG_USER && what == LOG_REQUEST) {
        ptr_str = "InP";
        event = "MACH_MSG_REQUEST_BEING_SENT";
    } else if (where == LOG_USER && what == LOG_REPLY) {
        ptr_str = "Out0P";
        event = "MACH_MSG_REPLY_BEING_RCVD";
    } else if (where == LOG_SERVER && what == LOG_REQUEST) {
        ptr_str = "In0P";
        event = "MACH_MSG_REQUEST_BEING_RCVD";
    } else {
        ptr_str = "OutP";
        event = "MACH_MSG_REPLY_BEING_SENT";
    }
    WriteStringDynArgs(rt->rtArgs,
                       (what == LOG_REQUEST) ? akbSendKPD : akbReturnKPD,
                       ptr_str, &StringOolPorts, &StringOOL);
    fprintf(file, "\t\t%s,\n", event);
    fprintf(file, "\t\t%s->Head.msgh_id,\n", ptr_str);
    if (where == LOG_USER && what == LOG_REQUEST) {
        if (rt->rtNumRequestVar)
            fprintf(file, "\t\tmsgh_size,\n");
        else
            fprintf(file, "\t\tsizeof(Request),\n");
    } else
        fprintf(file, "\t\t%s->Head.msgh_size,\n", ptr_str);
    if ((what == LOG_REQUEST && rt->rtSimpleRequest == FALSE) ||
        (what == LOG_REPLY && rt->rtSimpleReply == FALSE))
        fprintf(file, "\t\t%s->msgh_body.msgh_descriptor_count,\n", ptr_str);
    else
        fprintf(file, "\t\t0, /* Kernel Proc. Data entries */\n");
    if (what == LOG_REQUEST) {
        fprintf(file, "\t\t0, /* RetCode */\n");
        ports = rt->rtCountPortsIn;
        oolports = rt->rtCountOolPortsIn;
        ool = rt->rtCountOolIn;
    } else {
        if (akCheck(rt->rtRetCode->argKind, akbReply))
            fprintf(file, "\t\t%s->RetCode,\n", ptr_str);
        else
            fprintf(file, "\t\t0, /* RetCode */\n");
        ports = rt->rtCountPortsOut;
        oolports = rt->rtCountOolPortsOut;
        ool = rt->rtCountOolOut;
    }
    fprintf(file, "\t\t/* Ports */\n");
    fprintf(file, "\t\t%d,\n", ports);
    fprintf(file, "\t\t/* Out-of-Line Ports */\n");
    fprintf(file, "\t\t%d", oolports);
    if (StringOolPorts != strNULL)
        fprintf(file, "%s,\n", StringOolPorts);
    else
        fprintf(file, ",\n");
    fprintf(file, "\t\t/* Out-of-Line Bytes */\n");
    fprintf(file, "\t\t%d", ool);
    if (StringOOL != strNULL)
        fprintf(file, "%s,\n", StringOOL);
    else
        fprintf(file, ",\n");
    fprintf(file, "\t\t__FILE__, __LINE__);\n");
    fprintf(file, "#endif /* MIG_DEBUG */\n\n");
}

void
WriteLogDefines(FILE * file, string_t who)
{
    fprintf(file, "#if  MIG_DEBUG\n");
    fprintf(file, "#define LOG_W_E(X)\tLOG_ERRORS(%s, \\\n", who);
    fprintf(file, "\t\t\tMACH_MSG_ERROR_WHILE_PARSING, (void *)(X), __FILE__, __LINE__)\n");
    fprintf(file, "#else  /* MIG_DEBUG */\n");
    fprintf(file, "#define LOG_W_E(X)\n");
    fprintf(file, "#endif /* MIG_DEBUG */\n");
    fprintf(file, "\n");
}

/* common utility to report errors */
void
WriteReturnMsgError(FILE * file, routine_t * rt, boolean_t isuser, argument_t * arg, string_t error)
{
    char            space[MAX_STR_LEN];
    char           *string = &space[0];

    if (UseEventLogger && arg != argNULL)
        sprintf(string, "LOG_W_E(\"%s\"); ", arg->argVarName);
    else
        string = __DECONST(char *, "");

    fprintf(file, "\t\t{ ");

    if (isuser) {
        if (!rtMessOnStack(rt))
            fprintf(file, "%s((char *) Mess, sizeof(*Mess)); ", MessFreeRoutine);

        fprintf(file, "%sreturn %s; }\n", string, error);
    } else
        fprintf(file, "%sMIG_RETURN_ERROR(OutP, %s); }\n", string, error);
}

/* executed iff elements are defined */
void
WriteCheckTrailerHead(FILE * file, routine_t * rt __unused, boolean_t isuser)
{
    string_t        who = (isuser) ? "Out0P" : "In0P";

    fprintf(file, "\tTrailerP = (mach_msg_max_trailer_t *)((vm_offset_t)%s +\n", who);
    fprintf(file, "\t\tround_msg(%s->Head.msgh_size));\n", who);
    fprintf(file, "\tif (TrailerP->msgh_trailer_type != MACH_MSG_TRAILER_FORMAT_0)\n");
    if (isuser)
        fprintf(file, "\t\t{ return MIG_TRAILER_ERROR ; }\n");
    else
        fprintf(file, "\t\t{ MIG_RETURN_ERROR(%s, MIG_TRAILER_ERROR); }\n", who);

    fprintf(file, "#if\t__MigTypeCheck\n");
    fprintf(file, "\ttrailer_size = TrailerP->msgh_trailer_size -\n");
    fprintf(file, "\t\t(mach_msg_size_t)(sizeof(mach_msg_trailer_type_t) - sizeof(mach_msg_trailer_size_t));\n");
    fprintf(file, "#endif\t/* __MigTypeCheck */\n");
}

/* executed iff elements are defined */
void
WriteCheckTrailerSize(FILE * file, boolean_t isuser, register argument_t * arg)
{
    fprintf(file, "#if\t__MigTypeCheck\n");
    if (akIdent(arg->argKind) == akeMsgSeqno) {
        fprintf(file, "\tif (trailer_size < (mach_msg_size_t)sizeof(mach_port_seqno_t))\n");
        if (isuser)
            fprintf(file, "\t\t{ return MIG_TRAILER_ERROR ; }\n");
        else
            fprintf(file, "\t\t{ MIG_RETURN_ERROR(OutP, MIG_TRAILER_ERROR); }\n");
        fprintf(file, "\ttrailer_size -= (mach_msg_size_t)sizeof(mach_port_seqno_t);\n");
    } else if (akIdent(arg->argKind) == akeSecToken) {
        fprintf(file, "\tif (trailer_size < (mach_msg_size_t)sizeof(security_token_t))\n");
        if (isuser)
            fprintf(file, "\t\t{ return MIG_TRAILER_ERROR ; }\n");
        else
            fprintf(file, "\t\t{ MIG_RETURN_ERROR(OutP, MIG_TRAILER_ERROR); }\n");
        fprintf(file, "\ttrailer_size -= (mach_msg_size_t)sizeof(security_token_t);\n");
    } else if (akIdent(arg->argKind) == akeAuditToken) {
        fprintf(file, "\tif (trailer_size < (mach_msg_size_t)sizeof(audit_token_t))\n");
        if (isuser)
            fprintf(file, "\t\t{ return MIG_TRAILER_ERROR ; }\n");
        else
            fprintf(file, "\t\t{ MIG_RETURN_ERROR(OutP, MIG_TRAILER_ERROR); }\n");
        fprintf(file, "\ttrailer_size -= (mach_msg_size_t)sizeof(audit_token_t);\n");
    } else if (akIdent(arg->argKind) == akeContextToken) {
        fprintf(file, "\tif (trailer_size < (mach_msg_size_t)sizeof(mach_vm_address_t))\n");
        if (isuser)
            fprintf(file, "\t\t{ return MIG_TRAILER_ERROR ; }\n");
        else
            fprintf(file, "\t\t{ MIG_RETURN_ERROR(OutP, MIG_TRAILER_ERROR); }\n");
        fprintf(file, "\ttrailer_size -= (mach_msg_size_t)sizeof(mach_vm_address_t);\n");
    }
    fprintf(file, "#endif\t/* __MigTypeCheck */\n");
}
