import os
import re
from enum import Enum


class LavaFarm(Enum):
    """Enum class representing the different LAVA farms."""

    LIMA = 1
    COLLABORA = 2
    UNKNOWN = 3


LAVA_FARM_RUNNER_PATTERNS: dict[LavaFarm, str] = {
    # Lima pattern comes first, since it has the same prefix as the
    # Collabora pattern.
    LavaFarm.LIMA: r"^mesa-ci-[\x01-\x7F]+-lava-lima$",
    LavaFarm.COLLABORA: r"^mesa-ci-[\x01-\x7F]+-lava-[\x01-\x7F]+$",
    LavaFarm.UNKNOWN: r"^[\x01-\x7F]+",
}


def get_lava_farm() -> LavaFarm:
    """
    Returns the LAVA farm based on the RUNNER_TAG environment variable.

    :return: The LAVA farm
    """
    runner_tag: str = os.getenv("RUNNER_TAG", "unknown")

    for farm, pattern in LAVA_FARM_RUNNER_PATTERNS.items():
        if re.match(pattern, runner_tag):
            return farm

    raise ValueError(f"Unknown LAVA runner tag: {runner_tag}")
