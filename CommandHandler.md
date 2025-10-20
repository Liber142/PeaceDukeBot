Обработчик slash-команд бота.

```cpp
class CommandHandler
{
public:
	CommandHandler(dpp::cluster& bot);

	void Hui(DataBase* db);
	~CommandHandler();
	void RegisterCommands();
	bool HandleCommands(const dpp::slashcommand_t& event);
private:
	dpp::cluster& bot;
	std::unordered_map<std::string, std::unique_ptr<ICommand>> commands;
};
```