#!/bin/bash -e
#
# Copyright (c) 2013 Apple Inc. All rights reserved.
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

# This check equates to "is macosx or a simulator platform"
if [ "${PLATFORM_NAME}" == "${DEVICE_PLATFORM_NAME}" ]; then exit 0; fi

if [ "${DEPLOYMENT_LOCATION}" != YES ]; then
	DSTROOT="${CONFIGURATION_BUILD_DIR}"
fi

mkdir -p "${DSTROOT}${PUBLIC_HEADERS_FOLDER_PATH}" || true
cp -X "${SCRIPT_INPUT_FILE_1}" \
		"${DSTROOT}${PUBLIC_HEADERS_FOLDER_PATH}/${SCRIPT_OUTPUT_FILE_0##/*/}"
