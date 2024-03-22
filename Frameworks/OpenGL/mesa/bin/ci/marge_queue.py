#!/usr/bin/env python3
# Copyright © 2020 - 2023 Collabora Ltd.
# Authors:
#   David Heidelberg <david.heidelberg@collabora.com>
#
# SPDX-License-Identifier: MIT

"""
Monitors Marge-bot and return number of assigned MRs.
"""

import argparse
import time
import sys
from datetime import datetime, timezone
from dateutil import parser

import gitlab
from gitlab_common import read_token, pretty_duration

REFRESH_WAIT = 30
MARGE_BOT_USER_ID = 9716


def parse_args() -> None:
    """Parse args"""
    parse = argparse.ArgumentParser(
        description="Tool to show merge requests assigned to the marge-bot",
    )
    parse.add_argument(
        "--wait", action="store_true", help="wait until CI is free",
    )
    parse.add_argument(
        "--token",
        metavar="token",
        help="force GitLab token, otherwise it's read from ~/.config/gitlab-token",
    )
    return parse.parse_args()


if __name__ == "__main__":
    args = parse_args()
    token = read_token(args.token)
    gl = gitlab.Gitlab(url="https://gitlab.freedesktop.org", private_token=token)

    project = gl.projects.get("mesa/mesa")

    while True:
        mrs = project.mergerequests.list(assignee_id=MARGE_BOT_USER_ID, scope="all", state="opened", get_all=True)

        jobs_num = len(mrs)
        for mr in mrs:
            updated = parser.parse(mr.updated_at)
            now = datetime.now(timezone.utc)
            diff = (now - updated).total_seconds()
            print(
                f"⛭ \u001b]8;;{mr.web_url}\u001b\\{mr.title}\u001b]8;;\u001b\\ ({pretty_duration(diff)})"
            )

        print("Job waiting: " + str(jobs_num))

        if jobs_num == 0:
            sys.exit(0)
        if not args.wait:
            sys.exit(min(jobs_num, 127))

        time.sleep(REFRESH_WAIT)
