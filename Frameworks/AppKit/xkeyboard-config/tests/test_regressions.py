# SPDX-License-Identifier: MIT

from __future__ import annotations

import os
import re
from functools import reduce
from pathlib import Path
from typing import Optional

import pytest
import xkbcommon
from xkbcommon import Mod1, Mod4, Mod5, ModifierMask, NoModifier, Shift

###############################################################################
# pytest configuration
###############################################################################

# You may skip this section and go to the section “How-to write tests”
# if you only intend to write new tests.
#
# How the test suite works
# ------------------------
#
# Interfacing with xkbcommon requires:
# • Taking care of initialization and finalization of foreign objects.
#   This is done using `xkbcommon.ForeignKeymap` and `xkbcommon.ForeignState`
#   context managers.
# • Updating the state: this is down with `xkbcommon.State`.
#
# pytest fixtures:
# • The only fixture intended in the test code is `keymap`.
# • Other fixtures are just helpers that are used indirectly.
# • The intended use is documented in `TestSuiteDoc`.


KEYCODE_PATTERN = re.compile(
    r"""^(?:
        # Usual keycodes
          [A-Z]         # Start with an upper case letter
          [A-Z0-9]{1,3} # Followed by up to 3 characters
        # Latin aliases
        | Lat[A-Z]
        # Special cases
        | VOL-
        | VOL\+
        )$
     """,
    re.VERBOSE,
)


@pytest.mark.parametrize("key", ("UP", "TAB", "AE01", "RTRN", "VOL-", "I120", "LatA"))
def test_valid_keycode_pattern(key: str):
    assert KEYCODE_PATTERN.match(key)


@pytest.mark.parametrize(
    "key", ("U", "LFTSH", "Shift_L", "lfsh", "9", "1I20", "latA", "Lat9")
)
def test_invalid_keycode_pattern(key: str):
    assert not KEYCODE_PATTERN.match(key)


BASE_GROUP = 1
BASE_LEVEL = 1


def check_keycode(key: str) -> bool:
    """Check that keycode has the required syntax."""
    return bool(KEYCODE_PATTERN.match(key))


class Keymap:
    """Public test methods"""

    def __init__(self, state: xkbcommon.State):
        self._state = state

    def press(self, key: str) -> xkbcommon.Result:
        """Update the state by pressing a key"""
        assert check_keycode(key), "key must be a [2-4]-character keycode"
        return self._state.process_key_event(
            key, xkbcommon.xkb_key_direction.XKB_KEY_DOWN
        )

    def release(self, key: str) -> xkbcommon.Result:
        """Update the state by releasing a key"""
        assert check_keycode(key), "key must be a [2-4]-character keycode"
        return self._state.process_key_event(
            key, xkbcommon.xkb_key_direction.XKB_KEY_UP
        )

    def tap(self, key: str) -> xkbcommon.Result:
        """Update the state by tapping a key"""
        assert check_keycode(key), "key must be a [2-4]-character keycode"
        self.press(key)
        return self.release(key)

    def tap_and_check(
        self, key: str, keysym: str, group: int = BASE_GROUP, level: int = BASE_LEVEL
    ) -> xkbcommon.Result:
        """
        Check that tapping a key produces the expected keysym in the
        expected group and level.
        """
        r = self.tap(key)
        assert r.group == group
        assert r.level == level
        assert r.keysym == keysym
        # Return the result for optional further tests
        return r

    def key_down(self, *keys: str) -> _KeyDown:
        """Update the state by holding some keys"""
        assert all(map(check_keycode, keys)), "keys must be a [2-4]-character keycodes"
        return _KeyDown(self, *keys)


# NOTE: Abusing Python’s context manager to enable nice test syntax
class _KeyDown:
    """Context manager that will hold a key."""

    def __init__(self, keymap: Keymap, *keys: str):
        self.keys = keys
        self.keymap = keymap

    def __enter__(self) -> xkbcommon.Result:
        """Press the key in order, then return the last result."""
        return reduce(
            lambda _, key: self.keymap.press(key),
            self.keys,
            xkbcommon.Result(0, 0, "", "", 0, NoModifier, NoModifier, ()),
        )

    def __exit__(self, *_):
        for key in self.keys:
            self.keymap.release(key)


@pytest.fixture(scope="session")
def xkb_base():
    """Get the xkeyboard-config directory from the environment."""
    path = os.environ.get("XKB_CONFIG_ROOT")
    if path:
        return Path(path)
    else:
        raise ValueError("XKB_CONFIG_ROOT environment variable is not defined")


# The following fixtures enable them to have default values (i.e. None).


@pytest.fixture(scope="function")
def rules(request: pytest.FixtureRequest):
    return getattr(request, "param", None)


@pytest.fixture(scope="function")
def model(request: pytest.FixtureRequest):
    return getattr(request, "param", None)


@pytest.fixture(scope="function")
def layout(request: pytest.FixtureRequest):
    return getattr(request, "param", None)


@pytest.fixture(scope="function")
def variant(request: pytest.FixtureRequest):
    return getattr(request, "param", None)


@pytest.fixture(scope="function")
def options(request: pytest.FixtureRequest):
    return getattr(request, "param", None)


@pytest.fixture
def keymap(
    xkb_base: Path,
    rules: Optional[str],
    model: Optional[str],
    layout: Optional[str],
    variant: Optional[str],
    options: Optional[str],
):
    """Load a keymap, and return a new state."""
    with xkbcommon.ForeignKeymap(
        xkb_base,
        rules=rules,
        model=model,
        layout=layout,
        variant=variant,
        options=options,
    ) as km:
        with xkbcommon.ForeignState(km) as state:
            yield Keymap(state)


# Documented example
# The RMLVO parameters (“rules”, “model”, “layout”, “variant” and “options”)
# are optional and are implicitely consumed by the keymap fixture.
@pytest.mark.parametrize("layout", ["us"])
class TestSuiteDoc:
    # The keymap argument is mandatory. It will:
    # • Load the keymap corresponding to the RMLVO input;
    # • Initialize a new state;
    # • Return a convenient `Keymap` object, that will manage the
    #   low-level xkbcommon stuff and provide methods to safely change
    #   the state.
    def test_example(self, keymap: Keymap):
        # Use keymap to change keyboard state
        r = keymap.press("AC01")
        # The return value is used in assertions
        assert r.keysym == "a"
        # When the function returns, if will automatically run the
        # cleanup code of the keymap fixture, i.e. the __exit__
        # function of `xkbcommon.ForeignKeymap` and
        # `xkbcommon.ForeignKeymap`.
        # See further examples in the section “How-to write tests”.


###############################################################################
# How-to write tests
###############################################################################

# • Create one class per topic. It should have a meaningful name prefixed by
#   `Test` and refer to the topic: e.g. TestCompatibilityOption1Option2.
#   If there is a Gitlab issue it can be named after it: e.g. TestGitlabIssue382.
# • The intended use is commented in the following `TestExample` class.


# The RMLVO XKB configuration is set with parameters “rules”, “model”, “layout”,
# “variant” and “options”. They are optional and default to None.
@pytest.mark.parametrize("layout", ["de"])
# Name prefixed with `Test`.
class TestExample:
    # Define one function for each test. Its name must be prefixed by `test_`.
    # The keymap argument is mandatory. It provides methods to safely
    # change the keyboard state.
    def test_example(self, keymap: Keymap):
        # Use keymap to change keyboard state
        r = keymap.press("LFSH")
        # The return value is used in assertions
        assert r.keysym == "Shift_L"
        # We must not forget to release the key, if necessary:
        keymap.release("LFSH")
        # Or we could also use `Keymap.key_down` to achieve the same:
        with keymap.key_down("LFSH") as r:
            assert r.keysym == "Shift_L"
            # Now we can check the impact of modifier on other keys.
            # Manually:
            r = keymap.tap("AC01")
            assert r.level == 2
            assert r.keysym == "A"
            # With helper function:
            keymap.tap_and_check("AC01", "A", level=2)
        # We can also use multiple keys:
        with keymap.key_down("LFSH", "RALT") as r:
            # In this case the result refers to the last key
            assert r.keysym == "ISO_Level3_Shift"
            r = keymap.tap_and_check("AC01", "AE", level=4)
            # We can also check (real) modifiers directly
            assert r.active_mods == Shift | Mod5 == r.consumed_mods


###############################################################################
# Regression Tests
###############################################################################


# https://gitlab.freedesktop.org/xkeyboard-config/xkeyboard-config/-/issues/382
@pytest.mark.parametrize("layout,variant,options", [("us", "intl", "lv3:lwin_switch")])
class TestIssue382:
    @pytest.mark.parametrize("mod_key", ("RALT", "LWIN"))
    def test_LevelThree(self, keymap: Keymap, mod_key: str):
        """Both RALT and LWIN are LevelThree modifiers"""
        with keymap.key_down(mod_key):
            r = keymap.tap_and_check("AD01", "adiaeresis", level=3)
            assert r.active_mods == Mod5 == r.consumed_mods
            with keymap.key_down("LFSH"):
                r = keymap.tap_and_check("AD01", "Adiaeresis", level=4)
                assert r.active_mods == Shift | Mod5 == r.consumed_mods

    def test_ShiftAlt(self, keymap: Keymap):
        """LALT+LFSH works as if there was no option"""
        r = keymap.tap_and_check("AC10", "semicolon", level=1)
        assert r.active_mods == NoModifier
        with keymap.key_down("LFSH", "LALT"):
            r = keymap.tap_and_check("AC10", "colon", level=2)
            assert r.active_mods == Shift | Mod1
            assert r.consumed_mods == Shift


# https://gitlab.freedesktop.org/xkeyboard-config/xkeyboard-config/-/issues/90
# https://gitlab.freedesktop.org/xkeyboard-config/xkeyboard-config/-/issues/346
class TestIssues90And346:
    @pytest.mark.parametrize(
        "layout,key,keysyms",
        [
            ("fi,us", "TLDE", ("section", "grave")),
            ("dk,us", "TLDE", ("onehalf", "grave")),
            ("fi,us,dk", "TLDE", ("section", "grave", "onehalf")),
        ],
    )
    @pytest.mark.parametrize(
        "options,mod_key,mod",
        [
            ("grp:win_space_toggle", "LWIN", Mod4),
            ("grp:alt_space_toggle", "LALT", Mod1),
        ],
    )
    def test_group_switch_on_all_groups(
        self,
        keymap: Keymap,
        mod_key: str,
        mod: ModifierMask,
        key: str,
        keysyms: tuple[str],
    ):
        """LWIN/LALT + SPCE is a group switch on multiple groups"""
        for group, keysym in enumerate(keysyms, start=1):
            print(group, keysym)
            keymap.tap_and_check(key, keysym, group=group)
            self.switch_group(keymap, mod_key, mod, group % len(keysyms) + 1)
        # Check the group wraps
        keymap.tap_and_check(key, keysyms[0], group=1)

    @staticmethod
    def switch_group(keymap: Keymap, mod_key: str, mod: ModifierMask, group: int):
        with keymap.key_down(mod_key) as r:
            assert r.group == 1  # only defined on first group
            r = keymap.tap_and_check("SPCE", "ISO_Next_Group", group=group, level=2)
            assert r.active_mods == mod == r.consumed_mods


# https://gitlab.freedesktop.org/xkeyboard-config/xkeyboard-config/-/issues/383
@pytest.mark.parametrize("layout", ["us,ru"])
@pytest.mark.parametrize(
    "options,mod_key,mod",
    [
        ("misc:typo,grp:win_space_toggle,lv3:ralt_switch", "LWIN", Mod4),
        ("misc:typo,grp:alt_space_toggle,lv3:ralt_switch", "LALT", Mod1),
    ],
)
class TestIssue383:
    def test_group_switch(self, keymap: Keymap, mod_key: str, mod: ModifierMask):
        """LWIN + SPCE is a group switch on both groups"""
        # Start with us layout
        self.check_keysyms(keymap, 1, "AC01", "a", "combining_acute")
        # Switch to ru layout
        self.switch_group(keymap, mod_key, mod, 2)
        self.check_keysyms(keymap, 2, "AC01", "Cyrillic_ef", "combining_acute")
        # Switch back to us layout
        self.switch_group(keymap, mod_key, mod, 1)
        self.check_keysyms(keymap, 1, "AC01", "a", "combining_acute")

    @staticmethod
    def switch_group(keymap: Keymap, mod_key: str, mod: ModifierMask, group: int):
        with keymap.key_down(mod_key) as r:
            assert r.group == 1  # only defined on first group
            r = keymap.tap_and_check("SPCE", "ISO_Next_Group", group=group, level=2)
            assert r.active_mods == mod == r.consumed_mods

    @staticmethod
    def check_keysyms(
        keymap: Keymap, group: int, key: str, base_keysym: str, typo_keysym: str
    ):
        # Base keysym
        keymap.tap_and_check(key, base_keysym, group=group, level=1)
        # typo keysym
        with keymap.key_down("RALT") as r:
            assert r.group == 1  # only defined on first group
            r = keymap.tap_and_check(key, typo_keysym, group=group, level=3)
            assert r.active_mods == Mod5 == r.consumed_mods
