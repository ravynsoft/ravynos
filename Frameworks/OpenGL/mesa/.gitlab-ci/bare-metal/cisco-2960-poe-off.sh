#!/bin/bash
# shellcheck disable=SC2086 # we want word splitting

if [ -z "$BM_POE_INTERFACE" ]; then
    echo "Must supply the PoE Interface to power down"
    exit 1
fi

if [ -z "$BM_POE_ADDRESS" ]; then
    echo "Must supply the PoE Switch host"
    exit 1
fi

SNMP_KEY="1.3.6.1.4.1.9.9.402.1.2.1.1.1.$BM_POE_INTERFACE"
SNMP_OFF="i 4"

snmpset -v2c -r 3 -t 30 -cmesaci "$BM_POE_ADDRESS" "$SNMP_KEY" $SNMP_OFF
