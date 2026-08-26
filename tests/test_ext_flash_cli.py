"""
Test suite for libFRX's external flash CLI features.
Tests the jescore CLI commands listed in libFRX/src/ext_flash_demo.cpp.

Note: These tests require a live MCU running the ext_flash_demo.cpp app.
"""

import os
import re

import pytest
from jescorecli.jescorecli import CjescoreCli
from ext_flash_strings import *


EF_PID_RE = r"PID: 0x[0-9A-Fa-f]+"
EF_FW_RE = r"FW: \d+\.\d+."


class TestExtFlashCLI:
    @pytest.fixture
    def cli(self):
        return CjescoreCli(port=os.environ.get("JESCORE_CLI_PORT") or CjescoreCli.portAutoDetect(), verbose=False)

    def test_no_command(self, cli):
        result = cli.uartTransceive("ef")
        assert result is not None
        assert any(re.search(EF_MSG_ERROR_NO_CMD, s) for s in result)

    def test_rom(self, cli):
        result = cli.uartTransceive(f"ef {EF_CMD_ROM}")
        assert result is not None

        read_error = any(re.search(EF_MSG_ERROR_READ_ROM, s) for s in result) or any(
            re.search(EF_MSG_ERROR_READ_PID, s) for s in result
        )
        rom_output = (
            any(re.search(EF_PID_RE, s) for s in result)
            and any(re.search(EF_MSG_SN_FORMAT, s) for s in result)
            and any(re.search(EF_FW_RE, s) for s in result)
            and any(re.search(EF_MSG_UT_FORMAT, s) for s in result)
        )
        assert read_error or rom_output


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
