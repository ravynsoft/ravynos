/*
 * This file is part of the Kate project.
 *
 * SPDX-FileCopyrightText: 2021 Héctor Mesa Jiménez <wmj.py@gmx.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "json_utils_test.h"

#include <QJsonDocument>
#include <QTest>
#include <json_utils.h>

QTEST_MAIN(JsonUtilsTest)

/*
 * check that two json objects (A, B) are deeply merged correctly:
 *
 * - If a key is in A and not B, take A's value
 * - If a key is in B ant not A, take B's value
 * - If a key is present in A and B:
 *  - If both are objects, merge values
 *  - If any of them is not an object, take B's value
 */
void JsonUtilsTest::testMerge()
{
    const auto base = QJsonDocument::fromJson(QByteArray(R"JSON(
        {
            "nested_object": {
                "number_to_number": 1,
                "number": 2,
                "list_to_list": [5,7],
                "string_to_object": "literal1",
                "settings": {
                    "path": "/standard_path",
                    "param1": "checked",
                    "param2": "unchecked"
                }
            },
            "list_only_in_A": [1,2,3],
            "text": "literal2",
            "object_to_same": {"a": 1, "b": 2},
            "list_to_empty": [3,2,1],
            "string_to_null": "notnull",
            "object_only_in_A": {"b": 3}
        }
    )JSON"));

    QVERIFY(!base.isEmpty());

    const auto addenda = QJsonDocument::fromJson(QByteArray(R"JSON(
        {
            "nested_object": {
                "number_to_number": 100,
                "list_to_list": [1,2,3],
                "string_to_object": {"a": 1},
                "settings": {
                    "path": "/my_local_path",
                    "notes": "important notes"
                }
            },
            "int_only_in_B": 3,
            "object_to_same": {},
            "list_to_empty": [],
            "string_to_null": null
        }
    )JSON"));

    QVERIFY(!addenda.isEmpty());

    const auto expected = QJsonDocument::fromJson(QByteArray(R"JSON(
        {
            "nested_object": {
                "number_to_number": 100,
                "number": 2,
                "list_to_list": [1,2,3],
                "string_to_object": {"a": 1},
                "settings": {
                    "path": "/my_local_path",
                    "notes": "important notes",
                    "param1": "checked",
                    "param2": "unchecked"
                }
            },
            "list_only_in_A": [1,2,3],
            "text": "literal2",
            "object_to_same": {"a": 1, "b": 2},
            "list_to_empty": [],
            "string_to_null": null,
            "object_only_in_A": {"b": 3},
            "int_only_in_B": 3
        }
    )JSON"));

    QVERIFY(!expected.isEmpty());

    const auto result = json::merge(base.object(), addenda.object());

    QCOMPARE(result, expected.object());
}

// kate: space-indent on; indent-width 4; replace-tabs on;
