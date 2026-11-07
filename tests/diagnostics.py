"""Readable failure explanations; raw CTest/sanitizer evidence remains untouched."""
import json
from pathlib import Path


def explain_failures(root, build, mode, failed):
    catalog = {entry["test"]: entry for entry in json.loads((root / "tests/failures.json").read_text())}
    lines = [f"# Диагностика: {mode}", "",
             "Рекомендации ниже не применялись к исходникам приложения.",
             "Фактические сообщения текущего запуска: ctest.log и results.xml рядом с этим файлом.", ""]
    for name in failed:
        lines.extend([f"## {name}", ""])
        entry = catalog.get(name)
        if entry:
            lines.extend([
                f"Проблемный участок: `{entry['source']}` — {entry['function']}.",
                f"Условия: {entry['conditions']}.",
                f"Ожидалось: {entry['expected']}.",
                f"Известное проявление: {entry['actual']}.",
                f"Что можно сделать: {entry['recommendation']}",
            ])
        else:
            lines.append("Новая ошибка без готового объяснения. Проверьте assertion/stack trace в ctest.log; причина пока не установлена.")
        lines.extend(["", "Воспроизведение:", "```sh",
                      f"ctest --test-dir {build.relative_to(root)} -R '^{name.replace('.', '[.]')}$' --output-on-failure",
                      "```", ""])
    if not failed:
        lines.append("В этом запуске падений не зарегистрировано. Это не исключает недетерминированные гонки и UB.")
    result = "\n".join(lines) + "\n"
    (build / "diagnostics.md").write_text(result)
    return result
