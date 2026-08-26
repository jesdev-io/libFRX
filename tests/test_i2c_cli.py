"""
Test suite for libFRX's I2C base CLI features.
Tests the jescore CLI command listed in libFRX/src/i2c_demo.cpp.

Note: These tests require a live MCU running the i2c_demo.cpp app.
"""

import os
import re

import pytest
from jescorecli.jescorecli import CjescoreCli
from i2c_base_strings import *


I2C_SCAN_FOUND_RE = r"Found \d+ devices:"
I2C_SCAN_ADDR_RE = r"@ 0x[0-9A-Fa-f]+"


class TestI2CCLI:
    @pytest.fixture
    def cli(self):
        return CjescoreCli(port=os.environ.get("JESCORE_CLI_PORT") or CjescoreCli.portAutoDetect(), verbose=False)

    def test_scan(self, cli):
        result = cli.uartTransceive(I2C_BASE_SCAN_JOB_NAME)
        assert result is not None
        assert any(re.search(I2C_BASE_MSG_SCAN_START, s) for s in result)
        assert any(re.search(I2C_SCAN_FOUND_RE, s) for s in result)

        found_line = next(s for s in result if re.search(I2C_SCAN_FOUND_RE, s))
        device_count = int(re.search(r"Found (\d+) devices:", found_line).group(1))
        addresses = [s for s in result if re.search(I2C_SCAN_ADDR_RE, s)]
        assert len(addresses) == device_count


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
