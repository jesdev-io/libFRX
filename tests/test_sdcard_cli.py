"""
Test suite for libFRX's SDCard CLI features.
Tests the jescore CLI commands listed in libFRX/src/sdcard_demo.cpp.

Note: These tests require a live MCU running the sdcard_demo.cpp app.
"""

import pytest
import re
from jescorecli.jescorecli import CjescoreCli
from sdcard_strings import *


class TestSDCardCLI:
    @pytest.fixture
    def cli(self):
        return CjescoreCli(port=CjescoreCli.portAutoDetect(),verbose=False)

    def test_mnt(self, cli):
        result = cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_MOUNT}")
        assert result is not None
        assert any(re.search(SDCARD_MSG_MOUNTED, s) for s in result) or any(re.search(SDCARD_MSG_MOUNT_FAIL, s) for s in result)

    def test_unmnt(self, cli):
        result = cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_UNMOUNT}")
        assert result is not None
        assert any(re.search(SDCARD_MSG_UNMOUNTED, s) for s in result) or any(re.search(SDCARD_MSG_UNMOUNT_FAIL, s) for s in result)
        cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_MOUNT}") # remount for subsquent tests

    def test_help(self, cli):
        result = cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_HELP}")
        assert result is not None
        assert any(re.search(SDCARD_CMDS, s) for s in result)

    def test_ls(self, cli):
        result = cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_LIST}")
        assert result is not None

    def test_cat(self, cli):
        result = cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_READ} test.txt")
        assert result is not None

    def test_mk(self, cli):
        result = cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_CREATE} test.txt")
        assert result is not None

    def test_rm(self, cli):
        result = cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_REMOVE} test.txt")
        assert result is not None

    def test_mem(self, cli):
        result = cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_MEMORY}")
        assert result is not None
        assert any(re.search(SDCARD_MSG_MEM_FORMAT_free_tot, s) for s in result) or any(re.search(SDCARD_MSG_MEM_ERROR_errnum, s) for s in result)

    def test_cleanup(self, cli):
        cli.uartTransceive(f"{SDCARD_JOB_NAME} {SDCARD_CMD_UNMOUNT}")

if __name__ == "__main__":
    pytest.main([__file__, "-v"])
