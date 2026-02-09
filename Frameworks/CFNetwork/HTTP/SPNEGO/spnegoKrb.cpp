/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
 * spnegoKrb.cpp - Kerberos routines for SPNEGO tool
 *
 * Created July 7 2003 by dmitch 
 */
 
#include "spnegoKrb.h"
#include <Kerberos/Kerberos.h>
#include <stdio.h>
//#include "unBER.h"
#include "CFNetworkInternal.h"
#include <mach-o/dyld.h>
#include "spnegoDER.h"

#ifndef DYNAMICALLY_LOAD_KERBEROS
#define DYNAMICALLY_LOAD_KERBEROS 1
#endif

#if DYNAMICALLY_LOAD_KERBEROS

static const void* KerberosLibrary = NULL;

static int returns_bad_int_return(void) { return 1; }
static void returns(void) { return; }


#define GET_DYNAMIC_SYMBOL(sym, rettype, arglist, alt)													\
    static rettype (* sym##_proc)arglist = NULL;														\
    if (sym##_proc == NULL) {																\
        if (KerberosLibrary || (KerberosLibrary = __CFNetworkLoadFramework("/System/Library/Frameworks/Kerberos.framework/Versions/A/Kerberos")))			\
            sym##_proc = (rettype(*)arglist)NSAddressOfSymbol(NSLookupSymbolInImage((const mach_header*)KerberosLibrary, "_"#sym, NSLOOKUPSYMBOLINIMAGE_OPTION_BIND));	\
        if (! sym##_proc) sym##_proc = (rettype(*)arglist)alt;									\
    }
    

krb5_error_code KRB5_CALLCONV
krb5_init_context(krb5_context *ctxt) {

    GET_DYNAMIC_SYMBOL(krb5_init_context, krb5_error_code, (krb5_context *ctxt), returns_bad_int_return);
    
    return (*krb5_init_context_proc)(ctxt);
}

krb5_error_code KRB5_CALLCONV
krb5_cc_default(krb5_context ctxt, krb5_ccache *cache) {

    GET_DYNAMIC_SYMBOL(krb5_cc_default, krb5_error_code, (krb5_context ctxt, krb5_ccache *cache), returns_bad_int_return);
    
    return (*krb5_cc_default_proc)(ctxt, cache);
}

krb5_error_code KRB5_CALLCONV
krb5_cc_get_principal (krb5_context context, krb5_ccache cache, krb5_principal *principal) {
	
    GET_DYNAMIC_SYMBOL(krb5_cc_get_principal, krb5_error_code, (krb5_context context, krb5_ccache cache, krb5_principal *principal), returns_bad_int_return);
    
    return (*krb5_cc_get_principal_proc)(context, cache, principal);
}

krb5_error_code KRB5_CALLCONV
krb5_sname_to_principal(krb5_context context, const char *hostname, const char *sname, krb5_int32 type, krb5_principal *principal) {
	
    GET_DYNAMIC_SYMBOL(krb5_sname_to_principal, krb5_error_code, (krb5_context context, const char *hostname, const char *sname, krb5_int32 type, krb5_principal *principal), returns_bad_int_return);
    
    return (*krb5_sname_to_principal_proc)(context, hostname, sname, type, principal);
}

krb5_error_code KRB5_CALLCONV
krb5_cc_close(krb5_context context, krb5_ccache cache) {
	
    GET_DYNAMIC_SYMBOL(krb5_cc_close, krb5_error_code, (krb5_context context, krb5_ccache cache), returns_bad_int_return);
    
    return (*krb5_cc_close_proc)(context, cache);
}

void KRB5_CALLCONV
krb5_free_principal(krb5_context context, krb5_principal principal) {

    GET_DYNAMIC_SYMBOL(krb5_free_principal, void, (krb5_context context, krb5_principal principal), returns);
    
    (*krb5_free_principal_proc)(context, principal);
}

void KRB5_CALLCONV
krb5_free_context(krb5_context context) {

    GET_DYNAMIC_SYMBOL(krb5_free_context, void, (krb5_context context), returns);
    
    (*krb5_free_context_proc)(context);
}

OM_uint32 KRB5_CALLCONV
gss_init_sec_context(OM_uint32 *minor_status, gss_cred_id_t cred_handle, gss_ctx_id_t *context_handle, gss_name_t target_name, gss_OID mech_type, OM_uint32 req_flags, OM_uint32 time_req, gss_channel_bindings_t input_chan_bindings, gss_buffer_t input_token, gss_OID * actual_mech_type, gss_buffer_t output_token, OM_uint32 *ret_flags, OM_uint32 *time_rec ) {

    GET_DYNAMIC_SYMBOL(gss_init_sec_context, OM_uint32, (OM_uint32 *minor_status, gss_cred_id_t cred_handle, gss_ctx_id_t *context_handle, gss_name_t target_name, gss_OID mech_type, OM_uint32 req_flags, OM_uint32 time_req, gss_channel_bindings_t input_chan_bindings, gss_buffer_t input_token, gss_OID * actual_mech_type, gss_buffer_t output_token, OM_uint32 *ret_flags, OM_uint32 *time_rec), returns_bad_int_return);
    
    return (*gss_init_sec_context_proc)(minor_status, cred_handle, context_handle, target_name, mech_type, req_flags, time_req, input_chan_bindings, input_token, actual_mech_type, output_token, ret_flags, time_rec);
}

OM_uint32 KRB5_CALLCONV
gss_delete_sec_context(OM_uint32 *minor_status, gss_ctx_id_t *context_handle, gss_buffer_t output_token)
{
	GET_DYNAMIC_SYMBOL(gss_delete_sec_context, OM_uint32, (OM_uint32 *minor_status, gss_ctx_id_t *context_handle, gss_buffer_t output_token), returns_bad_int_return);
    
    return (*gss_delete_sec_context_proc)(minor_status, context_handle, output_token);	
}

OM_uint32 KRB5_CALLCONV
gss_import_name(OM_uint32 *minor_status, gss_buffer_t input_name_buffer, gss_OID input_name_type, gss_name_t *output_name )
{
    GET_DYNAMIC_SYMBOL(gss_import_name, OM_uint32, (OM_uint32 *minor_status, gss_buffer_t input_name_buffer, gss_OID input_name_type, gss_name_t *output_name), returns_bad_int_return);
    
    return (*gss_import_name_proc)(minor_status, input_name_buffer, input_name_type, output_name);
}

OM_uint32 KRB5_CALLCONV
gss_release_name(OM_uint32 *minor_status, gss_name_t *input_name)
{
    GET_DYNAMIC_SYMBOL(gss_release_name, OM_uint32, (OM_uint32 *minor_status, gss_name_t *input_name), returns_bad_int_return);
    
    return (*gss_release_name_proc)(minor_status, input_name);
}

OM_uint32 KRB5_CALLCONV
gss_release_buffer(OM_uint32 *minor_status, gss_buffer_t buffer)
{
    GET_DYNAMIC_SYMBOL(gss_release_buffer, OM_uint32, (OM_uint32 *minor_status, gss_buffer_t buffer), returns_bad_int_return);
    
    return (*gss_release_buffer_proc)(minor_status, buffer);
}

#endif	/* DYNAMICALLY_LOAD_KERBEROS */

krb5_error_code GetSvcTicketForHost(const char *inHostname,
									const char *inServiceType,  // http or ftp etc...
									unsigned *outTicketLen,
									unsigned char **outTicket)
{
	krb5_error_code		kerr;
	krb5_context		kctx					= NULL;
	krb5_principal		kuserPrinc				= NULL;
	krb5_principal		kservPrinc				= NULL;
	krb5_ccache			kcc						= NULL;
    gss_buffer_desc     outputToken				= GSS_C_EMPTY_BUFFER;
    gss_buffer_desc     inputToken				= GSS_C_EMPTY_BUFFER;
    gss_OID_desc        gss_nt_krb5_principal	= {10, (void *)"\052\206\110\206\367\022\001\002\002\002"}; // from krb5.h
    gss_name_t			gssServicePrincipal		= GSS_C_NO_NAME;
    gss_ctx_id_t		gssContext				= GSS_C_NO_CONTEXT;
    OM_uint32			minorStatus;
	
	*outTicketLen = 0;
	*outTicket = NULL;
	
	if (!strcmp(inHostname, "localhost"))
		inHostname = NULL;
	
	if((kerr = krb5_init_context(&kctx)))
		goto out;
	
	// since there is no name available, there is no reason not to use the default cache
	if ((kerr = krb5_cc_default(kctx, &kcc)))
        goto out;
    
	// if no error is returned, then there is a valid cache setup already, otherwise don't try SPNEGO
	if ((kerr = krb5_cc_get_principal(kctx, kcc, &kuserPrinc)))
        goto out;

	// this prevents reverse lookup issues for AD environments, if name is unparsed it does a lookup
	if ((kerr = krb5_sname_to_principal(kctx, inHostname, inServiceType, KRB5_NT_UNKNOWN, &kservPrinc)))
		goto out;
	
    gss_buffer_desc		inputName; // do not release

	inputName.value = &kservPrinc;
    inputName.length = sizeof( krb5_principal );
    
    kerr = gss_import_name( &minorStatus, &inputName, &gss_nt_krb5_principal, &gssServicePrincipal );
    if( kerr != GSS_S_COMPLETE )
        goto out;    
    
    kerr = gss_init_sec_context(
                                       &minorStatus,               /* minor_status */
                                       GSS_C_NO_CREDENTIAL,        /* claimant_cred_handle */
                                       &gssContext,                /* context_handle */
                                       gssServicePrincipal,        /* target_name */
                                       GSS_C_NO_OID,			   /* mech_type (used to be const) */
                                       0,                          /* no req_flags to be set */
                                       GSS_C_INDEFINITE,           /* time_req */
                                       GSS_C_NO_CHANNEL_BINDINGS,  /* input_chan_bindings */
                                       &inputToken,                /* empty input token */
                                       NULL,                       /* actual_mech_type */
                                       &outputToken,               /* output_token */
                                       NULL,                       /* ret_flags */
                                       NULL                        /* time_rec */
                                       );
    if( kerr != GSS_S_COMPLETE )
        goto out;

    // if we got a token out.., we should have
	if( outputToken.length ) {
		if ((*outTicket = (unsigned char *)malloc(outputToken.length))) {
			*outTicketLen = outputToken.length;
			bcopy(outputToken.value, *outTicket, outputToken.length);
		}
	}

	// release the outputToken now that it's been copied
	gss_release_buffer(&minorStatus, &outputToken);

out:
		
	if(gssContext != GSS_C_NO_CONTEXT)
		gss_delete_sec_context(&minorStatus, &gssContext, GSS_C_NO_BUFFER);
	
	if(gssServicePrincipal != GSS_C_NO_NAME)
		gss_release_name(&minorStatus, &gssServicePrincipal);
	
	if(kservPrinc)
		krb5_free_principal(kctx, kservPrinc);
	
	if(kuserPrinc)
		krb5_free_principal(kctx, kuserPrinc);
	
	if(kcc)
		krb5_cc_close(kctx,kcc);
	
	if(kctx)
		krb5_free_context(kctx);
	
	return kerr;
}

