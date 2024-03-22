from collections import defaultdict
from unittest.mock import MagicMock, patch

import pytest
import yaml
from freezegun import freeze_time
from hypothesis import settings

from .lava.helpers import generate_testsuite_result, jobs_logs_response

settings.register_profile("ci", max_examples=1000, derandomize=True)
settings.load_profile("ci")

def pytest_configure(config):
    config.addinivalue_line(
        "markers", "slow: marks tests as slow (deselect with '-m \"not slow\"')"
    )

@pytest.fixture
def mock_sleep():
    """Mock time.sleep to make test faster"""
    with patch("time.sleep", return_value=None):
        yield


@pytest.fixture
def frozen_time(mock_sleep):
    with freeze_time() as frozen_time:
        yield frozen_time


RESULT_GET_TESTJOB_RESULTS = [{"metadata": {"result": "test"}}]


@pytest.fixture
def mock_proxy(frozen_time):
    def create_proxy_mock(
        job_results=RESULT_GET_TESTJOB_RESULTS,
        testsuite_results=[generate_testsuite_result()],
        **kwargs
    ):
        proxy_mock = MagicMock()
        proxy_submit_mock = proxy_mock.scheduler.jobs.submit
        proxy_submit_mock.return_value = "1234"

        proxy_results_mock = proxy_mock.results.get_testjob_results_yaml
        proxy_results_mock.return_value = yaml.safe_dump(job_results)

        proxy_test_suites_mock = proxy_mock.results.get_testsuite_results_yaml
        proxy_test_suites_mock.return_value = yaml.safe_dump(testsuite_results)

        proxy_logs_mock = proxy_mock.scheduler.jobs.logs
        proxy_logs_mock.return_value = jobs_logs_response()

        proxy_job_state = proxy_mock.scheduler.job_state
        proxy_job_state.return_value = {"job_state": "Running"}
        proxy_job_state.side_effect = frozen_time.tick(1)

        proxy_show_mock = proxy_mock.scheduler.jobs.show
        proxy_show_mock.return_value = defaultdict(
            str,
            {
                "device_type": "test_device",
                "device": "test_device-cbg-1",
                "state": "created",
            },
        )

        for key, value in kwargs.items():
            setattr(proxy_logs_mock, key, value)

        return proxy_mock

    yield create_proxy_mock
