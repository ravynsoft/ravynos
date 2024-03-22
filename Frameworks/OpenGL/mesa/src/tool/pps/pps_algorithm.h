/*
 * Copyright © 2020 Collabora, Ltd.
 * Author: Antonio Caggiano <antonio.caggiano@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <algorithm>

#define FIND_IF(c, lambda) (std::find_if(std::begin(c), std::end(c), lambda))
#define FIND(c, e) (std::find(std::begin(c), std::end(c), e))
#define CONTAINS(c, e) (FIND(c, e) != std::end(c))
#define CONTAINS_IT(c, it) (it != std::end(c))
#define APPEND(a, b) (a.insert(std::end(a), std::begin(b), std::end(b)))
