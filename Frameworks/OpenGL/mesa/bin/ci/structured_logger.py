"""
A structured logging utility supporting multiple data formats such as CSV, JSON,
and YAML.

The main purpose of this script, besides having relevant information available
in a condensed and deserialized.

This script defines a protocol for different file handling strategies and provides
implementations for CSV, JSON, and YAML formats. The main class, StructuredLogger,
allows for easy interaction with log data, enabling users to load, save, increment,
set, and append fields in the log. The script also includes context managers for
file locking and editing log data to ensure data integrity and avoid race conditions.
"""

import json
import os
from collections.abc import MutableMapping, MutableSequence
from contextlib import contextmanager
from datetime import datetime
from pathlib import Path
from typing import Any, Protocol

import fire
from filelock import FileLock

try:
    import polars as pl

    CSV_LIB_EXCEPTION = None
except ImportError as e:
    CSV_LIB_EXCEPTION: ImportError = e

try:
    from ruamel.yaml import YAML

    YAML_LIB_EXCEPTION = None
except ImportError as e:
    YAML_LIB_EXCEPTION: ImportError = e


class ContainerProxy:
    """
    A proxy class that wraps a mutable container object (such as a dictionary or
    a list) and calls a provided save_callback function whenever the container
    or its contents
    are changed.
    """
    def __init__(self, container, save_callback):
        self.container = container
        self.save_callback = save_callback

    def __getitem__(self, key):
        value = self.container[key]
        if isinstance(value, (MutableMapping, MutableSequence)):
            return ContainerProxy(value, self.save_callback)
        return value

    def __setitem__(self, key, value):
        self.container[key] = value
        self.save_callback()

    def __delitem__(self, key):
        del self.container[key]
        self.save_callback()

    def __getattr__(self, name):
        attr = getattr(self.container, name)

        if callable(attr):
            def wrapper(*args, **kwargs):
                result = attr(*args, **kwargs)
                self.save_callback()
                return result

            return wrapper
        return attr

    def __iter__(self):
        return iter(self.container)

    def __len__(self):
        return len(self.container)

    def __repr__(self):
        return repr(self.container)


class AutoSaveDict(dict):
    """
    A subclass of the built-in dict class with additional functionality to
    automatically save changes to the dictionary. It maintains a timestamp of
    the last modification and automatically wraps nested mutable containers
    using ContainerProxy.
    """
    timestamp_key = "_timestamp"

    def __init__(self, *args, save_callback, register_timestamp=True, **kwargs):
        self.save_callback = save_callback
        self.__register_timestamp = register_timestamp
        self.__heartbeat()
        super().__init__(*args, **kwargs)
        self.__wrap_dictionaries()

    def __heartbeat(self):
        if self.__register_timestamp:
            self[AutoSaveDict.timestamp_key] = datetime.now().isoformat()

    def __save(self):
        self.__heartbeat()
        self.save_callback()

    def __wrap_dictionaries(self):
        for key, value in self.items():
            if isinstance(value, MutableMapping) and not isinstance(
                value, AutoSaveDict
            ):
                self[key] = AutoSaveDict(
                    value, save_callback=self.save_callback, register_timestamp=False
                )

    def __setitem__(self, key, value):
        if isinstance(value, MutableMapping) and not isinstance(value, AutoSaveDict):
            value = AutoSaveDict(
                value, save_callback=self.save_callback, register_timestamp=False
            )
        super().__setitem__(key, value)

        if self.__register_timestamp and key == AutoSaveDict.timestamp_key:
            return
        self.__save()

    def __getitem__(self, key):
        value = super().__getitem__(key)
        if isinstance(value, (MutableMapping, MutableSequence)):
            return ContainerProxy(value, self.__save)
        return value

    def __delitem__(self, key):
        super().__delitem__(key)
        self.__save()

    def pop(self, *args, **kwargs):
        result = super().pop(*args, **kwargs)
        self.__save()
        return result

    def update(self, *args, **kwargs):
        super().update(*args, **kwargs)
        self.__wrap_dictionaries()
        self.__save()


class StructuredLoggerStrategy(Protocol):
    def load_data(self, file_path: Path) -> dict:
        pass

    def save_data(self, file_path: Path, data: dict) -> None:
        pass


class CSVStrategy:
    def __init__(self) -> None:
        if CSV_LIB_EXCEPTION:
            raise RuntimeError(
                "Can't parse CSV files. Missing library"
            ) from CSV_LIB_EXCEPTION

    def load_data(self, file_path: Path) -> dict:
        dicts: list[dict[str, Any]] = pl.read_csv(
            file_path, try_parse_dates=True
        ).to_dicts()
        data = {}
        for d in dicts:
            for k, v in d.items():
                if k != AutoSaveDict.timestamp_key and k in data:
                    if isinstance(data[k], list):
                        data[k].append(v)
                        continue
                    data[k] = [data[k], v]
                else:
                    data[k] = v
        return data

    def save_data(self, file_path: Path, data: dict) -> None:
        pl.DataFrame(data).write_csv(file_path)


class JSONStrategy:
    def load_data(self, file_path: Path) -> dict:
        return json.loads(file_path.read_text())

    def save_data(self, file_path: Path, data: dict) -> None:
        with open(file_path, "w") as f:
            json.dump(data, f, indent=2)


class YAMLStrategy:
    def __init__(self):
        if YAML_LIB_EXCEPTION:
            raise RuntimeError(
                "Can't parse YAML files. Missing library"
            ) from YAML_LIB_EXCEPTION
        self.yaml = YAML()
        self.yaml.indent(sequence=4, offset=2)
        self.yaml.default_flow_style = False
        self.yaml.representer.add_representer(AutoSaveDict, self.represent_dict)

    @classmethod
    def represent_dict(cls, dumper, data):
        return dumper.represent_mapping("tag:yaml.org,2002:map", data)

    def load_data(self, file_path: Path) -> dict:
        return self.yaml.load(file_path.read_text())

    def save_data(self, file_path: Path, data: dict) -> None:
        with open(file_path, "w") as f:
            self.yaml.dump(data, f)


class StructuredLogger:
    def __init__(
        self, file_name: str, strategy: StructuredLoggerStrategy = None, truncate=False
    ):
        self.file_name: str = file_name
        self.file_path = Path(self.file_name)
        self._data: AutoSaveDict = AutoSaveDict(save_callback=self.save_data)

        if strategy is None:
            self.strategy: StructuredLoggerStrategy = self.guess_strategy_from_file(
                self.file_path
            )
        else:
            self.strategy = strategy

        if not self.file_path.exists():
            Path.mkdir(self.file_path.parent, exist_ok=True)
            self.save_data()
            return

        if truncate:
            with self.get_lock():
                os.truncate(self.file_path, 0)
                self.save_data()

    def load_data(self):
        self._data = self.strategy.load_data(self.file_path)

    def save_data(self):
        self.strategy.save_data(self.file_path, self._data)

    @property
    def data(self) -> AutoSaveDict:
        return self._data

    @contextmanager
    def get_lock(self):
        with FileLock(f"{self.file_path}.lock", timeout=10):
            yield

    @contextmanager
    def edit_context(self):
        """
        Context manager that ensures proper loading and saving of log data when
        performing multiple modifications.
        """
        with self.get_lock():
            try:
                self.load_data()
                yield
            finally:
                self.save_data()

    @staticmethod
    def guess_strategy_from_file(file_path: Path) -> StructuredLoggerStrategy:
        file_extension = file_path.suffix.lower().lstrip(".")
        return StructuredLogger.get_strategy(file_extension)

    @staticmethod
    def get_strategy(strategy_name: str) -> StructuredLoggerStrategy:
        strategies = {
            "csv": CSVStrategy,
            "json": JSONStrategy,
            "yaml": YAMLStrategy,
            "yml": YAMLStrategy,
        }

        try:
            return strategies[strategy_name]()
        except KeyError as e:
            raise ValueError(f"Unknown strategy for: {strategy_name}") from e


if __name__ == "__main__":
    fire.Fire(StructuredLogger)
