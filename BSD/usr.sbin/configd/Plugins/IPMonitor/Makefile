ifeq ($(PLATFORM),iphoneos)
# iOS internal SDK
ARCHS=arm64
endif

ifeq ($(PLATFORM),)
PLATFORM=macosx
endif

ifeq ($(PLATFORM),macosx)
# Mac OS X internal SDK
ARCHS=x86_64
endif

# Mac OS X or iOS internal SDK
SDK=$(PLATFORM).internal
SYSROOT=$(shell xcodebuild -version -sdk $(SDK) Path)
CC = xcrun -sdk $(SDK) cc
PF_INC = -F$(SYSROOT)/System/Library/PrivateFrameworks

ARCH_FLAGS=$(foreach a,$(ARCHS),-arch $(a))

EXTRA_CFLAGS=
TEST_INCLUDE=-I. -I../common -I../../dnsinfo -I../../nwi -I../../libSystemConfiguration -I../../SystemConfiguration.fproj -I../../IPMonitorControl -I../../IPMonitorControl/AWD -I$(SYSROOT)/System/Library/Frameworks/System.framework/PrivateHeaders

REFERENCE_OUTPUT=../../common/reference_output.sh

all: test_ipv4_routelist test_ipv6_routelist

# ----------

agent-monitor.o: agent-monitor.m
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} -Wall -O0 -g -c agent-monitor.m

configAgent.o: configAgent.m
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} -Wall -O0 -g -c configAgent.m

controller.o: controller.m
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} -Wall -O0 -g -c controller.m

dnsAgent.o: dnsAgent.m
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} -Wall -O0 -g -c dnsAgent.m

proxyAgent.o: proxyAgent.m
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} -Wall -O0 -g -c proxyAgent.m

dnsinfo_create.o: ../../dnsinfo/dnsinfo_create.h ../../dnsinfo/dnsinfo_create.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} -Wall -O0 -g -c ../../dnsinfo/dnsinfo_create.c

dnsinfo_flatfile.o: ../../dnsinfo/dnsinfo_copy.c ../../dnsinfo/dnsinfo_flatfile.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} -D_PATH_RESOLVER_DIR='"/var/tmp/resolver"' -Wall -O0 -g -c ../../dnsinfo/dnsinfo_flatfile.c

dnsinfo_server.o: ../../dnsinfo/dnsinfo_copy.c ../../dnsinfo/dnsinfo_server.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} -Wall -O0 -g -c ../../dnsinfo/dnsinfo_server.c

dns-configuration.o: dns-configuration.h dns-configuration.c dnsinfo_create.o
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} ${EXTRA_CFLAGS} -Wall -O0 -g -c dns-configuration.c

network_state_information_priv.o: ../../nwi/network_state_information_priv.h ../../nwi/network_state_information_priv.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} -Wall -O0 -g -c ../../nwi/network_state_information_priv.c

network_information_server.o: ../../nwi/network_information_server.h ../../nwi/network_information_server.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} -Wall -O0 -g -c ../../nwi/network_information_server.c

proxy-configuration.o: proxy-configuration.h proxy-configuration.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} -Wall -O0 -g -c proxy-configuration.c

set-hostname.o: set-hostname.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} ${EXTRA_CFLAGS} -Wall -O0 -g -c set-hostname.c

smb-configuration.o: smb-configuration.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} ${EXTRA_CFLAGS} -Wall -O0 -g -c smb-configuration.c

libSystemConfiguration_client.o: ../../libSystemConfiguration/libSystemConfiguration_client.h ../../libSystemConfiguration/libSystemConfiguration_client.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} -Wall -O0 -g -c ../../libSystemConfiguration/libSystemConfiguration_client.c

libSystemConfiguration_server.o: ../../libSystemConfiguration/libSystemConfiguration_server.h ../../libSystemConfiguration/libSystemConfiguration_server.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} -Wall -O0 -g -c ../../libSystemConfiguration/libSystemConfiguration_server.c

IPMonitorControlPrefs.o: ../common/IPMonitorControlPrefs.h ../common/IPMonitorControlPrefs.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} -Wall -O0 -g -c ../common/IPMonitorControlPrefs.c

IPMonitorControlServer.o: ../../IPMonitorControl/IPMonitorControlServer.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) ${TEST_INCLUDE} -Wall -O0 -g -c $^

# ----------

ip_pluginX.o: Makefile ip_plugin.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -DTEST_DNS ${TEST_INCLUDE} ${EXTRA_CFLAGS} -Wall -O0 -g -c -o ip_pluginX.o ip_plugin.c

# ----------

dns-configurationX.o: Makefile dns-configuration.h dns-configuration.c dnsinfo_create.o
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -DMAIN ${TEST_INCLUDE} ${EXTRA_CFLAGS} -Wall -O0 -g -c -o dns-configurationX.o dns-configuration.c

test_dns: ip_pluginX.o IPMonitorControlPrefs.o agent-monitor.o configAgent.o controller.o dnsAgent.o proxyAgent.o dnsinfo_create.o dnsinfo_flatfile.o dnsinfo_server.o network_state_information_priv.o network_information_server.o dns-configurationX.o proxy-configuration.o set-hostname.o smb-configuration.o IPMonitorControlServer.o libSystemConfiguration_client.o libSystemConfiguration_server.o
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -Wall -O0 -g -o test_dns ip_pluginX.o IPMonitorControlPrefs.o agent-monitor.o configAgent.o controller.o dnsAgent.o proxyAgent.o dnsinfo_create.o dnsinfo_flatfile.o dnsinfo_server.o network_state_information_priv.o network_information_server.o dns-configurationX.o proxy-configuration.o set-hostname.o smb-configuration.o IPMonitorControlServer.o libSystemConfiguration_client.o libSystemConfiguration_server.o ${EXTRA_CFLAGS} -lnetwork -framework SystemConfiguration -framework CoreFoundation -framework Foundation -framework Network -framework NetworkExtension

# ----------

proxy-configurationX.o: Makefile proxy-configuration.h proxy-configuration.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -DMAIN ${TEST_INCLUDE} ${EXTRA_CFLAGS} -Wall -O0 -g -c -o proxy-configurationX.o proxy-configuration.c

test_proxy: ip_pluginX.o IPMonitorControlPrefs.o agent-monitor.o configAgent.o controller.o dnsAgent.o proxyAgent.o dnsinfo_create.o dnsinfo_flatfile.o dnsinfo_server.o network_state_information_priv.o network_information_server.o dns-configuration.o proxy-configurationX.o set-hostname.o smb-configuration.o IPMonitorControlServer.o libSystemConfiguration_client.o libSystemConfiguration_server.o
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -Wall -O0 -g -o test_proxy ip_pluginX.o IPMonitorControlPrefs.o agent-monitor.o configAgent.o controller.o dnsAgent.o proxyAgent.o dnsinfo_create.o dnsinfo_flatfile.o dnsinfo_server.o network_state_information_priv.o network_information_server.o dns-configuration.o proxy-configurationX.o set-hostname.o smb-configuration.o IPMonitorControlServer.o libSystemConfiguration_client.o libSystemConfiguration_server.o ${EXTRA_CFLAGS} -lnetwork -framework SystemConfiguration -framework CoreFoundation -framework Foundation -framework Network -framework NetworkExtension

# ----------

set-hostnameX.o: Makefile set-hostname.h set-hostname.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -DMAIN -DDEBUG ${TEST_INCLUDE} ${EXTRA_CFLAGS} -Wall -O0 -g -c -o set-hostnameX.o set-hostname.c

test_hostname: ip_pluginX.o IPMonitorControlPrefs.o agent-monitor.o configAgent.o controller.o dnsAgent.o proxyAgent.o dnsinfo_create.o dnsinfo_flatfile.o dnsinfo_server.o network_state_information_priv.o network_information_server.o dns-configuration.o proxy-configuration.o set-hostnameX.o smb-configuration.o IPMonitorControlServer.o libSystemConfiguration_client.o libSystemConfiguration_server.o
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -Wall -O0 -g -o test_hostname ip_pluginX.o IPMonitorControlPrefs.o agent-monitor.o configAgent.o controller.o dnsAgent.o proxyAgent.o dnsinfo_create.o dnsinfo_flatfile.o dnsinfo_server.o network_state_information_priv.o network_information_server.o dns-configuration.o proxy-configuration.o set-hostnameX.o smb-configuration.o IPMonitorControlServer.o libSystemConfiguration_client.o libSystemConfiguration_server.o ${EXTRA_CFLAGS} -lnetwork -framework SystemConfiguration -framework CoreFoundation -framework Foundation -framework Network -framework NetworkExtension

# ----------

smb-configurationX.o: smb-configuration.h smb-configuration.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -DMAIN -DDEBUG ${TEST_INCLUDE} ${EXTRA_CFLAGS} -Wall -O0 -g -c -o smb-configurationX.o smb-configuration.c

test_smb: ip_pluginX.o IPMonitorControlPrefs.o agent-monitor.o configAgent.o controller.o dnsAgent.o proxyAgent.o dnsinfo_create.o dnsinfo_flatfile.o dnsinfo_server.o network_state_information_priv.o network_information_server.o dns-configuration.o proxy-configuration.o set-hostname.o smb-configurationX.o IPMonitorControlServer.o libSystemConfiguration_client.o libSystemConfiguration_server.o
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -Wall -O0 -g -o test_smb ip_pluginX.o IPMonitorControlPrefs.o agent-monitor.o configAgent.o controller.o dnsAgent.o proxyAgent.o dnsinfo_create.o dnsinfo_flatfile.o dnsinfo_server.o network_state_information_priv.o network_information_server.o dns-configuration.o proxy-configuration.o set-hostname.o smb-configurationX.o IPMonitorControlServer.o libSystemConfiguration_client.o libSystemConfiguration_server.o ${EXTRA_CFLAGS} -lnetwork -framework SystemConfiguration -framework CoreFoundation -framework Foundation -framework Network -framework NetworkExtension

# ----------

test_dns_order.o: ip_plugin.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -DTEST_DNS_ORDER ${TEST_INCLUDE} ${EXTRA_CFLAGS} -Wall -O0 -g -c -o $@ $^

test_dns_order: test_dns_order.o IPMonitorControlPrefs.o agent-monitor.o configAgent.o controller.o dnsAgent.o proxyAgent.o dnsinfo_create.o dnsinfo_flatfile.o dnsinfo_server.o network_state_information_priv.o network_information_server.o dns-configuration.o proxy-configuration.o set-hostname.o smb-configuration.o IPMonitorControlServer.o libSystemConfiguration_client.o libSystemConfiguration_server.o
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -Wall -O0 -g -o test_dns_order test_dns_order.o IPMonitorControlPrefs.o agent-monitor.o configAgent.o controller.o dnsAgent.o proxyAgent.o dnsinfo_create.o dnsinfo_flatfile.o dnsinfo_server.o network_state_information_priv.o network_information_server.o dns-configuration.o proxy-configuration.o set-hostname.o smb-configuration.o IPMonitorControlServer.o libSystemConfiguration_client.o libSystemConfiguration_server.o ${EXTRA_CFLAGS} -lnetwork -framework SystemConfiguration -framework CoreFoundation -framework Foundation -framework Network -framework NetworkExtension

# ----------
test_ipv4_routelist.o: ip_plugin.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -DTEST_IPV4_ROUTELIST ${TEST_INCLUDE} ${EXTRA_CFLAGS} -Wall -O0 -g -c -o $@ $^

test_ipv4_routelist: test_ipv4_routelist.o IPMonitorControlPrefs.o dnsinfo_create.o dnsinfo_flatfile.o dnsinfo_server.o dns-configuration.o proxy-configuration.o set-hostname.o smb-configuration.o libSystemConfiguration_client.o libSystemConfiguration_server.o
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -Wall -O0 -g -o test_ipv4_routelist $^ ${EXTRA_CFLAGS} -lnetwork -framework SystemConfiguration -framework CoreFoundation -framework Foundation -framework Network -framework NetworkExtension

test_ipv4_routelist_reference.txt: test_ipv4_routelist
	sh $(REFERENCE_OUTPUT) create ./test_ipv4_routelist test_ipv4_routelist_reference.txt routelist_output_filter.sh

test_ipv4_routelist_test: test_ipv4_routelist
	sh $(REFERENCE_OUTPUT) test ./test_ipv4_routelist test_ipv4_routelist_reference.txt routelist_output_filter.sh

test_ipv4_routelist_coverage: test_ipv4_routelist
	./test_ipv4_routelist -1 | grep Hit | awk '{print $$2}' | sort | uniq

# ----------

test_ipv6_routelist.o: ip_plugin.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -DTEST_IPV6_ROUTELIST=1 ${TEST_INCLUDE} ${EXTRA_CFLAGS} -Wall -O0 -g -c -o $@ $^

test_ipv6_routelist: test_ipv6_routelist.o IPMonitorControlPrefs.o dnsinfo_create.o dnsinfo_flatfile.o dnsinfo_server.o dns-configuration.o proxy-configuration.o set-hostname.o smb-configuration.o libSystemConfiguration_client.o libSystemConfiguration_server.o
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -Wall -O0 -g -o test_ipv6_routelist $^ ${EXTRA_CFLAGS} -lnetwork -framework SystemConfiguration -framework CoreFoundation -framework Foundation -framework Network -framework NetworkExtension

test_ipv6_routelist_reference.txt: test_ipv6_routelist
	sh $(REFERENCE_OUTPUT) create ./test_ipv6_routelist test_ipv6_routelist_reference.txt routelist_output_filter.sh

test_ipv6_routelist_test: test_ipv6_routelist
	sh $(REFERENCE_OUTPUT) test ./test_ipv6_routelist test_ipv6_routelist_reference.txt routelist_output_filter.sh

test_ipv6_routelist_coverage: test_ipv6_routelist
	./test_ipv6_routelist -1 | grep Hit | awk '{print $$2}' | sort | uniq

# ----------

IPMonitor.o: ip_plugin.c
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -DTEST_IPMONITOR ${TEST_INCLUDE} ${EXTRA_CFLAGS} -Wall -O0 -g -c -o IPMonitor.o ip_plugin.c

IPMonitor: IPMonitor.o IPMonitorControlPrefs.o agent-monitor.o configAgent.o controller.o dnsAgent.o proxyAgent.o dnsinfo_create.o dnsinfo_flatfile.o dnsinfo_server.o network_state_information_priv.o network_information_server.o dns-configuration.o proxy-configuration.o set-hostname.o smb-configuration.o IPMonitorControlServer.o libSystemConfiguration_client.o libSystemConfiguration_server.o
	$(CC) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -Wall -O0 -g -o IPMonitor IPMonitor.o IPMonitorControlPrefs.o agent-monitor.o configAgent.o controller.o dnsAgent.o proxyAgent.o dnsinfo_create.o dnsinfo_flatfile.o dnsinfo_server.o network_state_information_priv.o network_information_server.o dns-configuration.o proxy-configuration.o set-hostname.o smb-configuration.o IPMonitorControlServer.o libSystemConfiguration_client.o libSystemConfiguration_server.o ${EXTRA_CFLAGS} -lnetwork -framework SystemConfiguration -framework CoreFoundation -framework Foundation -framework Network -framework NetworkExtension

# ----------

nat64: nat64-configuration.c
	$(CC) -DTEST_NAT64_CONFIGURATION $(TEST_INCLUDE) $(PF_INC) $(ARCH_FLAGS) -isysroot $(SYSROOT) -lnetwork -framework CoreFoundation -framework SystemConfiguration -Wall -O0 -g -o $@ $^

# ----------

clean:
	rm -rf *.dSYM *.o test_dns test_hostname test_proxy test_smb test_ipv4_routelist test_ipv6_routelist test_dns_order IPMonitor nat64
