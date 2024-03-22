#!/usr/bin/env python3
#
# Copyright (C) 2020 - 2023 Collabora Limited
# Authors:
#     Gustavo Padovan <gustavo.padovan@collabora.com>
#     Guilherme Gallo <guilherme.gallo@collabora.com>
#
# SPDX-License-Identifier: MIT

"""Send a job to LAVA, track it and collect log back"""

import contextlib
import json
import pathlib
import sys
import time
from collections import defaultdict
from dataclasses import dataclass, fields
from datetime import datetime, timedelta
from os import environ, getenv, path
from typing import Any, Optional

import fire
from lavacli.utils import flow_yaml as lava_yaml

from lava.exceptions import (
    MesaCIException,
    MesaCIParseException,
    MesaCIRetryError,
    MesaCITimeoutError,
)
from lava.utils import (
    CONSOLE_LOG,
    GitlabSection,
    LAVAJob,
    LAVAJobDefinition,
    LogFollower,
    LogSectionType,
    call_proxy,
    fatal_err,
    hide_sensitive_data,
    print_log,
    setup_lava_proxy,
)
from lava.utils import DEFAULT_GITLAB_SECTION_TIMEOUTS as GL_SECTION_TIMEOUTS

# Initialize structural logging with a defaultdict, it can be changed for more
# sophisticated dict-like data abstractions.
STRUCTURAL_LOG = defaultdict(list)

try:
    from ci.structured_logger import StructuredLogger
except ImportError as e:
    print_log(
        f"Could not import StructuredLogger library: {e}. "
        "Falling back to defaultdict based structured logger."
    )

# Timeout in seconds to decide if the device from the dispatched LAVA job has
# hung or not due to the lack of new log output.
DEVICE_HANGING_TIMEOUT_SEC = int(getenv("DEVICE_HANGING_TIMEOUT_SEC",  5*60))

# How many seconds the script should wait before try a new polling iteration to
# check if the dispatched LAVA job is running or waiting in the job queue.
WAIT_FOR_DEVICE_POLLING_TIME_SEC = int(
    getenv("LAVA_WAIT_FOR_DEVICE_POLLING_TIME_SEC", 1)
)

# How many seconds the script will wait to let LAVA finalize the job and give
# the final details.
WAIT_FOR_LAVA_POST_PROCESSING_SEC = int(getenv("LAVA_WAIT_LAVA_POST_PROCESSING_SEC", 5))
WAIT_FOR_LAVA_POST_PROCESSING_RETRIES = int(
    getenv("LAVA_WAIT_LAVA_POST_PROCESSING_RETRIES", 6)
)

# How many seconds to wait between log output LAVA RPC calls.
LOG_POLLING_TIME_SEC = int(getenv("LAVA_LOG_POLLING_TIME_SEC", 5))

# How many retries should be made when a timeout happen.
NUMBER_OF_RETRIES_TIMEOUT_DETECTION = int(
    getenv("LAVA_NUMBER_OF_RETRIES_TIMEOUT_DETECTION", 2)
)


def raise_exception_from_metadata(metadata: dict, job_id: int) -> None:
    """
    Investigate infrastructure errors from the job metadata.
    If it finds an error, raise it as MesaCIException.
    """
    if "result" not in metadata or metadata["result"] != "fail":
        return
    if "error_type" in metadata:
        error_type = metadata["error_type"]
        if error_type == "Infrastructure":
            raise MesaCIException(
                f"LAVA job {job_id} failed with Infrastructure Error. Retry."
            )
        if error_type == "Job":
            # This happens when LAVA assumes that the job cannot terminate or
            # with mal-formed job definitions. As we are always validating the
            # jobs, only the former is probable to happen. E.g.: When some LAVA
            # action timed out more times than expected in job definition.
            raise MesaCIException(
                f"LAVA job {job_id} failed with JobError "
                "(possible LAVA timeout misconfiguration/bug). Retry."
            )
    if "case" in metadata and metadata["case"] == "validate":
        raise MesaCIException(
            f"LAVA job {job_id} failed validation (possible download error). Retry."
        )


def raise_lava_error(job) -> None:
    # Look for infrastructure errors, raise them, and retry if we see them.
    results_yaml = call_proxy(job.proxy.results.get_testjob_results_yaml, job.job_id)
    results = lava_yaml.load(results_yaml)
    for res in results:
        metadata = res["metadata"]
        raise_exception_from_metadata(metadata, job.job_id)

    # If we reach this far, it means that the job ended without hwci script
    # result and no LAVA infrastructure problem was found
    job.status = "fail"


def show_final_job_data(job, colour=f"{CONSOLE_LOG['BOLD']}{CONSOLE_LOG['FG_GREEN']}"):
    with GitlabSection(
        "job_data",
        "LAVA job info",
        type=LogSectionType.LAVA_POST_PROCESSING,
        start_collapsed=True,
        colour=colour,
    ):
        wait_post_processing_retries: int = WAIT_FOR_LAVA_POST_PROCESSING_RETRIES
        while not job.is_post_processed() and wait_post_processing_retries > 0:
            # Wait a little until LAVA finishes processing metadata
            time.sleep(WAIT_FOR_LAVA_POST_PROCESSING_SEC)
            wait_post_processing_retries -= 1

        if not job.is_post_processed():
            waited_for_sec: int = (
                WAIT_FOR_LAVA_POST_PROCESSING_RETRIES
                * WAIT_FOR_LAVA_POST_PROCESSING_SEC
            )
            print_log(
                f"Waited for {waited_for_sec} seconds "
                "for LAVA to post-process the job, it haven't finished yet. "
                "Dumping it's info anyway"
            )

        details: dict[str, str] = job.show()
        for field, value in details.items():
            print(f"{field:<15}: {value}")
        job.refresh_log()


def fetch_logs(job, max_idle_time, log_follower) -> None:
    is_job_hanging(job, max_idle_time)

    time.sleep(LOG_POLLING_TIME_SEC)
    new_log_lines = fetch_new_log_lines(job)
    parsed_lines = parse_log_lines(job, log_follower, new_log_lines)

    for line in parsed_lines:
        print_log(line)


def is_job_hanging(job, max_idle_time):
    # Poll to check for new logs, assuming that a prolonged period of
    # silence means that the device has died and we should try it again
    if datetime.now() - job.last_log_time > max_idle_time:
        max_idle_time_min = max_idle_time.total_seconds() / 60

        raise MesaCITimeoutError(
            f"{CONSOLE_LOG['BOLD']}"
            f"{CONSOLE_LOG['FG_YELLOW']}"
            f"LAVA job {job.job_id} does not respond for {max_idle_time_min} "
            "minutes. Retry."
            f"{CONSOLE_LOG['RESET']}",
            timeout_duration=max_idle_time,
        )


def parse_log_lines(job, log_follower, new_log_lines):

    if log_follower.feed(new_log_lines):
        # If we had non-empty log data, we can assure that the device is alive.
        job.heartbeat()
    parsed_lines = log_follower.flush()

    # Only parse job results when the script reaches the end of the logs.
    # Depending on how much payload the RPC scheduler.jobs.logs get, it may
    # reach the LAVA_POST_PROCESSING phase.
    if log_follower.current_section.type in (
        LogSectionType.TEST_CASE,
        LogSectionType.LAVA_POST_PROCESSING,
    ):
        parsed_lines = job.parse_job_result_from_log(parsed_lines)
    return parsed_lines


def fetch_new_log_lines(job):

    # The XMLRPC binary packet may be corrupted, causing a YAML scanner error.
    # Retry the log fetching several times before exposing the error.
    for _ in range(5):
        with contextlib.suppress(MesaCIParseException):
            new_log_lines = job.get_logs()
            break
    else:
        raise MesaCIParseException
    return new_log_lines


def submit_job(job):
    try:
        job.submit()
    except Exception as mesa_ci_err:
        raise MesaCIException(
            f"Could not submit LAVA job. Reason: {mesa_ci_err}"
        ) from mesa_ci_err


def wait_for_job_get_started(job):
    print_log(f"Waiting for job {job.job_id} to start.")
    while not job.is_started():
        time.sleep(WAIT_FOR_DEVICE_POLLING_TIME_SEC)
    job.refresh_log()
    print_log(f"Job {job.job_id} started.")


def bootstrap_log_follower() -> LogFollower:
    gl = GitlabSection(
        id="lava_boot",
        header="LAVA boot",
        type=LogSectionType.LAVA_BOOT,
        start_collapsed=True,
    )
    print(gl.start())
    return LogFollower(starting_section=gl)


def follow_job_execution(job, log_follower):
    with log_follower:
        max_idle_time = timedelta(seconds=DEVICE_HANGING_TIMEOUT_SEC)
        # Start to check job's health
        job.heartbeat()
        while not job.is_finished:
            fetch_logs(job, max_idle_time, log_follower)
            structural_log_phases(job, log_follower)

    # Mesa Developers expect to have a simple pass/fail job result.
    # If this does not happen, it probably means a LAVA infrastructure error
    # happened.
    if job.status not in ["pass", "fail"]:
        raise_lava_error(job)

    # LogFollower does some cleanup after the early exit (trigger by
    # `hwci: pass|fail` regex), let's update the phases after the cleanup.
    structural_log_phases(job, log_follower)


def structural_log_phases(job, log_follower):
    phases: dict[str, Any] = {
        s.header.split(" - ")[0]: {
            k: str(getattr(s, k)) for k in ("start_time", "end_time")
        }
        for s in log_follower.section_history
    }
    job.log["dut_job_phases"] = phases


def print_job_final_status(job):
    if job.status == "running":
        job.status = "hung"

    color = LAVAJob.COLOR_STATUS_MAP.get(job.status, CONSOLE_LOG["FG_RED"])
    print_log(
        f"{color}"
        f"LAVA Job finished with status: {job.status}"
        f"{CONSOLE_LOG['RESET']}"
    )

    job.refresh_log()
    show_final_job_data(job, colour=f"{CONSOLE_LOG['BOLD']}{color}")


def execute_job_with_retries(
    proxy, job_definition, retry_count, jobs_log
) -> Optional[LAVAJob]:
    last_failed_job = None
    for attempt_no in range(1, retry_count + 2):
        # Need to get the logger value from its object to enable autosave
        # features, if AutoSaveDict is enabled from StructuredLogging module
        jobs_log.append({})
        job_log = jobs_log[-1]
        job = LAVAJob(proxy, job_definition, job_log)
        STRUCTURAL_LOG["dut_attempt_counter"] = attempt_no
        try:
            job_log["submitter_start_time"] = datetime.now().isoformat()
            submit_job(job)
            wait_for_job_get_started(job)
            log_follower: LogFollower = bootstrap_log_follower()
            follow_job_execution(job, log_follower)
            return job

        except (MesaCIException, KeyboardInterrupt) as exception:
            job.handle_exception(exception)

        finally:
            print_job_final_status(job)
            # If LAVA takes too long to post process the job, the submitter
            # gives up and proceeds.
            job_log["submitter_end_time"] = datetime.now().isoformat()
            last_failed_job = job
            print_log(
                f"{CONSOLE_LOG['BOLD']}"
                f"Finished executing LAVA job in the attempt #{attempt_no}"
                f"{CONSOLE_LOG['RESET']}"
            )

    return last_failed_job


def retriable_follow_job(proxy, job_definition) -> LAVAJob:
    number_of_retries = NUMBER_OF_RETRIES_TIMEOUT_DETECTION

    last_attempted_job = execute_job_with_retries(
        proxy, job_definition, number_of_retries, STRUCTURAL_LOG["dut_jobs"]
    )

    if last_attempted_job.exception is not None:
        # Infra failed in all attempts
        raise MesaCIRetryError(
            f"{CONSOLE_LOG['BOLD']}"
            f"{CONSOLE_LOG['FG_RED']}"
            "Job failed after it exceeded the number of "
            f"{number_of_retries} retries."
            f"{CONSOLE_LOG['RESET']}",
            retry_count=number_of_retries,
            last_job=last_attempted_job,
        )

    return last_attempted_job


@dataclass
class PathResolver:
    def __post_init__(self):
        for field in fields(self):
            value = getattr(self, field.name)
            if not value:
                continue
            if field.type == pathlib.Path:
                value = pathlib.Path(value)
                setattr(self, field.name, value.resolve())


@dataclass
class LAVAJobSubmitter(PathResolver):
    boot_method: str
    ci_project_dir: str
    device_type: str
    job_timeout_min: int  # The job timeout in minutes
    build_url: str = None
    dtb_filename: str = None
    dump_yaml: bool = False  # Whether to dump the YAML payload to stdout
    first_stage_init: str = None
    jwt_file: pathlib.Path = None
    kernel_image_name: str = None
    kernel_image_type: str = ""
    kernel_url_prefix: str = None
    kernel_external: str = None
    lava_tags: str = ""  # Comma-separated LAVA tags for the job
    mesa_job_name: str = "mesa_ci_job"
    pipeline_info: str = ""
    rootfs_url_prefix: str = None
    validate_only: bool = False  # Whether to only validate the job, not execute it
    visibility_group: str = None  # Only affects LAVA farm maintainers
    job_rootfs_overlay_url: str = None
    structured_log_file: pathlib.Path = None  # Log file path with structured LAVA log
    ssh_client_image: str = None  # x86_64 SSH client image to follow the job's output
    project_name: str = None  # Project name to be used in the job name
    __structured_log_context = contextlib.nullcontext()  # Structured Logger context

    def __post_init__(self) -> None:
        super().__post_init__()
        # Remove mesa job names with spaces, which breaks the lava-test-case command
        self.mesa_job_name = self.mesa_job_name.split(" ")[0]

        if not self.structured_log_file:
            return

        self.__structured_log_context = StructuredLoggerWrapper(self).logger_context()
        self.proxy = setup_lava_proxy()

    def __prepare_submission(self) -> str:
        # Overwrite the timeout for the testcases with the value offered by the
        # user. The testcase running time should be at least 4 times greater than
        # the other sections (boot and setup), so we can safely ignore them.
        # If LAVA fails to stop the job at this stage, it will fall back to the
        # script section timeout with a reasonable delay.
        GL_SECTION_TIMEOUTS[LogSectionType.TEST_CASE] = timedelta(
            minutes=self.job_timeout_min
        )

        job_definition = LAVAJobDefinition(self).generate_lava_job_definition()

        if self.dump_yaml:
            self.dump_job_definition(job_definition)

        validation_job = LAVAJob(self.proxy, job_definition)
        if errors := validation_job.validate():
            fatal_err(f"Error in LAVA job definition: {errors}")
        print_log("LAVA job definition validated successfully")

        return job_definition

    @classmethod
    def is_under_ci(cls):
        ci_envvar: str = getenv("CI", "false")
        return ci_envvar.lower() == "true"

    def dump_job_definition(self, job_definition) -> None:
        with GitlabSection(
            "yaml_dump",
            "LAVA job definition (YAML)",
            type=LogSectionType.LAVA_BOOT,
            start_collapsed=True,
        ):
            print(hide_sensitive_data(job_definition))

    def submit(self) -> None:
        """
        Prepares and submits the LAVA job.
        If `validate_only` is True, it validates the job without submitting it.
        If the job finishes with a non-pass status or encounters an exception,
        the program exits with a non-zero return code.
        """
        job_definition: str = self.__prepare_submission()

        if self.validate_only:
            return

        with self.__structured_log_context:
            last_attempt_job = None
            try:
                last_attempt_job = retriable_follow_job(self.proxy, job_definition)

            except MesaCIRetryError as retry_exception:
                last_attempt_job = retry_exception.last_job

            except Exception as exception:
                STRUCTURAL_LOG["job_combined_fail_reason"] = str(exception)
                raise exception

            finally:
                self.finish_script(last_attempt_job)

    def print_log_artifact_url(self):
        relative_log_path = self.structured_log_file.relative_to(pathlib.Path.cwd())
        full_path = f"$ARTIFACTS_BASE_URL/{relative_log_path}"
        artifact_url = path.expandvars(full_path)

        print_log(f"Structural Logging data available at: {artifact_url}")

    def finish_script(self, last_attempt_job):
        if self.is_under_ci() and self.structured_log_file:
            self.print_log_artifact_url()

        if not last_attempt_job:
            # No job was run, something bad happened
            STRUCTURAL_LOG["job_combined_status"] = "script_crash"
            current_exception = str(sys.exc_info()[0])
            STRUCTURAL_LOG["job_combined_fail_reason"] = current_exception
            raise SystemExit(1)

        STRUCTURAL_LOG["job_combined_status"] = last_attempt_job.status

        if last_attempt_job.status != "pass":
            raise SystemExit(1)


class StructuredLoggerWrapper:
    def __init__(self, submitter: LAVAJobSubmitter) -> None:
        self.__submitter: LAVAJobSubmitter = submitter

    def _init_logger(self):
        STRUCTURAL_LOG["fixed_tags"] = self.__submitter.lava_tags
        STRUCTURAL_LOG["dut_job_type"] = self.__submitter.device_type
        STRUCTURAL_LOG["job_combined_fail_reason"] = None
        STRUCTURAL_LOG["job_combined_status"] = "not_submitted"
        STRUCTURAL_LOG["dut_attempt_counter"] = 0

        # Initialize dut_jobs list to enable appends
        STRUCTURAL_LOG["dut_jobs"] = []

    @contextlib.contextmanager
    def _simple_logger_context(self):
        log_file = pathlib.Path(self.__submitter.structured_log_file)
        log_file.parent.mkdir(parents=True, exist_ok=True)
        try:
            # Truncate the file
            log_file.write_text("")
            yield
        finally:
            log_file.write_text(json.dumps(STRUCTURAL_LOG, indent=2))

    def logger_context(self):
        context = contextlib.nullcontext()
        try:

            global STRUCTURAL_LOG
            STRUCTURAL_LOG = StructuredLogger(
                self.__submitter.structured_log_file, truncate=True
            ).data
        except NameError:
            context = self._simple_logger_context()

        self._init_logger()
        return context


if __name__ == "__main__":
    # given that we proxy from DUT -> LAVA dispatcher -> LAVA primary -> us ->
    # GitLab runner -> GitLab primary -> user, safe to say we don't need any
    # more buffering
    sys.stdout.reconfigure(line_buffering=True)
    sys.stderr.reconfigure(line_buffering=True)
    # LAVA farm is giving datetime in UTC timezone, let's set it locally for the
    # script run.
    # Setting environ here will not affect the system time, as the os.environ
    # lifetime follows the script one.
    environ["TZ"] = "UTC"
    time.tzset()

    fire.Fire(LAVAJobSubmitter)
