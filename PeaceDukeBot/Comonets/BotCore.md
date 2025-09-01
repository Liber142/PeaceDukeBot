Центральный класс, отвечает за инициализацию и управление бота.

```C++
class BotCore
{
private:
    DataBase* v_db;
    DataBase* m_db;

	dpp::cluster bot;

	void SetupEvent();
	void RegisterSlashCommands();
	void RegisterButton();

	CommandHandler cmdHandler;
	C_OnlineClanMember OnlineClanMember;
public:
	BotCore(std::string& token);
	void StartDataBase(std::string v_filepath, std::string m_filepath, std::string AplicationAceptedMessage, std::string AplicationRejectedMessage);
	void Start();
};
```