/*
    SPDX-FileCopyrightText: 2019 Mark Nauwelaerts <mark.nauwelaerts@gmail.com>

    SPDX-License-Identifier: MIT
*/

#ifndef KATE_SHARED_JSON_UTILS_H
#define KATE_SHARED_JSON_UTILS_H

#include <QJsonObject>

namespace json
{
// local helper;
// recursively merge top json top onto bottom json
QJsonObject merge(const QJsonObject &bottom, const QJsonObject &top)
{
    QJsonObject result;
    for (auto item = top.begin(); item != top.end(); item++) {
        const auto &key = item.key();
        if (item.value().isObject()) {
            result.insert(key, merge(bottom.value(key).toObject(), item.value().toObject()));
        } else {
            result.insert(key, item.value());
        }
    }
    // parts only in bottom
    for (auto item = bottom.begin(); item != bottom.end(); item++) {
        if (!result.contains(item.key())) {
            result.insert(item.key(), item.value());
        }
    }
    return result;
}

}

#endif
