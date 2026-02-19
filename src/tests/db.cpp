#include <engine/json_database.h>

#include <cstdlib>
#include <thread>
#include <csignal>

static std::atomic<bool> g_Work(true);

static void SignalHandler(int Sig)
{
	g_Work = false;
#ifdef _WIN32
	_close(0);
#else
	close(0);
#endif
}

struct CTest
{
	int m_Int;
	std::string m_String;
	size_t m_Id;
	std::vector<int> m_vVector;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE(CTest, m_Int, m_Id, m_String, m_vVector);
};

std::string Table = "test";

static void WriteSomeData(IDataBase* DataBase)
{
	CTest Data;
	Data.m_Int = std::rand();
	Data.m_String = std::to_string(std::rand());
	Data.m_Id = DataBase->GenerateNewKey(Table);
	int Nums = std::rand() % 50 + 1;
	for(int i = 0; i < Nums; i++)
	{
		Data.m_vVector.emplace_back(std::rand());
	}

	if(DataBase)
		DataBase->Save(Table, Data.m_Id, Data);
}

int main()
{
	std::signal(SIGINT, SignalHandler);
	std::srand(std::time(NULL));
	CJsonDataBase DataBase;
	DataBase.Connect("test");
	for(int a = 0; a < 50 && g_Work; a++)
	{
		std::vector<size_t> Keys = DataBase.GetKeys(Table);
		size_t Nums = Keys.size();

		for(int i = 0; i < 20; i++)
		{
			std::jthread T1(WriteSomeData, &DataBase);
			std::jthread T2(WriteSomeData, &DataBase);
			std::jthread T3(WriteSomeData, &DataBase);
			std::jthread T4(WriteSomeData, &DataBase);
			std::jthread T5(WriteSomeData, &DataBase);
			std::jthread T6(WriteSomeData, &DataBase);
			std::jthread T7(WriteSomeData, &DataBase);
			std::jthread T8(WriteSomeData, &DataBase);
		}

		Keys = DataBase.GetKeys(Table);
		CLogger::Info("main", "Write elements: " + std::to_string(Keys.size() - Nums));
	}
	std::vector<size_t> Keys = DataBase.GetKeys(Table);
	CLogger::Info("main", "Total elements: " + std::to_string(Keys.size()));
}
