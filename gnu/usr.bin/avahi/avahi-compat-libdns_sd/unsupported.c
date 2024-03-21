/***
  This file is part of avahi.

  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <avahi-common/gccmacro.h>

#include "dns_sd.h"
#include "warn.h"

DNSServiceErrorType DNSSD_API DNSServiceRegisterRecord (
    AVAHI_GCC_UNUSED DNSServiceRef sdRef,
    AVAHI_GCC_UNUSED DNSRecordRef *RecordRef,
    AVAHI_GCC_UNUSED DNSServiceFlags flags,
    AVAHI_GCC_UNUSED uint32_t interfaceIndex,
    AVAHI_GCC_UNUSED const char *fullname,
    AVAHI_GCC_UNUSED uint16_t rrtype,
    AVAHI_GCC_UNUSED uint16_t rrclass,
    AVAHI_GCC_UNUSED uint16_t rdlen,
    AVAHI_GCC_UNUSED const void *rdata,
    AVAHI_GCC_UNUSED uint32_t ttl,
    AVAHI_GCC_UNUSED DNSServiceRegisterRecordReply callBack,
    AVAHI_GCC_UNUSED void *context) {

    AVAHI_WARN_UNSUPPORTED;

    return kDNSServiceErr_Unsupported;
}

DNSServiceErrorType DNSSD_API DNSServiceReconfirmRecord (
    AVAHI_GCC_UNUSED DNSServiceFlags flags,
    AVAHI_GCC_UNUSED uint32_t interfaceIndex,
    AVAHI_GCC_UNUSED const char *fullname,
    AVAHI_GCC_UNUSED uint16_t rrtype,
    AVAHI_GCC_UNUSED uint16_t rrclass,
    AVAHI_GCC_UNUSED uint16_t rdlen,
    AVAHI_GCC_UNUSED const void *rdata) {

    AVAHI_WARN_UNSUPPORTED;

    return kDNSServiceErr_Unsupported;
}

DNSServiceErrorType DNSSD_API DNSServiceCreateConnection(AVAHI_GCC_UNUSED DNSServiceRef *sdRef) {
    AVAHI_WARN_UNSUPPORTED;

    return kDNSServiceErr_Unsupported;
}

DNSServiceErrorType DNSSD_API DNSServiceAddRecord(
    AVAHI_GCC_UNUSED DNSServiceRef sdRef,
    AVAHI_GCC_UNUSED DNSRecordRef *RecordRef,
    AVAHI_GCC_UNUSED DNSServiceFlags flags,
    AVAHI_GCC_UNUSED uint16_t rrtype,
    AVAHI_GCC_UNUSED uint16_t rdlen,
    AVAHI_GCC_UNUSED const void *rdata,
    AVAHI_GCC_UNUSED uint32_t ttl) {

    AVAHI_WARN_UNSUPPORTED;

    return kDNSServiceErr_Unsupported;
}

DNSServiceErrorType DNSSD_API DNSServiceRemoveRecord(
    AVAHI_GCC_UNUSED DNSServiceRef sdRef,
    AVAHI_GCC_UNUSED DNSRecordRef RecordRef,
    AVAHI_GCC_UNUSED DNSServiceFlags flags) {

    AVAHI_WARN_UNSUPPORTED;

    return kDNSServiceErr_Unsupported;
}
