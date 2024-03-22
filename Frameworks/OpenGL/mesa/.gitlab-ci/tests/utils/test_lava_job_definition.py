import importlib
import os
import re
from itertools import chain
from pathlib import Path
from typing import Any, Iterable, Literal
from unittest import mock

import lava.utils.constants
import pytest
from lava.lava_job_submitter import LAVAJobSubmitter
from lava.utils.lava_job_definition import LAVAJobDefinition
from ruamel.yaml import YAML


def flatten(iterable: Iterable[Iterable[Any]]) -> list[Any]:
    return list(chain.from_iterable(iterable))


# mock shell file
@pytest.fixture(scope="session")
def shell_file(tmp_path_factory):
    def create_shell_file(content: str = "# test"):
        shell_file = tmp_path_factory.mktemp("data") / "shell_file.sh"
        shell_file.write_text(content)
        return shell_file

    return create_shell_file


# fn to load the data file from $CWD/data using pathlib
def load_data_file(filename):
    return Path(__file__).parent.parent / "data" / filename


def load_yaml_file(filename) -> dict:
    with open(load_data_file(filename)) as f:
        return YAML().load(f)


def job_submitter_factory(mode: Literal["UBOOT", "FASTBOOT"], shell_file):
    if mode == "UBOOT":
        boot_method = "u-boot"
        device_type = "my_uboot_device_type"
    elif mode == "FASTBOOT":
        boot_method = "fastboot"
        device_type = "my_fastboot_device_type"

    job_timeout_min = 10
    mesa_job_name = "dut test"
    pipeline_info = "my_pipeline_info"
    project_name = "test-project"
    visibility_group = "my_visibility_group"

    return LAVAJobSubmitter(
        boot_method=boot_method,
        ci_project_dir="/ci/project/dir",
        device_type=device_type,
        dtb_filename="my_dtb_filename",
        first_stage_init=shell_file,
        job_timeout_min=job_timeout_min,
        mesa_job_name=mesa_job_name,
        pipeline_info=pipeline_info,
        visibility_group=visibility_group,
        project_name=project_name,
    )


@pytest.fixture
def clear_env_vars(autouse=True):
    with mock.patch.dict(os.environ) as environ:
        # Remove all LAVA-related environment variables to make the test more robust
        # and deterministic, once a envvar is capable of overriding the default value
        for key in environ:
            if any(kw in key for kw in ("LAVA_", "CI_", "JOB_", "RUNNER_", "DEVICE_")):
                del environ[key]
        # reload lava.utils.constants to update the JOB_PRIORITY value
        importlib.reload(lava.utils.constants)
        importlib.reload(lava.utils.lava_job_definition)
        yield


@pytest.fixture
def mock_collabora_farm(clear_env_vars, monkeypatch):
    # Mock a Collabora farm-like device runner tag to enable SSH execution
    monkeypatch.setenv("RUNNER_TAG", "mesa-ci-1234-lava-collabora")


@pytest.mark.parametrize("force_uart", [True, False], ids=["SSH", "UART"])
@pytest.mark.parametrize("mode", ["UBOOT", "FASTBOOT"])
def test_generate_lava_job_definition_sanity(
    force_uart, mode, shell_file, mock_collabora_farm, monkeypatch
):
    monkeypatch.setattr(lava.utils.lava_job_definition, "FORCE_UART", force_uart)

    init_script_content = f"echo test {mode}"
    job_submitter = job_submitter_factory(mode, shell_file(init_script_content))
    job_definition = LAVAJobDefinition(job_submitter).generate_lava_job_definition()

    # Load the YAML output and check that it contains the expected keys and values
    yaml = YAML()
    job_dict = yaml.load(job_definition)
    yaml.dump(job_dict, Path(f"/tmp/{mode}_force_uart={force_uart}_job_definition.yaml"))
    assert job_dict["device_type"] == job_submitter.device_type
    assert job_dict["visibility"]["group"] == [job_submitter.visibility_group]
    assert job_dict["timeouts"]["job"]["minutes"] == job_submitter.job_timeout_min
    assert job_dict["context"]["extra_nfsroot_args"]
    assert job_dict["timeouts"]["actions"]

    assert len(job_dict["actions"]) == 3 if mode == "UART" else 5

    last_test_action = job_dict["actions"][-1]["test"]
    # TODO: Remove hardcoded "mesa" test name, as this submitter is being used by other projects
    first_test_name = last_test_action["definitions"][0]["name"]
    is_running_ssh = "ssh" in first_test_name
    # if force_uart, is_ssh must be False. If is_ssh, force_uart must be False. Both can be False
    assert not (is_running_ssh and force_uart)
    assert last_test_action["failure_retry"] == 3 if is_running_ssh else 1

    run_steps = "".join(last_test_action["definitions"][0]["repository"]["run"]["steps"])
    # Check for project name in lava-test-case
    assert re.search(rf"lava.?\S*.test.case.*{job_submitter.project_name}", run_steps)

    action_names = flatten(j.keys() for j in job_dict["actions"])
    if is_running_ssh:
        assert action_names == (
            [
                "deploy",
                "boot",
                "test",  # DUT: SSH server
                "test",  # Docker: SSH client
            ]
            if mode == "UBOOT"
            else [
                "deploy",  # NFS
                "deploy",  # Image generation
                "deploy",  # Image deployment
                "boot",
                "test",  # DUT: SSH server
                "test",  # Docker: SSH client
            ]
        )
        test_action_server = job_dict["actions"][-2]["test"]
        # SSH server in the DUT
        assert test_action_server["namespace"] == "dut"
        # SSH client via docker
        assert last_test_action["namespace"] == "container"

        boot_action = next(a["boot"] for a in job_dict["actions"] if "boot" in a)
        assert boot_action["namespace"] == "dut"

        # SSH server bootstrapping
        assert "dropbear" in "".join(boot_action["auto_login"]["login_commands"])
        return

    # ---- Not SSH job
    assert action_names == (
        [
            "deploy",
            "boot",
            "test",
        ]
        if mode == "UBOOT"
        else [
            "deploy",  # NFS
            "deploy",  # Image generation
            "deploy",  # Image deployment
            "boot",
            "test",
        ]
    )
    assert init_script_content in run_steps


# use yaml files from tests/data/ to test the job definition generation
@pytest.mark.parametrize("force_uart", [False, True], ids=["SSH", "UART"])
@pytest.mark.parametrize("mode", ["UBOOT", "FASTBOOT"])
def test_lava_job_definition(mode, force_uart, shell_file, mock_collabora_farm, monkeypatch):
    monkeypatch.setattr(lava.utils.lava_job_definition, "FORCE_UART", force_uart)

    yaml = YAML()
    yaml.default_flow_style = False

    # Load the YAML output and check that it contains the expected keys and values
    expected_job_dict = load_yaml_file(f"{mode}_force_uart={force_uart}_job_definition.yaml")

    init_script_content = f"echo test {mode}"
    job_submitter = job_submitter_factory(mode, shell_file(init_script_content))
    job_definition = LAVAJobDefinition(job_submitter).generate_lava_job_definition()

    job_dict = yaml.load(job_definition)

    # Uncomment the following to update the expected YAML files
    # yaml.dump(job_dict, Path(f"../../data/{mode}_force_uart={force_uart}_job_definition.yaml"))

    # Check that the generated job definition matches the expected one
    assert job_dict == expected_job_dict
