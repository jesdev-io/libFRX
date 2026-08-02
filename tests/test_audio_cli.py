"""
Test suite for libFRX's Audio CLI features.
Tests the jescore CLI commands listed in libFRX/src/audio_demo.cpp.

Note: These tests require a live MCU running the audio_demo.cpp app.
"""

import re

import pytest
from jescorecli.jescorecli import CjescoreCli
from audio_strings import *


AUDIO_STATUS_RE = (
    r"Audio status: running=\d+ sr=\d+ bps=\d+ "
    r"gain=\d+/1000 nch=\d+ banks=\d+\."
)


class TestAudioCLI:
    @pytest.fixture
    def cli(self):
        return CjescoreCli(port=CjescoreCli.portAutoDetect(), verbose=False)

    def test_status(self, cli):
        result = cli.uartTransceive(f"{AUDIO_CONTROL_JOB_NAME} {AUDIO_CMD_STATUS}")
        assert result is not None
        assert any(re.search(AUDIO_STATUS_RE, s) for s in result)

    def test_volume(self, cli):
        result = cli.uartTransceive(f"{AUDIO_CONTROL_JOB_NAME} {AUDIO_CMD_VOLUME} 0.5")
        assert result is not None
        assert any(re.search(AUDIO_MSG_VOLUME, s) for s in result)

    def test_mute(self, cli):
        result = cli.uartTransceive(f"{AUDIO_CONTROL_JOB_NAME} {AUDIO_CMD_MUTE}")
        assert result is not None
        assert any(re.search(AUDIO_MSG_VOLUME, s) for s in result)

    def test_usage_without_command(self, cli):
        result = cli.uartTransceive(AUDIO_CONTROL_JOB_NAME)
        assert result is not None
        assert any(re.search(AUDIO_MSG_ERROR_USAGE, s) for s in result)
        assert any(re.search(AUDIO_CMDS, s) for s in result)

    def test_unknown_command(self, cli):
        result = cli.uartTransceive(f"{AUDIO_CONTROL_JOB_NAME} unknown")
        assert result is not None
        assert any(re.search(AUDIO_MSG_UNKNOWN_CMD, s) for s in result)
        assert any(re.search(AUDIO_CMDS, s) for s in result)

    def test_stop_restart(self, cli):
        result = cli.uartTransceive(f"{AUDIO_CONTROL_JOB_NAME} {AUDIO_CMD_STOP}")
        assert result is not None
        assert any(re.search(AUDIO_MSG_STOPPED, s) for s in result) or any(
            re.search(AUDIO_MSG_OFFLINE, s) for s in result
        )

        result = cli.uartTransceive(f"{AUDIO_CONTROL_JOB_NAME} {AUDIO_CMD_RESTART}")
        assert result is not None
        assert any(re.search(AUDIO_MSG_RESTARTED, s) for s in result)


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
