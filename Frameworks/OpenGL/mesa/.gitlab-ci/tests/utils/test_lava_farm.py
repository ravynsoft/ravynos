import re

import pytest
from hypothesis import given
from hypothesis import strategies as st
from lava.utils.lava_farm import LAVA_FARM_RUNNER_PATTERNS, LavaFarm, get_lava_farm


@given(
    runner_tag=st.text(
        alphabet=st.characters(
            min_codepoint=1, max_codepoint=127, blacklist_categories=("C",)
        ),
        min_size=1,
    )
)
def test_get_lava_farm_invalid_tags(runner_tag):
    with pytest.MonkeyPatch().context() as mp:
        mp.setenv("RUNNER_TAG", runner_tag)
        assert get_lava_farm() == LavaFarm.UNKNOWN


def test_get_lava_farm_no_tag(monkeypatch):
    monkeypatch.delenv("RUNNER_TAG", raising=False)
    assert get_lava_farm() == LavaFarm.UNKNOWN


@given(
    st.fixed_dictionaries(
        {k: st.from_regex(v) for k, v in LAVA_FARM_RUNNER_PATTERNS.items()}
    )
)
def test_get_lava_farm_valid_tags(runner_farm_tag: dict):
    with pytest.MonkeyPatch().context() as mp:
        for farm, tag in runner_farm_tag.items():
            try:
                mp.setenv("RUNNER_TAG", tag)
            except ValueError:
                # hypothesis may generate null bytes in the string
                continue
            assert get_lava_farm() == farm
