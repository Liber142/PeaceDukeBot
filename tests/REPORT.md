# Отчёт о тестировании PeaceDukeBot

Исходная ревизия: `ae07396f624d39fd84b00a0764ddf0a709c9306f`.
Дата: 2026-11-06. ОС Linux x86_64, GCC/gcov 14.2.0, CMake 4.4.4,
doctest 2.4.11, nlohmann/json 3.11.3, gcovr 8.8, C++20, Debug.
Изменений в `src/` нет. Единственное изменение существующего файла —
опция `PEACE_TESTS_ONLY` в CMake, не затрагивающая обычную сборку приложения.

## Итог последнего полного прогона

Команда: `python3 tests/run.py`. Все четыре сборки завершились; каждый режим
выполнил 65 CTest (59 doctest cases + 6 CLI-проверок). Пропусков нет.

| Режим | Всего | Прошло | Провалилось |
|---|---:|---:|---:|
| normal | 65 | 43 | 22 |
| address | 65 | 42 | 23 |
| undefined | 65 | 43 | 22 |
| thread | 65 | 44 | 21 |

Покрытие: **92,3% строк (1032/1118), 83,5% ветвей (1203/1441), 91,8% функций
(179/195)**. Это метрики instrumented production sources, включая оригинальные
main и мигратор, с исключением `src/tests/db.cpp`, vendor и doubles. Ветви
compiler-unreachable и exception-only исключены явно, как описано в README.

Найдены **23 дефектных сценария**: 22 упали в обычном прогоне и ещё один
(`members.deferred_callback_lifetime`) выявлен ASan. Количество проходов
различается из-за UB/гонок: например, shutdown_flush прошёл под TSan, но упал
в остальных режимах; это не исправление и не основание считать дефект закрытым.

ASan (с включённым LeakSanitizer, вне ptrace) подтвердил stack-buffer-overflow
в ParseLine, alloc-dealloc-mismatch в DecodeMP3, heap-use-after-free при
конкурентной работе БД, stack-use-after-scope в callback участника. UBSan:
signed integer overflow в GenerateNewKey. TSan: data race при удалении/перечислении
ключей БД; также выход за границы стека парсера. Диагностики не подавлялись.

Сохранённые доказательства: [summary.json](results/summary.json),
[coverage-summary.json](results/coverage-summary.json),
[normal log](results/normal/ctest.log), [ASan log](results/address/ctest.log),
[UBSan log](results/undefined/ctest.log), [TSan log](results/thread/ctest.log).
JUnit-файлы лежат рядом с логами. HTML покрытия генерируется в build и
публикуется как artifact CI; CI в GitHub в рамках этой сессии ещё не выполнялся.

## Инвентаризация кода и сценариев

Изучены все файлы `src/`, CMake, Dockerfile, compose, cmake/FindDPP.cmake,
скрипты и настройки форматирования. Имеющийся `src/tests/db.cpp` — генератор нагрузки без assertions. Он также
собирается и запускается в sandbox: Python-оболочка проверяет код выхода,
все 8000 сохранённых записей, соответствие ID и размеры векторов.

| Объект / функции | Проверки и сценарии |
|---|---|
| CConsole::IResult, GetString/GetInt/NumArguments | строки, INT_MIN/MAX, пустая строка, отрицательные, ошибочные и переполненные числа; индексы только в пределах аргументов |
| CCommand, Register, FindCommand (обе перегрузки), GetAllCommands | метаданные, отсутствие команды, пересекающиеся/разные флаги, дубликаты с ненулевыми и нулевыми флагами |
| ParseLine через ExecuteLine | пробелы, tab, `mc;`, несколько команд, разделители, комментарии, кавычки, escape, пустые/незакрытые строки, пустой quoted argument, завершающий backslash |
| ExecuteFile | несколько строк, комментарий, отсутствие файла, последняя строка без newline |
| ExecuteInteraction | button/slash/modal, указатель события, флаги, экранирование текста формы, неподдерживаемое событие, выбор одноимённых команд по флагу |
| CConfig::OnInit и деструктор, все macro-поля | дефолты, регистрация, чтение файла, uint64 maximum, ошибочное/отрицательное число, отсутствие аргумента, сохранение, roundtrip пробелов и специальных символов |
| optional serializer, SBirthDate, SUserData | JSON roundtrip Unicode/escape/отрицательного рейтинга, отсутствующие optional, missing fields и неверные типы |
| CLogger::Log/Info/Warning/Error/Debug | уровни, stdout/stderr, сообщение и источник; точное wall-clock время не фиксируется |
| IDataBase::Save/Load | реальный JSON backend, отсутствующие/пустые/null записи, неверный тип, перезапись и JSON exceptions |
| CJsonDataBase::Connect/ReadRaw/WriteRaw/Erase/GetKeys | новый/существующий/повреждённый файл, неверный тип таблицы, некорректные ключи, удаление существующего/отсутствующего ключа, недоступный путь |
| GenerateNewKey | первая/следующая запись, отдельные таблицы, `_meta`, восстановление счётчика, INT_MAX, конкурентная уникальность, ReadOnly |
| WorkerLoop/Sync/деструктор | периодическая запись и reopen, отсутствие Connect, read-only сохранность файла, missing ReadOnly файл, shutdown flush, многопоточные Save/Load/Erase/GetKeys |
| CBotCore ctor/Init/ConsoleInit/accessors/dtor | создание модулей, регистрация команд/событий, ready, exec файла и lifecycle в sandbox |
| IModule, ICommand, CCommandHandler | привязка core, доступ к зависимостям, распространение OnInit/OnConsoleInit, имена; абстрактные интерфейсы через реализации |
| CMemberManager | синхронизация модераторов, ошибка API, назначение роли новому участнику, повышение/снятие роли, повторное событие, поиск/число модераторов |
| CClanMemberManager::AddClanMember/ChangeRole/ChangeNick | роль, снятие дефолтной роли, nickname, сохранение клана, API errors, отложенный callback |
| CApplyCommand::OnInit/OnConsoleInit/Execute/Message/Modal | нет core, регистрация, slash/button dispatch, текст и ID кнопки, поля и ограничения формы, вызов через консоль |
| CProfileCommand все методы | self/explicit/empty/missing пользователь, missing DB row, неверный event, консоль invalid/missing/found ID, optional clan/about/age, embed и дата |
| CFindTas::Player/GetPlayer/from_json | HTTP запрос/параметры, cache hit, deleted ranks, missing fields, неверные типы, malformed JSON, HTTP 500 |
| CApplyVoteManager::OnModuleInit/OnInit/OnConsoleInit | загрузка сохранённых голосований, установка ID, команды формы/кнопок/update_vote |
| ParseBirthday/CalculateAge | полная/короткая дата, pivot двухзначного года, без года, неверные день/месяц/текст, невозможные даты, trailing garbage, сегодняшний/будущий день рождения, год в будущем |
| FormSubmit/ButtonClick | неверные flags/число аргументов, malformed birthday, создание и сериализация заявки, permissions, отсутствующий vote ID, yes/no/invalid option, сохранение изменения |
| CClanVote::AddVote/FinaleVote/OnFinale | первое/повторное/изменённое голосование в обе стороны, majority acceptance, tie rejection, callback, DM, роль и сохранение участника |
| StartVote/SyncMessage/GenerateMessage/FillContainer/SetState/AddInfo/AddUrls/AddDeletedRanks/AddVotes/AddButtons | pending/accepted/declined, поля/ID компонентов, возраст/без возраста, нет user cache, >5 удалённых карт, message_edit failure с fallback create failure |
| CMusic::OnInit/OnConsoleInit/Test/DecodeMP3 | регистрация, событие неверного типа, синтетический partial decode, количество PCM байтов и управление памятью (ASan) |
| src/main.cpp | missing token, offline mock cluster startup/ready/EOF/shutdown; настоящий процесс с оригинальным main |
| src/tools/parse_old_db.cpp | missing arguments, валидная миграция/сохранность полей, некорректный возраст; отдельный CLI-процесс |
| src/tests/db.cpp | исходный многопоточный workload; wrapper проверяет 8000 записей и их структуру после завершения |
| scripts/git_revision.py | используется только тестовый git_revision.h; сам Python-скрипт не входит в C++ suite |
| scripts/fix_style.py, Docker/VPN/FindDPP | инфраструктура production, не предмет unit-тестов C++ |

## Воспроизведение ошибок

Каждый идентификатор в следующей таблице — суффикс имени CTest; точный запуск:

```sh
ctest --test-dir build/tests-normal -R 'regression.console.empty_quoted_argument' --output-on-failure
```

Префикс `regression.` подразумевается, кроме явно указанных `stress.` и
`integration.cli.`. Для санитайзера заменить `tests-normal` на `tests-address`,
`tests-undefined`, `tests-thread`. Не использовать `--should-fail`: ошибки
сохраняются как настоящие failures.

| Проверка / функция | Вход | Ожидалось | Фактически |
|---|---|---|---|
| console.empty_quoted_argument / ParseLine | `x "" tail` | два аргумента: пустой и tail | пустой аргумент потерян |
| console.trailing_escape / ParseLine | `x trailing\` | безопасный разбор/отказ | итератор увеличивается за end, UB; возможен SIGSEGV, ASan диагностирует выход за границы |
| console.dispatch_respects_flags / ExecuteInteraction | одно имя зарегистрировано для BUTTON и SLASH_COMMAND, приходит slash | slash callback | вызывается первый button callback |
| console.zero_flag_duplicate / Register | два одинаковых имени с flags=0 | одна регистрация | две команды: FindCommand проверяет побитовое пересечение с 0 |
| config.escaped_string_roundtrip / ~CConfig, OnInit | about с кавычкой, backslash, newline | исходная строка после reopen | строка повреждена: запись не экранирует значения |
| config.negative_id_rejected / numeric config callback | `role_id -1` | отказ, прежний ID | `UINT64_MAX` |
| database.unconnected_destructor / ~CJsonDataBase | создать/удалить без Connect | безопасный выход | join() незапущенного потока вызывает terminate |
| database.readonly_does_not_rewrite / WorkerLoop, Sync | существующий форматированный JSON, ReadOnly, 650 мс | байты неизменны | файл переписан компактным JSON |
| database.readonly_missing_does_not_create / Connect | отсутствующий путь, ReadOnly | файл не создан | создаётся `{}` |
| database.readonly_key_generation / GenerateNewKey | ReadOnly, новая таблица | logic_error, без изменения | metadata изменяется |
| database.shutdown_flush / деструктор | Connect, немедленно Save, destroy, reopen | запись восстановлена | запись может теряться, зависит от планирования worker |
| database.strict_numeric_keys / GetKeys | ключи `1x`, `-1`, `2` | только 2 | принимаются префикс 1 и отрицательный ключ как size_t |
| database.large_key / GenerateNewKey | next_id=2147483647, два вызова | 2147483647, 2147483648 | signed overflow, затем 18446744071562067968 на x86_64 |
| database.filename_suffix / Connect | `name.json.backup` | путь `name.json.backup.json` | проверяется подстрока `.json`, расширение не добавлено |
| votes.invalid_calendar_dates / ParseBirthday | 31.02.2000, 29.02.2001, 31.04.2000, мусор после года, bad/negative year | nullopt | принимаются невозможные даты/неполный разбор |
| votes.future_birthdate / CalculateAge | 01.01.3000 | отказ для будущей даты | отрицательный возраст |
| profile.absent_optional_target / GetTargetUser | slash без необязательного user | issuing user | bad_variant_access |
| findtas.http_error_is_exception / GetPlayer | HTTP 500 | обрабатываемое исключение | голый `throw;` вне catch вызывает terminate/SIGABRT |
| music.respects_decoded_byte_count / DecodeMP3 | mock: block=8, Done=3 | 3 PCM байта | 8 байтов; `delete` вместо `delete[]` обнаруживается ASan |
| members.deferred_callback_lifetime / ChangeNick, ChangeRole | AddClanMember возвращает до callback | корректный nickname/успех | callback хранит ссылки на уничтоженный стековый Member; ASan/ошибка nickname |
| stress.database.concurrent_enumeration_and_erase / GetKeys, Erase | Save+Erase в writer и GetKeys в другом потоке | безопасные операции, пустая таблица в конце | конкурентный доступ без общей блокировки; наблюдался SIGSEGV (недетерминированно) |
| integration.cli.converter_roundtrip / main мигратора | одна валидная старая запись | код 0, новая clan_members запись | выходная БД открыта ReadOnly; logic_error, SIGABRT |
| integration.cli.converter_invalid_age / main мигратора | age="invalid" | контролируемый ненулевой выход | необработанный invalid_argument из stoi, SIGABRT |

## Ограничения и оставшиеся сценарии

- Реальный Discord transport, rate limits, REST/voice protocol, SDK validation,
  доставка сообщений, права сервера и настоящая авторизация не проверялись:
  suite намеренно offline и использует doubles. Она не доказывает production-сборку.
- Мок DPP моделирует только используемый API, callbacks синхронные по умолчанию;
  отдельный тест использует deferred mode. Реальные сетевые гонки и порядок
  асинхронных событий exhaustively не исследованы.
- MP3 тест использует детерминированный fake mpg123; корректность настоящего
  декодера, аудиоформатов, missing file/codec error и Discord audio transport не
  подтверждена. CMusic::Download и CClanMemberManager::KickClanMember объявлены,
  но не реализованы; выполнение невозможно без изменения приложения.
- Replxx заменён EOF-заглушкой; интерактивное редактирование, completion callback,
  реальные сигналы, CKiller connection timeout/log exit не проверены.
- Не внедряются malloc/disk-full/fsync failures, аварийное выключение машины,
  различные ОС, смена часовых поясов/времени и сетевые таймауты. Age проверяется
  относительно текущей локальной даты, без жёстко прописанной даты запуска.
- Неверные индексы IResult::GetString/GetInt и неверные variants формы могут
  приводить к UB/exception; полной генеративной fuzzing-кампании не проводилось.
- Многопоточность проверяется нагрузкой 8×100 операций и конкурентным перечислением/
  удалением. Отсутствие диагноза TSan в одном прогоне не доказывает отсутствие гонок.
- На abort/segfault gcov может не сбросить counters; в частности мигратор реально
  достигает Save, но покрытие после аварии занижено. Непройденная миграция не
  считается успешно проверенным сохранением результата.
- Serialization NLOHMANN macros и templates создают несколько инстанцирований;
  проценты функций/ветвей зависят от компилятора и учитывают только указанную
  методику gcovr, не являются покрытием всех возможных пользовательских сценариев.
- Все баги оставлены в исходниках. CI намеренно красный при обнаружении ошибки;
  отсутствие исправлений соответствует условию задачи.

## Подключение зависимостей через CMake

Полные заголовки doctest и nlohmann/json удалены из Git; CMake FetchContent
загружает те же фиксированные ревизии. Первоначальная конфигурация требует
сети либо локальных исходников через FETCHCONTENT_SOURCE_DIR_*. Сами тесты
и исходники приложения не изменены. Результаты выше относятся к исходному
полному прогону; повторная проверка сборки с FetchContent описана в истории Git.

Повторный запуск после подключения FetchContent и понятной диагностики:
65 тестов, 43 прошли, 22 провалились. Различия с исходным
прогоном вызваны недетерминированными UB/гонками; исправлений приложения нет.
Санитайзеры в рамках этой правки повторно не запускались. Подробные пояснения
создаются в build/tests-normal/diagnostics.md, исходные логи сохранены рядом.
