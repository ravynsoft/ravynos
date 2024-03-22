/*
 * Copyright 2023 Intel Corporation
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

void *xe_device_query_alloc_fetch(int fd, uint32_t query_id, uint32_t *len);
