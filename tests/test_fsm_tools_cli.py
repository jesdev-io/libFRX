"""CLI tests for the minimal fsm_tools demo."""

import os

import pytest
from jescorecli.jescorecli import CjescoreCli

JOB = "fsm_demo"


def text(lines):
    assert lines is not None
    return "\n".join(lines)


class TestFsmToolsDemoCLI:
    @pytest.fixture
    def cli(self):
        return CjescoreCli(port=os.environ.get("JESCORE_CLI_PORT") or CjescoreCli.portAutoDetect(), verbose=False)

    def test_help(self, cli):
        assert "state, on, off, worker, status" in text(cli.uartTransceive(f"{JOB} help"))

    def test_control_transition_and_worker(self, cli):
        assert "transition 13" in text(cli.uartTransceive(f"{JOB} on"))
        assert "state 1" in text(cli.uartTransceive(f"{JOB} state"))
        out = text(cli.uartTransceive(f"{JOB} worker"))
        assert "worker state=1 requested_on=1" in out
        assert "worker_ret 13" in out

    def test_status_slot(self, cli):
        out = text(cli.uartTransceive(f"{JOB} status"))
        assert "control_on 1" in out
        assert "status_runs" in out
        assert "status_state 1" in out
