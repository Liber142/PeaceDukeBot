#include <stdint.h>

class IConsole 
{
    enum class AccessLevel
    {
        ADMIN,
        MODERATOR,
        CLAN_MEMBER,
        USER,
    };

    class IResult 
    {
    protected:
        unsigned m_NumArgs;
    public: 
        IResult(uint64_t userId) :
            m_NumArgs(0),
            m_UserId(userId) {}

        int NumArguments() const { return m_NumArgs; }
        uint64_t m_UserId;
    };

    class ICommandInfo 
    {
        virtual AccessLevel GetAccessLevel() const = 0;
    };

    typedef void (*CommandCallback)(IResult *pResult);

    virtual void Register();
    virtual void Execute();
};
