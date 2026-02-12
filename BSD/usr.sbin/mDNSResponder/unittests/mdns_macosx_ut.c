#include "DNSCommon.h"                  // Defines general DNS utility routines
#include "unittest_common.h"
#include "mDNSMacOSX.h"

// To match *either* a v4 or v6 instance of this interface
mDNSlocal mDNSInterfaceID SearchForInterfaceByAddr(mDNSAddr* addr)
{
	NetworkInterfaceInfoOSX *i;
	for (i = mDNSStorage.p->InterfaceList; i; i = i->next)
		if (i->Exists)
		{
			if ((i->ifinfo.ip.type == mDNSAddrType_IPv4) &&
				i->ifinfo.ip.ip.v4.NotAnInteger == addr->ip.v4.NotAnInteger)
				return i->ifinfo.InterfaceID;
			else if ((i->ifinfo.ip.type == mDNSAddrType_IPv6) &&
					 (i->ifinfo.ip.ip.v6.l[0] == addr->ip.v6.l[0] &&
					  i->ifinfo.ip.ip.v6.l[1] == addr->ip.v6.l[1] &&
					  i->ifinfo.ip.ip.v6.l[2] == addr->ip.v6.l[2] &&
					  i->ifinfo.ip.ip.v6.l[3] == addr->ip.v6.l[3])
					 )
				return i->ifinfo.InterfaceID;
		}
	return(NULL);
}

mDNSexport void SetInterfaces_ut(mDNSInterfaceID* pri_id, mDNSAddr *pri_v4, mDNSAddr* pri_v6, mDNSAddr* pri_router)
{
	mDNSs32 utc = mDNSPlatformUTC();

	MarkAllInterfacesInactive(utc);
	UpdateInterfaceList(utc);
	ClearInactiveInterfaces(utc);
	SetupActiveInterfaces(utc);

	// set primary interface info
	{
		mDNSAddr* addr;
		NetworkChangedKey_IPv4         = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainState, kSCEntNetIPv4);
		NetworkChangedKey_IPv6         = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainState, kSCEntNetIPv6);
		NetworkChangedKey_Hostnames    = SCDynamicStoreKeyCreateHostNames(NULL);
		NetworkChangedKey_Computername = SCDynamicStoreKeyCreateComputerName(NULL);
		NetworkChangedKey_DNS          = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainState, kSCEntNetDNS);
		NetworkChangedKey_StateInterfacePrefix = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL, kSCDynamicStoreDomainState, CFSTR(""), NULL);

		mDNSPlatformGetPrimaryInterface(pri_v4, pri_v6, pri_router);
		addr = (pri_v4->type == mDNSAddrType_IPv4) ? pri_v4 : pri_v6;
		*pri_id = SearchForInterfaceByAddr(addr);

		CFRelease(NetworkChangedKey_IPv4);
		CFRelease(NetworkChangedKey_IPv6);
		CFRelease(NetworkChangedKey_Hostnames);
		CFRelease(NetworkChangedKey_Computername);
		CFRelease(NetworkChangedKey_DNS);
		CFRelease(NetworkChangedKey_StateInterfacePrefix);
	}
}

mDNSexport mDNSBool mDNSMacOSXCreateEtcHostsEntry_ut(const domainname *domain, const struct sockaddr *sa, const domainname *cname, char *ifname, AuthHash *auth)
{
	return mDNSMacOSXCreateEtcHostsEntry(domain, sa, cname, ifname, auth);
}

mDNSexport void UpdateEtcHosts_ut(void *context)
{
	mDNS_Lock(&mDNSStorage);
	UpdateEtcHosts(&mDNSStorage, context);
	mDNS_Unlock(&mDNSStorage);
}

mDNSexport void mDNSDomainLabelFromCFString_ut(CFStringRef cfs, domainlabel *const namelabel)
{
    mDNSDomainLabelFromCFString(cfs, namelabel);
}

mDNSexport mDNSu32 IndexForInterfaceByName_ut(const char *ifname)
{
    NetworkInterfaceInfoOSX * i = SearchForInterfaceByName(ifname, AF_UNSPEC);
    return (i ? i->scope_id : 0);
}
