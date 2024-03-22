#!/usr/bin/env python3
# Copyright © 2020 - 2022 Collabora Ltd.
# Authors:
#   Tomeu Vizoso <tomeu.vizoso@collabora.com>
#   David Heidelberg <david.heidelberg@collabora.com>
#
# SPDX-License-Identifier: MIT
'''Shared functions between the scripts.'''

import os
import time
from typing import Optional


def pretty_duration(seconds):
    """Pretty print duration"""
    hours, rem = divmod(seconds, 3600)
    minutes, seconds = divmod(rem, 60)
    if hours:
        return f"{hours:0.0f}h{minutes:0.0f}m{seconds:0.0f}s"
    if minutes:
        return f"{minutes:0.0f}m{seconds:0.0f}s"
    return f"{seconds:0.0f}s"


def get_gitlab_project(glab, name: str):
    """Finds a specified gitlab project for given user"""
    if "/" in name:
        project_path = name
    else:
        glab.auth()
        username = glab.user.username
        project_path = f"{username}/{name}"
    return glab.projects.get(project_path)


def read_token(token_arg: Optional[str]) -> str:
    """pick token from args or file"""
    if token_arg:
        return token_arg
    return (
        open(os.path.expanduser("~/.config/gitlab-token"), encoding="utf-8")
        .readline()
        .rstrip()
    )


def wait_for_pipeline(projects, sha: str, timeout=None):
    """await until pipeline appears in Gitlab"""
    project_names = [project.path_with_namespace for project in projects]
    print(f"⏲ for the pipeline to appear in {project_names}..", end="")
    start_time = time.time()
    while True:
        for project in projects:
            pipelines = project.pipelines.list(sha=sha)
            if pipelines:
                print("", flush=True)
                return (pipelines[0], project)
        print("", end=".", flush=True)
        if timeout and time.time() - start_time > timeout:
            print(" not found", flush=True)
            return (None, None)
        time.sleep(1)
