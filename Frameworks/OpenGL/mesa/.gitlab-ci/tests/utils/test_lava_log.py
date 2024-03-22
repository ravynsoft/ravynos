#!/usr/bin/env python3
#
# Copyright (C) 2022 Collabora Limited
# Author: Guilherme Gallo <guilherme.gallo@collabora.com>
#
# SPDX-License-Identifier: MIT

from datetime import datetime, timedelta

import pytest
from lava.exceptions import MesaCIKnownIssueException, MesaCITimeoutError
from lava.utils import (
    GitlabSection,
    LogFollower,
    LogSectionType,
    fix_lava_gitlab_section_log,
    hide_sensitive_data,
)

from ..lava.helpers import create_lava_yaml_msg, does_not_raise, lava_yaml, yaml_dump

GITLAB_SECTION_SCENARIOS = {
    "start collapsed": (
        "start",
        True,
        f"\x1b[0Ksection_start:mock_date:my_first_section[collapsed=true]\r\x1b[0K{GitlabSection.colour}my_header\x1b[0m",
    ),
    "start non_collapsed": (
        "start",
        False,
        f"\x1b[0Ksection_start:mock_date:my_first_section\r\x1b[0K{GitlabSection.colour}my_header\x1b[0m",
    ),
    "end collapsed": (
        "end",
        True,
        "\x1b[0Ksection_end:mock_date:my_first_section\r\x1b[0K",
    ),
    "end non_collapsed": (
        "end",
        False,
        "\x1b[0Ksection_end:mock_date:my_first_section\r\x1b[0K",
    ),
}

@pytest.mark.parametrize(
    "method, collapsed, expectation",
    GITLAB_SECTION_SCENARIOS.values(),
    ids=GITLAB_SECTION_SCENARIOS.keys(),
)
def test_gitlab_section(method, collapsed, expectation):
    gs = GitlabSection(
        id="my_first_section",
        header="my_header",
        type=LogSectionType.TEST_CASE,
        start_collapsed=collapsed,
    )
    gs.get_timestamp = lambda x: "mock_date"
    gs.start()
    result = getattr(gs, method)()
    assert result == expectation


def test_gl_sections():
    lines = [
        {
            "dt": datetime.now(),
            "lvl": "debug",
            "msg": "Received signal: <STARTRUN> 0_setup-ssh-server 10145749_1.3.2.3.1",
        },
        {
            "dt": datetime.now(),
            "lvl": "debug",
            "msg": "Received signal: <STARTRUN> 0_mesa 5971831_1.3.2.3.1",
        },
        # Redundant log message which triggers the same Gitlab Section, it
        # should be ignored, unless the id is different
        {
            "dt": datetime.now(),
            "lvl": "target",
            "msg": "[    7.778836] <LAVA_SIGNAL_STARTRUN 0_mesa 5971831_1.3.2.3.1>",
        },
        {
            "dt": datetime.now(),
            "lvl": "debug",
            "msg": "Received signal: <STARTTC> mesa-ci_iris-kbl-traces",
        },
        # Another redundant log message
        {
            "dt": datetime.now(),
            "lvl": "target",
            "msg": "[   16.997829] <LAVA_SIGNAL_STARTTC mesa-ci_iris-kbl-traces>",
        },
        {
            "dt": datetime.now(),
            "lvl": "target",
            "msg": "<LAVA_SIGNAL_ENDTC mesa-ci_iris-kbl-traces>",
        },
    ]
    lf = LogFollower()
    with lf:
        for line in lines:
            lf.manage_gl_sections(line)
        parsed_lines = lf.flush()

    section_types = [s.type for s in lf.section_history]

    assert "section_start" in parsed_lines[0]
    assert "collapsed=true" not in parsed_lines[0]
    assert "section_end" in parsed_lines[1]
    assert "section_start" in parsed_lines[2]
    assert "collapsed=true" not in parsed_lines[2]
    assert "section_end" in parsed_lines[3]
    assert "section_start" in parsed_lines[4]
    assert "collapsed=true" not in parsed_lines[4]
    assert "section_end" in parsed_lines[5]
    assert "section_start" in parsed_lines[6]
    assert "collapsed=true" in parsed_lines[6]
    assert section_types == [
        # LogSectionType.LAVA_BOOT,  True, if LogFollower started with Boot section
        LogSectionType.TEST_DUT_SUITE,
        LogSectionType.TEST_SUITE,
        LogSectionType.TEST_CASE,
        LogSectionType.LAVA_POST_PROCESSING,
    ]


def test_log_follower_flush():
    lines = [
        {
            "dt": datetime.now(),
            "lvl": "debug",
            "msg": "Received signal: <STARTTC> mesa-ci_iris-kbl-traces",
        },
        {
            "dt": datetime.now(),
            "lvl": "target",
            "msg": "<LAVA_SIGNAL_ENDTC mesa-ci_iris-kbl-traces>",
        },
    ]
    lf = LogFollower()
    lf.feed(lines)
    parsed_lines = lf.flush()
    empty = lf.flush()
    lf.feed(lines)
    repeated_parsed_lines = lf.flush()

    assert parsed_lines
    assert not empty
    assert repeated_parsed_lines


SENSITIVE_DATA_SCENARIOS = {
    "no sensitive data tagged": (
        ["bla  bla", "mytoken: asdkfjsde1341=="],
        ["bla  bla", "mytoken: asdkfjsde1341=="],
        ["HIDEME"],
    ),
    "sensitive data tagged": (
        ["bla  bla", "mytoken: asdkfjsde1341== # HIDEME"],
        ["bla  bla"],
        ["HIDEME"],
    ),
    "sensitive data tagged with custom word": (
        ["bla  bla", "mytoken: asdkfjsde1341== # DELETETHISLINE", "third line # NOTANYMORE"],
        ["bla  bla", "third line # NOTANYMORE"],
        ["DELETETHISLINE", "NOTANYMORE"],
    ),
}


@pytest.mark.parametrize(
    "input, expectation, tags",
    SENSITIVE_DATA_SCENARIOS.values(),
    ids=SENSITIVE_DATA_SCENARIOS.keys(),
)
def test_hide_sensitive_data(input, expectation, tags):
    yaml_data = yaml_dump(input)
    yaml_result = hide_sensitive_data(yaml_data, *tags)
    result = lava_yaml.load(yaml_result)

    assert result == expectation


GITLAB_SECTION_SPLIT_SCENARIOS = {
    "Split section_start at target level": (
        "\x1b[0Ksection_start:1668454947:test_post_process[collapsed=true]\r\x1b[0Kpost-processing test results",
        (
            "\x1b[0Ksection_start:1668454947:test_post_process[collapsed=true]",
            "\x1b[0Kpost-processing test results",
        ),
    ),
    "Split section_end at target level": (
        "\x1b[0Ksection_end:1666309222:test_post_process\r\x1b[0K",
        ("\x1b[0Ksection_end:1666309222:test_post_process", "\x1b[0K"),
    ),
    "Second line is not split from the first": (
        ("\x1b[0Ksection_end:1666309222:test_post_process", "Any message"),
        ("\x1b[0Ksection_end:1666309222:test_post_process", "Any message"),
    ),
}


@pytest.mark.parametrize(
    "expected_message, messages",
    GITLAB_SECTION_SPLIT_SCENARIOS.values(),
    ids=GITLAB_SECTION_SPLIT_SCENARIOS.keys(),
)
def test_fix_lava_gitlab_section_log(expected_message, messages):
    fixed_messages = []
    gen = fix_lava_gitlab_section_log()
    next(gen)

    for message in messages:
        lava_log = create_lava_yaml_msg(msg=message, lvl="target")
        if recovered_line := gen.send(lava_log):
            fixed_messages.append((recovered_line, lava_log["msg"]))
        fixed_messages.append(lava_log["msg"])

    assert expected_message in fixed_messages


@pytest.mark.parametrize(
    "expected_message, messages",
    GITLAB_SECTION_SPLIT_SCENARIOS.values(),
    ids=GITLAB_SECTION_SPLIT_SCENARIOS.keys(),
)
def test_lava_gitlab_section_log_collabora(expected_message, messages, monkeypatch):
    """Check if LogFollower does not change the message if we are running in Collabora farm."""
    monkeypatch.setenv("RUNNER_TAG", "mesa-ci-x86_64-lava-test")
    lf = LogFollower()
    for message in messages:
        lf.feed([create_lava_yaml_msg(msg=message)])
    new_messages = lf.flush()
    new_messages = tuple(new_messages) if len(new_messages) > 1 else new_messages[0]
    assert new_messages == expected_message


CARRIAGE_RETURN_SCENARIOS = {
    "Carriage return at the end of the previous line": (
        (
            "\x1b[0Ksection_start:1677609903:test_setup[collapsed=true]\r\x1b[0K\x1b[0;36m[303:44] deqp: preparing test setup\x1b[0m",
        ),
        (
            "\x1b[0Ksection_start:1677609903:test_setup[collapsed=true]\r",
            "\x1b[0K\x1b[0;36m[303:44] deqp: preparing test setup\x1b[0m\r\n",
        ),
    ),
    "Newline at the end of the line": (
        ("\x1b[0K\x1b[0;36m[303:44] deqp: preparing test setup\x1b[0m", "log"),
        ("\x1b[0K\x1b[0;36m[303:44] deqp: preparing test setup\x1b[0m\r\n", "log"),
    ),
}


@pytest.mark.parametrize(
    "expected_message, messages",
    CARRIAGE_RETURN_SCENARIOS.values(),
    ids=CARRIAGE_RETURN_SCENARIOS.keys(),
)
def test_lava_log_merge_carriage_return_lines(expected_message, messages):
    lf = LogFollower()
    for message in messages:
        lf.feed([create_lava_yaml_msg(msg=message)])
    new_messages = tuple(lf.flush())
    assert new_messages == expected_message


WATCHDOG_SCENARIOS = {
    "1 second before timeout": ({"seconds": -1}, does_not_raise()),
    "1 second after timeout": ({"seconds": 1}, pytest.raises(MesaCITimeoutError)),
}


@pytest.mark.parametrize(
    "timedelta_kwargs, exception",
    WATCHDOG_SCENARIOS.values(),
    ids=WATCHDOG_SCENARIOS.keys(),
)
def test_log_follower_watchdog(frozen_time, timedelta_kwargs, exception):
    lines = [
        {
            "dt": datetime.now(),
            "lvl": "debug",
            "msg": "Received signal: <STARTTC> mesa-ci_iris-kbl-traces",
        },
    ]
    td = {LogSectionType.TEST_CASE: timedelta(minutes=1)}
    lf = LogFollower(timeout_durations=td)
    lf.feed(lines)
    frozen_time.tick(
        lf.timeout_durations[LogSectionType.TEST_CASE] + timedelta(**timedelta_kwargs)
    )
    lines = [create_lava_yaml_msg()]
    with exception:
        lf.feed(lines)


GITLAB_SECTION_ID_SCENARIOS = [
    ("a-good_name", "a-good_name"),
    ("spaces are not welcome", "spaces-are-not-welcome"),
    ("abc:amd64 1/3", "abc-amd64-1-3"),
]


@pytest.mark.parametrize("case_name, expected_id", GITLAB_SECTION_ID_SCENARIOS)
def test_gitlab_section_id(case_name, expected_id):
    gl = GitlabSection(
        id=case_name, header=case_name, type=LogSectionType.LAVA_POST_PROCESSING
    )

    assert gl.id == expected_id


A618_NETWORK_ISSUE_LOGS = [
    create_lava_yaml_msg(
        msg="[ 1733.599402] r8152 2-1.3:1.0 eth0: Tx status -71", lvl="target"
    ),
    create_lava_yaml_msg(
        msg="[ 1733.604506] nfs: server 192.168.201.1 not responding, still trying",
        lvl="target",
    ),
]
TEST_PHASE_LAVA_SIGNAL = create_lava_yaml_msg(
    msg="Received signal: <STARTTC> mesa-ci_a618_vk", lvl="debug"
)


A618_NETWORK_ISSUE_SCENARIOS = {
    "Pass - R8152 kmsg during boot": (A618_NETWORK_ISSUE_LOGS, does_not_raise()),
    "Fail - R8152 kmsg during test phase": (
        [TEST_PHASE_LAVA_SIGNAL, *A618_NETWORK_ISSUE_LOGS],
        pytest.raises(MesaCIKnownIssueException),
    ),
    "Pass - Partial (1) R8152 kmsg during test phase": (
        [TEST_PHASE_LAVA_SIGNAL, A618_NETWORK_ISSUE_LOGS[0]],
        does_not_raise(),
    ),
    "Pass - Partial (2) R8152 kmsg during test phase": (
        [TEST_PHASE_LAVA_SIGNAL, A618_NETWORK_ISSUE_LOGS[1]],
        does_not_raise(),
    ),
    "Pass - Partial subsequent (3) R8152 kmsg during test phase": (
        [
            TEST_PHASE_LAVA_SIGNAL,
            A618_NETWORK_ISSUE_LOGS[0],
            A618_NETWORK_ISSUE_LOGS[0],
        ],
        does_not_raise(),
    ),
    "Pass - Partial subsequent (4) R8152 kmsg during test phase": (
        [
            TEST_PHASE_LAVA_SIGNAL,
            A618_NETWORK_ISSUE_LOGS[1],
            A618_NETWORK_ISSUE_LOGS[1],
        ],
        does_not_raise(),
    ),
}


@pytest.mark.parametrize(
    "messages, expectation",
    A618_NETWORK_ISSUE_SCENARIOS.values(),
    ids=A618_NETWORK_ISSUE_SCENARIOS.keys(),
)
def test_detect_failure(messages, expectation):
    lf = LogFollower()
    with expectation:
        lf.feed(messages)
