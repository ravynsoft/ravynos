//
//  liblog_mdnsresponder.m
//  liblog_mdnsresponder
//

#import <Foundation/Foundation.h>
#import <arpa/inet.h>
#import <os/log_private.h>
#import "DNSCommon.h"
#undef DomainNameLength // undefines DomainNameLength since we need to use DomainNameLength that is also defined in DNSMessage.h
#import "DNSMessage.h"

// MDNS Mutable Attribute String
#define MDNSAS(str) [[NSAttributedString alloc] initWithString:(str)]
#define MDNSASWithFormat(format, ...) MDNSAS(([[NSString alloc] initWithFormat:format, ##__VA_ARGS__]))
#define MAX_MDNS_ADDR_STRING_LENGTH 45

// os_log(OS_LOG_DEFAULT, "IP Address(IPv4/IPv6): %{mdnsresponder:ip_addr}.20P", <the address of mDNSAddr structure>);
static NS_RETURNS_RETAINED NSAttributedString *
MDNSOLCopyFormattedStringmDNSIPAddr(id value)
{
    const mDNSAddr *mdns_addr_p;
    char buffer[MAX_MDNS_ADDR_STRING_LENGTH + 1];
    buffer[MAX_MDNS_ADDR_STRING_LENGTH] = 0;

    if ([(NSObject *)value isKindOfClass:[NSData class]]) {
        NSData *data = (NSData *)value;
        if (data.bytes == NULL || data.length == 0) {
            return MDNSAS(@"<NULL IP ADDRESS>");
        }

        if (data.length != sizeof(mDNSAddr)) {
            return MDNSASWithFormat(@"<fail decode - size> %zd != %zd", (size_t)data.length, sizeof(mDNSAddr));
        }

        mdns_addr_p = (const mDNSAddr *)data.bytes;
    } else {
        return MDNSASWithFormat(@"<fail decode - data type> %@", [(NSObject *)value description]);
    }

    bool failed_conversion = false;
    switch (mdns_addr_p->type) {
        case mDNSAddrType_IPv4:
        {
            __unused char sizecheck_buffer[(sizeof(buffer) >= INET_ADDRSTRLEN) ? 1 : -1];
            if (!inet_ntop(AF_INET, (const void *)&mdns_addr_p->ip.v4.NotAnInteger, buffer, sizeof(buffer)))
                failed_conversion = true;
            break;
        }
        case mDNSAddrType_IPv6:
        {
            __unused char sizecheck_buffer[(sizeof(buffer) >= INET6_ADDRSTRLEN) ? 1 : -1];
            if (!inet_ntop(AF_INET6, (const void *)mdns_addr_p->ip.v6.b, buffer, sizeof(buffer)))
                failed_conversion = true;
            break;
        }
        default:
            failed_conversion = true;
            break;
    }
    if (failed_conversion) {
        return MDNSAS(@"<failed conversion>");
    }

    NSString *str = @(buffer);
    return MDNSAS(str ? str : @("<Could not create NSString>"));
}

// os_log(OS_LOG_DEFAULT, "MAC Address: %{mdnsresponder:mac_addr}.6P", <the address of 6-byte MAC address>);
#define MAC_ADDRESS_LEN 6
static NS_RETURNS_RETAINED NSAttributedString *
MDNSOLCopyFormattedStringmDNSMACAddr(id value)
{
    const uint8_t *mac_addr = NULL;
    char buffer[MAX_MDNS_ADDR_STRING_LENGTH + 1];
    buffer[MAX_MDNS_ADDR_STRING_LENGTH] = 0;

    if ([(NSObject *)value isKindOfClass:[NSData class]]) {
        NSData *data = (NSData *)value;
        if (data.bytes == NULL || data.length == 0) {
            return MDNSAS(@"<NULL MAC ADDRESS>");
        }

        if (data.length != MAC_ADDRESS_LEN) {
            return MDNSASWithFormat(@"<fail decode - size> %zd != %zd", (size_t)data.length, MAC_ADDRESS_LEN);
        }

        mac_addr = (const uint8_t *)data.bytes;
    } else {
        return MDNSASWithFormat(@"<fail decode - data type> %@", [(NSObject *)value description]);
    }

    int ret_snprintf = snprintf(buffer, MAX_MDNS_ADDR_STRING_LENGTH, "%02X:%02X:%02X:%02X:%02X:%02X",
                                mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    if (ret_snprintf < 0) {
        return MDNSAS(@"<failed conversion>");
    }

    NSString *str = @(buffer);
    return MDNSAS(str ? str : @("<Could not create NSString>"));
}

// os_log(OS_LOG_DEFAULT, "Domain Name: %{mdnsresponder:domain_name}.*P", <the address of domainname structure>);
// Leave some extra space to allow log routine to put error message when decode fails at the end of the buffer.
static NS_RETURNS_RETAINED NSAttributedString *
MDNSOLCopyFormattedStringmDNSLabelSequenceName(id value)
{
    char buffer[kDNSServiceMaxDomainName];
    NSData *data = (NSData *)value;
    OSStatus ret;

    if ([(NSObject *)value isKindOfClass:[NSData class]]) {
        if (data.bytes == NULL || data.length == 0) {
            return MDNSAS(@"<NULL DOMAIN NAME>");
        }
    } else {
        return MDNSASWithFormat(@"<fail decode - data type> %@", [(NSObject *)value description]);
    }

    buffer[0] = '\0';
    ret = DomainNameToString((const uint8_t *)data.bytes, ((const uint8_t *) data.bytes) + data.length, buffer, NULL);
    if (ret != kNoErr) {
        snprintf(buffer, sizeof(buffer), "<Malformed Domain Name>");
    }

    NSString *str = @(buffer);
    return MDNSAS(str ? str : @("<Could not create NSString>"));
}

struct MDNSOLFormatters {
    const char *type;
    NS_RETURNS_RETAINED NSAttributedString *(*function)(id);
};

NS_RETURNS_RETAINED
NSAttributedString *
OSLogCopyFormattedString(const char *type, id value, __unused os_log_type_info_t info)
{
    static const struct MDNSOLFormatters formatters[] = {
        { .type = "ip_addr",       .function = MDNSOLCopyFormattedStringmDNSIPAddr },
        { .type = "mac_addr",      .function = MDNSOLCopyFormattedStringmDNSMACAddr },
        { .type = "domain_name",   .function = MDNSOLCopyFormattedStringmDNSLabelSequenceName },
    };

    for (int i = 0; i < (int)(sizeof(formatters) / sizeof(formatters[0])); i++) {
        if (strcmp(type, formatters[i].type) == 0) {
            return formatters[i].function(value);
        }
    }

    return nil;
}
