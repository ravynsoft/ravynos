# SPDX-License-Identifier: MIT

import os
from ctypes import (
    POINTER,
    Structure,
    _Pointer,
    byref,
    c_char,
    c_char_p,
    c_int,
    c_size_t,
    c_uint32,
    cdll,
    create_string_buffer,
)
from ctypes.util import find_library
from enum import Enum, IntFlag
from functools import reduce
from pathlib import Path
from typing import TYPE_CHECKING, Any, NamedTuple, Optional, TypeAlias

###############################################################################
# Types
###############################################################################


class xkb_context(Structure):
    pass


class xkb_rule_names(Structure):
    _fields_ = [
        ("rules", POINTER(c_char)),
        ("model", POINTER(c_char)),
        ("layout", POINTER(c_char)),
        ("variant", POINTER(c_char)),
        ("options", POINTER(c_char)),
    ]


class xkb_keymap(Structure):
    pass


class xkb_state(Structure):
    pass


class xkb_key_direction(Enum):
    XKB_KEY_UP = c_int(0)
    XKB_KEY_DOWN = c_int(1)


# [HACK] Typing ctypes correctly is difficult. The following works, but there
#        could be another better way.
if TYPE_CHECKING:
    xkb_context_p: TypeAlias = _Pointer[xkb_context]
    xkb_rule_names_p: TypeAlias = _Pointer[xkb_rule_names]
    xkb_keymap_p: TypeAlias = _Pointer[xkb_keymap]
    xkb_state_p = _Pointer[xkb_state]
else:
    xkb_context_p = Any
    xkb_rule_names_p = Any
    xkb_keymap_p = Any
    xkb_state_p = Any
xkb_context_flags = c_int
xkb_keymap_compile_flags = c_int
xkb_keymap_format = c_int
xkb_keycode_t = c_uint32
xkb_keysym_t = c_uint32
xkb_mod_index_t = c_uint32
xkb_led_index_t = c_uint32
xkb_level_index_t = c_uint32
xkb_layout_index_t = c_uint32
xkb_state_component = c_int
xkb_consumed_mode = c_int


###############################################################################
# Constants
###############################################################################


XKB_CONTEXT_NO_DEFAULT_INCLUDES = 1 << 0
XKB_CONTEXT_NO_ENVIRONMENT_NAMES = 1 << 1
XKB_KEYCODE_INVALID = 0xFFFFFFFF
XKB_STATE_MODS_EFFECTIVE = 1 << 3
XKB_CONSUMED_MODE_XKB = 0
XKB_KEYMAP_FORMAT_TEXT_V1 = 1


class ModifierMask(IntFlag):
    """Built-in standard definitions of modifiers masks"""

    Shift = 1 << 0
    Lock = 1 << 1
    Control = 1 << 2
    Mod1 = 1 << 3
    Mod2 = 1 << 4
    Mod3 = 1 << 5
    Mod4 = 1 << 6
    Mod5 = 1 << 7


NoModifier = ModifierMask(0)
Shift = ModifierMask.Shift
Lock = ModifierMask.Lock
Control = ModifierMask.Control
Mod1 = ModifierMask.Mod1
Mod2 = ModifierMask.Mod2
Mod3 = ModifierMask.Mod3
Mod4 = ModifierMask.Mod4
Mod5 = ModifierMask.Mod5


###############################################################################
# Binding to libxkbcommon
###############################################################################


xkbcommon_path = os.environ.get("XKBCOMMON_LIB_PATH")

if xkbcommon_path:
    xkbcommon_path = str(Path(xkbcommon_path).resolve())
    xkbcommon = cdll.LoadLibrary(xkbcommon_path)
else:
    xkbcommon_path = find_library("xkbcommon")
    if xkbcommon_path:
        xkbcommon = cdll.LoadLibrary(xkbcommon_path)
    else:
        raise OSError("Cannot load libxbcommon")

xkbcommon.xkb_context_new.argtypes = [xkb_context_flags]
xkbcommon.xkb_context_new.restype = POINTER(xkb_context)

xkbcommon.xkb_keymap_new_from_names.argtypes = [
    POINTER(xkb_context),
    POINTER(xkb_rule_names),
    xkb_keymap_compile_flags,
]
xkbcommon.xkb_keymap_new_from_names.restype = POINTER(xkb_keymap)

xkbcommon.xkb_keymap_key_by_name.argtypes = [POINTER(xkb_keymap), c_char_p]
xkbcommon.xkb_keymap_key_by_name.restype = xkb_keycode_t

xkbcommon.xkb_state_new.argtypes = [POINTER(xkb_keymap)]
xkbcommon.xkb_state_new.restype = POINTER(xkb_state)

xkbcommon.xkb_state_get_keymap.argtypes = [POINTER(xkb_state)]
xkbcommon.xkb_state_get_keymap.restype = POINTER(xkb_keymap)

xkbcommon.xkb_state_key_get_one_sym.argtypes = [POINTER(xkb_state), xkb_keycode_t]
xkbcommon.xkb_state_key_get_one_sym.restype = xkb_keysym_t

xkbcommon.xkb_keymap_led_get_name.argtypes = [POINTER(xkb_keymap), xkb_led_index_t]
xkbcommon.xkb_keymap_led_get_name.restype = c_char_p

xkbcommon.xkb_state_key_get_layout.argtypes = [POINTER(xkb_state), xkb_keycode_t]
xkbcommon.xkb_state_key_get_layout.restype = xkb_layout_index_t

xkbcommon.xkb_state_key_get_level.argtypes = [
    POINTER(xkb_state),
    xkb_keycode_t,
    xkb_layout_index_t,
]
xkbcommon.xkb_state_key_get_level.restype = xkb_level_index_t

xkbcommon.xkb_keymap_num_mods.argtypes = [POINTER(xkb_keymap)]
xkbcommon.xkb_keymap_num_mods.restype = xkb_mod_index_t

xkbcommon.xkb_state_mod_index_is_active.argtypes = [
    POINTER(xkb_state),
    xkb_mod_index_t,
    xkb_state_component,
]
xkbcommon.xkb_state_mod_index_is_active.restype = c_int

xkbcommon.xkb_state_mod_index_is_consumed2.argtypes = [
    POINTER(xkb_state),
    xkb_keycode_t,
    xkb_mod_index_t,
    xkb_consumed_mode,
]
xkbcommon.xkb_state_mod_index_is_consumed2.restype = c_int

xkbcommon.xkb_keymap_num_leds.argtypes = [POINTER(xkb_keymap)]
xkbcommon.xkb_keymap_num_leds.restype = xkb_led_index_t

xkbcommon.xkb_state_led_index_is_active.argtypes = [POINTER(xkb_state), xkb_led_index_t]
xkbcommon.xkb_state_led_index_is_active.restype = c_int


def load_keymap(
    xkb_config_root: Path,
    rules=None,
    model=None,
    layout=None,
    variant=None,
    options=None,
) -> xkb_keymap_p:
    # Create context
    context = xkbcommon.xkb_context_new(
        XKB_CONTEXT_NO_DEFAULT_INCLUDES | XKB_CONTEXT_NO_ENVIRONMENT_NAMES
    )
    if not context:
        raise ValueError("Couldn't create xkb context")
    raw_path = create_string_buffer(str(xkb_config_root).encode("utf-8"))
    xkbcommon.xkb_context_include_path_append(context, raw_path)
    rmlvo = xkb_rule_names(
        rules=create_string_buffer(rules.encode("utf-8")) if rules else None,
        model=create_string_buffer(model.encode("utf-8")) if model else None,
        layout=create_string_buffer(layout.encode("utf-8")) if layout else None,
        variant=create_string_buffer(variant.encode("utf-8")) if variant else None,
        options=create_string_buffer(options.encode("utf-8")) if options else None,
    )
    # Load keymap
    keymap = xkbcommon.xkb_keymap_new_from_names(context, byref(rmlvo), 0)
    if not keymap:
        raise ValueError(
            f"Failed to compile RMLVO: {rules=}, {model=}, {layout=}, "
            f"{variant=}, {options=}"
        )

    xkbcommon.xkb_context_unref(context)
    return keymap


def unref_keymap(keymap: xkb_keymap_p) -> None:
    xkbcommon.xkb_keymap_unref(keymap)


def new_state(keymap: xkb_keymap_p) -> xkb_state_p:
    state = xkbcommon.xkb_state_new(keymap)
    if not state:
        raise ValueError("Cannot create state")
    return state


def init_state(
    xkeyboard_config_path: Path,
    rules=None,
    model=None,
    layout=None,
    variant=None,
    options=None,
) -> xkb_state_p:
    keymap = load_keymap(xkeyboard_config_path, rules, model, layout, variant, options)
    return new_state(keymap)


def unref_state(state: xkb_state_p) -> None:
    xkbcommon.xkb_state_unref(state)


def xkb_keymap_led_get_name(keymap: xkb_keymap_p, led: int) -> str:
    return xkbcommon.xkb_keymap_led_get_name(keymap, led).decode("utf-8")


def xkb_keysym_get_name(keysym: int) -> str:
    buf_len = 90
    buf = create_string_buffer(buf_len)
    n = xkbcommon.xkb_keysym_get_name(keysym, buf, c_size_t(buf_len))
    if n > 0:
        return buf.value.decode("utf-8")
    elif n >= buf_len:
        raise ValueError(f"Truncated: expected {buf_len}, got: {n + 1}.")
    else:
        raise ValueError(f"Unsupported keysym: {keysym}")


def xkb_state_key_get_utf8(state: xkb_state_p, key: int) -> str:
    buf_len = 8
    buf = create_string_buffer(buf_len)
    n = xkbcommon.xkb_state_key_get_utf8(state, key, buf, c_size_t(buf_len))
    if n >= buf_len:
        raise ValueError(f"Truncated: expected {buf_len}, got: {n + 1}.")
    else:
        return buf.value.decode("utf-8")


###############################################################################
# Process key events
###############################################################################


class Result(NamedTuple):
    keycode: int
    layout: int
    keysym: str
    unicode: str
    level: int
    active_mods: ModifierMask
    consumed_mods: ModifierMask
    leds: tuple[str, ...]

    @property
    def group(self) -> int:
        """Alias for Result.layout"""
        return self.layout


def process_key_event(
    state: xkb_state_p, key: str, direction: xkb_key_direction
) -> Result:
    keymap: xkb_keymap_p = xkbcommon.xkb_state_get_keymap(state)
    keycode: int = xkbcommon.xkb_keymap_key_by_name(
        keymap, create_string_buffer(key.encode("utf-8"))
    )
    if keycode == XKB_KEYCODE_INVALID:
        raise ValueError(f"Unsupported key name: {key}")

    # Modifiers
    mods_count = xkbcommon.xkb_keymap_num_mods(keymap)
    active_mods = reduce(
        (lambda acc, m: acc | ModifierMask(1 << m)),
        filter(
            lambda m: xkbcommon.xkb_state_mod_index_is_active(
                state, m, XKB_STATE_MODS_EFFECTIVE
            ),
            range(0, mods_count),
        ),
        ModifierMask(0),
    )
    consumed_mods = reduce(
        (lambda acc, m: acc | ModifierMask(1 << m)),
        filter(
            lambda m: xkbcommon.xkb_state_mod_index_is_active(
                state, m, XKB_STATE_MODS_EFFECTIVE
            )
            and xkbcommon.xkb_state_mod_index_is_consumed2(
                state, keycode, m, XKB_CONSUMED_MODE_XKB
            ),
            range(0, mods_count),
        ),
        ModifierMask(0),
    )

    # Level
    layout: int = xkbcommon.xkb_state_key_get_layout(state, keycode)
    level: int = xkbcommon.xkb_state_key_get_level(state, keycode, layout)

    # Keysyms
    # [TODO] multiple keysyms
    # keysyms = (xkb_keysym_t * 2)()
    # keysyms_count = xkb_state_key_get_syms(state, keycode, byref(keysyms))
    keysym = xkbcommon.xkb_state_key_get_one_sym(state, keycode)
    keysym_str = xkb_keysym_get_name(keysym)

    # [TODO] Compose?
    # Unicode
    unicode = xkb_state_key_get_utf8(state, keycode)

    # LEDs
    leds_count = xkbcommon.xkb_keymap_num_leds(keymap)
    leds = tuple(
        xkb_keymap_led_get_name(keymap, led)
        for led in range(0, leds_count)
        if xkbcommon.xkb_state_led_index_is_active(state, led)
    )

    # Update state
    # [TODO] use return value?
    xkbcommon.xkb_state_update_key(state, keycode, direction.value)

    # Return result
    return Result(
        keycode=keycode,
        layout=layout + 1,
        keysym=keysym_str,
        unicode=unicode,
        active_mods=active_mods,
        consumed_mods=consumed_mods,
        level=level + 1,
        leds=leds,
    )


###############################################################################
# Pythonic interface
###############################################################################


class ForeignKeymap:
    """
    Context manager to ensure proper handling of foreign `xkb_keymap` object.

    Intended use::

        with ForeignKeymap(xkb_base, layout="de") as keymap:
            with ForeignState(keymap) as state:
                # Use state safely here
                state.process_key_event(...)
    """

    def __init__(
        self,
        xkb_base: Path,
        rules: Optional[str] = None,
        model: Optional[str] = None,
        layout: Optional[str] = None,
        variant: Optional[str] = None,
        options: Optional[str] = None,
    ):
        self.xkb_base = xkb_base
        self._keymap = POINTER(xkb_keymap)()  # NULL pointer
        self.rules = rules
        self.model = model
        self.layout = layout
        self.variant = variant
        self.options = options

    def __enter__(self) -> xkb_keymap_p:
        self._keymap = load_keymap(
            self.xkb_base,
            model=self.model,
            layout=self.layout,
            variant=self.variant,
            options=self.options,
        )
        return self._keymap

    def __exit__(self, exc_type, exc_val, exc_tb):
        unref_keymap(self._keymap)


class State:
    """
    Convenient interface to use `xkb_state`.

    Intended use: see `ForeignKeymap`.
    """

    def __init__(self, state: xkb_state_p):
        self._state = state

    def process_key_event(self, key: str, direction: xkb_key_direction) -> Result:
        return process_key_event(self._state, key, direction)


class ForeignState:
    """
    Context manager to ensure proper handling of foreign `xkb_state` object.

    Intended use: see `ForeignKeymap`.
    """

    def __init__(self, keymap: xkb_keymap_p):
        self._keymap = keymap
        self._state = POINTER(xkb_state)()  # NULL pointer

    def __enter__(self) -> State:
        self._state = new_state(self._keymap)
        return State(self._state)

    def __exit__(self, exc_type, exc_val, exc_tb):
        unref_state(self._state)
