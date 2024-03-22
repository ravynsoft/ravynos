#!/usr/bin/env python3
# For the dependencies, see the requirements.txt

import logging
import re
import traceback
from argparse import ArgumentDefaultsHelpFormatter, ArgumentParser, Namespace
from collections import OrderedDict
from copy import deepcopy
from dataclasses import dataclass, field
from itertools import accumulate
from os import getenv
from pathlib import Path
from subprocess import check_output
from textwrap import dedent
from typing import Any, Iterable, Optional, Pattern, TypedDict, Union

import yaml
from filecache import DAY, filecache
from gql import Client, gql
from gql.transport.requests import RequestsHTTPTransport
from graphql import DocumentNode


class DagNode(TypedDict):
    needs: set[str]
    stage: str
    # `name` is redundant but is here for retro-compatibility
    name: str


# see create_job_needs_dag function for more details
Dag = dict[str, DagNode]


StageSeq = OrderedDict[str, set[str]]
TOKEN_DIR = Path(getenv("XDG_CONFIG_HOME") or Path.home() / ".config")


def get_token_from_default_dir() -> str:
    token_file = TOKEN_DIR / "gitlab-token"
    try:
        return str(token_file.resolve())
    except FileNotFoundError as ex:
        print(
            f"Could not find {token_file}, please provide a token file as an argument"
        )
        raise ex


def get_project_root_dir():
    root_path = Path(__file__).parent.parent.parent.resolve()
    gitlab_file = root_path / ".gitlab-ci.yml"
    assert gitlab_file.exists()

    return root_path


@dataclass
class GitlabGQL:
    _transport: Any = field(init=False)
    client: Client = field(init=False)
    url: str = "https://gitlab.freedesktop.org/api/graphql"
    token: Optional[str] = None

    def __post_init__(self) -> None:
        self._setup_gitlab_gql_client()

    def _setup_gitlab_gql_client(self) -> None:
        # Select your transport with a defined url endpoint
        headers = {}
        if self.token:
            headers["Authorization"] = f"Bearer {self.token}"
        self._transport = RequestsHTTPTransport(url=self.url, headers=headers)

        # Create a GraphQL client using the defined transport
        self.client = Client(transport=self._transport, fetch_schema_from_transport=True)

    def query(
        self,
        gql_file: Union[Path, str],
        params: dict[str, Any] = {},
        operation_name: Optional[str] = None,
        paginated_key_loc: Iterable[str] = [],
        disable_cache: bool = False,
    ) -> dict[str, Any]:
        def run_uncached() -> dict[str, Any]:
            if paginated_key_loc:
                return self._sweep_pages(gql_file, params, operation_name, paginated_key_loc)
            return self._query(gql_file, params, operation_name)

        if disable_cache:
            return run_uncached()

        try:
            # Create an auxiliary variable to deliver a cached result and enable catching exceptions
            # Decorate the query to be cached
            if paginated_key_loc:
                result = self._sweep_pages_cached(
                    gql_file, params, operation_name, paginated_key_loc
                )
            else:
                result = self._query_cached(gql_file, params, operation_name)
            return result  # type: ignore
        except Exception as ex:
            logging.error(f"Cached query failed with {ex}")
            # print exception traceback
            traceback_str = "".join(traceback.format_exception(ex))
            logging.error(traceback_str)
            self.invalidate_query_cache()
            logging.error("Cache invalidated, retrying without cache")
        finally:
            return run_uncached()

    def _query(
        self,
        gql_file: Union[Path, str],
        params: dict[str, Any] = {},
        operation_name: Optional[str] = None,
    ) -> dict[str, Any]:
        # Provide a GraphQL query
        source_path: Path = Path(__file__).parent
        pipeline_query_file: Path = source_path / gql_file

        query: DocumentNode
        with open(pipeline_query_file, "r") as f:
            pipeline_query = f.read()
            query = gql(pipeline_query)

        # Execute the query on the transport
        return self.client.execute_sync(
            query, variable_values=params, operation_name=operation_name
        )

    @filecache(DAY)
    def _sweep_pages_cached(self, *args, **kwargs):
        return self._sweep_pages(*args, **kwargs)

    @filecache(DAY)
    def _query_cached(self, *args, **kwargs):
        return self._query(*args, **kwargs)

    def _sweep_pages(
        self, query, params, operation_name=None, paginated_key_loc: Iterable[str] = []
    ) -> dict[str, Any]:
        """
        Retrieve paginated data from a GraphQL API and concatenate the results into a single
        response.

        Args:
            query: represents a filepath with the GraphQL query to be executed.
            params: a dictionary that contains the parameters to be passed to the query. These
                parameters can be used to filter or modify the results of the query.
            operation_name: The `operation_name` parameter is an optional parameter that specifies
                the name of the GraphQL operation to be executed. It is used when making a GraphQL
                query to specify which operation to execute if there are multiple operations defined
                in the GraphQL schema. If not provided, the default operation will be executed.
            paginated_key_loc (Iterable[str]): The `paginated_key_loc` parameter is an iterable of
                strings that represents the location of the paginated field within the response. It
                is used to extract the paginated field from the response and append it to the final
                result. The node has to be a list of objects with a `pageInfo` field that contains
                at least the `hasNextPage` and `endCursor` fields.

        Returns:
            a dictionary containing the response from the query with the paginated field
            concatenated.
        """

        def fetch_page(cursor: str | None = None) -> dict[str, Any]:
            if cursor:
                params["cursor"] = cursor
                logging.info(
                    f"Found more than 100 elements, paginating. "
                    f"Current cursor at {cursor}"
                )

            return self._query(query, params, operation_name)

        # Execute the initial query
        response: dict[str, Any] = fetch_page()

        # Initialize an empty list to store the final result
        final_partial_field: list[dict[str, Any]] = []

        # Loop until all pages have been retrieved
        while True:
            # Get the partial field to be appended to the final result
            partial_field = response
            for key in paginated_key_loc:
                partial_field = partial_field[key]

            # Append the partial field to the final result
            final_partial_field += partial_field["nodes"]

            # Check if there are more pages to retrieve
            page_info = partial_field["pageInfo"]
            if not page_info["hasNextPage"]:
                break

            # Execute the query with the updated cursor parameter
            response = fetch_page(page_info["endCursor"])

        # Replace the "nodes" field in the original response with the final result
        partial_field["nodes"] = final_partial_field
        return response

    def invalidate_query_cache(self) -> None:
        logging.warning("Invalidating query cache")
        try:
            self._sweep_pages._db.clear()
            self._query._db.clear()
        except AttributeError as ex:
            logging.warning(f"Could not invalidate cache, maybe it was not used in {ex.args}?")


def insert_early_stage_jobs(stage_sequence: StageSeq, jobs_metadata: Dag) -> Dag:
    pre_processed_dag: dict[str, set[str]] = {}
    jobs_from_early_stages = list(accumulate(stage_sequence.values(), set.union))
    for job_name, metadata in jobs_metadata.items():
        final_needs: set[str] = deepcopy(metadata["needs"])
        # Pre-process jobs that are not based on needs field
        # e.g. sanity job in mesa MR pipelines
        if not final_needs:
            job_stage: str = jobs_metadata[job_name]["stage"]
            stage_index: int = list(stage_sequence.keys()).index(job_stage)
            if stage_index > 0:
                final_needs |= jobs_from_early_stages[stage_index - 1]
        pre_processed_dag[job_name] = final_needs

    for job_name, needs in pre_processed_dag.items():
        jobs_metadata[job_name]["needs"] = needs

    return jobs_metadata


def traverse_dag_needs(jobs_metadata: Dag) -> None:
    created_jobs = set(jobs_metadata.keys())
    for job, metadata in jobs_metadata.items():
        final_needs: set = deepcopy(metadata["needs"]) & created_jobs
        # Post process jobs that are based on needs field
        partial = True

        while partial:
            next_depth: set[str] = {n for dn in final_needs for n in jobs_metadata[dn]["needs"]}
            partial: bool = not final_needs.issuperset(next_depth)
            final_needs = final_needs.union(next_depth)

        jobs_metadata[job]["needs"] = final_needs


def extract_stages_and_job_needs(
    pipeline_jobs: dict[str, Any], pipeline_stages: dict[str, Any]
) -> tuple[StageSeq, Dag]:
    jobs_metadata = Dag()
    # Record the stage sequence to post process deps that are not based on needs
    # field, for example: sanity job
    stage_sequence: OrderedDict[str, set[str]] = OrderedDict()
    for stage in pipeline_stages["nodes"]:
        stage_sequence[stage["name"]] = set()

    for job in pipeline_jobs["nodes"]:
        stage_sequence[job["stage"]["name"]].add(job["name"])
        dag_job: DagNode = {
            "name": job["name"],
            "stage": job["stage"]["name"],
            "needs": set([j["node"]["name"] for j in job["needs"]["edges"]]),
        }
        jobs_metadata[job["name"]] = dag_job

    return stage_sequence, jobs_metadata


def create_job_needs_dag(gl_gql: GitlabGQL, params, disable_cache: bool = True) -> Dag:
    """
    This function creates a Directed Acyclic Graph (DAG) to represent a sequence of jobs, where each
    job has a set of jobs that it depends on (its "needs") and belongs to a certain "stage".
    The "name" of the job is used as the key in the dictionary.

    For example, consider the following DAG:

        1. build stage: job1 -> job2 -> job3
        2. test stage: job2 -> job4

    - The job needs for job3 are: job1, job2
    - The job needs for job4 are: job2
    - The job2 needs to wait all jobs from build stage to finish.

    The resulting DAG would look like this:

        dag = {
            "job1": {"needs": set(), "stage": "build", "name": "job1"},
            "job2": {"needs": {"job1", "job2", job3"}, "stage": "test", "name": "job2"},
            "job3": {"needs": {"job1", "job2"}, "stage": "build", "name": "job3"},
            "job4": {"needs": {"job2"}, "stage": "test", "name": "job4"},
        }

    To access the job needs, one can do:

        dag["job3"]["needs"]

    This will return the set of jobs that job3 needs: {"job1", "job2"}

    Args:
        gl_gql (GitlabGQL): The `gl_gql` parameter is an instance of the `GitlabGQL` class, which is
            used to make GraphQL queries to the GitLab API.
        params (dict): The `params` parameter is a dictionary that contains the necessary parameters
            for the GraphQL query. It is used to specify the details of the pipeline for which the
            job needs DAG is being created.
            The specific keys and values in the `params` dictionary will depend on
            the requirements of the GraphQL query being executed
        disable_cache (bool): The `disable_cache` parameter is a boolean that specifies whether the

    Returns:
        The final DAG (Directed Acyclic Graph) representing the job dependencies sourced from needs
        or stages rule.
    """
    stages_jobs_gql = gl_gql.query(
        "pipeline_details.gql",
        params=params,
        paginated_key_loc=["project", "pipeline", "jobs"],
        disable_cache=disable_cache,
    )
    pipeline_data = stages_jobs_gql["project"]["pipeline"]
    if not pipeline_data:
        raise RuntimeError(f"Could not find any pipelines for {params}")

    stage_sequence, jobs_metadata = extract_stages_and_job_needs(
        pipeline_data["jobs"], pipeline_data["stages"]
    )
    # Fill the DAG with the job needs from stages that don't have any needs but still need to wait
    # for previous stages
    final_dag = insert_early_stage_jobs(stage_sequence, jobs_metadata)
    # Now that each job has its direct needs filled correctly, update the "needs" field for each job
    # in the DAG by performing a topological traversal
    traverse_dag_needs(final_dag)

    return final_dag


def filter_dag(dag: Dag, regex: Pattern) -> Dag:
    jobs_with_regex: set[str] = {job for job in dag if regex.fullmatch(job)}
    return Dag({job: data for job, data in dag.items() if job in sorted(jobs_with_regex)})


def print_dag(dag: Dag) -> None:
    for job, data in dag.items():
        print(f"{job}:")
        print(f"\t{' '.join(data['needs'])}")
        print()


def fetch_merged_yaml(gl_gql: GitlabGQL, params) -> dict[str, Any]:
    params["content"] = dedent("""\
    include:
      - local: .gitlab-ci.yml
    """)
    raw_response = gl_gql.query("job_details.gql", params)
    if merged_yaml := raw_response["ciConfig"]["mergedYaml"]:
        return yaml.safe_load(merged_yaml)

    gl_gql.invalidate_query_cache()
    raise ValueError(
        """
    Could not fetch any content for merged YAML,
    please verify if the git SHA exists in remote.
    Maybe you forgot to `git push`?  """
    )


def recursive_fill(job, relationship_field, target_data, acc_data: dict, merged_yaml):
    if relatives := job.get(relationship_field):
        if isinstance(relatives, str):
            relatives = [relatives]

        for relative in relatives:
            parent_job = merged_yaml[relative]
            acc_data = recursive_fill(parent_job, acc_data, merged_yaml)  # type: ignore

    acc_data |= job.get(target_data, {})

    return acc_data


def get_variables(job, merged_yaml, project_path, sha) -> dict[str, str]:
    p = get_project_root_dir() / ".gitlab-ci" / "image-tags.yml"
    image_tags = yaml.safe_load(p.read_text())

    variables = image_tags["variables"]
    variables |= merged_yaml["variables"]
    variables |= job["variables"]
    variables["CI_PROJECT_PATH"] = project_path
    variables["CI_PROJECT_NAME"] = project_path.split("/")[1]
    variables["CI_REGISTRY_IMAGE"] = "registry.freedesktop.org/${CI_PROJECT_PATH}"
    variables["CI_COMMIT_SHA"] = sha

    while recurse_among_variables_space(variables):
        pass

    return variables


# Based on: https://stackoverflow.com/a/2158532/1079223
def flatten(xs):
    for x in xs:
        if isinstance(x, Iterable) and not isinstance(x, (str, bytes)):
            yield from flatten(x)
        else:
            yield x


def get_full_script(job) -> list[str]:
    script = []
    for script_part in ("before_script", "script", "after_script"):
        script.append(f"# {script_part}")
        lines = flatten(job.get(script_part, []))
        script.extend(lines)
        script.append("")

    return script


def recurse_among_variables_space(var_graph) -> bool:
    updated = False
    for var, value in var_graph.items():
        value = str(value)
        dep_vars = []
        if match := re.findall(r"(\$[{]?[\w\d_]*[}]?)", value):
            all_dep_vars = [v.lstrip("${").rstrip("}") for v in match]
            # print(value, match, all_dep_vars)
            dep_vars = [v for v in all_dep_vars if v in var_graph]

        for dep_var in dep_vars:
            dep_value = str(var_graph[dep_var])
            new_value = var_graph[var]
            new_value = new_value.replace(f"${{{dep_var}}}", dep_value)
            new_value = new_value.replace(f"${dep_var}", dep_value)
            var_graph[var] = new_value
            updated |= dep_value != new_value

    return updated


def print_job_final_definition(job_name, merged_yaml, project_path, sha):
    job = merged_yaml[job_name]
    variables = get_variables(job, merged_yaml, project_path, sha)

    print("# --------- variables ---------------")
    for var, value in sorted(variables.items()):
        print(f"export {var}={value!r}")

    # TODO: Recurse into needs to get full script
    # TODO: maybe create a extra yaml file to avoid too much rework
    script = get_full_script(job)
    print()
    print()
    print("# --------- full script ---------------")
    print("\n".join(script))

    if image := variables.get("MESA_IMAGE"):
        print()
        print()
        print("# --------- container image ---------------")
        print(image)


def from_sha_to_pipeline_iid(gl_gql: GitlabGQL, params) -> str:
    result = gl_gql.query("pipeline_utils.gql", params)

    return result["project"]["pipelines"]["nodes"][0]["iid"]


def parse_args() -> Namespace:
    parser = ArgumentParser(
        formatter_class=ArgumentDefaultsHelpFormatter,
        description="CLI and library with utility functions to debug jobs via Gitlab GraphQL",
        epilog=f"""Example:
        {Path(__file__).name} --print-dag""",
    )
    parser.add_argument("-pp", "--project-path", type=str, default="mesa/mesa")
    parser.add_argument("--sha", "--rev", type=str, default='HEAD')
    parser.add_argument(
        "--regex",
        type=str,
        required=False,
        help="Regex pattern for the job name to be considered",
    )
    mutex_group_print = parser.add_mutually_exclusive_group()
    mutex_group_print.add_argument(
        "--print-dag",
        action="store_true",
        help="Print job needs DAG",
    )
    mutex_group_print.add_argument(
        "--print-merged-yaml",
        action="store_true",
        help="Print the resulting YAML for the specific SHA",
    )
    mutex_group_print.add_argument(
        "--print-job-manifest",
        metavar='JOB_NAME',
        type=str,
        help="Print the resulting job data"
    )
    parser.add_argument(
        "--gitlab-token-file",
        type=str,
        default=get_token_from_default_dir(),
        help="force GitLab token, otherwise it's read from $XDG_CONFIG_HOME/gitlab-token",
    )

    args = parser.parse_args()
    args.gitlab_token = Path(args.gitlab_token_file).read_text().strip()
    return args


def main():
    args = parse_args()
    gl_gql = GitlabGQL(token=args.gitlab_token)

    sha = check_output(['git', 'rev-parse', args.sha]).decode('ascii').strip()

    if args.print_dag:
        iid = from_sha_to_pipeline_iid(gl_gql, {"projectPath": args.project_path, "sha": sha})
        dag = create_job_needs_dag(
            gl_gql, {"projectPath": args.project_path, "iid": iid}, disable_cache=True
        )

        if args.regex:
            dag = filter_dag(dag, re.compile(args.regex))

        print_dag(dag)

    if args.print_merged_yaml or args.print_job_manifest:
        merged_yaml = fetch_merged_yaml(
            gl_gql, {"projectPath": args.project_path, "sha": sha}
        )

        if args.print_merged_yaml:
            print(yaml.dump(merged_yaml, indent=2))

        if args.print_job_manifest:
            print_job_final_definition(
                args.print_job_manifest, merged_yaml, args.project_path, sha
            )


if __name__ == "__main__":
    main()
