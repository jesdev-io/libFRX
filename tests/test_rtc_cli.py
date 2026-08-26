"""
Test suite for libFRX's DS3231 RTC CLI features.
Tests the jescore CLI commands listed in libFRX/src/rtc_demo.cpp.

Note: These tests require a live MCU running the rtc_demo.cpp app.
"""

import os
import re

import pytest
from jescorecli.jescorecli import CjescoreCli
from i2c_rtc_ds3231_strings import *


RTC_TIME_RE = r"\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}"
RTC_TEMP_RE = r"-?\d+(?:\.\d+)? C"


class TestRTCCLI:
    @pytest.fixture
    def cli(self):
        return CjescoreCli(port=os.environ.get("JESCORE_CLI_PORT") or CjescoreCli.portAutoDetect(), verbose=False)

    def test_help(self, cli):
        result = cli.uartTransceive(f"{DS3231_JOB_NAME} {DS3231_CMD_HELP}")
        assert result is not None
        assert any(re.search(DS3231_CMDS, s) for s in result)
        assert any(DS3231_CMD_TIME in s for s in result)
        assert any(DS3231_CMD_TEMP in s for s in result)

    def test_time(self, cli):
        result = cli.uartTransceive(f"{DS3231_JOB_NAME} {DS3231_CMD_TIME}")
        assert result is not None
        assert any(re.search(RTC_TIME_RE, s) for s in result) or any(
            re.search(DS3231_MSG_ERROR_TIME_errnum, s) for s in result
        )

    def test_temp(self, cli):
        result = cli.uartTransceive(f"{DS3231_JOB_NAME} {DS3231_CMD_TEMP}")
        assert result is not None
        assert any(re.search(RTC_TEMP_RE, s) for s in result) or any(
            re.search(DS3231_MSG_ERROR_TEMP_errnum, s) for s in result
        )

    def test_unknown_command(self, cli):
        result = cli.uartTransceive(f"{DS3231_JOB_NAME} unknown")
        assert result is not None
        assert any(re.search(DS3231_MSG_UNKNOWN_CMD, s) for s in result)


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
