import re
import xmlrpc
from collections import defaultdict
from datetime import datetime
from typing import Any, Optional

from lava.exceptions import (
    MesaCIException,
    MesaCIKnownIssueException,
    MesaCIParseException,
    MesaCITimeoutError,
)
from lava.utils import CONSOLE_LOG
from lava.utils.log_follower import print_log
from lavacli.utils import flow_yaml as lava_yaml

from .lava_proxy import call_proxy


class LAVAJob:
    COLOR_STATUS_MAP: dict[str, str] = {
        "pass": CONSOLE_LOG["FG_GREEN"],
        "hung": CONSOLE_LOG["FG_YELLOW"],
        "fail": CONSOLE_LOG["FG_RED"],
        "canceled": CONSOLE_LOG["FG_MAGENTA"],
    }

    def __init__(self, proxy, definition, log=defaultdict(str)) -> None:
        self._job_id = None
        self.proxy = proxy
        self.definition = definition
        self.last_log_line = 0
        self.last_log_time = None
        self._is_finished = False
        self.log: dict[str, Any] = log
        self.status = "not_submitted"
        self.__exception: Optional[str] = None

    def heartbeat(self) -> None:
        self.last_log_time: datetime = datetime.now()
        self.status = "running"

    @property
    def status(self) -> str:
        return self._status

    @status.setter
    def status(self, new_status: str) -> None:
        self._status = new_status
        self.log["status"] = self._status

    @property
    def job_id(self) -> int:
        return self._job_id

    @job_id.setter
    def job_id(self, new_id: int) -> None:
        self._job_id = new_id
        self.log["lava_job_id"] = self._job_id

    @property
    def is_finished(self) -> bool:
        return self._is_finished

    @property
    def exception(self) -> str:
        return self.__exception

    @exception.setter
    def exception(self, exception: Exception) -> None:
        self.__exception = repr(exception)
        self.log["dut_job_fail_reason"] = self.__exception

    def validate(self) -> Optional[dict]:
        """Returns a dict with errors, if the validation fails.

        Returns:
            Optional[dict]: a dict with the validation errors, if any
        """
        return call_proxy(self.proxy.scheduler.jobs.validate, self.definition, True)

    def show(self) -> dict[str, str]:
        return call_proxy(self.proxy.scheduler.jobs.show, self._job_id)

    def get_lava_time(self, key, data) -> Optional[str]:
        return data[key].value if data[key] else None

    def refresh_log(self) -> None:
        details = self.show()
        self.log["dut_start_time"] = self.get_lava_time("start_time", details)
        self.log["dut_submit_time"] = self.get_lava_time("submit_time", details)
        self.log["dut_end_time"] = self.get_lava_time("end_time", details)
        self.log["dut_name"] = details.get("device")
        self.log["dut_state"] = details.get("state")

    def submit(self) -> bool:
        try:
            self.job_id = call_proxy(self.proxy.scheduler.jobs.submit, self.definition)
            self.status = "submitted"
            self.refresh_log()
        except MesaCIException:
            return False
        return True

    def lava_state(self) -> str:
        job_state: dict[str, str] = call_proxy(
            self.proxy.scheduler.job_state, self._job_id
        )
        return job_state["job_state"]

    def cancel(self):
        if self._job_id:
            self.proxy.scheduler.jobs.cancel(self._job_id)
            # If we don't have yet set another job's status, let's update it
            # with canceled one
            if self.status == "running":
                self.status = "canceled"

    def is_started(self) -> bool:
        waiting_states = ("Submitted", "Scheduling", "Scheduled")
        return self.lava_state() not in waiting_states

    def is_post_processed(self) -> bool:
        return self.lava_state() != "Running"

    def _load_log_from_data(self, data) -> list[str]:
        lines = []
        if isinstance(data, xmlrpc.client.Binary):
            # We are dealing with xmlrpc.client.Binary
            # Let's extract the data
            data = data.data
        # When there is no new log data, the YAML is empty
        if loaded_lines := lava_yaml.load(data):
            lines: list[str] = loaded_lines
            self.last_log_line += len(lines)
        return lines

    def get_logs(self) -> list[str]:
        try:
            (finished, data) = call_proxy(
                self.proxy.scheduler.jobs.logs, self._job_id, self.last_log_line
            )
            self._is_finished = finished
            return self._load_log_from_data(data)

        except Exception as mesa_ci_err:
            raise MesaCIParseException(
                f"Could not get LAVA job logs. Reason: {mesa_ci_err}"
            ) from mesa_ci_err

    def parse_job_result_from_log(
        self, lava_lines: list[dict[str, str]]
    ) -> list[dict[str, str]]:
        """Use the console log to catch if the job has completed successfully or
        not. Returns the list of log lines until the result line."""

        last_line = None  # Print all lines. lines[:None] == lines[:]

        for idx, line in enumerate(lava_lines):
            if result := re.search(r"hwci: mesa: (pass|fail)", line):
                self._is_finished = True
                self.status = result[1]

                last_line = idx + 1
                # We reached the log end here. hwci script has finished.
                break
        return lava_lines[:last_line]

    def handle_exception(self, exception: Exception):
        print_log(exception)
        self.cancel()
        self.exception = exception

        # Give more accurate status depending on exception
        if isinstance(exception, MesaCIKnownIssueException):
            self.status = "canceled"
        elif isinstance(exception, MesaCITimeoutError):
            self.status = "hung"
        elif isinstance(exception, MesaCIException):
            self.status = "failed"
        elif isinstance(exception, KeyboardInterrupt):
            self.status = "interrupted"
            print_log("LAVA job submitter was interrupted. Cancelling the job.")
            raise
        else:
            self.status = "job_submitter_error"
