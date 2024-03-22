from __future__ import annotations

import re
from dataclasses import dataclass, field
from datetime import datetime, timedelta
from typing import TYPE_CHECKING, Optional

from lava.utils.console_format import CONSOLE_LOG

if TYPE_CHECKING:
    from lava.utils.log_section import LogSectionType


# TODO: Add section final status to assist with monitoring
@dataclass
class GitlabSection:
    id: str
    header: str
    type: LogSectionType
    start_collapsed: bool = False
    escape: str = "\x1b[0K"
    colour: str = f"{CONSOLE_LOG['BOLD']}{CONSOLE_LOG['FG_GREEN']}"
    __start_time: Optional[datetime] = field(default=None, init=False)
    __end_time: Optional[datetime] = field(default=None, init=False)

    @classmethod
    def section_id_filter(cls, value) -> str:
        return str(re.sub(r"[^\w_-]+", "-", value))

    def __post_init__(self):
        self.id = self.section_id_filter(self.id)

    @property
    def has_started(self) -> bool:
        return self.__start_time is not None

    @property
    def has_finished(self) -> bool:
        return self.__end_time is not None

    @property
    def start_time(self) -> datetime:
        return self.__start_time

    @property
    def end_time(self) -> Optional[datetime]:
        return self.__end_time

    def get_timestamp(self, time: datetime) -> str:
        unix_ts = datetime.timestamp(time)
        return str(int(unix_ts))

    def section(self, marker: str, header: str, time: datetime) -> str:
        preamble = f"{self.escape}section_{marker}"
        collapse = marker == "start" and self.start_collapsed
        collapsed = "[collapsed=true]" if collapse else ""
        section_id = f"{self.id}{collapsed}"

        timestamp = self.get_timestamp(time)
        before_header = ":".join([preamble, timestamp, section_id])
        colored_header = f"{self.colour}{header}\x1b[0m" if header else ""
        header_wrapper = "\r" + f"{self.escape}{colored_header}"

        return f"{before_header}{header_wrapper}"

    def __str__(self) -> str:
        status = "NS" if not self.has_started else "F" if self.has_finished else "IP"
        delta = self.delta_time()
        elapsed_time = "N/A" if delta is None else str(delta)
        return (
            f"GitlabSection({self.id}, {self.header}, {self.type}, "
            f"SC={self.start_collapsed}, S={status}, ST={self.start_time}, "
            f"ET={self.end_time}, ET={elapsed_time})"
        )

    def __enter__(self):
        print(self.start())
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        print(self.end())

    def start(self) -> str:
        assert not self.has_finished, "Starting an already finished section"
        self.__start_time = datetime.now()
        return self.section(marker="start", header=self.header, time=self.__start_time)

    def end(self) -> str:
        assert self.has_started, "Ending an uninitialized section"
        self.__end_time = datetime.now()
        assert (
            self.__end_time >= self.__start_time
        ), "Section execution time will be negative"
        return self.section(marker="end", header="", time=self.__end_time)

    def delta_time(self) -> Optional[timedelta]:
        if self.__start_time and self.__end_time:
            return self.__end_time - self.__start_time

        if self.has_started:
            return datetime.now() - self.__start_time

        return None
