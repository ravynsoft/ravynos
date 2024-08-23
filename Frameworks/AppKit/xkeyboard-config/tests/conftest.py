import os
import sys
from pathlib import Path

tests_dir = Path(__file__).parent.resolve()
sys.path.insert(0, str(tests_dir))

try:
    import xdist  # noqa: F401

    # Otherwise we get unknown hook 'pytest_xdist_auto_num_workers'
    def pytest_xdist_auto_num_workers(config):
        return os.getenv("FDO_CI_CONCURRENT", None)

except ImportError:
    pass
