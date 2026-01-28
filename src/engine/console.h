#pragma once

#include <functional>
#include <string>
#include <vector>
#include <dpp/dpp.h>

enum : int {
    SLASH_COMMAND = 1 << 1,
};

class CConsole 
{
public: 
    class IResult 
    {
    public:
        IResult(std::string& Name) : m_Name(Name) {}
        std::string& m_Name;
        std::vector<std::string> m_Args;
        
        int m_Flags;
        dpp::slashcommand_t m_Event;
    };

    using FnCallBack = std::function<void(IResult Result)>;

    class CCommand 
    {
    public:
        CCommand(const std::string& Name, FnCallBack& Callback);
        const std::string& m_Name;
        std::vector<std::string> m_vParams;
        int m_Flags;
        FnCallBack& m_CallBack;
        std::string m_Help;
    };
    CCommand* FindCommand(const std::string& Name, int Flags);

    /* std::string& Name
    * std::vector<std::string>& Params
    * int& Flags
    * FnCallBack Callback
    * std::string& Help
    */
    void Register(const std::string& Name, const std::vector<std::string>& Params, int Flags, FnCallBack Callback, const std::string& Help);

    void ExecuteSlash(const dpp::slashcommand_t& Event);
    void ExecuteLine(std::string& Line);
    void ExecuteFile(std::string& Path);

private: 
    std::vector<CCommand> m_vCommands;
};
