"""
Test suite for libFRX's SDCard CLI features.
Tests the jescore CLI commands listed in libFRX/src/sdcard_demo.cpp.

Note: These tests require a live MCU running the sdcard_demo.cpp app.
"""

import pytest
import re
from jescorecli.jescorecli import CjescoreCli
from sdcard_strings import *


SDCARD_TEST_FILE = "cli_test.txt"
SDCARD_ERROR_PATTERNS = [
    SDCARD_MSG_MOUNT_FAIL,
    SDCARD_MSG_UNMOUNT_FAIL,
    SDCARD_MSG_LS_ERROR_errnum,
    SDCARD_MSG_CAT_ERROR_errnum,
    SDCARD_MSG_MK_ERROR_errnum,
    SDCARD_MSG_RM_ERROR_errnum,
    SDCARD_MSG_MEM_ERROR_errnum,
    SDCARD_MSG_UNKNOWN_CMD,
    r"\bsdmmc_",
    r"\bdiskio_",
    r"\bfailed\b",
    r"\bfailed \(",
    r"\bError\b",
]


def assert_response(result):
    assert result is not None
    assert len(result) > 0
    return result


def assert_no_sdcard_error(result):
    result = assert_response(result)
    joined = "\n".join(result)
    for pattern in SDCARD_ERROR_PATTERNS:
        assert not re.search(pattern, joined, re.IGNORECASE), joined
    return result


def assert_has(result, pattern):
    assert any(re.search(pattern, s) for s in result), result


class TestSDCardCLI:
    @pytest.fixture
    def cli(self):
        return CjescoreCli(port=CjescoreCli.portAutoDetect(), verbose=False)

    def test_mnt(self, cli):
        result = assert_no_sdcard_error(
            cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_MOUNT}")
        )
        assert_has(result, SDCARD_MSG_MOUNTED)

    def test_help(self, cli):
        result = assert_no_sdcard_error(
            cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_HELP}")
        )
        assert_has(result, SDCARD_CMDS)

    def test_mem(self, cli):
        result = assert_no_sdcard_error(
            cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_MEMORY}")
        )
        assert_has(result, SDCARD_MSG_MEM_FORMAT_free_tot)

    def test_ls(self, cli):
        assert_no_sdcard_error(cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_LIST}"))

    def test_file_lifecycle(self, cli):
        # Cleanup from a previous interrupted run is allowed to fail when the file is absent.
        cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_REMOVE} {SDCARD_TEST_FILE}")

        assert_no_sdcard_error(
            cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_CREATE} {SDCARD_TEST_FILE}")
        )
        assert_no_sdcard_error(
            cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_READ} {SDCARD_TEST_FILE}")
        )
        assert_no_sdcard_error(cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_LIST}"))
        assert_no_sdcard_error(
            cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_REMOVE} {SDCARD_TEST_FILE}")
        )

    def test_unmnt(self, cli):
        result = assert_no_sdcard_error(
            cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_UNMOUNT}")
        )
        assert_has(result, SDCARD_MSG_UNMOUNTED)


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
