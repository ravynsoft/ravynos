from datetime import timedelta


class MesaCIException(Exception):
    pass


class MesaCITimeoutError(MesaCIException):
    def __init__(self, *args, timeout_duration: timedelta) -> None:
        super().__init__(*args)
        self.timeout_duration = timeout_duration


class MesaCIRetryError(MesaCIException):
    def __init__(self, *args, retry_count: int, last_job: None) -> None:
        super().__init__(*args)
        self.retry_count = retry_count
        self.last_job = last_job


class MesaCIParseException(MesaCIException):
    pass


class MesaCIKnownIssueException(MesaCIException):
    """Exception raised when the Mesa CI script finds something in the logs that
    is known to cause the LAVA job to eventually fail"""

    pass
