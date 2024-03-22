import argparse
import logging
from datetime import datetime
from pathlib import Path

from structured_logger import StructuredLogger


class CustomLogger:
    def __init__(self, log_file):
        self.log_file = log_file
        self.logger = StructuredLogger(file_name=self.log_file)

    def get_last_dut_job(self):
        """
        Gets the details of the most recent DUT job.

        Returns:
            dict: Details of the most recent DUT job.

        Raises:
            ValueError: If no DUT jobs are found in the logger's data.
        """
        try:
            job = self.logger.data["dut_jobs"][-1]
        except KeyError:
            raise ValueError(
                "No DUT jobs found. Please create a job via create_dut_job call."
            )

        return job

    def update(self, **kwargs):
        """
        Updates the log file with provided key-value pairs.

        Args:
            **kwargs: Key-value pairs to be updated.

        """
        with self.logger.edit_context():
            for key, value in kwargs.items():
                self.logger.data[key] = value

    def create_dut_job(self, **kwargs):
        """
        Creates a new DUT job with provided key-value pairs.

        Args:
            **kwargs: Key-value pairs for the new DUT job.

        """
        with self.logger.edit_context():
            if "dut_jobs" not in self.logger.data:
                self.logger.data["dut_jobs"] = []
            new_job = {
                "status": "",
                "submitter_start_time": datetime.now().isoformat(),
                "dut_submit_time": "",
                "dut_start_time": "",
                "dut_end_time": "",
                "dut_name": "",
                "dut_state": "pending",
                "dut_job_phases": [],
                **kwargs,
            }
            self.logger.data["dut_jobs"].append(new_job)

    def update_dut_job(self, key, value):
        """
        Updates the last DUT job with a key-value pair.

        Args:
            key : The key to be updated.
            value: The value to be assigned.

        """
        with self.logger.edit_context():
            job = self.get_last_dut_job()
            job[key] = value

    def update_status_fail(self, reason=""):
        """
        Sets the status of the last DUT job to 'fail' and logs the failure reason.

        Args:
            reason (str, optional): The reason for the failure. Defaults to "".

        """
        with self.logger.edit_context():
            job = self.get_last_dut_job()
            job["status"] = "fail"
            job["dut_job_fail_reason"] = reason

    def create_job_phase(self, phase_name):
        """
        Creates a new job phase for the last DUT job.

        Args:
            phase_name : The name of the new job phase.

        """
        with self.logger.edit_context():
            job = self.get_last_dut_job()
            if job["dut_job_phases"] and job["dut_job_phases"][-1]["end_time"] == "":
                # If the last phase exists and its end time is empty, set the end time
                job["dut_job_phases"][-1]["end_time"] = datetime.now().isoformat()

            # Create a new phase
            phase_data = {
                "name": phase_name,
                "start_time": datetime.now().isoformat(),
                "end_time": "",
            }
            job["dut_job_phases"].append(phase_data)

    def check_dut_timings(self, job):
        """
        Check the timing sequence of a job to ensure logical consistency.

        The function verifies that the job's submission time is not earlier than its start time and that
        the job's end time is not earlier than its start time. If either of these conditions is found to be true,
        an error is logged for each instance of inconsistency.

        Args:
        job (dict): A dictionary containing timing information of a job. Expected keys are 'dut_start_time',
                    'dut_submit_time', and 'dut_end_time'.

        Returns:
        None: This function does not return a value; it logs errors if timing inconsistencies are detected.

        The function checks the following:
        - If 'dut_start_time' and 'dut_submit_time' are both present and correctly sequenced.
        - If 'dut_start_time' and 'dut_end_time' are both present and correctly sequenced.
        """

        # Check if the start time and submit time exist
        if job.get("dut_start_time") and job.get("dut_submit_time"):
            # If they exist, check if the submission time is before the start time
            if job["dut_start_time"] < job["dut_submit_time"]:
                logging.error("Job submission is happening before job start.")

        # Check if the start time and end time exist
        if job.get("dut_start_time") and job.get("dut_end_time"):
            # If they exist, check if the end time is after the start time
            if job["dut_end_time"] < job["dut_start_time"]:
                logging.error("Job ended before it started.")

    # Method to update DUT start, submit and end time
    def update_dut_time(self, value, custom_time):
        """
        Updates DUT start, submit, and end times.

        Args:
            value : Specifies which DUT time to update. Options: 'start', 'submit', 'end'.
            custom_time : Custom time to set. If None, use current time.

        Raises:
            ValueError: If an invalid argument is provided for value.

        """
        with self.logger.edit_context():
            job = self.get_last_dut_job()
            timestamp = custom_time if custom_time else datetime.now().isoformat()
            if value == "start":
                job["dut_start_time"] = timestamp
                job["dut_state"] = "running"
            elif value == "submit":
                job["dut_submit_time"] = timestamp
                job["dut_state"] = "submitted"
            elif value == "end":
                job["dut_end_time"] = timestamp
                job["dut_state"] = "finished"
            else:
                raise ValueError(
                    "Error: Invalid argument provided for --update-dut-time. Use 'start', 'submit', 'end'."
                )
            # check the sanity of the partial structured log
            self.check_dut_timings(job)

    def close_dut_job(self):
        """
        Closes the most recent DUT (Device Under Test) job in the logger's data.

        The method performs the following operations:
        1. Validates if there are any DUT jobs in the logger's data.
        2. If the last phase of the most recent DUT job has an empty end time, it sets the end time to the current time.

        Raises:
            ValueError: If no DUT jobs are found in the logger's data.
        """
        with self.logger.edit_context():
            job = self.get_last_dut_job()
            # Check if the last phase exists and its end time is empty, then set the end time
            if job["dut_job_phases"] and job["dut_job_phases"][-1]["end_time"] == "":
                job["dut_job_phases"][-1]["end_time"] = datetime.now().isoformat()

    def close(self):
        """
        Closes the most recent DUT (Device Under Test) job in the logger's data.

        The method performs the following operations:
        1. Determines the combined status of all DUT jobs.
        2. Sets the submitter's end time to the current time.
        3. Updates the DUT attempt counter to reflect the total number of DUT jobs.

        """
        with self.logger.edit_context():
            job_status = []
            for job in self.logger.data["dut_jobs"]:
                if "status" in job:
                    job_status.append(job["status"])

            if not job_status:
                job_combined_status = "null"
            else:
                # Get job_combined_status
                if "pass" in job_status:
                    job_combined_status = "pass"
                else:
                    job_combined_status = "fail"

            self.logger.data["job_combined_status"] = job_combined_status
            self.logger.data["dut_attempt_counter"] = len(self.logger.data["dut_jobs"])
            job["submitter_end_time"] = datetime.now().isoformat()


def process_args(args):
    # Function to process key-value pairs and call corresponding logger methods
    def process_key_value_pairs(args_list, action_func):
        if not args_list:
            raise ValueError(
                f"No key-value pairs provided for {action_func.__name__.replace('_', '-')}"
            )
        if len(args_list) % 2 != 0:
            raise ValueError(
                f"Incomplete key-value pairs for {action_func.__name__.replace('_', '-')}"
            )
        kwargs = dict(zip(args_list[::2], args_list[1::2]))
        action_func(**kwargs)

    # Create a CustomLogger object with the specified log file path
    custom_logger = CustomLogger(Path(args.log_file))

    if args.update:
        process_key_value_pairs(args.update, custom_logger.update)

    if args.create_dut_job:
        process_key_value_pairs(args.create_dut_job, custom_logger.create_dut_job)

    if args.update_dut_job:
        key, value = args.update_dut_job
        custom_logger.update_dut_job(key, value)

    if args.create_job_phase:
        custom_logger.create_job_phase(args.create_job_phase)

    if args.update_status_fail:
        custom_logger.update_status_fail(args.update_status_fail)

    if args.update_dut_time:
        if len(args.update_dut_time) == 2:
            action, custom_time = args.update_dut_time
        elif len(args.update_dut_time) == 1:
            action, custom_time = args.update_dut_time[0], None
        else:
            raise ValueError("Invalid number of values for --update-dut-time")

        if action in ["start", "end", "submit"]:
            custom_logger.update_dut_time(action, custom_time)
        else:
            raise ValueError(
                "Error: Invalid argument provided for --update-dut-time. Use 'start', 'submit', 'end'."
            )

    if args.close_dut_job:
        custom_logger.close_dut_job()

    if args.close:
        custom_logger.close()


def main():
    parser = argparse.ArgumentParser(description="Custom Logger Command Line Tool")
    parser.add_argument("log_file", help="Path to the log file")
    parser.add_argument(
        "--update",
        nargs=argparse.ZERO_OR_MORE,
        metavar=("key", "value"),
        help="Update a key-value pair e.g., --update key1 value1 key2 value2)",
    )
    parser.add_argument(
        "--create-dut-job",
        nargs=argparse.ZERO_OR_MORE,
        metavar=("key", "value"),
        help="Create a new DUT job with key-value pairs (e.g., --create-dut-job key1 value1 key2 value2)",
    )
    parser.add_argument(
        "--update-dut-job",
        nargs=argparse.ZERO_OR_MORE,
        metavar=("key", "value"),
        help="Update a key-value pair in DUT job",
    )
    parser.add_argument(
        "--create-job-phase",
        help="Create a new job phase (e.g., --create-job-phase name)",
    )
    parser.add_argument(
        "--update-status-fail",
        help="Update fail as the status and log the failure reason (e.g., --update-status-fail reason)",
    )
    parser.add_argument(
        "--update-dut-time",
        nargs=argparse.ZERO_OR_MORE,
        metavar=("action", "custom_time"),
        help="Update DUT start and end time. Provide action ('start', 'submit', 'end') and custom_time (e.g., '2023-01-01T12:00:00')",
    )
    parser.add_argument(
        "--close-dut-job",
        action="store_true",
        help="Close the dut job by updating end time of last dut job)",
    )
    parser.add_argument(
        "--close",
        action="store_true",
        help="Updates combined status, submitter's end time and DUT attempt counter",
    )
    args = parser.parse_args()

    process_args(args)


if __name__ == "__main__":
    main()
