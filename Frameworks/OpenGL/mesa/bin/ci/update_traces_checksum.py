#!/usr/bin/env python3
# Copyright © 2022 Collabora Ltd.
# Authors:
#   David Heidelberg <david.heidelberg@collabora.com>
#
# For the dependencies, see the requirements.txt
# SPDX-License-Identifier: MIT

"""
Helper script to update traces checksums
"""

import argparse
import bz2
import glob
import re
import json
import sys
from ruamel.yaml import YAML

import gitlab
from colorama import Fore, Style
from gitlab_common import get_gitlab_project, read_token, wait_for_pipeline


DESCRIPTION_FILE = "export PIGLIT_REPLAY_DESCRIPTION_FILE='.*/install/(.*)'$"
DEVICE_NAME = "export PIGLIT_REPLAY_DEVICE_NAME='(.*)'$"


def gather_results(
    project,
    pipeline,
) -> None:
    """Gather results"""

    target_jobs_regex = re.compile(".*-traces([:].*)?$")

    for job in pipeline.jobs.list(all=True, sort="desc"):
        if target_jobs_regex.match(job.name) and job.status == "failed":
            cur_job = project.jobs.get(job.id)
            # get variables
            print(f"👁  {job.name}...")
            log: list[str] = cur_job.trace().decode("unicode_escape").splitlines()
            filename: str = ''
            dev_name: str = ''
            for logline in log:
                desc_file = re.search(DESCRIPTION_FILE, logline)
                device_name = re.search(DEVICE_NAME, logline)
                if desc_file:
                    filename = desc_file.group(1)
                if device_name:
                    dev_name = device_name.group(1)

            if not filename or not dev_name:
                print(Fore.RED + "Couldn't find device name or YML file in the logs!" + Style.RESET_ALL)
                return

            print(f"👁 Found {dev_name} and file {filename}")

            # find filename in Mesa source
            traces_file = glob.glob('./**/' + filename, recursive=True)
            # write into it
            with open(traces_file[0], 'r', encoding='utf-8') as target_file:
                yaml = YAML()
                yaml.compact(seq_seq=False, seq_map=False)
                yaml.version = 1,2
                yaml.width = 2048  # do not break the text fields
                yaml.default_flow_style = None
                target = yaml.load(target_file)

                # parse artifact
                results_json_bz2 = cur_job.artifact(path="results/results.json.bz2", streamed=False)
                results_json = bz2.decompress(results_json_bz2).decode("utf-8", errors="replace")
                results = json.loads(results_json)

                for _, value in results["tests"].items():
                    if (
                        not value['images'] or
                        not value['images'][0] or
                        "image_desc" not in value['images'][0]
                    ):
                        continue

                    trace: str = value['images'][0]['image_desc']
                    checksum: str = value['images'][0]['checksum_render']

                    if not checksum:
                        print(Fore.RED + f"{dev_name}: {trace}: checksum is missing! Crash?" + Style.RESET_ALL)
                        continue

                    if checksum == "error":
                        print(Fore.RED + f"{dev_name}: {trace}: crashed" + Style.RESET_ALL)
                        continue

                    if target['traces'][trace][dev_name].get('checksum') == checksum:
                        continue

                    if "label" in target['traces'][trace][dev_name]:
                        print(f'{dev_name}: {trace}: please verify that label {Fore.BLUE}{target["traces"][trace][dev_name]["label"]}{Style.RESET_ALL} is still valid')

                    print(Fore.GREEN + f'{dev_name}: {trace}: checksum updated' + Style.RESET_ALL)
                    target['traces'][trace][dev_name]['checksum'] = checksum

            with open(traces_file[0], 'w', encoding='utf-8') as target_file:
                yaml.dump(target, target_file)



def parse_args() -> None:
    """Parse args"""
    parser = argparse.ArgumentParser(
        description="Tool to generate patch from checksums ",
        epilog="Example: update_traces_checksum.py --rev $(git rev-parse HEAD) "
    )
    parser.add_argument(
        "--rev", metavar="revision", help="repository git revision", required=True
    )
    parser.add_argument(
        "--token",
        metavar="token",
        help="force GitLab token, otherwise it's read from ~/.config/gitlab-token",
    )
    return parser.parse_args()


if __name__ == "__main__":
    try:
        args = parse_args()

        token = read_token(args.token)

        gl = gitlab.Gitlab(url="https://gitlab.freedesktop.org", private_token=token)

        cur_project = get_gitlab_project(gl, "mesa")

        print(f"Revision: {args.rev}")
        (pipe, cur_project) = wait_for_pipeline([cur_project], args.rev)
        print(f"Pipeline: {pipe.web_url}")
        gather_results(cur_project, pipe)

        sys.exit()
    except KeyboardInterrupt:
        sys.exit(1)
