#!/bin/bash

APITRACE_VERSION="11.1"
APITRACE_VERSION_DATE=""

curl -L -O --retry 4 -f --retry-all-errors --retry-delay 60 \
  "https://github.com/apitrace/apitrace/releases/download/${APITRACE_VERSION}/apitrace-${APITRACE_VERSION}${APITRACE_VERSION_DATE}-win64.7z"
7zr x "apitrace-${APITRACE_VERSION}${APITRACE_VERSION_DATE}-win64.7z" \
      "apitrace-${APITRACE_VERSION}${APITRACE_VERSION_DATE}-win64/bin/apitrace.exe" \
      "apitrace-${APITRACE_VERSION}${APITRACE_VERSION_DATE}-win64/bin/d3dretrace.exe"
mv "apitrace-${APITRACE_VERSION}${APITRACE_VERSION_DATE}-win64" /apitrace-msvc-win64
rm "apitrace-${APITRACE_VERSION}${APITRACE_VERSION_DATE}-win64.7z"


