PeaceDukeBot - это многофункциональный Discord-бот для управления игровым кланом, написанный на C++ с использованием библиотеки DPP. Бот предоставляет функциональность для подачи заявок в клан, модерации, отслеживания онлайн-статуса участников и организации событий.

## Архитектура

[[Компонентная диаграмма.canvas|Компонентная диаграмма]]

### Ключевые компоненты

1. [[BotCore]]
2. [[CommandHandler]]
3. [[DataBase]]
   
## Установка и запуск

### Требования

- Компилятор C++ с поддержкой C++17
    
- CMake 3.12+
    
- Библиотека DPP
    
- Библиотека nlohmann/json
    
- Библиотека libcurl
### Установка DPP
```bash
git clone https://github.com/brainboxdotcc/DPP
cd DPP
cmake -B build .
cmake --build build -j4
sudo cmake --install build
```

### Сборка проекта

```bash
git clone https://github.com/Liber142/PeaceDukeBot.git
cd PeaceDukeBot|
mkdir build
cd build
cmake ..
make -j4
```

### Настройте конфигурацию

Создайте файл *config.json* в корневой директории
```json
{
    "token": "YOUR_DISCORD_BOT_TOKEN",
    "guild_id": "YOUR_GUILD_ID",
    "channel_moderation_id": "MODERATION_CHANNEL_ID",
    "channel_online_members_id": "ONLINE_MEMBERS_CHANNEL_ID",
    "default_role_id": "DEFAULT_ROLE_ID",
    "clan_role_id": "CLAN_ROLE_ID",
    "aplication_accepted_message": "Сообщение при принятии заявки",
    "aplication_rejected_message": "Сообщение при отклонении заявки"
}
```

## Конфигурация

### Файлы конфигурации

1. **config.json** - основные настройки бота
    
2.  **members_data.json** - база данных участников
    
3. **active_votes.json** - активные голосования
   
   
## Команды бота

### Список команд

1. **/apply** - Создать сообщение с кнопкой на подачу заявки
    
2. **/event** - Создаст сообщение с кнопкой для подачи заявки
    
3. **/invite** - не завершена
    
4. **/profile** - Просмотреть профиль участника
   
### Добавление новой команды

1. Создайте класс команды в `include/Commands/`:
    

```cpp
#pragma once
#include "../../ICommand.h"

class NewCommand : public ICommand {
public:
    NewCommand(dpp::cluster& bot_instance);
    void Execute(const dpp::slashcommand_t& event) override;
    dpp::slashcommand Register() override;
};
```

2. Реализуйте команду в `src/Commands/`:
    

```cpp

#include "../../include/Commands/NewCommand.h"

NewCommand::NewCommand(dpp::cluster& bot_instance) : bot(bot_instance) {}

void NewCommand::Execute(const dpp::slashcommand_t& event) {
    // Реализация команды
}

dpp::slashcommand NewCommand::Register() {
    return dpp::slashcommand("newcommand", "Описание команды", bot.me.id);
}
```
3. Зарегистрируйте команду в CommandHandler:
    

```cpp

CommandHandler::CommandHandler(dpp::cluster& bot) : bot(bot) {
    commands["apply"] = std::make_unique<ApplyCommand>(bot);
    commands["event"] = std::make_unique<EventCommand>(bot);
    commands["invite"] = std::make_unique<invite>(bot);
    commands["profile"] = std::make_unique<ProfileCommand>(bot);
    commands["newcommand"] = std::make_unique<NewCommand>(bot); // Добавьте эту строку
}
```

