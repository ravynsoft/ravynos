#!/bin/bash

relay=$1

if [ -z "$relay" ]; then
    echo "Must supply a relay arg"
    exit 1
fi

"$CI_PROJECT_DIR"/install/bare-metal/eth008-power-relay.py "$ETH_HOST" "$ETH_PORT" off "$relay"
