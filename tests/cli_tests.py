"""Isolated executable integration checks; no shell, network, tokens or user files."""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

kind, binary = sys.argv[1:]
with tempfile.TemporaryDirectory(prefix="peace-cli-") as d:
    env = dict(os.environ)
    env.pop("DISCORD_TOKEN", None)
    Path(d, "data").mkdir()
    args = [binary]
    if kind == "converter_roundtrip":
        old = {"": {"42": {"about": "old", "age": "25", "game_nick": "nick", "clan": "clan", "social_rating": 7}}}
        Path(d, "old.json").write_text(json.dumps(old))
        args += ["old.json", "new.json"]
    elif kind == "converter_invalid_age":
        old = {"": {"42": {"about": "", "age": "invalid", "game_nick": "nick", "clan": "", "social_rating": 0}}}
        Path(d, "old.json").write_text(json.dumps(old))
        args += ["old.json", "new.json"]
    elif kind == "main_offline_start":
        # Consumed only by our in-process cluster double, never sent anywhere.
        env["DISCORD_TOKEN"] = "offline-test-double"
    p = subprocess.run(args, cwd=d, env=env, capture_output=True, text=True, timeout=10)
    print(p.stdout, end="")
    print(p.stderr, end="", file=sys.stderr)
    if kind == "main_missing_token":
        assert p.returncode == 1 and "Missing env DISCORD_TOKEN" in p.stderr
    elif kind == "main_offline_start":
        assert p.returncode == 0, f"startup exit={p.returncode}"
        assert "ready!" in p.stdout
    elif kind == "converter_missing_args":
        assert p.returncode == 1 and "Missing any files" in p.stderr
    elif kind == "converter_invalid_age":
        assert p.returncode in (1, 2), f"expected graceful error, exit={p.returncode}"
    elif kind == "legacy_db_stress":
        assert p.returncode == 0, f"legacy stress exit={p.returncode}"
        stored = json.loads(Path(d, "test.json").read_text())["test"]
        stored.pop("_meta", None)
        assert len(stored) == 8000, f"expected 8000 rows, got {len(stored)}"
        for key, row in stored.items():
            assert row["m_Id"] == int(key)
            assert 1 <= len(row["m_vVector"]) <= 50
    else:
        assert p.returncode == 0, f"migration exit={p.returncode}"
        migrated = json.loads(Path(d, "new.json").read_text())["clan_members"]["42"]
        assert migrated["m_GameNick"] == "nick" and migrated["m_Age"] == 25
