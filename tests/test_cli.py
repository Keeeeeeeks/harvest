import hashlib
import json

import pytest

from hv import builds, cli


@pytest.fixture
def pinned(tmp_path, monkeypatch):
    """A builds.json with one present image, pointed at a temporary orig/."""
    payload = b"harvest"
    (tmp_path / "orig" / "1.0-test").mkdir(parents=True)
    (tmp_path / "orig" / "1.0-test" / "Game").write_bytes(payload)
    spec = {
        "canonical": "1.0-test",
        "builds": {
            "1.0-test": {
                "version": "1.0",
                "platform": "linux",
                "arch": "amd64",
                "role": "target",
                "compiler": "test",
                "images": {"Game": {"size": len(payload), "sha256": hashlib.sha256(payload).hexdigest()}},
            }
        },
    }
    (tmp_path / "builds.json").write_text(json.dumps(spec))
    monkeypatch.setattr(builds, "BUILDS_JSON", tmp_path / "builds.json")
    monkeypatch.setattr(builds, "ORIG", tmp_path / "orig")
    return tmp_path


def test_verify_checks_all_builds(pinned, capsys):
    assert cli.main(["verify"]) == 0
    assert "1.0-test" in capsys.readouterr().out


def test_verify_rejects_only_unknown_builds(pinned, capsys):
    assert cli.main(["verify", "typo"]) == 2
    captured = capsys.readouterr()
    assert captured.out == ""
    assert "unknown builds: typo" in captured.err


def test_verify_rejects_mixed_known_and_unknown(pinned, capsys):
    assert cli.main(["verify", "1.0-test", "typo"]) == 2
    assert capsys.readouterr().out == ""


def test_verify_reports_mismatch(pinned, capsys):
    (pinned / "orig" / "1.0-test" / "Game").write_bytes(b"tampered")
    assert cli.main(["verify", "1.0-test"]) == 1
    assert "size" in capsys.readouterr().out
