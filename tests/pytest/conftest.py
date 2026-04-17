from pathlib import Path
import pytest

TESTS_DIR = Path(__file__).resolve().parent
TEST_MOTION_DATA_DIR = TESTS_DIR.parent / "testdata" / "motions"


@pytest.fixture(scope="session")
def test_motion_data_dir() -> Path:
    return TEST_MOTION_DATA_DIR


@pytest.fixture
def temp_vmd_path(tmp_path: Path) -> Path:
    return tmp_path / "temp_test_output.vmd"