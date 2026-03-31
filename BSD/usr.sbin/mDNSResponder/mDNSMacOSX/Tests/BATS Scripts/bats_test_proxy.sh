#!/bin/sh
#
# bats_test_proxy.sh
# mDNSResponder Tests
#
# Copyright (c) 2019 Apple Inc. All rights reserved.

# tests whether the state dump will create at most MAX_NUM_DUMP_FILES, to avoid wasting too much space.
base_test="dnssdutil dnsquery -s 127.0.0.1 -n apple.com -t a"

# Enable the proxy and test over TCP
function test_proxy_tcp {
    test_proxy_on $base_test --tcp
    return $?
}

# Enable the proxy and test over UDP
function test_proxy_udp {
    test_proxy_on $base_test
    return $?
}

# Test the proxy over TCP without enabling it (should fail)
function test_noproxy_tcp {
    test_proxy $base_test --tcp
    if [ $? == 0 ]; then
        return 1;
    fi
    return 0
}

# Test the proxy over UDP without enabling it (should fail)
function test_noproxy_udp {
    test_proxy $base_test
    if [ $? == 0 ]; then
        return 1;
    fi
    return 0
}

function test_proxy_on {
    local command_line="$*"
    local ret=1
    
    # Enable the proxy
    dnssdutil dnsproxy -i lo0 &
    local dnssdutil_pid=$!

    # See if that worked
    sleep 5
    local dnssdutil_pid_now=$(ps xa |sed -n -e 's/^ *\([0-9][0-9]*\).*$/\1/' -e "/$dnssdutil_pid/p")
    if [ $dnssdutil_pid != "$dnssdutil_pid_now" ]; then
        echo "Failed to enable DNS proxy $dnssdutil_pid $dnssdutil_pid_now."
        return 1
    fi
    
    test_proxy $command_line
    ret=$?

    # Disable the proxy and wait for that to finish
    kill -HUP $dnssdutil_pid
    wait $dnssdutil_pid
    return $ret
}

function test_proxy {
    local command_line="$*"
    local ret=1
    # Try to do the DNS lookup
    local output=$($command_line |egrep "^End reason:")
    if [ "$output" = "End reason: received response" ]; then
        echo "Proxy is working: $output"
        ret=0
    else
        echo "Proxy is not working: $output"
    fi
    return $ret
}

ret=0
# Functions are put inside an array, use ($test) to evaluate it
declare -a tests=("test_proxy_tcp"
                  "test_proxy_udp"
                  "test_noproxy_tcp"
                  "test_noproxy_udp")
echo ""
echo "----Proxy Test Start, $(date)----"
for test in "${tests[@]}"; do
    echo "running $test:"
    ($test)
    if [[ $? -eq 0 ]]; then
        echo "passed"$'\n' # use $'\n' to print one more newline character
    else
        ret=1
        echo "failed"$'\n'
    fi
done
echo "----Proxy Test End, $(date)----"
exit $ret

# Local Variables:
# tab-width: 4
# fill-column: 108
# indent-tabs-mode: nil
# End:
