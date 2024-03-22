import json
from pathlib import Path

import pytest
from mock import MagicMock, patch
from structured_logger import (
    AutoSaveDict,
    CSVStrategy,
    JSONStrategy,
    StructuredLogger,
    YAMLStrategy,
)


@pytest.fixture(params=[CSVStrategy, JSONStrategy, YAMLStrategy])
def strategy(request):
    return request.param


@pytest.fixture
def file_extension(strategy):
    if strategy == CSVStrategy:
        return "csv"
    elif strategy == JSONStrategy:
        return "json"
    elif strategy == YAMLStrategy:
        return "yaml"


@pytest.fixture
def tmp_file(tmp_path):
    return tmp_path / "test.json"


def test_guess_strategy_from_file(tmp_path, strategy, file_extension):
    file_name = tmp_path / f"test_guess.{file_extension}"
    Path(file_name).touch()
    guessed_strategy = StructuredLogger.guess_strategy_from_file(file_name)
    assert isinstance(guessed_strategy, strategy)


def test_get_strategy(strategy, file_extension):
    result = StructuredLogger.get_strategy(file_extension)
    assert isinstance(result, strategy)


def test_invalid_file_extension(tmp_path):
    file_name = tmp_path / "test_invalid.xyz"
    Path(file_name).touch()

    with pytest.raises(ValueError, match="Unknown strategy for: xyz"):
        StructuredLogger.guess_strategy_from_file(file_name)


def test_non_existent_file(tmp_path, strategy, file_extension):
    file_name = tmp_path / f"non_existent.{file_extension}"
    logger = StructuredLogger(file_name, strategy())

    assert logger.file_path.exists()
    assert "_timestamp" in logger._data


@pytest.fixture
def structured_logger_module():
    with patch.dict("sys.modules", {"polars": None, "ruamel.yaml": None}):
        import importlib

        import structured_logger

        importlib.reload(structured_logger)
        yield structured_logger


def test_missing_csv_library(tmp_path, structured_logger_module):
    with pytest.raises(RuntimeError, match="Can't parse CSV files. Missing library"):
        structured_logger_module.CSVStrategy()


def test_missing_yaml_library(tmp_path, structured_logger_module):
    with pytest.raises(RuntimeError, match="Can't parse YAML files. Missing library"):
        structured_logger_module.YAMLStrategy()


def test_autosavedict_setitem():
    save_callback = MagicMock()
    d = AutoSaveDict(save_callback=save_callback)
    d["key"] = "value"
    assert d["key"] == "value"
    save_callback.assert_called_once()


def test_autosavedict_delitem():
    save_callback = MagicMock()
    d = AutoSaveDict({"key": "value"}, save_callback=save_callback)
    del d["key"]
    assert "key" not in d
    save_callback.assert_called_once()


def test_autosavedict_pop():
    save_callback = MagicMock()
    d = AutoSaveDict({"key": "value"}, save_callback=save_callback)
    result = d.pop("key")
    assert result == "value"
    assert "key" not in d
    save_callback.assert_called_once()


def test_autosavedict_update():
    save_callback = MagicMock()
    d = AutoSaveDict({"key": "old_value"}, save_callback=save_callback)
    d.update({"key": "new_value"})
    assert d["key"] == "new_value"
    save_callback.assert_called_once()


def test_structured_logger_setitem(tmp_file):
    logger = StructuredLogger(tmp_file, JSONStrategy())
    logger.data["field"] = "value"

    with open(tmp_file, "r") as f:
        data = json.load(f)

    assert data["field"] == "value"


def test_structured_logger_set_recursive(tmp_file):
    logger = StructuredLogger(tmp_file, JSONStrategy())
    logger.data["field"] = {"test": True}
    other = logger.data["field"]
    other["late"] = True

    with open(tmp_file, "r") as f:
        data = json.load(f)

    assert data["field"]["test"]
    assert data["field"]["late"]


def test_structured_logger_set_list(tmp_file):
    logger = StructuredLogger(tmp_file, JSONStrategy())
    logger.data["field"] = [True]
    other = logger.data["field"]
    other.append(True)

    with open(tmp_file, "r") as f:
        data = json.load(f)

    assert data["field"][0]
    assert data["field"][1]


def test_structured_logger_delitem(tmp_file):
    logger = StructuredLogger(tmp_file, JSONStrategy())
    logger.data["field"] = "value"
    del logger.data["field"]

    with open(tmp_file, "r") as f:
        data = json.load(f)

    assert "field" not in data


def test_structured_logger_pop(tmp_file):
    logger = StructuredLogger(tmp_file, JSONStrategy())
    logger.data["field"] = "value"
    logger.data.pop("field")

    with open(tmp_file, "r") as f:
        data = json.load(f)

    assert "field" not in data


def test_structured_logger_update(tmp_file):
    logger = StructuredLogger(tmp_file, JSONStrategy())
    logger.data.update({"field": "value"})

    with open(tmp_file, "r") as f:
        data = json.load(f)

    assert data["field"] == "value"
