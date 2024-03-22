import logging
import subprocess
from datetime import datetime

import pytest
from custom_logger import CustomLogger


@pytest.fixture
def tmp_log_file(tmp_path):
    return tmp_path / "test_log.json"


@pytest.fixture
def custom_logger(tmp_log_file):
    return CustomLogger(tmp_log_file)


def run_script_with_args(args):
    import custom_logger

    script_path = custom_logger.__file__
    return subprocess.run(
        ["python3", str(script_path), *args], capture_output=True, text=True
    )


# Test case for missing log file
@pytest.mark.parametrize(
    "key, value", [("dut_attempt_counter", "1"), ("job_combined_status", "pass")]
)
def test_missing_log_file_argument(key, value):
    result = run_script_with_args(["--update", "key", "value"])
    assert result.returncode != 0


# Parametrize test case for valid update arguments
@pytest.mark.parametrize(
    "key, value", [("dut_attempt_counter", "1"), ("job_combined_status", "pass")]
)
def test_update_argument_valid(custom_logger, tmp_log_file, key, value):
    result = run_script_with_args([str(tmp_log_file), "--update", key, value])
    assert result.returncode == 0


# Test case for passing only the key without a value
def test_update_argument_key_only(custom_logger, tmp_log_file):
    key = "dut_attempt_counter"
    result = run_script_with_args([str(tmp_log_file), "--update", key])
    assert result.returncode != 0


# Test case for not passing any key-value pair
def test_update_argument_no_values(custom_logger, tmp_log_file):
    result = run_script_with_args([str(tmp_log_file), "--update"])
    assert result.returncode == 0


# Parametrize test case for valid arguments
@pytest.mark.parametrize(
    "key, value", [("dut_attempt_counter", "1"), ("job_combined_status", "pass")]
)
def test_create_argument_valid(custom_logger, tmp_log_file, key, value):
    result = run_script_with_args([str(tmp_log_file), "--create-dut-job", key, value])
    assert result.returncode == 0


# Test case for passing only the key without a value
def test_create_argument_key_only(custom_logger, tmp_log_file):
    key = "dut_attempt_counter"
    result = run_script_with_args([str(tmp_log_file), "--create-dut-job", key])
    assert result.returncode != 0


# Test case for not passing any key-value pair
def test_create_argument_no_values(custom_logger, tmp_log_file):
    result = run_script_with_args([str(tmp_log_file), "--create-dut-job"])
    assert result.returncode == 0


# Test case for updating a DUT job
@pytest.mark.parametrize(
    "key, value", [("status", "hung"), ("dut_state", "Canceling"), ("dut_name", "asus")]
)
def test_update_dut_job(custom_logger, tmp_log_file, key, value):
    result = run_script_with_args([str(tmp_log_file), "--update-dut-job", key, value])
    assert result.returncode != 0

    result = run_script_with_args([str(tmp_log_file), "--create-dut-job", key, value])
    assert result.returncode == 0

    result = run_script_with_args([str(tmp_log_file), "--update-dut-job", key, value])
    assert result.returncode == 0


# Test case for updating last DUT job
def test_update_dut_multiple_job(custom_logger, tmp_log_file):
    # Create the first DUT job with the first key
    result = run_script_with_args(
        [str(tmp_log_file), "--create-dut-job", "status", "hung"]
    )
    assert result.returncode == 0

    # Create the second DUT job with the second key
    result = run_script_with_args(
        [str(tmp_log_file), "--create-dut-job", "dut_state", "Canceling"]
    )
    assert result.returncode == 0

    result = run_script_with_args(
        [str(tmp_log_file), "--update-dut-job", "dut_name", "asus"]
    )
    assert result.returncode == 0


# Parametrize test case for valid phase arguments
@pytest.mark.parametrize(
    "phase_name",
    [("Phase1"), ("Phase2"), ("Phase3")],
)
def test_create_job_phase_valid(custom_logger, tmp_log_file, phase_name):
    custom_logger.create_dut_job(status="pass")

    result = run_script_with_args([str(tmp_log_file), "--create-job-phase", phase_name])
    assert result.returncode == 0


# Test case for not passing any arguments for create-job-phase
def test_create_job_phase_no_arguments(custom_logger, tmp_log_file):
    custom_logger.create_dut_job(status="pass")

    result = run_script_with_args([str(tmp_log_file), "--create-job-phase"])
    assert result.returncode != 0


# Test case for trying to create a phase job without an existing DUT job
def test_create_job_phase_no_dut_job(custom_logger, tmp_log_file):
    phase_name = "Phase1"

    result = run_script_with_args([str(tmp_log_file), "--create-job-phase", phase_name])
    assert result.returncode != 0


# Combined test cases for valid scenarios
def test_valid_scenarios(custom_logger, tmp_log_file):
    valid_update_args = [("dut_attempt_counter", "1"), ("job_combined_status", "pass")]
    for key, value in valid_update_args:
        result = run_script_with_args([str(tmp_log_file), "--update", key, value])
        assert result.returncode == 0

    valid_create_args = [
        ("status", "hung"),
        ("dut_state", "Canceling"),
        ("dut_name", "asus"),
        ("phase_name", "Bootloader"),
    ]
    for key, value in valid_create_args:
        result = run_script_with_args(
            [str(tmp_log_file), "--create-dut-job", key, value]
        )
        assert result.returncode == 0

    result = run_script_with_args(
        [str(tmp_log_file), "--create-dut-job", "status", "hung"]
    )
    assert result.returncode == 0

    result = run_script_with_args(
        [str(tmp_log_file), "--update-dut-job", "dut_name", "asus"]
    )
    assert result.returncode == 0

    result = run_script_with_args(
        [
            str(tmp_log_file),
            "--create-job-phase",
            "phase_name",
        ]
    )
    assert result.returncode == 0


# Parametrize test case for valid update arguments
@pytest.mark.parametrize(
    "key, value", [("dut_attempt_counter", "1"), ("job_combined_status", "pass")]
)
def test_update(custom_logger, key, value):
    custom_logger.update(**{key: value})
    logger_data = custom_logger.logger.data

    assert key in logger_data
    assert logger_data[key] == value


# Test case for updating with a key that already exists
def test_update_existing_key(custom_logger):
    key = "status"
    value = "new_value"
    custom_logger.logger.data[key] = "old_value"
    custom_logger.update(**{key: value})
    logger_data = custom_logger.logger.data

    assert key in logger_data
    assert logger_data[key] == value


# Test case for updating "dut_jobs"
def test_update_dut_jobs(custom_logger):
    key1 = "status"
    value1 = "fail"
    key2 = "state"
    value2 = "hung"

    custom_logger.create_dut_job(**{key1: value1})
    logger_data = custom_logger.logger.data

    job1 = logger_data["dut_jobs"][0]
    assert key1 in job1
    assert job1[key1] == value1

    custom_logger.update_dut_job(key2, value2)
    logger_data = custom_logger.logger.data

    job2 = logger_data["dut_jobs"][0]
    assert key2 in job2
    assert job2[key2] == value2


# Test case for creating and updating DUT job
def test_create_dut_job(custom_logger):
    key = "status"
    value1 = "pass"
    value2 = "fail"
    value3 = "hung"

    reason = "job_combined_status"
    result = "Finished"

    custom_logger.update(**{reason: result})
    logger_data = custom_logger.logger.data

    assert reason in logger_data
    assert logger_data[reason] == result

    # Create the first DUT job
    custom_logger.create_dut_job(**{key: value1})
    logger_data = custom_logger.logger.data

    assert "dut_jobs" in logger_data
    assert isinstance(logger_data["dut_jobs"], list)
    assert len(logger_data["dut_jobs"]) == 1
    assert isinstance(logger_data["dut_jobs"][0], dict)

    # Check the values of the keys in the created first DUT job
    job1 = logger_data["dut_jobs"][0]
    assert key in job1
    assert job1[key] == value1

    # Create the second DUT job
    custom_logger.create_dut_job(**{key: value2})
    logger_data = custom_logger.logger.data

    assert "dut_jobs" in logger_data
    assert isinstance(logger_data["dut_jobs"], list)
    assert len(logger_data["dut_jobs"]) == 2
    assert isinstance(logger_data["dut_jobs"][1], dict)

    # Check the values of the keys in the created second DUT job
    job2 = logger_data["dut_jobs"][1]
    assert key in job2
    assert job2[key] == value2

    # Update the second DUT job with value3
    custom_logger.update_dut_job(key, value3)
    logger_data = custom_logger.logger.data

    # Check the updated value in the second DUT job
    job2 = logger_data["dut_jobs"][1]
    assert key in job2
    assert job2[key] == value3

    # Find the index of the last DUT job
    last_job_index = len(logger_data["dut_jobs"]) - 1

    # Update the last DUT job
    custom_logger.update_dut_job("dut_name", "asus")
    logger_data = custom_logger.logger.data

    # Check the updated value in the last DUT job
    job2 = logger_data["dut_jobs"][last_job_index]
    assert "dut_name" in job2
    assert job2["dut_name"] == "asus"

    # Check that "dut_name" is not present in other DUT jobs
    for idx, job in enumerate(logger_data["dut_jobs"]):
        if idx != last_job_index:
            assert job.get("dut_name") == ""


# Test case for updating with missing "dut_jobs" key
def test_update_dut_job_missing_dut_jobs(custom_logger):
    key = "status"
    value = "fail"

    # Attempt to update a DUT job when "dut_jobs" is missing
    with pytest.raises(ValueError, match="No DUT jobs found."):
        custom_logger.update_dut_job(key, value)


# Test case for creating a job phase
def test_create_job_phase(custom_logger):
    custom_logger.create_dut_job(status="pass")
    phase_name = "Phase1"

    custom_logger.create_job_phase(phase_name)
    logger_data = custom_logger.logger.data

    assert "dut_jobs" in logger_data
    assert isinstance(logger_data["dut_jobs"], list)
    assert len(logger_data["dut_jobs"]) == 1

    job = logger_data["dut_jobs"][0]
    assert "dut_job_phases" in job
    assert isinstance(job["dut_job_phases"], list)
    assert len(job["dut_job_phases"]) == 1

    phase = job["dut_job_phases"][0]
    assert phase["name"] == phase_name
    try:
        datetime.fromisoformat(phase["start_time"])
        assert True
    except ValueError:
        assert False
    assert phase["end_time"] == ""


# Test case for creating multiple phase jobs
def test_create_multiple_phase_jobs(custom_logger):
    custom_logger.create_dut_job(status="pass")

    phase_data = [
        {
            "phase_name": "Phase1",
        },
        {
            "phase_name": "Phase2",
        },
        {
            "phase_name": "Phase3",
        },
    ]

    for data in phase_data:
        phase_name = data["phase_name"]

        custom_logger.create_job_phase(phase_name)

    logger_data = custom_logger.logger.data

    assert "dut_jobs" in logger_data
    assert isinstance(logger_data["dut_jobs"], list)
    assert len(logger_data["dut_jobs"]) == 1

    job = logger_data["dut_jobs"][0]
    assert "dut_job_phases" in job
    assert isinstance(job["dut_job_phases"], list)
    assert len(job["dut_job_phases"]) == len(phase_data)

    for data in phase_data:
        phase_name = data["phase_name"]

        phase = job["dut_job_phases"][phase_data.index(data)]

        assert phase["name"] == phase_name
        try:
            datetime.fromisoformat(phase["start_time"])
            assert True
        except ValueError:
            assert False

        if phase_data.index(data) != len(phase_data) - 1:
            try:
                datetime.fromisoformat(phase["end_time"])
                assert True
            except ValueError:
                assert False

    # Check if the end_time of the last phase is an empty string
    last_phase = job["dut_job_phases"][-1]
    assert last_phase["end_time"] == ""


# Test case for creating multiple dut jobs and updating phase job for last dut job
def test_create_two_dut_jobs_and_add_phase(custom_logger):
    # Create the first DUT job
    custom_logger.create_dut_job(status="pass")

    # Create the second DUT job
    custom_logger.create_dut_job(status="fail")

    logger_data = custom_logger.logger.data

    assert "dut_jobs" in logger_data
    assert isinstance(logger_data["dut_jobs"], list)
    assert len(logger_data["dut_jobs"]) == 2

    first_dut_job = logger_data["dut_jobs"][0]
    second_dut_job = logger_data["dut_jobs"][1]

    # Add a phase to the second DUT job
    custom_logger.create_job_phase("Phase1")

    logger_data = custom_logger.logger.data

    assert "dut_jobs" in logger_data
    assert isinstance(logger_data["dut_jobs"], list)
    assert len(logger_data["dut_jobs"]) == 2

    first_dut_job = logger_data["dut_jobs"][0]
    second_dut_job = logger_data["dut_jobs"][1]

    # Check first DUT job does not have a phase
    assert not first_dut_job.get("dut_job_phases")

    # Check second DUT job has a phase
    assert second_dut_job.get("dut_job_phases")
    assert isinstance(second_dut_job["dut_job_phases"], list)
    assert len(second_dut_job["dut_job_phases"]) == 1


# Test case for updating DUT start time
def test_update_dut_start_time(custom_logger):
    custom_logger.create_dut_job(status="pass")
    custom_logger.update_dut_time("start", None)

    logger_data = custom_logger.logger.data
    assert "dut_jobs" in logger_data
    assert len(logger_data["dut_jobs"]) == 1

    dut_job = logger_data["dut_jobs"][0]
    assert "dut_start_time" in dut_job
    assert dut_job["dut_start_time"] != ""

    try:
        datetime.fromisoformat(dut_job["dut_start_time"])
        assert True
    except ValueError:
        assert False


# Test case for updating DUT submit time
def test_update_dut_submit_time(custom_logger):
    custom_time = "2023-11-09T02:37:06Z"
    custom_logger.create_dut_job(status="pass")
    custom_logger.update_dut_time("submit", custom_time)

    logger_data = custom_logger.logger.data
    assert "dut_jobs" in logger_data
    assert len(logger_data["dut_jobs"]) == 1

    dut_job = logger_data["dut_jobs"][0]
    assert "dut_submit_time" in dut_job

    try:
        datetime.fromisoformat(dut_job["dut_submit_time"])
        assert True
    except ValueError:
        assert False


# Test case for updating DUT end time
def test_update_dut_end_time(custom_logger):
    custom_logger.create_dut_job(status="pass")
    custom_logger.update_dut_time("end", None)

    logger_data = custom_logger.logger.data
    assert "dut_jobs" in logger_data
    assert len(logger_data["dut_jobs"]) == 1

    dut_job = logger_data["dut_jobs"][0]
    assert "dut_end_time" in dut_job

    try:
        datetime.fromisoformat(dut_job["dut_end_time"])
        assert True
    except ValueError:
        assert False


# Test case for updating DUT time with invalid value
def test_update_dut_time_invalid_value(custom_logger):
    custom_logger.create_dut_job(status="pass")
    with pytest.raises(
        ValueError,
        match="Error: Invalid argument provided for --update-dut-time. Use 'start', 'submit', 'end'.",
    ):
        custom_logger.update_dut_time("invalid_value", None)


# Test case for close_dut_job
def test_close_dut_job(custom_logger):
    custom_logger.create_dut_job(status="pass")

    custom_logger.create_job_phase("Phase1")
    custom_logger.create_job_phase("Phase2")

    custom_logger.close_dut_job()

    logger_data = custom_logger.logger.data
    assert "dut_jobs" in logger_data
    assert len(logger_data["dut_jobs"]) == 1

    dut_job = logger_data["dut_jobs"][0]
    assert "dut_job_phases" in dut_job
    dut_job_phases = dut_job["dut_job_phases"]

    phase1 = dut_job_phases[0]
    assert phase1["name"] == "Phase1"

    try:
        datetime.fromisoformat(phase1["start_time"])
        assert True
    except ValueError:
        assert False

    try:
        datetime.fromisoformat(phase1["end_time"])
        assert True
    except ValueError:
        assert False

    phase2 = dut_job_phases[1]
    assert phase2["name"] == "Phase2"

    try:
        datetime.fromisoformat(phase2["start_time"])
        assert True
    except ValueError:
        assert False

    try:
        datetime.fromisoformat(phase2["end_time"])
        assert True
    except ValueError:
        assert False


# Test case for close
def test_close(custom_logger):
    custom_logger.create_dut_job(status="pass")

    custom_logger.close()

    logger_data = custom_logger.logger.data
    assert "dut_jobs" in logger_data
    assert len(logger_data["dut_jobs"]) == 1
    assert "dut_attempt_counter" in logger_data
    assert logger_data["dut_attempt_counter"] == len(logger_data["dut_jobs"])
    assert "job_combined_status" in logger_data
    assert logger_data["job_combined_status"] != ""

    dut_job = logger_data["dut_jobs"][0]
    assert "submitter_end_time" in dut_job
    try:
        datetime.fromisoformat(dut_job["submitter_end_time"])
        assert True
    except ValueError:
        assert False


# Test case for updating status to fail with a reason
def test_update_status_fail_with_reason(custom_logger):
    custom_logger.create_dut_job()

    reason = "kernel panic"
    custom_logger.update_status_fail(reason)

    logger_data = custom_logger.logger.data
    assert "dut_jobs" in logger_data
    assert len(logger_data["dut_jobs"]) == 1

    dut_job = logger_data["dut_jobs"][0]
    assert "status" in dut_job
    assert dut_job["status"] == "fail"
    assert "dut_job_fail_reason" in dut_job
    assert dut_job["dut_job_fail_reason"] == reason


# Test case for updating status to fail without providing a reason
def test_update_status_fail_without_reason(custom_logger):
    custom_logger.create_dut_job()

    custom_logger.update_status_fail()

    # Check if the status is updated and fail reason is empty
    logger_data = custom_logger.logger.data
    assert "dut_jobs" in logger_data
    assert len(logger_data["dut_jobs"]) == 1

    dut_job = logger_data["dut_jobs"][0]
    assert "status" in dut_job
    assert dut_job["status"] == "fail"
    assert "dut_job_fail_reason" in dut_job
    assert dut_job["dut_job_fail_reason"] == ""


# Test case for check_dut_timings with submission time earlier than start time
def test_check_dut_timings_submission_earlier_than_start(custom_logger, caplog):
    custom_logger.create_dut_job()

    # Set submission time to be earlier than start time
    custom_logger.update_dut_time("start", "2023-01-01T11:00:00")
    custom_logger.update_dut_time("submit", "2023-01-01T12:00:00")

    logger_data = custom_logger.logger.data
    assert "dut_jobs" in logger_data
    assert len(logger_data["dut_jobs"]) == 1

    job = logger_data["dut_jobs"][0]

    # Call check_dut_timings
    custom_logger.check_dut_timings(job)

    # Check if an error message is logged
    assert "Job submission is happening before job start." in caplog.text


# Test case for check_dut_timings with end time earlier than start time
def test_check_dut_timings_end_earlier_than_start(custom_logger, caplog):
    custom_logger.create_dut_job()

    # Set end time to be earlier than start time
    custom_logger.update_dut_time("end", "2023-01-01T11:00:00")
    custom_logger.update_dut_time("start", "2023-01-01T12:00:00")

    logger_data = custom_logger.logger.data
    assert "dut_jobs" in logger_data
    assert len(logger_data["dut_jobs"]) == 1

    job = logger_data["dut_jobs"][0]

    # Call check_dut_timings
    custom_logger.check_dut_timings(job)

    # Check if an error message is logged
    assert "Job ended before it started." in caplog.text


# Test case for check_dut_timings with valid timing sequence
def test_check_dut_timings_valid_timing_sequence(custom_logger, caplog):
    custom_logger.create_dut_job()

    # Set valid timing sequence
    custom_logger.update_dut_time("submit", "2023-01-01T12:00:00")
    custom_logger.update_dut_time("start", "2023-01-01T12:30:00")
    custom_logger.update_dut_time("end", "2023-01-01T13:00:00")

    logger_data = custom_logger.logger.data
    assert "dut_jobs" in logger_data
    assert len(logger_data["dut_jobs"]) == 1

    job = logger_data["dut_jobs"][0]

    # Call check_dut_timings
    custom_logger.check_dut_timings(job)

    # Check that no error messages are logged
    assert "Job submission is happening before job start." not in caplog.text
    assert "Job ended before it started." not in caplog.text
