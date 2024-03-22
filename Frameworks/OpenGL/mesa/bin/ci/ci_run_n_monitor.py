#!/usr/bin/env python3
# Copyright © 2020 - 2022 Collabora Ltd.
# Authors:
#   Tomeu Vizoso <tomeu.vizoso@collabora.com>
#   David Heidelberg <david.heidelberg@collabora.com>
#
# For the dependencies, see the requirements.txt
# SPDX-License-Identifier: MIT

"""
Helper script to restrict running only required CI jobs
and show the job(s) logs.
"""

import argparse
import re
import sys
import time
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor
from functools import partial
from itertools import chain
from subprocess import check_output
from typing import TYPE_CHECKING, Iterable, Literal, Optional

import gitlab
from colorama import Fore, Style
from gitlab_common import (
    get_gitlab_project,
    read_token,
    wait_for_pipeline,
    pretty_duration,
)
from gitlab_gql import GitlabGQL, create_job_needs_dag, filter_dag, print_dag

if TYPE_CHECKING:
    from gitlab_gql import Dag

GITLAB_URL = "https://gitlab.freedesktop.org"

REFRESH_WAIT_LOG = 10
REFRESH_WAIT_JOBS = 6

URL_START = "\033]8;;"
URL_END = "\033]8;;\a"

STATUS_COLORS = {
    "created": "",
    "running": Fore.BLUE,
    "success": Fore.GREEN,
    "failed": Fore.RED,
    "canceled": Fore.MAGENTA,
    "manual": "",
    "pending": "",
    "skipped": "",
}

COMPLETED_STATUSES = ["success", "failed"]


def print_job_status(job, new_status=False) -> None:
    """It prints a nice, colored job status with a link to the job."""
    if job.status == "canceled":
        return

    if job.duration:
        duration = job.duration
    elif job.started_at:
        duration = time.perf_counter() - time.mktime(job.started_at.timetuple())

    print(
        STATUS_COLORS[job.status]
        + "🞋 job "
        + URL_START
        + f"{job.web_url}\a{job.name}"
        + URL_END
        + (f" has new status: {job.status}" if new_status else f" :: {job.status}")
        + (f" ({pretty_duration(duration)})" if job.started_at else "")
        + Style.RESET_ALL
    )


def pretty_wait(sec: int) -> None:
    """shows progressbar in dots"""
    for val in range(sec, 0, -1):
        print(f"⏲  {val} seconds", end="\r")
        time.sleep(1)


def monitor_pipeline(
    project,
    pipeline,
    target_jobs_regex: re.Pattern,
    dependencies,
    force_manual: bool,
    stress: int,
) -> tuple[Optional[int], Optional[int]]:
    """Monitors pipeline and delegate canceling jobs"""
    statuses: dict[str, str] = defaultdict(str)
    target_statuses: dict[str, str] = defaultdict(str)
    stress_status_counter = defaultdict(lambda: defaultdict(int))
    target_id = None

    while True:
        deps_failed = []
        to_cancel = []
        for job in pipeline.jobs.list(all=True, sort="desc"):
            # target jobs
            if target_jobs_regex.fullmatch(job.name):
                target_id = job.id

                if stress and job.status in ["success", "failed"]:
                    if (
                        stress < 0
                        or sum(stress_status_counter[job.name].values()) < stress
                    ):
                        enable_job(project, job, "retry", force_manual)
                        stress_status_counter[job.name][job.status] += 1
                else:
                    enable_job(project, job, "target", force_manual)

                print_job_status(job, job.status not in target_statuses[job.name])
                target_statuses[job.name] = job.status
                continue

            # all jobs
            if job.status != statuses[job.name]:
                print_job_status(job, True)
                statuses[job.name] = job.status

            # run dependencies and cancel the rest
            if job.name in dependencies:
                enable_job(project, job, "dep", True)
                if job.status == "failed":
                    deps_failed.append(job.name)
            else:
                to_cancel.append(job)

        cancel_jobs(project, to_cancel)

        if stress:
            enough = True
            for job_name, status in stress_status_counter.items():
                print(
                    f"{job_name}\tsucc: {status['success']}; "
                    f"fail: {status['failed']}; "
                    f"total: {sum(status.values())} of {stress}",
                    flush=False,
                )
                if stress < 0 or sum(status.values()) < stress:
                    enough = False

            if not enough:
                pretty_wait(REFRESH_WAIT_JOBS)
                continue

        print("---------------------------------", flush=False)

        if len(target_statuses) == 1 and {"running"}.intersection(
            target_statuses.values()
        ):
            return target_id, None

        if (
            {"failed"}.intersection(target_statuses.values())
            and not set(["running", "pending"]).intersection(target_statuses.values())
        ):
            return None, 1

        if (
            {"skipped"}.intersection(target_statuses.values())
            and not {"running", "pending"}.intersection(target_statuses.values())
        ):
            print(
                Fore.RED,
                "Target in skipped state, aborting. Failed dependencies:",
                deps_failed,
                Fore.RESET,
            )
            return None, 1

        if {"success", "manual"}.issuperset(target_statuses.values()):
            return None, 0

        pretty_wait(REFRESH_WAIT_JOBS)


def enable_job(
    project, job, action_type: Literal["target", "dep", "retry"], force_manual: bool
) -> None:
    """enable job"""
    if (
        (job.status in ["success", "failed"] and action_type != "retry")
        or (job.status == "manual" and not force_manual)
        or job.status in ["skipped", "running", "created", "pending"]
    ):
        return

    pjob = project.jobs.get(job.id, lazy=True)

    if job.status in ["success", "failed", "canceled"]:
        pjob.retry()
    else:
        pjob.play()

    if action_type == "target":
        jtype = "🞋 "
    elif action_type == "retry":
        jtype = "↻"
    else:
        jtype = "(dependency)"

    print(Fore.MAGENTA + f"{jtype} job {job.name} manually enabled" + Style.RESET_ALL)


def cancel_job(project, job) -> None:
    """Cancel GitLab job"""
    if job.status in [
        "canceled",
        "success",
        "failed",
        "skipped",
    ]:
        return
    pjob = project.jobs.get(job.id, lazy=True)
    pjob.cancel()
    print(f"♲ {job.name}", end=" ")


def cancel_jobs(project, to_cancel) -> None:
    """Cancel unwanted GitLab jobs"""
    if not to_cancel:
        return

    with ThreadPoolExecutor(max_workers=6) as exe:
        part = partial(cancel_job, project)
        exe.map(part, to_cancel)
    print()


def print_log(project, job_id) -> None:
    """Print job log into output"""
    printed_lines = 0
    while True:
        job = project.jobs.get(job_id)

        # GitLab's REST API doesn't offer pagination for logs, so we have to refetch it all
        lines = job.trace().decode("raw_unicode_escape").splitlines()
        for line in lines[printed_lines:]:
            print(line)
        printed_lines = len(lines)

        if job.status in COMPLETED_STATUSES:
            print(Fore.GREEN + f"Job finished: {job.web_url}" + Style.RESET_ALL)
            return
        pretty_wait(REFRESH_WAIT_LOG)


def parse_args() -> None:
    """Parse args"""
    parser = argparse.ArgumentParser(
        description="Tool to trigger a subset of container jobs "
        + "and monitor the progress of a test job",
        epilog="Example: mesa-monitor.py --rev $(git rev-parse HEAD) "
        + '--target ".*traces" ',
    )
    parser.add_argument(
        "--target",
        metavar="target-job",
        help="Target job regex. For multiple targets, separate with pipe | character",
        required=True,
    )
    parser.add_argument(
        "--token",
        metavar="token",
        help="force GitLab token, otherwise it's read from ~/.config/gitlab-token",
    )
    parser.add_argument(
        "--force-manual", action="store_true", help="Force jobs marked as manual"
    )
    parser.add_argument(
        "--stress",
        default=0,
        type=int,
        help="Stresstest job(s). Number or repetitions or -1 for infinite.",
    )
    parser.add_argument(
        "--project",
        default="mesa",
        help="GitLab project in the format <user>/<project> or just <project>",
    )

    mutex_group1 = parser.add_mutually_exclusive_group()
    mutex_group1.add_argument(
        "--rev", default="HEAD", metavar="revision", help="repository git revision (default: HEAD)"
    )
    mutex_group1.add_argument(
        "--pipeline-url",
        help="URL of the pipeline to use, instead of auto-detecting it.",
    )
    mutex_group1.add_argument(
        "--mr",
        type=int,
        help="ID of a merge request; the latest pipeline in that MR will be used.",
    )

    args = parser.parse_args()

    # argparse doesn't support groups inside add_mutually_exclusive_group(),
    # which means we can't just put `--project` and `--rev` in a group together,
    # we have to do this by heand instead.
    if args.pipeline_url and args.project != parser.get_default("project"):
        # weird phrasing but it's the error add_mutually_exclusive_group() gives
        parser.error("argument --project: not allowed with argument --pipeline-url")

    return args


def print_detected_jobs(
    target_dep_dag: "Dag", dependency_jobs: Iterable[str], target_jobs: Iterable[str]
) -> None:
    def print_job_set(color: str, kind: str, job_set: Iterable[str]):
        print(
            color + f"Running {len(job_set)} {kind} jobs: ",
            "\n",
            ", ".join(sorted(job_set)),
            Fore.RESET,
            "\n",
        )

    print(Fore.YELLOW + "Detected target job and its dependencies:", "\n")
    print_dag(target_dep_dag)
    print_job_set(Fore.MAGENTA, "dependency", dependency_jobs)
    print_job_set(Fore.BLUE, "target", target_jobs)


def find_dependencies(target_jobs_regex: re.Pattern, project_path: str, iid: int) -> set[str]:
    gql_instance = GitlabGQL()
    dag = create_job_needs_dag(
        gql_instance, {"projectPath": project_path.path_with_namespace, "iid": iid}
    )

    target_dep_dag = filter_dag(dag, target_jobs_regex)
    if not target_dep_dag:
        print(Fore.RED + "The job(s) were not found in the pipeline." + Fore.RESET)
        sys.exit(1)

    dependency_jobs = set(chain.from_iterable(d["needs"] for d in target_dep_dag.values()))
    target_jobs = set(target_dep_dag.keys())
    print_detected_jobs(target_dep_dag, dependency_jobs, target_jobs)
    return target_jobs.union(dependency_jobs)


if __name__ == "__main__":
    try:
        t_start = time.perf_counter()

        args = parse_args()

        token = read_token(args.token)

        gl = gitlab.Gitlab(url=GITLAB_URL,
                           private_token=token,
                           retry_transient_errors=True)

        REV: str = args.rev

        if args.pipeline_url:
            assert args.pipeline_url.startswith(GITLAB_URL)
            url_path = args.pipeline_url[len(GITLAB_URL):]
            url_path_components = url_path.split("/")
            project_name = "/".join(url_path_components[1:3])
            assert url_path_components[3] == "-"
            assert url_path_components[4] == "pipelines"
            pipeline_id = int(url_path_components[5])
            cur_project = gl.projects.get(project_name)
            pipe = cur_project.pipelines.get(pipeline_id)
            REV = pipe.sha
        else:
            mesa_project = gl.projects.get("mesa/mesa")
            projects = [mesa_project]
            if args.mr:
                REV = mesa_project.mergerequests.get(args.mr).sha
            else:
                REV = check_output(['git', 'rev-parse', REV]).decode('ascii').strip()
                projects.append(get_gitlab_project(gl, args.project))
            (pipe, cur_project) = wait_for_pipeline(projects, REV)

        print(f"Revision: {REV}")
        print(f"Pipeline: {pipe.web_url}")

        target_jobs_regex = re.compile(args.target.strip())

        deps = set()
        if args.target:
            print("🞋 job: " + Fore.BLUE + args.target + Style.RESET_ALL)
            deps = find_dependencies(
                target_jobs_regex=target_jobs_regex, iid=pipe.iid, project_path=cur_project
            )
        target_job_id, ret = monitor_pipeline(
            cur_project, pipe, target_jobs_regex, deps, args.force_manual, args.stress
        )

        if target_job_id:
            print_log(cur_project, target_job_id)

        t_end = time.perf_counter()
        spend_minutes = (t_end - t_start) / 60
        print(f"⏲ Duration of script execution: {spend_minutes:0.1f} minutes")

        sys.exit(ret)
    except KeyboardInterrupt:
        sys.exit(1)
