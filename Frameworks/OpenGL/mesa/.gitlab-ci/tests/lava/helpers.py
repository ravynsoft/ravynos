from contextlib import nullcontext as does_not_raise
from datetime import datetime
from io import StringIO
from itertools import cycle
from typing import Any, Callable, Generator, Iterable, Optional, Tuple, Union

from freezegun import freeze_time
from lava.utils.log_section import (
    DEFAULT_GITLAB_SECTION_TIMEOUTS,
    FALLBACK_GITLAB_SECTION_TIMEOUT,
    LogSectionType,
)
from lavacli.utils import flow_yaml as lava_yaml


def yaml_dump(data: dict[str, Any]) -> str:
    stream = StringIO()
    lava_yaml.dump(data, stream)
    return stream.getvalue()


def section_timeout(section_type: LogSectionType) -> int:
    return int(
        DEFAULT_GITLAB_SECTION_TIMEOUTS.get(
            section_type, FALLBACK_GITLAB_SECTION_TIMEOUT
        ).total_seconds()
    )


def create_lava_yaml_msg(
    dt: Callable = datetime.now, msg="test", lvl="target"
) -> dict[str, str]:
    return {"dt": str(dt()), "msg": msg, "lvl": lvl}


def generate_testsuite_result(
    name="test-mesa-ci", result="pass", metadata_extra=None, extra=None
):
    if metadata_extra is None:
        metadata_extra = {}
    if extra is None:
        extra = {}
    return {"metadata": {"result": result, **metadata_extra}, "name": name}


def jobs_logs_response(
    finished=False, msg=None, lvl="target", result=None
) -> Tuple[bool, str]:
    timed_msg = {"dt": str(datetime.now()), "msg": "New message", "lvl": lvl}
    if result:
        timed_msg["lvl"] = "target"
        timed_msg["msg"] = f"hwci: mesa: {result}"

    logs = [timed_msg] if msg is None else msg

    return finished, yaml_dump(logs)


def section_aware_message_generator(
    messages: dict[LogSectionType, Iterable[int]], result: Optional[str] = None
) -> Iterable[tuple[dict, Iterable[int]]]:
    default = [1]

    result_message_section = LogSectionType.TEST_CASE

    for section_type in LogSectionType:
        delay = messages.get(section_type, default)
        yield mock_lava_signal(section_type), delay
        if result and section_type == result_message_section:
            # To consider the job finished, the result `echo` should be produced
            # in the correct section
            yield create_lava_yaml_msg(msg=f"hwci: mesa: {result}"), delay


def message_generator():
    for section_type in LogSectionType:
        yield mock_lava_signal(section_type)


def level_generator():
    # Tests all known levels by default
    yield from cycle(("results", "feedback", "warning", "error", "debug", "target"))


def generate_n_logs(
    n=1,
    tick_fn: Union[Generator, Iterable[int], int] = 1,
    level_fn=level_generator,
    result="pass",
):
    """Simulate a log partitionated in n components"""
    level_gen = level_fn()

    if isinstance(tick_fn, Generator):
        tick_gen = tick_fn
    elif isinstance(tick_fn, Iterable):
        tick_gen = cycle(tick_fn)
    else:
        tick_gen = cycle((tick_fn,))

    with freeze_time(datetime.now()) as time_travel:
        tick_sec: int = next(tick_gen)
        while True:
            # Simulate a scenario where the target job is waiting for being started
            for _ in range(n - 1):
                level: str = next(level_gen)

                time_travel.tick(tick_sec)
                yield jobs_logs_response(finished=False, msg=[], lvl=level)

            time_travel.tick(tick_sec)
            yield jobs_logs_response(finished=True, result=result)


def to_iterable(tick_fn):
    if isinstance(tick_fn, Generator):
        return tick_fn
    elif isinstance(tick_fn, Iterable):
        return cycle(tick_fn)
    else:
        return cycle((tick_fn,))


def mock_logs(messages=None, result=None):
    if messages is None:
        messages = {}
    with freeze_time(datetime.now()) as time_travel:
        # Simulate a complete run given by message_fn
        for msg, tick_list in section_aware_message_generator(messages, result):
            for tick_sec in tick_list:
                yield jobs_logs_response(finished=False, msg=[msg])
                time_travel.tick(tick_sec)


def mock_lava_signal(type: LogSectionType) -> dict[str, str]:
    return {
        LogSectionType.TEST_CASE: create_lava_yaml_msg(
            msg="<STARTTC> case", lvl="debug"
        ),
        LogSectionType.TEST_SUITE: create_lava_yaml_msg(
            msg="<STARTRUN> suite", lvl="debug"
        ),
        LogSectionType.LAVA_POST_PROCESSING: create_lava_yaml_msg(
            msg="<LAVA_SIGNAL_ENDTC case>", lvl="target"
        ),
    }.get(type, create_lava_yaml_msg())
