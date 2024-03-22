"""
In a few words: some devices in Mesa CI has problematic serial connection, they
may hang (become silent) intermittently. Every time it hangs for minutes, the
job is retried, causing delays in the overall pipeline executing, ultimately
blocking legit MRs to merge.

To reduce reliance on UART, we explored LAVA features, such as running docker
containers as a test alongside the DUT one, to be able to create an SSH server
in the DUT the earliest possible and an SSH client in a docker container, to
establish a SSH session between both, allowing the console output to be passed
via SSH pseudo terminal, instead of relying in the error-prone UART.

In more detail, we aim to use "export -p" to share the initial boot environment
with SSH LAVA test-cases.
The "init-stage1.sh" script handles tasks such as system mounting and network
setup, which are necessary for allocating a pseudo-terminal under "/dev/pts".
Although these chores are not required for establishing an SSH session, they are
essential for proper functionality to the target script given by HWCI_SCRIPT
environment variable.

Therefore, we have divided the job definition into four parts:

1. [DUT] Logging in to DUT and run the SSH server with root access.
2. [DUT] Running the "init-stage1.sh" script for the first SSH test case.
3. [DUT] Export the first boot environment to `/dut-env-vars.sh` file.
4. [SSH] Enabling the pseudo-terminal for colors and running the "init-stage2.sh"
script after sourcing "dut-env-vars.sh" again for the second SSH test case.
"""


import re
from typing import TYPE_CHECKING, Any, Iterable

from ruamel.yaml.scalarstring import LiteralScalarString

from .constants import NUMBER_OF_ATTEMPTS_LAVA_BOOT

if TYPE_CHECKING:
    from ..lava_job_submitter import LAVAJobSubmitter

# Very early SSH server setup. Uses /dut_ready file to flag it is done.
SSH_SERVER_COMMANDS = {
    "auto_login": {
        "login_commands": [
            "dropbear -R -B",
            "touch /dut_ready",
        ],
        "login_prompt": "ogin:",
        # To login as root, the username should be empty
        "username": "",
    }
}

# TODO: Extract this inline script to a shell file, like we do with
# init-stage[12].sh
# The current way is difficult to maintain because one has to deal with escaping
# characters for both Python and the resulting job definition YAML.
# Plus, it always good to lint bash scripts with shellcheck.
DOCKER_COMMANDS = [
    """set -ex
timeout 1m bash << EOF
while [ -z "$(lava-target-ip)" ]; do
    echo Waiting for DUT to join LAN;
    sleep 1;
done
EOF

ping -c 5 -w 60 $(lava-target-ip)

lava_ssh_test_case() {
    set -x
    local test_case="${1}"
    shift
    lava-test-case \"${test_case}\" --shell \\
        ssh ${SSH_PTY_ARGS:--T} \\
        -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null \\
        root@$(lava-target-ip) \"${@}\"
}""",
]


def to_yaml_block(steps_array: Iterable[str], escape_vars=[]) -> LiteralScalarString:
    def escape_envvar(match):
        return "\\" + match.group(0)

    filtered_array = [s for s in steps_array if s.strip() and not s.startswith("#")]
    final_str = "\n".join(filtered_array)

    for escape_var in escape_vars:
        # Find env vars and add '\\' before them
        final_str = re.sub(rf"\${escape_var}*", escape_envvar, final_str)
    return LiteralScalarString(final_str)


def generate_dut_test(args: "LAVAJobSubmitter", first_stage_steps: list[str]) -> dict[str, Any]:
    # Commands executed on DUT.
    # Trying to execute the minimal number of commands, because the console data is
    # retrieved via UART, which is hang-prone in some devices.
    return {
        "namespace": "dut",
        "definitions": [
            {
                "from": "inline",
                "name": "setup-ssh-server",
                "path": "inline-setup-ssh-server",
                "repository": {
                    "metadata": {
                        "format": "Lava-Test Test Definition 1.0",
                        "name": "dut-env-export",
                    },
                    "run": {
                        "steps": [
                            to_yaml_block(first_stage_steps),
                            "export -p > /dut-env-vars.sh",  # Exporting the first boot environment
                        ],
                    },
                },
            }
        ],
    }


def generate_docker_test(
    args: "LAVAJobSubmitter", artifact_download_steps: list[str]
) -> dict[str, Any]:
    # This is a growing list of commands that will be executed by the docker
    # guest, which will be the SSH client.
    docker_commands = []

    # LAVA test wrapping Mesa CI job in a SSH session.
    init_stages_test = {
        "namespace": "container",
        "timeout": {"minutes": args.job_timeout_min},
        "failure_retry": 3,
        "definitions": [
            {
                "name": "docker_ssh_client",
                "from": "inline",
                "path": "inline/docker_ssh_client.yaml",
                "repository": {
                    "metadata": {
                        "name": "mesa",
                        "description": "Mesa test plan",
                        "format": "Lava-Test Test Definition 1.0",
                    },
                    "run": {"steps": docker_commands},
                },
            }
        ],
        "docker": {
            "image": args.ssh_client_image,
        },
    }

    docker_commands += [
        to_yaml_block(DOCKER_COMMANDS, escape_vars=["LAVA_TARGET_IP"]),
        "lava_ssh_test_case 'wait_for_dut_login' << EOF",
        "while [ ! -e /dut_ready ]; do sleep 1; done;",
        "EOF",
        to_yaml_block(
            (
                "lava_ssh_test_case 'artifact_download' 'bash --' << EOF",
                "source /dut-env-vars.sh",
                *artifact_download_steps,
                "EOF",
            )
        ),
        "export SSH_PTY_ARGS=-tt",
        # Putting CI_JOB name as the testcase name, it may help LAVA farm
        # maintainers with monitoring
        f"lava_ssh_test_case '{args.project_name}_{args.mesa_job_name}' "
        # Changing directory to /, as the HWCI_SCRIPT expects that
        "'\"cd / && /init-stage2.sh\"'",
    ]

    return init_stages_test


def wrap_final_deploy_action(final_deploy_action: dict):
    wrap = {
        "namespace": "dut",
        "failure_retry": NUMBER_OF_ATTEMPTS_LAVA_BOOT,
        "timeout": {"minutes": 10},
    }

    final_deploy_action.update(wrap)


def wrap_boot_action(boot_action: dict):
    wrap = {
        "namespace": "dut",
        "failure_retry": NUMBER_OF_ATTEMPTS_LAVA_BOOT,
        **SSH_SERVER_COMMANDS,
    }

    boot_action.update(wrap)
