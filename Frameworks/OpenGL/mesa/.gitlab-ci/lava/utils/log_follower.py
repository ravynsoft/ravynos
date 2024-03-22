#!/usr/bin/env python3
#
# Copyright (C) 2022 Collabora Limited
# Author: Guilherme Gallo <guilherme.gallo@collabora.com>
#
# SPDX-License-Identifier: MIT

"""
Some utilities to analyse logs, create gitlab sections and other quality of life
improvements
"""

import logging
import re
import sys
from dataclasses import dataclass, field
from datetime import datetime, timedelta
from typing import Optional, Union

from lava.exceptions import MesaCITimeoutError
from lava.utils.console_format import CONSOLE_LOG
from lava.utils.gitlab_section import GitlabSection
from lava.utils.lava_farm import LavaFarm, get_lava_farm
from lava.utils.lava_log_hints import LAVALogHints
from lava.utils.log_section import (
    DEFAULT_GITLAB_SECTION_TIMEOUTS,
    FALLBACK_GITLAB_SECTION_TIMEOUT,
    LOG_SECTIONS,
    LogSectionType,
)


@dataclass
class LogFollower:
    starting_section: Optional[GitlabSection] = None
    _current_section: Optional[GitlabSection] = None
    section_history: list[GitlabSection] = field(default_factory=list, init=False)
    timeout_durations: dict[LogSectionType, timedelta] = field(
        default_factory=lambda: DEFAULT_GITLAB_SECTION_TIMEOUTS,
    )
    fallback_timeout: timedelta = FALLBACK_GITLAB_SECTION_TIMEOUT
    _buffer: list[str] = field(default_factory=list, init=False)
    log_hints: LAVALogHints = field(init=False)
    lava_farm: LavaFarm = field(init=False, default=get_lava_farm())
    _merge_next_line: str = field(default_factory=str, init=False)

    def __post_init__(self):
        # Make it trigger current_section setter to populate section history
        self.current_section = self.starting_section
        section_is_created = bool(self._current_section)
        section_has_started = bool(
            self._current_section and self._current_section.has_started
        )
        self.log_hints = LAVALogHints(self)
        assert (
            section_is_created == section_has_started
        ), "Can't follow logs beginning from uninitialized GitLab sections."

        # Initialize fix_lava_gitlab_section_log generator
        self.gl_section_fix_gen = fix_lava_gitlab_section_log()
        next(self.gl_section_fix_gen)

    @property
    def current_section(self):
        return self._current_section

    @current_section.setter
    def current_section(self, new_section: GitlabSection) -> None:
        if old_section := self._current_section:
            self.section_history.append(old_section)
        self._current_section = new_section

    @property
    def phase(self) -> LogSectionType:
        return (
            self._current_section.type
            if self._current_section
            else LogSectionType.UNKNOWN
        )

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Cleanup existing buffer if this object gets out from the context"""
        self.clear_current_section()
        last_lines = self.flush()
        for line in last_lines:
            print(line)

    def watchdog(self):
        if not self._current_section:
            return

        timeout_duration = self.timeout_durations.get(
            self._current_section.type, self.fallback_timeout
        )

        if self._current_section.delta_time() > timeout_duration:
            raise MesaCITimeoutError(
                f"Gitlab Section {self._current_section} has timed out",
                timeout_duration=timeout_duration,
            )

    def clear_current_section(self):
        if self._current_section and not self._current_section.has_finished:
            self._buffer.append(self._current_section.end())
            self.current_section = None

    def update_section(self, new_section: GitlabSection):
        # Sections can have redundant regex to find them to mitigate LAVA
        # interleaving kmsg and stderr/stdout issue.
        if self.current_section and self.current_section.id == new_section.id:
            return
        self.clear_current_section()
        self.current_section = new_section
        self._buffer.append(new_section.start())

    def manage_gl_sections(self, line):
        if isinstance(line["msg"], list):
            logging.debug("Ignoring messages as list. Kernel dumps.")
            return

        for log_section in LOG_SECTIONS:
            if new_section := log_section.from_log_line_to_section(line):
                self.update_section(new_section)
                break

    def detect_kernel_dump_line(self, line: dict[str, Union[str, list]]) -> bool:
        # line["msg"] can be a list[str] when there is a kernel dump
        if isinstance(line["msg"], list):
            return line["lvl"] == "debug"

        # result level has dict line["msg"]
        if not isinstance(line["msg"], str):
            return False

        # we have a line, check if it is a kernel message
        if re.search(r"\[[\d\s]{5}\.[\d\s]{6}\] +\S{2,}", line["msg"]):
            print_log(f"{CONSOLE_LOG['BOLD']}{line['msg']}{CONSOLE_LOG['RESET']}")
            return True

        return False

    def remove_trailing_whitespace(self, line: dict[str, str]) -> None:
        """
        Removes trailing whitespace from the end of the `msg` value in the log line dictionary.

        Args:
            line: A dictionary representing a single log line.

        Note:
            LAVA treats carriage return characters as a line break, so each carriage return in an output console
            is mapped to a console line in LAVA. This method removes trailing `\r\n` characters from log lines.
        """
        msg: Optional[str] = line.get("msg")
        if not msg:
            return False

        messages = [msg] if isinstance(msg, str) else msg

        for message in messages:
            # LAVA logs brings raw messages, which includes newlines characters as \r\n.
            line["msg"]: str = re.sub(r"\r\n$", "", message)

    def merge_carriage_return_lines(self, line: dict[str, str]) -> bool:
        """
        Merges lines that end with a carriage return character into a single line.

        Args:
            line: A dictionary representing a single log line.

        Returns:
            A boolean indicating whether the current line has been merged with the next line.

        Note:
            LAVA treats carriage return characters as a line break, so each carriage return in an output console
            is mapped to a console line in LAVA.
        """
        if line["msg"].endswith("\r"):
            self._merge_next_line += line["msg"]
            return True

        if self._merge_next_line:
            line["msg"] = self._merge_next_line + line["msg"]
            self._merge_next_line = ""

        return False


    def feed(self, new_lines: list[dict[str, str]]) -> bool:
        """Input data to be processed by LogFollower instance
        Returns true if the DUT (device under test) seems to be alive.
        """

        self.watchdog()

        # No signal of job health in the log
        is_job_healthy = False

        for line in new_lines:
            self.remove_trailing_whitespace(line)

            if self.detect_kernel_dump_line(line):
                continue

            if self.merge_carriage_return_lines(line):
                continue

            # At least we are fed with a non-kernel dump log, it seems that the
            # job is progressing
            is_job_healthy = True
            self.manage_gl_sections(line)
            if parsed_line := self.parse_lava_line(line):
                self._buffer.append(parsed_line)

        self.log_hints.detect_failure(new_lines)

        return is_job_healthy

    def flush(self) -> list[str]:
        buffer = self._buffer
        self._buffer = []
        return buffer

    def parse_lava_line(self, line) -> Optional[str]:
        prefix = ""
        suffix = ""

        if line["lvl"] in ["results", "feedback", "debug"]:
            return
        elif line["lvl"] in ["warning", "error"]:
            prefix = CONSOLE_LOG["FG_RED"]
            suffix = CONSOLE_LOG["RESET"]
        elif line["lvl"] == "input":
            prefix = "$ "
            suffix = ""
        elif line["lvl"] == "target" and self.lava_farm != LavaFarm.COLLABORA:
            # gl_section_fix_gen will output the stored line if it can't find a
            # match for the first split line
            # So we can recover it and put it back to the buffer
            if recovered_first_line := self.gl_section_fix_gen.send(line):
                self._buffer.append(recovered_first_line)

        return f'{prefix}{line["msg"]}{suffix}'

def fix_lava_gitlab_section_log():
    """This function is a temporary solution for the Gitlab section markers
    splitting problem. Gitlab parses the following lines to define a collapsible
    gitlab section in their log:
    - \x1b[0Ksection_start:timestamp:section_id[collapsible=true/false]\r\x1b[0Ksection_header
    - \x1b[0Ksection_end:timestamp:section_id\r\x1b[0K
    There is some problem in message passing between the LAVA dispatcher and the
    device under test (DUT), that replaces \r control characters into \n. When
    this problem is fixed on the LAVA side, one should remove this function.
    """
    while True:
        line = yield False
        first_line = None
        split_line_pattern = re.compile(r"\x1b\[0K(section_\w+):(\d+):([^\s\r]+)$")
        second_line_pattern = re.compile(r"\x1b\[0K([\S ]+)?")

        if not re.search(split_line_pattern, line["msg"]):
            continue

        first_line = line["msg"]
        # Delete the current line and hold this log line stream to be able to
        # possibly merge it with the next line.
        line["msg"] = ""
        line = yield False

        # This code reached when we detect a possible first split line
        if re.search(second_line_pattern, line["msg"]):
            assert first_line
            line["msg"] = f"{first_line}\r{line['msg']}"
        else:
            # The current line doesn't match with the previous one, send back the
            # latter to give the user the chance to recover it.
            yield first_line



def print_log(msg: str, *args) -> None:
    # Reset color from timestamp, since `msg` can tint the terminal color
    print(f"{CONSOLE_LOG['RESET']}{datetime.now()}: {msg}", *args)


def fatal_err(msg, exception=None):
    colored_msg = f"{CONSOLE_LOG['FG_RED']}"
    print_log(colored_msg, f"{msg}", f"{CONSOLE_LOG['RESET']}")
    if exception:
        raise exception
    sys.exit(1)


def hide_sensitive_data(yaml_data: str, start_hide: str = "HIDE_START", end_hide: str = "HIDE_END") -> str:
    skip_line = False
    dump_data: list[str] = []
    for line in yaml_data.splitlines(True):
        if start_hide in line:
            skip_line = True
        elif end_hide in line:
            skip_line = False

        if skip_line:
            continue

        dump_data.append(line)

    return "".join(dump_data)
