#!/usr/bin/env python3
#
# Copyright (C) 2022 Collabora Limited
# Author: Guilherme Gallo <guilherme.gallo@collabora.com>
#
# SPDX-License-Identifier: MIT

import os
import xmlrpc.client
from contextlib import nullcontext as does_not_raise
from datetime import datetime
from itertools import chain, repeat
from pathlib import Path
from unittest.mock import MagicMock, patch

import pytest
from lava.exceptions import MesaCIException, MesaCIRetryError
from lava.lava_job_submitter import (
    DEVICE_HANGING_TIMEOUT_SEC,
    NUMBER_OF_RETRIES_TIMEOUT_DETECTION,
    LAVAJob,
    LAVAJobSubmitter,
    bootstrap_log_follower,
    follow_job_execution,
    retriable_follow_job,
)
from lava.utils import LogSectionType

from .lava.helpers import (
    generate_n_logs,
    generate_testsuite_result,
    jobs_logs_response,
    mock_lava_signal,
    mock_logs,
    section_timeout,
)

NUMBER_OF_MAX_ATTEMPTS = NUMBER_OF_RETRIES_TIMEOUT_DETECTION + 1


@pytest.fixture
def mock_proxy_waiting_time(mock_proxy):
    def update_mock_proxy(frozen_time, **kwargs):
        wait_time = kwargs.pop("wait_time", 1)
        proxy_mock = mock_proxy(**kwargs)
        proxy_job_state = proxy_mock.scheduler.job_state
        proxy_job_state.return_value = {"job_state": "Running"}
        proxy_job_state.side_effect = frozen_time.tick(wait_time)

        return proxy_mock

    return update_mock_proxy


@pytest.fixture(params=[{"CI": "true"}, {"CI": "false"}], ids=["Under CI", "Local run"])
def ci_environment(request):
    with patch.dict(os.environ, request.param):
        yield


@pytest.fixture
def lava_job_submitter(
    ci_environment,
    tmp_path,
    mock_proxy,
):
    os.chdir(tmp_path)
    tmp_file = Path(tmp_path) / "log.json"

    with patch("lava.lava_job_submitter.setup_lava_proxy") as mock_setup_lava_proxy:
        mock_setup_lava_proxy.return_value = mock_proxy()
        yield LAVAJobSubmitter(
            boot_method="test_boot",
            ci_project_dir="test_dir",
            device_type="test_device",
            job_timeout_min=1,
            structured_log_file=tmp_file,
        )


@pytest.mark.parametrize("exception", [RuntimeError, SystemError, KeyError])
def test_submit_and_follow_respects_exceptions(mock_sleep, mock_proxy, exception):
    with pytest.raises(MesaCIException):
        proxy = mock_proxy(side_effect=exception)
        job = LAVAJob(proxy, '')
        log_follower = bootstrap_log_follower()
        follow_job_execution(job, log_follower)


NETWORK_EXCEPTION = xmlrpc.client.ProtocolError("", 0, "test", {})
XMLRPC_FAULT = xmlrpc.client.Fault(0, "test")

PROXY_SCENARIOS = {
    "simple pass case": (mock_logs(result="pass"), does_not_raise(), "pass", {}),
    "simple fail case": (mock_logs(result="fail"), does_not_raise(), "fail", {}),
    "simple hung case": (
        mock_logs(
            messages={
                LogSectionType.TEST_CASE: [
                    section_timeout(LogSectionType.TEST_CASE) + 1
                ]
                * 1000
            },
            result="fail",
        ),
        pytest.raises(MesaCIRetryError),
        "hung",
        {},
    ),
    "leftover dump from last job in boot section": (
        (
            mock_lava_signal(LogSectionType.LAVA_BOOT),
            jobs_logs_response(finished=False, msg=None, result="fail"),
        ),
        pytest.raises(MesaCIRetryError),
        "hung",
        {},
    ),
    "boot works at last retry": (
        mock_logs(
            messages={
                LogSectionType.LAVA_BOOT: [
                    section_timeout(LogSectionType.LAVA_BOOT) + 1
                ]
                * NUMBER_OF_RETRIES_TIMEOUT_DETECTION
                + [1]
            },
            result="pass",
        ),
        does_not_raise(),
        "pass",
        {},
    ),
    "test case took too long": pytest.param(
        mock_logs(
            messages={
                LogSectionType.TEST_CASE: [
                    section_timeout(LogSectionType.TEST_CASE) + 1
                ]
                * (NUMBER_OF_MAX_ATTEMPTS + 1)
            },
            result="pass",
        ),
        pytest.raises(MesaCIRetryError),
        "pass",
        {},
    ),
    "timed out more times than retry attempts": (
        generate_n_logs(n=4, tick_fn=9999999),
        pytest.raises(MesaCIRetryError),
        "fail",
        {},
    ),
    "long log case, no silence": (
        mock_logs(
            messages={LogSectionType.TEST_CASE: [1] * (1000)},
            result="pass",
        ),
        does_not_raise(),
        "pass",
        {},
    ),
    "no retries, testsuite succeed": (
        mock_logs(result="pass"),
        does_not_raise(),
        "pass",
        {
            "testsuite_results": [
                generate_testsuite_result(result="pass")
            ]
        },
    ),
    "no retries, but testsuite fails": (
        mock_logs(result="fail"),
        does_not_raise(),
        "fail",
        {
            "testsuite_results": [
                generate_testsuite_result(result="fail")
            ]
        },
    ),
    "no retries, one testsuite fails": (
        generate_n_logs(n=1, tick_fn=0, result="fail"),
        does_not_raise(),
        "fail",
        {
            "testsuite_results": [
                generate_testsuite_result(result="fail"),
                generate_testsuite_result(result="pass")
            ]
        },
    ),
    "very long silence": (
        generate_n_logs(n=NUMBER_OF_MAX_ATTEMPTS + 1, tick_fn=100000),
        pytest.raises(MesaCIRetryError),
        "fail",
        {},
    ),
    # If a protocol error happens, _call_proxy will retry without affecting timeouts
    "unstable connection, ProtocolError followed by final message": (
        (NETWORK_EXCEPTION, *list(mock_logs(result="pass"))),
        does_not_raise(),
        "pass",
        {},
    ),
    # After an arbitrary number of retries, _call_proxy should call sys.exit
    "unreachable case, subsequent ProtocolErrors": (
        repeat(NETWORK_EXCEPTION),
        pytest.raises(SystemExit),
        "fail",
        {},
    ),
    "XMLRPC Fault": ([XMLRPC_FAULT], pytest.raises(MesaCIRetryError), False, {}),
}


@pytest.mark.parametrize(
    "test_log, expectation, job_result, proxy_args",
    PROXY_SCENARIOS.values(),
    ids=PROXY_SCENARIOS.keys(),
)
def test_retriable_follow_job(
    mock_sleep,
    test_log,
    expectation,
    job_result,
    proxy_args,
    mock_proxy,
):
    with expectation:
        proxy = mock_proxy(side_effect=test_log, **proxy_args)
        job: LAVAJob = retriable_follow_job(proxy, "")
        assert job_result == job.status


WAIT_FOR_JOB_SCENARIOS = {"one log run taking (sec):": (mock_logs(result="pass"))}


@pytest.mark.parametrize("wait_time", (DEVICE_HANGING_TIMEOUT_SEC * 2,))
@pytest.mark.parametrize(
    "side_effect",
    WAIT_FOR_JOB_SCENARIOS.values(),
    ids=WAIT_FOR_JOB_SCENARIOS.keys(),
)
def test_simulate_a_long_wait_to_start_a_job(
    frozen_time,
    wait_time,
    side_effect,
    mock_proxy_waiting_time,
):
    start_time = datetime.now()
    job: LAVAJob = retriable_follow_job(
        mock_proxy_waiting_time(
            frozen_time, side_effect=side_effect, wait_time=wait_time
        ),
        "",
    )

    end_time = datetime.now()
    delta_time = end_time - start_time

    assert job.status == "pass"
    assert delta_time.total_seconds() >= wait_time



CORRUPTED_LOG_SCENARIOS = {
    "too much subsequent corrupted data": (
        [(False, "{'msg': 'Incomplete}")] * 100 + [jobs_logs_response(True)],
        pytest.raises((MesaCIRetryError)),
    ),
    "one subsequent corrupted data": (
        [(False, "{'msg': 'Incomplete}")] * 2 + [jobs_logs_response(True)],
        does_not_raise(),
    ),
}


@pytest.mark.parametrize(
    "data_sequence, expected_exception",
    CORRUPTED_LOG_SCENARIOS.values(),
    ids=CORRUPTED_LOG_SCENARIOS.keys(),
)
def test_log_corruption(mock_sleep, data_sequence, expected_exception, mock_proxy):
    proxy_mock = mock_proxy()
    proxy_logs_mock = proxy_mock.scheduler.jobs.logs
    proxy_logs_mock.side_effect = data_sequence
    with expected_exception:
        retriable_follow_job(proxy_mock, "")


LAVA_RESULT_LOG_SCENARIOS = {
    # the submitter should accept xtrace logs
    "Bash xtrace echo with kmsg interleaving": (
        "echo hwci: mesa: pass[  737.673352] <LAVA_SIGNAL_ENDTC mesa-ci>",
        "pass",
    ),
    # the submitter should accept xtrace logs
    "kmsg result print": (
        "[  737.673352] hwci: mesa: pass",
        "pass",
    ),
    # if the job result echo has a very bad luck, it still can be interleaved
    # with kmsg
    "echo output with kmsg interleaving": (
        "hwci: mesa: pass[  737.673352] <LAVA_SIGNAL_ENDTC mesa-ci>",
        "pass",
    ),
    "fail case": (
        "hwci: mesa: fail",
        "fail",
    ),
}


@pytest.mark.parametrize(
    "message, expectation",
    LAVA_RESULT_LOG_SCENARIOS.values(),
    ids=LAVA_RESULT_LOG_SCENARIOS.keys(),
)
def test_parse_job_result_from_log(message, expectation, mock_proxy):
    job = LAVAJob(mock_proxy(), "")
    job.parse_job_result_from_log([message])

    assert job.status == expectation


@pytest.mark.slow(
    reason="Slow and sketchy test. Needs a LAVA log raw file at /tmp/log.yaml"
)
@pytest.mark.skipif(
    not Path("/tmp/log.yaml").is_file(), reason="Missing /tmp/log.yaml file."
)
def test_full_yaml_log(mock_proxy, frozen_time, lava_job_submitter):
    import random

    from lavacli.utils import flow_yaml as lava_yaml

    def time_travel_from_log_chunk(data_chunk):
        if not data_chunk:
            return

        first_log_time = data_chunk[0]["dt"]
        frozen_time.move_to(first_log_time)
        yield

        last_log_time = data_chunk[-1]["dt"]
        frozen_time.move_to(last_log_time)
        return

    def time_travel_to_test_time():
        # Suppose that the first message timestamp of the entire LAVA job log is
        # the same of from the job submitter execution
        with open("/tmp/log.yaml", "r") as f:
            first_log = f.readline()
            first_log_time = lava_yaml.load(first_log)[0]["dt"]
            frozen_time.move_to(first_log_time)

    def load_lines() -> list:
        with open("/tmp/log.yaml", "r") as f:
            # data = yaml.safe_load(f)
            data = f.readlines()
            stream = chain(data)
            try:
                while True:
                    data_chunk = [next(stream) for _ in range(random.randint(0, 50))]
                    serial_message = "".join(data_chunk)
                    # Suppose that the first message timestamp is the same of
                    # log fetch RPC call
                    time_travel_from_log_chunk(data_chunk)
                    yield False, "[]"
                    # Travel to the same datetime of the last fetched log line
                    # in the chunk
                    time_travel_from_log_chunk(data_chunk)
                    yield False, serial_message
            except StopIteration:
                yield True, serial_message
                return

    proxy = mock_proxy()

    def reset_logs(*args):
        proxy.scheduler.jobs.logs.side_effect = load_lines()

    proxy.scheduler.jobs.submit = reset_logs
    with pytest.raises(MesaCIRetryError):
        time_travel_to_test_time()
        lava_job_submitter.submit()
        retriable_follow_job(proxy, "")
        print(lava_job_submitter.structured_log_file.read_text())


@pytest.mark.parametrize(
    "validate_only,finished_job_status,expected_combined_status,expected_exit_code",
    [
        (True, "pass", None, None),
        (False, "pass", "pass", 0),
        (False, "fail", "fail", 1),
    ],
    ids=[
        "validate_only_no_job_submission",
        "successful_job_submission",
        "failed_job_submission",
    ],
)
def test_job_combined_status(
    lava_job_submitter,
    validate_only,
    finished_job_status,
    expected_combined_status,
    expected_exit_code,
):
    lava_job_submitter.validate_only = validate_only

    with patch(
        "lava.lava_job_submitter.retriable_follow_job"
    ) as mock_retriable_follow_job, patch(
        "lava.lava_job_submitter.LAVAJobSubmitter._LAVAJobSubmitter__prepare_submission"
    ) as mock_prepare_submission, patch(
        "sys.exit"
    ):
        from lava.lava_job_submitter import STRUCTURAL_LOG

        mock_retriable_follow_job.return_value = MagicMock(status=finished_job_status)

        mock_job_definition = MagicMock(spec=str)
        mock_prepare_submission.return_value = mock_job_definition
        original_status: str = STRUCTURAL_LOG.get("job_combined_status")

        if validate_only:
            lava_job_submitter.submit()
            mock_retriable_follow_job.assert_not_called()
            assert STRUCTURAL_LOG.get("job_combined_status") == original_status
            return

        try:
            lava_job_submitter.submit()

        except SystemExit as e:
            assert e.code == expected_exit_code

        assert STRUCTURAL_LOG["job_combined_status"] == expected_combined_status
