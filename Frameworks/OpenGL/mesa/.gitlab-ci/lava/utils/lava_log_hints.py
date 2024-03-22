from __future__ import annotations

import re
from dataclasses import dataclass, field
from typing import TYPE_CHECKING, Any

if TYPE_CHECKING:
    from lava.utils import LogFollower

from lava.exceptions import MesaCIKnownIssueException
from lava.utils.console_format import CONSOLE_LOG
from lava.utils.log_section import LogSectionType


@dataclass
class LAVALogHints:
    log_follower: LogFollower
    has_r8152_issue_history: bool = field(default=False, init=False)

    def detect_failure(self, new_lines: list[dict[str, Any]]):
        for line in new_lines:
            self.detect_r8152_issue(line)

    def detect_r8152_issue(self, line):
        if (
            self.log_follower.phase == LogSectionType.TEST_CASE
            and line["lvl"] == "target"
        ):
            if re.search(r"r8152 \S+ eth0: Tx status -71", line["msg"]):
                self.has_r8152_issue_history = True
                return

            if self.has_r8152_issue_history and re.search(
                r"nfs: server \d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3} not responding, still trying",
                line["msg"],
            ):
                raise MesaCIKnownIssueException(
                    f"{CONSOLE_LOG['FG_MAGENTA']}"
                    "Probable network issue failure encountered, retrying the job"
                    f"{CONSOLE_LOG['RESET']}"
                )

        self.has_r8152_issue_history = False
