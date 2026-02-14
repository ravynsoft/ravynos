#!/bin/bash -e
#
# Copyright (c) 2012 Apple Inc. All rights reserved.
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

if [ "${DEPLOYMENT_LOCATION}" != YES ]; then
	DSTROOT="${CONFIGURATION_BUILD_DIR}"
fi

mkdir -p "${DSTROOT}${OS_PUBLIC_HEADERS_FOLDER_PATH}" || true
mkdir -p "${DSTROOT}${OS_PRIVATE_HEADERS_FOLDER_PATH}" || true
cp -X "${SCRIPT_INPUT_FILE_1}" "${DSTROOT}${OS_PUBLIC_HEADERS_FOLDER_PATH}"
cp -X "${SCRIPT_INPUT_FILE_2}" "${DSTROOT}${OS_PRIVATE_HEADERS_FOLDER_PATH}"
cp -X "${SCRIPT_INPUT_FILE_3}" "${DSTROOT}${OS_PRIVATE_HEADERS_FOLDER_PATH}"
cp -X "${SCRIPT_INPUT_FILE_4}" "${DSTROOT}${OS_PRIVATE_HEADERS_FOLDER_PATH}"
cp -X "${SCRIPT_INPUT_FILE_5}" "${DSTROOT}${OS_PRIVATE_HEADERS_FOLDER_PATH}"
