#!/bin/bash -e
#
# Copyright (c) 2010-2011 Apple Inc. All rights reserved.
#
# @APPLE_APACHE_LICENSE_HEADER_START@
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# @APPLE_APACHE_LICENSE_HEADER_END@
#


export MIGCC="$(xcrun -find cc)"
export MIGCOM="$(xcrun -find migcom)"
export PATH="${PLATFORM_DEVELOPER_BIN_DIR}:${DEVELOPER_BIN_DIR}:${PATH}"

for p in ${HEADER_SEARCH_PATHS}; do
	OTHER_MIGFLAGS="${OTHER_MIGFLAGS} -I${p}"
done

for a in ${ARCHS}; do
	xcrun mig ${OTHER_MIGFLAGS} -arch $a -header "${SCRIPT_OUTPUT_FILE_0}" \
			-sheader "${SCRIPT_OUTPUT_FILE_1}" -user /dev/null \
			-server /dev/null "${SCRIPT_INPUT_FILE_0}"
	xcrun mig ${OTHER_MIGFLAGS} -arch $a -header "${SCRIPT_OUTPUT_FILE_2}" \
			-sheader "${SCRIPT_OUTPUT_FILE_3}" -user /dev/null \
			-server /dev/null "${SCRIPT_INPUT_FILE_1}"
	xcrun mig ${OTHER_MIGFLAGS} -arch $a -header "${SCRIPT_OUTPUT_FILE_4}" \
			-sheader "${SCRIPT_OUTPUT_FILE_5}" -user /dev/null \
			-server /dev/null "${SCRIPT_INPUT_FILE_2}"
done
