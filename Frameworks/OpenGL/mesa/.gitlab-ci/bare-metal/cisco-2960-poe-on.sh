#!/bin/bash
# shellcheck disable=SC2086 # we want word splitting

if [ -z "$BM_POE_INTERFACE" ]; then
    echo "Must supply the PoE Interface to power up"
    exit 1
fi

if [ -z "$BM_POE_ADDRESS" ]; then
    echo "Must supply the PoE Switch host"
    exit 1
fi

set -ex

SNMP_KEY="1.3.6.1.4.1.9.9.402.1.2.1.1.1.$BM_POE_INTERFACE"
SNMP_ON="i 1"
SNMP_OFF="i 4"

snmpset -v2c -r 3 -t 10 -cmesaci "$BM_POE_ADDRESS" "$SNMP_KEY" $SNMP_OFF
sleep 3s
snmpset -v2c -r 3 -t 10 -cmesaci "$BM_POE_ADDRESS" "$SNMP_KEY" $SNMP_ON
