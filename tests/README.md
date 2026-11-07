# Автономные тесты PeaceDukeBot

Требования: Linux, GCC 14 (C++20 с `<format>`), CMake ≥3.22, Python ≥3.9,
потоки POSIX. Для покрытия — gcov той же версии, что GCC, и gcovr 8.8.
Discord, VPN, токены и рабочие данные не нужны. При первой конфигурации CMake
загружает doctest и nlohmann/json через FetchContent с фиксированными commit SHA.
После загрузки сборка и выполнение тестов не требуют сети.

```sh
python3 tests/run.py
```

Команда выполняет четыре отдельных сборки: обычную с coverage, ASan, UBSan,
TSan. Продолжает после падений, сохраняет все результаты и возвращает ненулевой
код, если есть ошибки. Это ожидается для текущего приложения: дефекты не
исправлены и не скрыты. Быстрый запуск: `python3 tests/run.py --mode normal`.

```sh
cmake -S . -B build/tests-manual -DPEACE_TESTS_ONLY=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/tests-manual -j4
ctest --test-dir build/tests-manual --output-on-failure
ctest --test-dir build/tests-manual -R 'regression.database.large_key' --output-on-failure
```

Можно также собирать отдельный проект `cmake -S tests -B build/tests-manual`.
Сборка production по умолчанию не меняется. Тестовый режим не запускает поиск
DPP, cpr, replxx, TBB, mpg123. Зависимости тестов хранятся в build, вне исходников.
Для полностью автономной первой сборки можно задать локальные checkout:
`-DFETCHCONTENT_SOURCE_DIR_DOCTEST=/path/to/doctest` и
`-DFETCHCONTENT_SOURCE_DIR_NLOHMANN_JSON=/path/to/json`.

## Что именно проверяется

doctest 2.4.11 запускает **настоящие исходники** engine, bot_core и всех modules;
CLI и мигратор компилируются из оригинальных файлов. Тестовые DPP/cpr/mpg123/
replxx-заголовки стоят раньше include-путей приложения. Они записывают исходящие
операции и подают события/ответы, но не реализуют бизнес-логику. JSON — настоящая
nlohmann/json 3.11.3, не заглушка. `-fno-access-control` только в тестовой сборке
позволяет проверять приватные помощники без редактирования заголовков/раскладки
классов. Совместимость с настоящим SDK этой сборкой не доказывается.

Каждый CTest запускает один doctest case в отдельном процессе, с timeout 15 с.
SUBCASE и таблицы входов расширяют один case и не считаются отдельными CTest.
Все записи выполняются во временных каталогах `mkdtemp`/TemporaryDirectory;
`data/` пользователя не используется. Тесты запускаются параллельно только
разными процессами. Проверки потоков создают несколько потоков внутри case.

`build/tests-<mode>/results.xml` — результат CTest; `ctest.log` — диагностика;
`build/summary-*.json` — сводка; `coverage.html`, `coverage.json`,
`coverage-summary.json` — покрытие. Удаляются только старые `.gcda` выбранной
сборки. Повторные прогоны не суммируют устаревшие счётчики.

Покрытие ограничено `src/`, исключает старый ручной `src/tests/db.cpp`, тесты,
моки, библиотеки и vendor. gcovr исключает compiler-unreachable и exception-only
ветви (`--exclude-unreachable-branches --exclude-throw-branches`); это не полный
процент всех ветвей компилятора. Из-за abort/segfault счётчики отдельного процесса
могут не записаться. Отчёт не подменяет проверки поведения процентом.

## Зависимости тестов

- doctest v2.4.11, commit `ae7a13539fb71f270b87eb2e874fbac80bc8dda2`,
  https://github.com/doctest/doctest, MIT (лицензия в загруженном проекте).
- nlohmann/json v3.11.3, commit `9cca280a4d0ccf0c08f47a99aa71d1b0e52f8d03`,
  https://github.com/nlohmann/json, MIT (лицензия в загруженном проекте).

CMake получает зависимости через HTTPS с проверкой TLS и фиксированными SHA.
Полные библиотеки в репозитории тестов не хранятся. CI загружает зависимости
при конфигурации, затем выполняет автономный набор тестов.

Подробный перечень проверок, ошибок и ограничений — [REPORT.md](REPORT.md).

Для каждого падения runner печатает файл/функцию, условия возникновения, ожидаемый
результат, известное проявление, рекомендацию и точную команду воспроизведения.
Эти пояснения также сохраняются в `build/tests-<mode>/diagnostics.md` и artifacts CI.
Каталог объяснений — `tests/failures.json`; исходные логи санитайзеров сохраняются
без изменений. Для новой неизвестной ошибки причина явно отмечается как неустановленная.
