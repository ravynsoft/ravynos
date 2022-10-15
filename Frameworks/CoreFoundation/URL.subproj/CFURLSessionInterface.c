//===-- CoreFoundation/URL/CFURLSessionInterface.c - Very brief description -----------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2016 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains wrappes / helpers to import libcurl into Swift.
/// It is used to implement the NSURLSession API.
///
/// - SeeAlso: CFURLSessionInterface.h
///
//===----------------------------------------------------------------------===//

#include "CFURLSessionInterface.h"
#include <CoreFoundation/CFString.h>
#include <curl/curl.h>

FILE* aa = NULL;
CURL * gcurl = NULL;

static CFURLSessionEasyCode MakeEasyCode(CURLcode value) {
    return (CFURLSessionEasyCode) { value };
}
static CFURLSessionMultiCode MakeMultiCode(CURLMcode value) {
    return (CFURLSessionMultiCode) { value };
}

const char *CFURLSessionEasyCodeDescription(CFURLSessionEasyCode code) {
    return curl_easy_strerror(code.value);
}

CFURLSessionEasyHandle _Nonnull CFURLSessionEasyHandleInit() {
    return curl_easy_init();
}
void CFURLSessionEasyHandleDeinit(CFURLSessionEasyHandle _Nonnull handle) {
    curl_easy_cleanup(handle);
}
CFURLSessionEasyCode CFURLSessionEasyHandleSetPauseState(CFURLSessionEasyHandle _Nonnull handle, int send, int receive) {
    int bitmask = 0 | (send ? CURLPAUSE_SEND : CURLPAUSE_SEND_CONT) | (receive ? CURLPAUSE_RECV : CURLPAUSE_RECV_CONT);
    return MakeEasyCode(curl_easy_pause(handle, bitmask));
}

CFURLSessionMultiHandle _Nonnull CFURLSessionMultiHandleInit() {
    return curl_multi_init();
}
CFURLSessionMultiCode CFURLSessionMultiHandleDeinit(CFURLSessionMultiHandle _Nonnull handle) {
    return MakeMultiCode(curl_multi_cleanup(handle));
}
CFURLSessionMultiCode CFURLSessionMultiHandleAddHandle(CFURLSessionMultiHandle _Nonnull handle, CFURLSessionEasyHandle _Nonnull curl) {
    return MakeMultiCode(curl_multi_add_handle(handle, curl));
}
CFURLSessionMultiCode CFURLSessionMultiHandleRemoveHandle(CFURLSessionMultiHandle _Nonnull handle, CFURLSessionEasyHandle _Nonnull curl) {
    return MakeMultiCode(curl_multi_remove_handle(handle, curl));
}
CFURLSessionMultiCode CFURLSessionMultiHandleAssign(CFURLSessionMultiHandle _Nonnull handle, CFURLSession_socket_t socket, void *  _Nullable  sockp) {
    return MakeMultiCode(curl_multi_assign(handle, socket, sockp));
}
CFURLSessionMultiCode CFURLSessionMultiHandleAction(CFURLSessionMultiHandle _Nonnull handle, CFURLSession_socket_t socket, int bitmask, int * _Nonnull running_handles)
{
    return MakeMultiCode(curl_multi_socket_action(handle, socket, bitmask, running_handles));
}
CFURLSessionMultiHandleInfo CFURLSessionMultiHandleInfoRead(CFURLSessionMultiHandle _Nonnull handle, int * _Nonnull msgs_in_queue) {
    CFURLSessionMultiHandleInfo info = {};
    CURLMsg *msg = curl_multi_info_read(handle, msgs_in_queue);
    if (msg == NULL) {
        return info;
    }
    if (msg->msg != CURLMSG_DONE) {
        return info;
    }
    info.resultCode = MakeEasyCode(msg->data.result);
    info.easyHandle = msg->easy_handle;
    return info;
}

CFURLSessionEasyCode CFURLSession_easy_setopt_ptr(CFURLSessionEasyHandle _Nonnull curl, CFURLSessionOption option, void *_Nullable a) {
    return MakeEasyCode(curl_easy_setopt(curl, option.value, a));
}
CFURLSessionEasyCode CFURLSession_easy_setopt_int(CFURLSessionEasyHandle _Nonnull curl, CFURLSessionOption option, int a) {
    return MakeEasyCode(curl_easy_setopt(curl, option.value, a));
}
CFURLSessionEasyCode CFURLSession_easy_setopt_long(CFURLSessionEasyHandle _Nonnull curl, CFURLSessionOption option, long a) {
    return MakeEasyCode(curl_easy_setopt(curl, option.value, a));
}
CFURLSessionEasyCode CFURLSession_easy_setopt_int64(CFURLSessionEasyHandle _Nonnull curl, CFURLSessionOption option, long long a) {
    return MakeEasyCode(curl_easy_setopt(curl, option.value, (int64_t)a));
}

CFURLSessionEasyCode CFURLSession_easy_setopt_wc(CFURLSessionEasyHandle _Nonnull curl, CFURLSessionOption option, size_t(*_Nonnull a)(char *_Nonnull, size_t, size_t, void *_Nullable)) {
    return MakeEasyCode(curl_easy_setopt(curl, option.value, a));
}
CFURLSessionEasyCode CFURLSession_easy_setopt_dc(CFURLSessionEasyHandle _Nonnull curl, CFURLSessionOption option, int(*_Nonnull a)(CFURLSessionEasyHandle _Nonnull handle, int type, char *_Nonnull data, size_t size, void *_Nullable userptr)) {
    return MakeEasyCode(curl_easy_setopt(curl, option.value, a));
}
CFURLSessionEasyCode CFURLSession_easy_setopt_sc(CFURLSessionEasyHandle _Nonnull curl, CFURLSessionOption option, CFURLSessionSocketOptionCallback * _Nullable a) {
    return MakeEasyCode(curl_easy_setopt(curl, option.value, a));
}
CFURLSessionEasyCode CFURLSession_easy_setopt_seek(CFURLSessionEasyHandle _Nonnull curl, CFURLSessionOption option, CFURLSessionSeekCallback * _Nullable a) {
    return MakeEasyCode(curl_easy_setopt(curl, option.value, a));
}

CFURLSessionEasyCode CFURLSession_easy_setopt_tc(CFURLSessionEasyHandle _Nonnull curl, CFURLSessionOption option, CFURLSessionTransferInfoCallback * _Nullable a) {
    return MakeEasyCode(curl_easy_setopt(curl,  option.value, a));
}

CFURLSessionEasyCode CFURLSession_easy_getinfo_long(CFURLSessionEasyHandle _Nonnull curl, CFURLSessionInfo info, long *_Nonnull a) {
    return MakeEasyCode(curl_easy_getinfo(curl, info.value, a));
}

CFURLSessionEasyCode CFURLSession_easy_getinfo_double(CFURLSessionEasyHandle _Nonnull curl, CFURLSessionInfo info, double *_Nonnull a) {
    return MakeEasyCode(curl_easy_getinfo(curl, info.value, a));
}

CFURLSessionEasyCode CFURLSession_easy_getinfo_charp(CFURLSessionEasyHandle _Nonnull curl, CFURLSessionInfo info, char *_Nullable*_Nonnull a) {
    return MakeEasyCode(curl_easy_getinfo(curl, info.value, a));
}

CFURLSessionMultiCode CFURLSession_multi_setopt_ptr(CFURLSessionMultiHandle _Nonnull multi_handle, CFURLSessionMultiOption option, void *_Nullable a) {
    return MakeMultiCode(curl_multi_setopt(multi_handle, option.value, a));
}
CFURLSessionMultiCode CFURLSession_multi_setopt_l(CFURLSessionMultiHandle _Nonnull multi_handle, CFURLSessionMultiOption option, long a) {
    return MakeMultiCode(curl_multi_setopt(multi_handle, option.value, a));
}
CFURLSessionMultiCode CFURLSession_multi_setopt_sf(CFURLSessionMultiHandle _Nonnull multi_handle, CFURLSessionMultiOption option, int (*_Nonnull a)(CFURLSessionEasyHandle _Nonnull, CFURLSession_socket_t, int, void *_Nullable, void *_Nullable)) {
    return MakeMultiCode(curl_multi_setopt(multi_handle, option.value, a));
}
CFURLSessionMultiCode CFURLSession_multi_setopt_tf(CFURLSessionMultiHandle _Nonnull multi_handle, CFURLSessionMultiOption option, int (*_Nonnull a)(CFURLSessionMultiHandle _Nonnull, long, void *_Nullable)) {
    return MakeMultiCode(curl_multi_setopt(multi_handle, option.value, a));
}

CFURLSessionEasyCode CFURLSessionInit(void) {
    return MakeEasyCode(curl_global_init(CURL_GLOBAL_SSL));
}

int const CFURLSessionEasyErrorSize = { CURL_ERROR_SIZE + 1 };

CFURLSessionEasyCode const CFURLSessionEasyCodeOK = { CURLE_OK };
CFURLSessionEasyCode const CFURLSessionEasyCodeUNSUPPORTED_PROTOCOL = { CURLE_UNSUPPORTED_PROTOCOL };
CFURLSessionEasyCode const CFURLSessionEasyCodeFAILED_INIT = { CURLE_FAILED_INIT };
CFURLSessionEasyCode const CFURLSessionEasyCodeURL_MALFORMAT = { CURLE_URL_MALFORMAT };
CFURLSessionEasyCode const CFURLSessionEasyCodeNOT_BUILT_IN = { CURLE_NOT_BUILT_IN };
CFURLSessionEasyCode const CFURLSessionEasyCodeCOULDNT_RESOLVE_PROXY = { CURLE_COULDNT_RESOLVE_PROXY };
CFURLSessionEasyCode const CFURLSessionEasyCodeCOULDNT_RESOLVE_HOST = { CURLE_COULDNT_RESOLVE_HOST };
CFURLSessionEasyCode const CFURLSessionEasyCodeCOULDNT_CONNECT = { CURLE_COULDNT_CONNECT };
CFURLSessionEasyCode const CFURLSessionEasyCodeFTP_WEIRD_SERVER_REPLY = { CURLE_FTP_WEIRD_SERVER_REPLY };
CFURLSessionEasyCode const CFURLSessionEasyCodeREMOTE_ACCESS_DENIED = { CURLE_REMOTE_ACCESS_DENIED };
CFURLSessionEasyCode const CFURLSessionEasyCodeFTP_ACCEPT_FAILED = { CURLE_FTP_ACCEPT_FAILED };
CFURLSessionEasyCode const CFURLSessionEasyCodeFTP_WEIRD_PASS_REPLY = { CURLE_FTP_WEIRD_PASS_REPLY };
CFURLSessionEasyCode const CFURLSessionEasyCodeFTP_ACCEPT_TIMEOUT = { CURLE_FTP_ACCEPT_TIMEOUT };
CFURLSessionEasyCode const CFURLSessionEasyCodeFTP_WEIRD_PASV_REPLY = { CURLE_FTP_WEIRD_PASV_REPLY };
CFURLSessionEasyCode const CFURLSessionEasyCodeFTP_WEIRD_227_FORMAT = { CURLE_FTP_WEIRD_227_FORMAT };
CFURLSessionEasyCode const CFURLSessionEasyCodeFTP_CANT_GET_HOST = { CURLE_FTP_CANT_GET_HOST };
CFURLSessionEasyCode const CFURLSessionEasyCodeFTP_COULDNT_SET_TYPE = { CURLE_FTP_COULDNT_SET_TYPE };
CFURLSessionEasyCode const CFURLSessionEasyCodePARTIAL_FILE = { CURLE_PARTIAL_FILE };
CFURLSessionEasyCode const CFURLSessionEasyCodeFTP_COULDNT_RETR_FILE = { CURLE_FTP_COULDNT_RETR_FILE };
#if LIBCURL_VERSION_MAJOR > 7 || (LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR > 67) || (LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR == 67 && LIBCURL_VERSION_PATCH >= 1)
CFURLSessionEasyCode const CFURLSessionEasyCodeHTTP3 = { CURLE_HTTP3 };
#else
CFURLSessionEasyCode const CFURLSessionEasyCodeOBSOLETE20 = { CURLE_OBSOLETE20 };
#endif
CFURLSessionEasyCode const CFURLSessionEasyCodeQUOTE_ERROR = { CURLE_QUOTE_ERROR };
CFURLSessionEasyCode const CFURLSessionEasyCodeHTTP_RETURNED_ERROR = { CURLE_HTTP_RETURNED_ERROR };
CFURLSessionEasyCode const CFURLSessionEasyCodeWRITE_ERROR = { CURLE_WRITE_ERROR };
CFURLSessionEasyCode const CFURLSessionEasyCodeOBSOLETE24 = { CURLE_OBSOLETE24 };
CFURLSessionEasyCode const CFURLSessionEasyCodeUPLOAD_FAILED = { CURLE_UPLOAD_FAILED };
CFURLSessionEasyCode const CFURLSessionEasyCodeREAD_ERROR = { CURLE_READ_ERROR };
CFURLSessionEasyCode const CFURLSessionEasyCodeOUT_OF_MEMORY = { CURLE_OUT_OF_MEMORY };
CFURLSessionEasyCode const CFURLSessionEasyCodeOPERATION_TIMEDOUT = { CURLE_OPERATION_TIMEDOUT };
CFURLSessionEasyCode const CFURLSessionEasyCodeOBSOLETE29 = { CURLE_OBSOLETE29 };
CFURLSessionEasyCode const CFURLSessionEasyCodeFTP_PORT_FAILED = { CURLE_FTP_PORT_FAILED };
CFURLSessionEasyCode const CFURLSessionEasyCodeFTP_COULDNT_USE_REST = { CURLE_FTP_COULDNT_USE_REST };
CFURLSessionEasyCode const CFURLSessionEasyCodeOBSOLETE32 = { CURLE_OBSOLETE32 };
CFURLSessionEasyCode const CFURLSessionEasyCodeRANGE_ERROR = { CURLE_RANGE_ERROR };
CFURLSessionEasyCode const CFURLSessionEasyCodeHTTP_POST_ERROR = { CURLE_HTTP_POST_ERROR };
CFURLSessionEasyCode const CFURLSessionEasyCodeSSL_CONNECT_ERROR = { CURLE_SSL_CONNECT_ERROR };
CFURLSessionEasyCode const CFURLSessionEasyCodeBAD_DOWNLOAD_RESUME = { CURLE_BAD_DOWNLOAD_RESUME };
CFURLSessionEasyCode const CFURLSessionEasyCodeFILE_COULDNT_READ_FILE = { CURLE_FILE_COULDNT_READ_FILE };
CFURLSessionEasyCode const CFURLSessionEasyCodeLDAP_CANNOT_BIND = { CURLE_LDAP_CANNOT_BIND };
CFURLSessionEasyCode const CFURLSessionEasyCodeLDAP_SEARCH_FAILED = { CURLE_LDAP_SEARCH_FAILED };
CFURLSessionEasyCode const CFURLSessionEasyCodeFUNCTION_NOT_FOUND = { CURLE_FUNCTION_NOT_FOUND };
CFURLSessionEasyCode const CFURLSessionEasyCodeABORTED_BY_CALLBACK = { CURLE_ABORTED_BY_CALLBACK };
CFURLSessionEasyCode const CFURLSessionEasyCodeBAD_FUNCTION_ARGUMENT = { CURLE_BAD_FUNCTION_ARGUMENT };
CFURLSessionEasyCode const CFURLSessionEasyCodeOBSOLETE44 = { CURLE_OBSOLETE44 };
CFURLSessionEasyCode const CFURLSessionEasyCodeINTERFACE_FAILED = { CURLE_INTERFACE_FAILED };
CFURLSessionEasyCode const CFURLSessionEasyCodeOBSOLETE46 = { CURLE_OBSOLETE46 };
CFURLSessionEasyCode const CFURLSessionEasyCodeTOO_MANY_REDIRECTS = { CURLE_TOO_MANY_REDIRECTS };
CFURLSessionEasyCode const CFURLSessionEasyCodeUNKNOWN_OPTION = { CURLE_UNKNOWN_OPTION };
CFURLSessionEasyCode const CFURLSessionEasyCodeTELNET_OPTION_SYNTAX = { CURLE_TELNET_OPTION_SYNTAX };
CFURLSessionEasyCode const CFURLSessionEasyCodeOBSOLETE50 = { CURLE_OBSOLETE50 };
CFURLSessionEasyCode const CFURLSessionEasyCodePEER_FAILED_VERIFICATION = { CURLE_PEER_FAILED_VERIFICATION };
CFURLSessionEasyCode const CFURLSessionEasyCodeGOT_NOTHING = { CURLE_GOT_NOTHING };
CFURLSessionEasyCode const CFURLSessionEasyCodeSSL_ENGINE_NOTFOUND = { CURLE_SSL_ENGINE_NOTFOUND };
CFURLSessionEasyCode const CFURLSessionEasyCodeSSL_ENGINE_SETFAILED = { CURLE_SSL_ENGINE_SETFAILED };
CFURLSessionEasyCode const CFURLSessionEasyCodeSEND_ERROR = { CURLE_SEND_ERROR };
CFURLSessionEasyCode const CFURLSessionEasyCodeRECV_ERROR = { CURLE_RECV_ERROR };
CFURLSessionEasyCode const CFURLSessionEasyCodeOBSOLETE57 = { CURLE_OBSOLETE57 };
CFURLSessionEasyCode const CFURLSessionEasyCodeSSL_CERTPROBLEM = { CURLE_SSL_CERTPROBLEM };
CFURLSessionEasyCode const CFURLSessionEasyCodeSSL_CIPHER = { CURLE_SSL_CIPHER };
CFURLSessionEasyCode const CFURLSessionEasyCodeSSL_CACERT = { CURLE_SSL_CACERT };
CFURLSessionEasyCode const CFURLSessionEasyCodeBAD_CONTENT_ENCODING = { CURLE_BAD_CONTENT_ENCODING };
CFURLSessionEasyCode const CFURLSessionEasyCodeLDAP_INVALID_URL = { CURLE_LDAP_INVALID_URL };
CFURLSessionEasyCode const CFURLSessionEasyCodeFILESIZE_EXCEEDED = { CURLE_FILESIZE_EXCEEDED };
CFURLSessionEasyCode const CFURLSessionEasyCodeUSE_SSL_FAILED = { CURLE_USE_SSL_FAILED };
CFURLSessionEasyCode const CFURLSessionEasyCodeSEND_FAIL_REWIND = { CURLE_SEND_FAIL_REWIND };
CFURLSessionEasyCode const CFURLSessionEasyCodeSSL_ENGINE_INITFAILED = { CURLE_SSL_ENGINE_INITFAILED };
CFURLSessionEasyCode const CFURLSessionEasyCodeLOGIN_DENIED = { CURLE_LOGIN_DENIED };
CFURLSessionEasyCode const CFURLSessionEasyCodeTFTP_NOTFOUND = { CURLE_TFTP_NOTFOUND };
CFURLSessionEasyCode const CFURLSessionEasyCodeTFTP_PERM = { CURLE_TFTP_PERM };
CFURLSessionEasyCode const CFURLSessionEasyCodeREMOTE_DISK_FULL = { CURLE_REMOTE_DISK_FULL };
CFURLSessionEasyCode const CFURLSessionEasyCodeTFTP_ILLEGAL = { CURLE_TFTP_ILLEGAL };
CFURLSessionEasyCode const CFURLSessionEasyCodeTFTP_UNKNOWNID = { CURLE_TFTP_UNKNOWNID };
CFURLSessionEasyCode const CFURLSessionEasyCodeREMOTE_FILE_EXISTS = { CURLE_REMOTE_FILE_EXISTS };
CFURLSessionEasyCode const CFURLSessionEasyCodeTFTP_NOSUCHUSER = { CURLE_TFTP_NOSUCHUSER };
CFURLSessionEasyCode const CFURLSessionEasyCodeCONV_FAILED = { CURLE_CONV_FAILED };
CFURLSessionEasyCode const CFURLSessionEasyCodeCONV_REQD = { CURLE_CONV_REQD };
CFURLSessionEasyCode const CFURLSessionEasyCodeSSL_CACERT_BADFILE = { CURLE_SSL_CACERT_BADFILE };
CFURLSessionEasyCode const CFURLSessionEasyCodeREMOTE_FILE_NOT_FOUND = { CURLE_REMOTE_FILE_NOT_FOUND };
CFURLSessionEasyCode const CFURLSessionEasyCodeSSH = { CURLE_SSH };
CFURLSessionEasyCode const CFURLSessionEasyCodeSSL_SHUTDOWN_FAILED = { CURLE_SSL_SHUTDOWN_FAILED };
CFURLSessionEasyCode const CFURLSessionEasyCodeAGAIN = { CURLE_AGAIN };
CFURLSessionEasyCode const CFURLSessionEasyCodeSSL_CRL_BADFILE = { CURLE_SSL_CRL_BADFILE };
CFURLSessionEasyCode const CFURLSessionEasyCodeSSL_ISSUER_ERROR = { CURLE_SSL_ISSUER_ERROR };
CFURLSessionEasyCode const CFURLSessionEasyCodeFTP_PRET_FAILED = { CURLE_FTP_PRET_FAILED };
CFURLSessionEasyCode const CFURLSessionEasyCodeRTSP_CSEQ_ERROR = { CURLE_RTSP_CSEQ_ERROR };
CFURLSessionEasyCode const CFURLSessionEasyCodeRTSP_SESSION_ERROR = { CURLE_RTSP_SESSION_ERROR };
CFURLSessionEasyCode const CFURLSessionEasyCodeFTP_BAD_FILE_LIST = { CURLE_FTP_BAD_FILE_LIST };
CFURLSessionEasyCode const CFURLSessionEasyCodeCHUNK_FAILED = { CURLE_CHUNK_FAILED };

CFURLSessionProtocol const CFURLSessionProtocolHTTP = CURLPROTO_HTTP;
CFURLSessionProtocol const CFURLSessionProtocolHTTPS = CURLPROTO_HTTPS;
CFURLSessionProtocol const CFURLSessionProtocolFTP = CURLPROTO_FTP;
CFURLSessionProtocol const CFURLSessionProtocolFTPS = CURLPROTO_FTPS;
CFURLSessionProtocol const CFURLSessionProtocolSCP = CURLPROTO_SCP;
CFURLSessionProtocol const CFURLSessionProtocolSFTP = CURLPROTO_SFTP;
CFURLSessionProtocol const CFURLSessionProtocolTELNET = CURLPROTO_TELNET;
CFURLSessionProtocol const CFURLSessionProtocolLDAP = CURLPROTO_LDAP;
CFURLSessionProtocol const CFURLSessionProtocolLDAPS = CURLPROTO_LDAPS;
CFURLSessionProtocol const CFURLSessionProtocolDICT = CURLPROTO_DICT;
CFURLSessionProtocol const CFURLSessionProtocolFILE = CURLPROTO_FILE;
CFURLSessionProtocol const CFURLSessionProtocolTFTP = CURLPROTO_TFTP;
CFURLSessionProtocol const CFURLSessionProtocolIMAP = CURLPROTO_IMAP;
CFURLSessionProtocol const CFURLSessionProtocolIMAPS = CURLPROTO_IMAPS;
CFURLSessionProtocol const CFURLSessionProtocolPOP3 = CURLPROTO_POP3;
CFURLSessionProtocol const CFURLSessionProtocolPOP3S = CURLPROTO_POP3S;
CFURLSessionProtocol const CFURLSessionProtocolSMTP = CURLPROTO_SMTP;
CFURLSessionProtocol const CFURLSessionProtocolSMTPS = CURLPROTO_SMTPS;
CFURLSessionProtocol const CFURLSessionProtocolRTSP = CURLPROTO_RTSP;
CFURLSessionProtocol const CFURLSessionProtocolRTMP = CURLPROTO_RTMP;
CFURLSessionProtocol const CFURLSessionProtocolRTMPT = CURLPROTO_RTMPT;
CFURLSessionProtocol const CFURLSessionProtocolRTMPE = CURLPROTO_RTMPE;
CFURLSessionProtocol const CFURLSessionProtocolRTMPTE = CURLPROTO_RTMPTE;
CFURLSessionProtocol const CFURLSessionProtocolRTMPS = CURLPROTO_RTMPS;
CFURLSessionProtocol const CFURLSessionProtocolRTMPTS = CURLPROTO_RTMPTS;
CFURLSessionProtocol const CFURLSessionProtocolGOPHER = CURLPROTO_GOPHER;
CFURLSessionProtocol const CFURLSessionProtocolALL = CURLPROTO_ALL;


size_t const CFURLSessionMaxWriteSize = CURL_MAX_WRITE_SIZE;


CFURLSessionOption const CFURLSessionOptionWRITEDATA = { CURLOPT_WRITEDATA };
CFURLSessionOption const CFURLSessionOptionURL = { CURLOPT_URL };
CFURLSessionOption const CFURLSessionOptionPORT = { CURLOPT_PORT };
CFURLSessionOption const CFURLSessionOptionPROXY = { CURLOPT_PROXY };
CFURLSessionOption const CFURLSessionOptionUSERPWD = { CURLOPT_USERPWD };
CFURLSessionOption const CFURLSessionOptionPROXYUSERPWD = { CURLOPT_PROXYUSERPWD };
CFURLSessionOption const CFURLSessionOptionRANGE = { CURLOPT_RANGE };
CFURLSessionOption const CFURLSessionOptionREADDATA = { CURLOPT_READDATA };
CFURLSessionOption const CFURLSessionOptionERRORBUFFER = { CURLOPT_ERRORBUFFER };
CFURLSessionOption const CFURLSessionOptionWRITEFUNCTION = { CURLOPT_WRITEFUNCTION };
CFURLSessionOption const CFURLSessionOptionREADFUNCTION = { CURLOPT_READFUNCTION };
CFURLSessionOption const CFURLSessionOptionTIMEOUT = { CURLOPT_TIMEOUT };
CFURLSessionOption const CFURLSessionOptionINFILESIZE = { CURLOPT_INFILESIZE };
CFURLSessionOption const CFURLSessionOptionPOSTFIELDS = { CURLOPT_POSTFIELDS };
CFURLSessionOption const CFURLSessionOptionREFERER = { CURLOPT_REFERER };
CFURLSessionOption const CFURLSessionOptionFTPPORT = { CURLOPT_FTPPORT };
CFURLSessionOption const CFURLSessionOptionUSERAGENT = { CURLOPT_USERAGENT };
CFURLSessionOption const CFURLSessionOptionLOW_SPEED_LIMIT = { CURLOPT_LOW_SPEED_LIMIT };
CFURLSessionOption const CFURLSessionOptionLOW_SPEED_TIME = { CURLOPT_LOW_SPEED_TIME };
CFURLSessionOption const CFURLSessionOptionRESUME_FROM = { CURLOPT_RESUME_FROM };
CFURLSessionOption const CFURLSessionOptionCOOKIE = { CURLOPT_COOKIE };
CFURLSessionOption const CFURLSessionOptionHTTPHEADER = { CURLOPT_HTTPHEADER };
CFURLSessionOption const CFURLSessionOptionHTTPPOST = { CURLOPT_HTTPPOST };
CFURLSessionOption const CFURLSessionOptionSSLCERT = { CURLOPT_SSLCERT };
CFURLSessionOption const CFURLSessionOptionKEYPASSWD = { CURLOPT_KEYPASSWD };
CFURLSessionOption const CFURLSessionOptionCRLF = { CURLOPT_CRLF };
CFURLSessionOption const CFURLSessionOptionQUOTE = { CURLOPT_QUOTE };
CFURLSessionOption const CFURLSessionOptionHEADERDATA = { CURLOPT_HEADERDATA };
CFURLSessionOption const CFURLSessionOptionCOOKIEFILE = { CURLOPT_COOKIEFILE };
CFURLSessionOption const CFURLSessionOptionSSLVERSION = { CURLOPT_SSLVERSION };
CFURLSessionOption const CFURLSessionOptionTIMECONDITION = { CURLOPT_TIMECONDITION };
CFURLSessionOption const CFURLSessionOptionTIMEVALUE = { CURLOPT_TIMEVALUE };
CFURLSessionOption const CFURLSessionOptionCUSTOMREQUEST = { CURLOPT_CUSTOMREQUEST };
CFURLSessionOption const CFURLSessionOptionSTDERR = { CURLOPT_STDERR };
CFURLSessionOption const CFURLSessionOptionPOSTQUOTE = { CURLOPT_POSTQUOTE };
/*CFURLSessionOption const CFURLSessionOptionOBSOLETE40 = { CURLOPT_OBSOLETE40 };*/
CFURLSessionOption const CFURLSessionOptionVERBOSE = { CURLOPT_VERBOSE };
CFURLSessionOption const CFURLSessionOptionHEADER = { CURLOPT_HEADER };
CFURLSessionOption const CFURLSessionOptionNOPROGRESS = { CURLOPT_NOPROGRESS };
CFURLSessionOption const CFURLSessionOptionNOBODY = { CURLOPT_NOBODY };
CFURLSessionOption const CFURLSessionOptionFAILONERROR = { CURLOPT_FAILONERROR };
CFURLSessionOption const CFURLSessionOptionUPLOAD = { CURLOPT_UPLOAD };
CFURLSessionOption const CFURLSessionOptionPOST = { CURLOPT_POST };
CFURLSessionOption const CFURLSessionOptionDIRLISTONLY = { CURLOPT_DIRLISTONLY };
CFURLSessionOption const CFURLSessionOptionAPPEND = { CURLOPT_APPEND };
CFURLSessionOption const CFURLSessionOptionNETRC = { CURLOPT_NETRC };
CFURLSessionOption const CFURLSessionOptionFOLLOWLOCATION = { CURLOPT_FOLLOWLOCATION };
CFURLSessionOption const CFURLSessionOptionTRANSFERTEXT = { CURLOPT_TRANSFERTEXT };
CFURLSessionOption const CFURLSessionOptionPUT = { CURLOPT_PUT };
CFURLSessionOption const CFURLSessionOptionPROGRESSFUNCTION = { CURLOPT_PROGRESSFUNCTION };
CFURLSessionOption const CFURLSessionOptionPROGRESSDATA = { CURLOPT_PROGRESSDATA };
CFURLSessionOption const CFURLSessionOptionAUTOREFERER = { CURLOPT_AUTOREFERER };
CFURLSessionOption const CFURLSessionOptionPROXYPORT = { CURLOPT_PROXYPORT };
CFURLSessionOption const CFURLSessionOptionPOSTFIELDSIZE = { CURLOPT_POSTFIELDSIZE };
CFURLSessionOption const CFURLSessionOptionHTTPPROXYTUNNEL = { CURLOPT_HTTPPROXYTUNNEL };
CFURLSessionOption const CFURLSessionOptionINTERFACE = { CURLOPT_INTERFACE };
CFURLSessionOption const CFURLSessionOptionKRBLEVEL = { CURLOPT_KRBLEVEL };
CFURLSessionOption const CFURLSessionOptionSSL_VERIFYPEER = { CURLOPT_SSL_VERIFYPEER };
CFURLSessionOption const CFURLSessionOptionCAINFO = { CURLOPT_CAINFO };
CFURLSessionOption const CFURLSessionOptionMAXREDIRS = { CURLOPT_MAXREDIRS };
CFURLSessionOption const CFURLSessionOptionFILETIME = { CURLOPT_FILETIME };
CFURLSessionOption const CFURLSessionOptionTELNETOPTIONS = { CURLOPT_TELNETOPTIONS };
CFURLSessionOption const CFURLSessionOptionMAXCONNECTS = { CURLOPT_MAXCONNECTS };
CFURLSessionOption const CFURLSessionOptionFRESH_CONNECT = { CURLOPT_FRESH_CONNECT };
CFURLSessionOption const CFURLSessionOptionFORBID_REUSE = { CURLOPT_FORBID_REUSE };
CFURLSessionOption const CFURLSessionOptionRANDOM_FILE = { CURLOPT_RANDOM_FILE };
CFURLSessionOption const CFURLSessionOptionEGDSOCKET = { CURLOPT_EGDSOCKET };
CFURLSessionOption const CFURLSessionOptionCONNECTTIMEOUT = { CURLOPT_CONNECTTIMEOUT };
CFURLSessionOption const CFURLSessionOptionHEADERFUNCTION = { CURLOPT_HEADERFUNCTION };
CFURLSessionOption const CFURLSessionOptionHTTPGET = { CURLOPT_HTTPGET };
CFURLSessionOption const CFURLSessionOptionSSL_VERIFYHOST = { CURLOPT_SSL_VERIFYHOST };
CFURLSessionOption const CFURLSessionOptionCOOKIEJAR = { CURLOPT_COOKIEJAR };
CFURLSessionOption const CFURLSessionOptionSSL_CIPHER_LIST = { CURLOPT_SSL_CIPHER_LIST };
CFURLSessionOption const CFURLSessionOptionHTTP_VERSION = { CURLOPT_HTTP_VERSION };
CFURLSessionOption const CFURLSessionOptionFTP_USE_EPSV = { CURLOPT_FTP_USE_EPSV };
CFURLSessionOption const CFURLSessionOptionSSLCERTTYPE = { CURLOPT_SSLCERTTYPE };
CFURLSessionOption const CFURLSessionOptionSSLKEY = { CURLOPT_SSLKEY };
CFURLSessionOption const CFURLSessionOptionSSLKEYTYPE = { CURLOPT_SSLKEYTYPE };
CFURLSessionOption const CFURLSessionOptionSSLENGINE = { CURLOPT_SSLENGINE };
CFURLSessionOption const CFURLSessionOptionSSLENGINE_DEFAULT = { CURLOPT_SSLENGINE_DEFAULT };
CFURLSessionOption const CFURLSessionOptionDNS_USE_GLOBAL_CACHE = { CURLOPT_DNS_USE_GLOBAL_CACHE };
CFURLSessionOption const CFURLSessionOptionDNS_CACHE_TIMEOUT = { CURLOPT_DNS_CACHE_TIMEOUT };
CFURLSessionOption const CFURLSessionOptionPREQUOTE = { CURLOPT_PREQUOTE };
CFURLSessionOption const CFURLSessionOptionDEBUGFUNCTION = { CURLOPT_DEBUGFUNCTION };
CFURLSessionOption const CFURLSessionOptionDEBUGDATA = { CURLOPT_DEBUGDATA };
CFURLSessionOption const CFURLSessionOptionCOOKIESESSION = { CURLOPT_COOKIESESSION };
CFURLSessionOption const CFURLSessionOptionCAPATH = { CURLOPT_CAPATH };
CFURLSessionOption const CFURLSessionOptionBUFFERSIZE = { CURLOPT_BUFFERSIZE };
CFURLSessionOption const CFURLSessionOptionNOSIGNAL = { CURLOPT_NOSIGNAL };
CFURLSessionOption const CFURLSessionOptionSHARE = { CURLOPT_SHARE };
CFURLSessionOption const CFURLSessionOptionPROXYTYPE = { CURLOPT_PROXYTYPE };
CFURLSessionOption const CFURLSessionOptionACCEPT_ENCODING = { CURLOPT_ACCEPT_ENCODING };
CFURLSessionOption const CFURLSessionOptionPRIVATE = { CURLOPT_PRIVATE };
CFURLSessionOption const CFURLSessionOptionHTTP200ALIASES = { CURLOPT_HTTP200ALIASES };
CFURLSessionOption const CFURLSessionOptionUNRESTRICTED_AUTH = { CURLOPT_UNRESTRICTED_AUTH };
CFURLSessionOption const CFURLSessionOptionFTP_USE_EPRT = { CURLOPT_FTP_USE_EPRT };
CFURLSessionOption const CFURLSessionOptionHTTPAUTH = { CURLOPT_HTTPAUTH };
CFURLSessionOption const CFURLSessionOptionSSL_CTX_FUNCTION = { CURLOPT_SSL_CTX_FUNCTION };
CFURLSessionOption const CFURLSessionOptionSSL_CTX_DATA = { CURLOPT_SSL_CTX_DATA };
CFURLSessionOption const CFURLSessionOptionFTP_CREATE_MISSING_DIRS = { CURLOPT_FTP_CREATE_MISSING_DIRS };
CFURLSessionOption const CFURLSessionOptionPROXYAUTH = { CURLOPT_PROXYAUTH };
CFURLSessionOption const CFURLSessionOptionFTP_RESPONSE_TIMEOUT = { CURLOPT_FTP_RESPONSE_TIMEOUT };
CFURLSessionOption const CFURLSessionOptionIPRESOLVE = { CURLOPT_IPRESOLVE };
CFURLSessionOption const CFURLSessionOptionMAXFILESIZE = { CURLOPT_MAXFILESIZE };
CFURLSessionOption const CFURLSessionOptionINFILESIZE_LARGE = { CURLOPT_INFILESIZE_LARGE };
CFURLSessionOption const CFURLSessionOptionRESUME_FROM_LARGE = { CURLOPT_RESUME_FROM_LARGE };
CFURLSessionOption const CFURLSessionOptionMAXFILESIZE_LARGE = { CURLOPT_MAXFILESIZE_LARGE };
CFURLSessionOption const CFURLSessionOptionNETRC_FILE = { CURLOPT_NETRC_FILE };
CFURLSessionOption const CFURLSessionOptionUSE_SSL = { CURLOPT_USE_SSL };
CFURLSessionOption const CFURLSessionOptionPOSTFIELDSIZE_LARGE = { CURLOPT_POSTFIELDSIZE_LARGE };
CFURLSessionOption const CFURLSessionOptionTCP_NODELAY = { CURLOPT_TCP_NODELAY };
CFURLSessionOption const CFURLSessionOptionFTPSSLAUTH = { CURLOPT_FTPSSLAUTH };
CFURLSessionOption const CFURLSessionOptionIOCTLFUNCTION = { CURLOPT_IOCTLFUNCTION };
CFURLSessionOption const CFURLSessionOptionIOCTLDATA = { CURLOPT_IOCTLDATA };
CFURLSessionOption const CFURLSessionOptionFTP_ACCOUNT = { CURLOPT_FTP_ACCOUNT };
CFURLSessionOption const CFURLSessionOptionCOOKIELIST = { CURLOPT_COOKIELIST };
CFURLSessionOption const CFURLSessionOptionIGNORE_CONTENT_LENGTH = { CURLOPT_IGNORE_CONTENT_LENGTH };
CFURLSessionOption const CFURLSessionOptionFTP_SKIP_PASV_IP = { CURLOPT_FTP_SKIP_PASV_IP };
CFURLSessionOption const CFURLSessionOptionFTP_FILEMETHOD = { CURLOPT_FTP_FILEMETHOD };
CFURLSessionOption const CFURLSessionOptionLOCALPORT = { CURLOPT_LOCALPORT };
CFURLSessionOption const CFURLSessionOptionLOCALPORTRANGE = { CURLOPT_LOCALPORTRANGE };
CFURLSessionOption const CFURLSessionOptionCONNECT_ONLY = { CURLOPT_CONNECT_ONLY };
CFURLSessionOption const CFURLSessionOptionCONV_FROM_NETWORK_FUNCTION = { CURLOPT_CONV_FROM_NETWORK_FUNCTION };
CFURLSessionOption const CFURLSessionOptionCONV_TO_NETWORK_FUNCTION = { CURLOPT_CONV_TO_NETWORK_FUNCTION };
CFURLSessionOption const CFURLSessionOptionCONV_FROM_UTF8_FUNCTION = { CURLOPT_CONV_FROM_UTF8_FUNCTION };
CFURLSessionOption const CFURLSessionOptionMAX_SEND_SPEED_LARGE = { CURLOPT_MAX_SEND_SPEED_LARGE };
CFURLSessionOption const CFURLSessionOptionMAX_RECV_SPEED_LARGE = { CURLOPT_MAX_RECV_SPEED_LARGE };
CFURLSessionOption const CFURLSessionOptionFTP_ALTERNATIVE_TO_USER = { CURLOPT_FTP_ALTERNATIVE_TO_USER };
CFURLSessionOption const CFURLSessionOptionSOCKOPTFUNCTION = { CURLOPT_SOCKOPTFUNCTION };
CFURLSessionOption const CFURLSessionOptionSOCKOPTDATA = { CURLOPT_SOCKOPTDATA };
CFURLSessionOption const CFURLSessionOptionSSL_SESSIONID_CACHE = { CURLOPT_SSL_SESSIONID_CACHE };
CFURLSessionOption const CFURLSessionOptionSSH_AUTH_TYPES = { CURLOPT_SSH_AUTH_TYPES };
CFURLSessionOption const CFURLSessionOptionSSH_PUBLIC_KEYFILE = { CURLOPT_SSH_PUBLIC_KEYFILE };
CFURLSessionOption const CFURLSessionOptionSSH_PRIVATE_KEYFILE = { CURLOPT_SSH_PRIVATE_KEYFILE };
CFURLSessionOption const CFURLSessionOptionFTP_SSL_CCC = { CURLOPT_FTP_SSL_CCC };
CFURLSessionOption const CFURLSessionOptionTIMEOUT_MS = { CURLOPT_TIMEOUT_MS };
CFURLSessionOption const CFURLSessionOptionCONNECTTIMEOUT_MS = { CURLOPT_CONNECTTIMEOUT_MS };
CFURLSessionOption const CFURLSessionOptionHTTP_TRANSFER_DECODING = { CURLOPT_HTTP_TRANSFER_DECODING };
CFURLSessionOption const CFURLSessionOptionHTTP_CONTENT_DECODING = { CURLOPT_HTTP_CONTENT_DECODING };
CFURLSessionOption const CFURLSessionOptionNEW_FILE_PERMS = { CURLOPT_NEW_FILE_PERMS };
CFURLSessionOption const CFURLSessionOptionNEW_DIRECTORY_PERMS = { CURLOPT_NEW_DIRECTORY_PERMS };
CFURLSessionOption const CFURLSessionOptionPOSTREDIR = { CURLOPT_POSTREDIR };
CFURLSessionOption const CFURLSessionOptionSSH_HOST_PUBLIC_KEY_MD5 = { CURLOPT_SSH_HOST_PUBLIC_KEY_MD5 };
CFURLSessionOption const CFURLSessionOptionOPENSOCKETFUNCTION = { CURLOPT_OPENSOCKETFUNCTION };
CFURLSessionOption const CFURLSessionOptionOPENSOCKETDATA = { CURLOPT_OPENSOCKETDATA };
CFURLSessionOption const CFURLSessionOptionCOPYPOSTFIELDS = { CURLOPT_COPYPOSTFIELDS };
CFURLSessionOption const CFURLSessionOptionPROXY_TRANSFER_MODE = { CURLOPT_PROXY_TRANSFER_MODE };
CFURLSessionOption const CFURLSessionOptionSEEKFUNCTION = { CURLOPT_SEEKFUNCTION };
CFURLSessionOption const CFURLSessionOptionSEEKDATA = { CURLOPT_SEEKDATA };
CFURLSessionOption const CFURLSessionOptionCRLFILE = { CURLOPT_CRLFILE };
CFURLSessionOption const CFURLSessionOptionISSUERCERT = { CURLOPT_ISSUERCERT };
CFURLSessionOption const CFURLSessionOptionADDRESS_SCOPE = { CURLOPT_ADDRESS_SCOPE };
CFURLSessionOption const CFURLSessionOptionCERTINFO = { CURLOPT_CERTINFO };
CFURLSessionOption const CFURLSessionOptionUSERNAME = { CURLOPT_USERNAME };
CFURLSessionOption const CFURLSessionOptionPASSWORD = { CURLOPT_PASSWORD };
CFURLSessionOption const CFURLSessionOptionPROXYUSERNAME = { CURLOPT_PROXYUSERNAME };
CFURLSessionOption const CFURLSessionOptionPROXYPASSWORD = { CURLOPT_PROXYPASSWORD };
CFURLSessionOption const CFURLSessionOptionNOPROXY = { CURLOPT_NOPROXY };
CFURLSessionOption const CFURLSessionOptionTFTP_BLKSIZE = { CURLOPT_TFTP_BLKSIZE };
CFURLSessionOption const CFURLSessionOptionSOCKS5_GSSAPI_SERVICE = { CURLOPT_SOCKS5_GSSAPI_SERVICE };
CFURLSessionOption const CFURLSessionOptionSOCKS5_GSSAPI_NEC = { CURLOPT_SOCKS5_GSSAPI_NEC };
CFURLSessionOption const CFURLSessionOptionPROTOCOLS = { CURLOPT_PROTOCOLS };
CFURLSessionOption const CFURLSessionOptionREDIR_PROTOCOLS = { CURLOPT_REDIR_PROTOCOLS };
CFURLSessionOption const CFURLSessionOptionSSH_KNOWNHOSTS = { CURLOPT_SSH_KNOWNHOSTS };
CFURLSessionOption const CFURLSessionOptionSSH_KEYFUNCTION = { CURLOPT_SSH_KEYFUNCTION };
CFURLSessionOption const CFURLSessionOptionSSH_KEYDATA = { CURLOPT_SSH_KEYDATA };
CFURLSessionOption const CFURLSessionOptionMAIL_FROM = { CURLOPT_MAIL_FROM };
CFURLSessionOption const CFURLSessionOptionMAIL_RCPT = { CURLOPT_MAIL_RCPT };
CFURLSessionOption const CFURLSessionOptionFTP_USE_PRET = { CURLOPT_FTP_USE_PRET };
CFURLSessionOption const CFURLSessionOptionRTSP_REQUEST = { CURLOPT_RTSP_REQUEST };
CFURLSessionOption const CFURLSessionOptionRTSP_SESSION_ID = { CURLOPT_RTSP_SESSION_ID };
CFURLSessionOption const CFURLSessionOptionRTSP_STREAM_URI = { CURLOPT_RTSP_STREAM_URI };
CFURLSessionOption const CFURLSessionOptionRTSP_TRANSPORT = { CURLOPT_RTSP_TRANSPORT };
CFURLSessionOption const CFURLSessionOptionRTSP_CLIENT_CSEQ = { CURLOPT_RTSP_CLIENT_CSEQ };
CFURLSessionOption const CFURLSessionOptionRTSP_SERVER_CSEQ = { CURLOPT_RTSP_SERVER_CSEQ };
CFURLSessionOption const CFURLSessionOptionINTERLEAVEDATA = { CURLOPT_INTERLEAVEDATA };
CFURLSessionOption const CFURLSessionOptionINTERLEAVEFUNCTION = { CURLOPT_INTERLEAVEFUNCTION };
CFURLSessionOption const CFURLSessionOptionWILDCARDMATCH = { CURLOPT_WILDCARDMATCH };
CFURLSessionOption const CFURLSessionOptionCHUNK_BGN_FUNCTION = { CURLOPT_CHUNK_BGN_FUNCTION };
CFURLSessionOption const CFURLSessionOptionCHUNK_END_FUNCTION = { CURLOPT_CHUNK_END_FUNCTION };
CFURLSessionOption const CFURLSessionOptionFNMATCH_FUNCTION = { CURLOPT_FNMATCH_FUNCTION };
CFURLSessionOption const CFURLSessionOptionCHUNK_DATA = { CURLOPT_CHUNK_DATA };
CFURLSessionOption const CFURLSessionOptionFNMATCH_DATA = { CURLOPT_FNMATCH_DATA };
CFURLSessionOption const CFURLSessionOptionRESOLVE = { CURLOPT_RESOLVE };
CFURLSessionOption const CFURLSessionOptionTLSAUTH_USERNAME = { CURLOPT_TLSAUTH_USERNAME };
CFURLSessionOption const CFURLSessionOptionTLSAUTH_PASSWORD = { CURLOPT_TLSAUTH_PASSWORD };
CFURLSessionOption const CFURLSessionOptionTLSAUTH_TYPE = { CURLOPT_TLSAUTH_TYPE };
CFURLSessionOption const CFURLSessionOptionTRANSFER_ENCODING = { CURLOPT_TRANSFER_ENCODING };
CFURLSessionOption const CFURLSessionOptionCLOSESOCKETFUNCTION = { CURLOPT_CLOSESOCKETFUNCTION };
CFURLSessionOption const CFURLSessionOptionCLOSESOCKETDATA = { CURLOPT_CLOSESOCKETDATA };
CFURLSessionOption const CFURLSessionOptionGSSAPI_DELEGATION = { CURLOPT_GSSAPI_DELEGATION };
CFURLSessionOption const CFURLSessionOptionDNS_SERVERS = { CURLOPT_DNS_SERVERS };
CFURLSessionOption const CFURLSessionOptionACCEPTTIMEOUT_MS = { CURLOPT_ACCEPTTIMEOUT_MS };
CFURLSessionOption const CFURLSessionOptionTCP_KEEPALIVE = { CURLOPT_TCP_KEEPALIVE };
CFURLSessionOption const CFURLSessionOptionTCP_KEEPIDLE = { CURLOPT_TCP_KEEPIDLE };
CFURLSessionOption const CFURLSessionOptionTCP_KEEPINTVL = { CURLOPT_TCP_KEEPINTVL };
CFURLSessionOption const CFURLSessionOptionSSL_OPTIONS = { CURLOPT_SSL_OPTIONS };
CFURLSessionOption const CFURLSessionOptionMAIL_AUTH = { CURLOPT_MAIL_AUTH };
#if !NS_CURL_MISSING_XFERINFOFUNCTION
CFURLSessionOption const CFURLSessionOptionXFERINFOFUNCTION = { CURLOPT_XFERINFOFUNCTION };
#endif

CFURLSessionInfo const CFURLSessionInfoTEXT = { CURLINFO_TEXT };
CFURLSessionInfo const CFURLSessionInfoHEADER_IN = { CURLINFO_HEADER_IN };
CFURLSessionInfo const CFURLSessionInfoHEADER_OUT = { CURLINFO_HEADER_OUT };
CFURLSessionInfo const CFURLSessionInfoDATA_IN = { CURLINFO_DATA_IN };
CFURLSessionInfo const CFURLSessionInfoDATA_OUT = { CURLINFO_DATA_OUT };
CFURLSessionInfo const CFURLSessionInfoSSL_DATA_IN = { CURLINFO_SSL_DATA_IN };
CFURLSessionInfo const CFURLSessionInfoSSL_DATA_OUT = { CURLINFO_SSL_DATA_OUT };
CFURLSessionInfo const CFURLSessionInfoEND = { CURLINFO_END };
CFURLSessionInfo const CFURLSessionInfoNONE = { CURLINFO_NONE };
CFURLSessionInfo const CFURLSessionInfoEFFECTIVE_URL = { CURLINFO_EFFECTIVE_URL };
CFURLSessionInfo const CFURLSessionInfoRESPONSE_CODE = { CURLINFO_RESPONSE_CODE };
CFURLSessionInfo const CFURLSessionInfoTOTAL_TIME = { CURLINFO_TOTAL_TIME };
CFURLSessionInfo const CFURLSessionInfoNAMELOOKUP_TIME = { CURLINFO_NAMELOOKUP_TIME };
CFURLSessionInfo const CFURLSessionInfoCONNECT_TIME = { CURLINFO_CONNECT_TIME };
CFURLSessionInfo const CFURLSessionInfoPRETRANSFER_TIME = { CURLINFO_PRETRANSFER_TIME };
CFURLSessionInfo const CFURLSessionInfoSIZE_UPLOAD = { CURLINFO_SIZE_UPLOAD };
CFURLSessionInfo const CFURLSessionInfoSIZE_DOWNLOAD = { CURLINFO_SIZE_DOWNLOAD };
CFURLSessionInfo const CFURLSessionInfoSPEED_DOWNLOAD = { CURLINFO_SPEED_DOWNLOAD };
CFURLSessionInfo const CFURLSessionInfoSPEED_UPLOAD = { CURLINFO_SPEED_UPLOAD };
CFURLSessionInfo const CFURLSessionInfoHEADER_SIZE = { CURLINFO_HEADER_SIZE };
CFURLSessionInfo const CFURLSessionInfoREQUEST_SIZE = { CURLINFO_REQUEST_SIZE };
CFURLSessionInfo const CFURLSessionInfoSSL_VERIFYRESULT = { CURLINFO_SSL_VERIFYRESULT };
CFURLSessionInfo const CFURLSessionInfoFILETIME = { CURLINFO_FILETIME };
CFURLSessionInfo const CFURLSessionInfoCONTENT_LENGTH_DOWNLOAD = { CURLINFO_CONTENT_LENGTH_DOWNLOAD };
CFURLSessionInfo const CFURLSessionInfoCONTENT_LENGTH_UPLOAD = { CURLINFO_CONTENT_LENGTH_UPLOAD };
CFURLSessionInfo const CFURLSessionInfoSTARTTRANSFER_TIME = { CURLINFO_STARTTRANSFER_TIME };
CFURLSessionInfo const CFURLSessionInfoCONTENT_TYPE = { CURLINFO_CONTENT_TYPE };
CFURLSessionInfo const CFURLSessionInfoREDIRECT_TIME = { CURLINFO_REDIRECT_TIME };
CFURLSessionInfo const CFURLSessionInfoREDIRECT_COUNT = { CURLINFO_REDIRECT_COUNT };
CFURLSessionInfo const CFURLSessionInfoPRIVATE = { CURLINFO_PRIVATE };
CFURLSessionInfo const CFURLSessionInfoHTTP_CONNECTCODE = { CURLINFO_HTTP_CONNECTCODE };
CFURLSessionInfo const CFURLSessionInfoHTTPAUTH_AVAIL = { CURLINFO_HTTPAUTH_AVAIL };
CFURLSessionInfo const CFURLSessionInfoPROXYAUTH_AVAIL = { CURLINFO_PROXYAUTH_AVAIL };
CFURLSessionInfo const CFURLSessionInfoOS_ERRNO = { CURLINFO_OS_ERRNO };
CFURLSessionInfo const CFURLSessionInfoNUM_CONNECTS = { CURLINFO_NUM_CONNECTS };
CFURLSessionInfo const CFURLSessionInfoSSL_ENGINES = { CURLINFO_SSL_ENGINES };
CFURLSessionInfo const CFURLSessionInfoCOOKIELIST = { CURLINFO_COOKIELIST };
CFURLSessionInfo const CFURLSessionInfoLASTSOCKET = { CURLINFO_LASTSOCKET };
CFURLSessionInfo const CFURLSessionInfoFTP_ENTRY_PATH = { CURLINFO_FTP_ENTRY_PATH };
CFURLSessionInfo const CFURLSessionInfoREDIRECT_URL = { CURLINFO_REDIRECT_URL };
CFURLSessionInfo const CFURLSessionInfoPRIMARY_IP = { CURLINFO_PRIMARY_IP };
CFURLSessionInfo const CFURLSessionInfoAPPCONNECT_TIME = { CURLINFO_APPCONNECT_TIME };
CFURLSessionInfo const CFURLSessionInfoCERTINFO = { CURLINFO_CERTINFO };
CFURLSessionInfo const CFURLSessionInfoCONDITION_UNMET = { CURLINFO_CONDITION_UNMET };
CFURLSessionInfo const CFURLSessionInfoRTSP_SESSION_ID = { CURLINFO_RTSP_SESSION_ID };
CFURLSessionInfo const CFURLSessionInfoRTSP_CLIENT_CSEQ = { CURLINFO_RTSP_CLIENT_CSEQ };
CFURLSessionInfo const CFURLSessionInfoRTSP_SERVER_CSEQ = { CURLINFO_RTSP_SERVER_CSEQ };
CFURLSessionInfo const CFURLSessionInfoRTSP_CSEQ_RECV = { CURLINFO_RTSP_CSEQ_RECV };
CFURLSessionInfo const CFURLSessionInfoPRIMARY_PORT = { CURLINFO_PRIMARY_PORT };
CFURLSessionInfo const CFURLSessionInfoLOCAL_IP = { CURLINFO_LOCAL_IP };
CFURLSessionInfo const CFURLSessionInfoLOCAL_PORT = { CURLINFO_LOCAL_PORT };
CFURLSessionInfo const CFURLSessionInfoLASTONE = { CURLINFO_LASTONE };


CFURLSessionMultiOption const CFURLSessionMultiOptionSOCKETFUNCTION = { CURLMOPT_SOCKETFUNCTION };
CFURLSessionMultiOption const CFURLSessionMultiOptionSOCKETDATA = { CURLMOPT_SOCKETDATA };
CFURLSessionMultiOption const CFURLSessionMultiOptionPIPELINING = { CURLMOPT_PIPELINING };
CFURLSessionMultiOption const CFURLSessionMultiOptionTIMERFUNCTION = { CURLMOPT_TIMERFUNCTION };
CFURLSessionMultiOption const CFURLSessionMultiOptionTIMERDATA = { CURLMOPT_TIMERDATA };
CFURLSessionMultiOption const CFURLSessionMultiOptionMAXCONNECTS = { CURLMOPT_MAXCONNECTS };
#if !NS_CURL_MISSING_MAX_HOST_CONNECTIONS
CFURLSessionMultiOption const CFURLSessionMultiOptionMAX_HOST_CONNECTIONS = { CURLMOPT_MAX_HOST_CONNECTIONS };
#endif


CFURLSessionMultiCode const CFURLSessionMultiCodeCALL_MULTI_PERFORM = { CURLM_CALL_MULTI_PERFORM };
CFURLSessionMultiCode const CFURLSessionMultiCodeOK = { CURLM_OK };
CFURLSessionMultiCode const CFURLSessionMultiCodeBAD_HANDLE = { CURLM_BAD_HANDLE };
CFURLSessionMultiCode const CFURLSessionMultiCodeBAD_EASY_HANDLE = { CURLM_BAD_EASY_HANDLE };
CFURLSessionMultiCode const CFURLSessionMultiCodeOUT_OF_MEMORY = { CURLM_OUT_OF_MEMORY };
CFURLSessionMultiCode const CFURLSessionMultiCodeINTERNAL_ERROR = { CURLM_INTERNAL_ERROR };
CFURLSessionMultiCode const CFURLSessionMultiCodeBAD_SOCKET = { CURLM_BAD_SOCKET };
CFURLSessionMultiCode const CFURLSessionMultiCodeUNKNOWN_OPTION = { CURLM_UNKNOWN_OPTION };
CFURLSessionMultiCode const CFURLSessionMultiCodeLAST = { CURLM_LAST };


CFURLSessionPoll const CFURLSessionPollNone = { CURL_POLL_NONE };
CFURLSessionPoll const CFURLSessionPollIn = { CURL_POLL_IN };
CFURLSessionPoll const CFURLSessionPollOut = { CURL_POLL_OUT };
CFURLSessionPoll const CFURLSessionPollInOut = { CURL_POLL_INOUT };
CFURLSessionPoll const CFURLSessionPollRemove = { CURL_POLL_REMOVE };

char *CFURLSessionCurlVersionString(void) {
    return curl_version();
}
CFURLSessionCurlVersion CFURLSessionCurlVersionInfo(void) {
    curl_version_info_data *info = curl_version_info(CURLVERSION_NOW);
    CFURLSessionCurlVersion v = {
        info->version_num >> 16 & 0xff,
        info->version_num >> 8 & 0xff,
        info->version_num & 0xff,
    };
    return v;
}


int const CFURLSessionWriteFuncPause = CURL_WRITEFUNC_PAUSE;
int const CFURLSessionReadFuncPause = CURL_READFUNC_PAUSE;
int const CFURLSessionReadFuncAbort = CURL_READFUNC_ABORT;


CFURLSession_socket_t const CFURLSessionSocketTimeout = CURL_SOCKET_TIMEOUT;

int const CFURLSessionSeekOk = CURL_SEEKFUNC_OK;
int const CFURLSessionSeekCantSeek = CURL_SEEKFUNC_CANTSEEK;
int const CFURLSessionSeekFail = CURL_SEEKFUNC_FAIL;

CFURLSessionSList *_Nullable CFURLSessionSListAppend(CFURLSessionSList *_Nullable list, const char * _Nullable string) {
    return (CFURLSessionSList *) curl_slist_append((struct curl_slist *) list, string);
}
void CFURLSessionSListFreeAll(CFURLSessionSList *_Nullable list) {
    curl_slist_free_all((struct curl_slist *) list);
}
