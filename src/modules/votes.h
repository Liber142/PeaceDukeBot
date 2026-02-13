#include "module.h"

#include <engine/logger.h>
#include <engine/console.h>
#include <engine/data_strucs.h>

enum class EVoteOptions : int
{
    NO = 0,
    YES = 1,
};

class IVoteManager : public IModule
{
public:
    class IVote : public IModule 
    {
    protected:
        size_t m_Id;
        int m_Yes = 0;
        int m_No = 0;
        std::vector<std::pair<size_t, EVoteOptions>> m_vVotersIds;
    public:
        virtual void SetId(size_t Id) { m_Id = Id; };
        virtual size_t Id() { return m_Id; };

        virtual void AddVote(size_t VoterId, EVoteOptions Option) = 0;

        const std::string Name() const override { return "vote"; }
    };

    const std::string Name() const override { return "vote_manager"; }
private:
};

class CApplyVoteManager : public IVoteManager
{
public:
    class CClanVote : public IVote
    {
    private:
        friend CApplyVoteManager;
        enum class EVoteState 
        {
            PENDING,
            ACCEPTED,
            DECLINE,
            NUM_STATES
        } m_State;
        size_t m_MessageId = 0;
        SUserData m_TargetUser;

        dpp::message GenerateMessage();
    public:
        CClanVote() = default; 
        CClanVote(const SUserData &User) : m_TargetUser(User) {}

        void OnInit() override {} 
        void OnConsoleInit() override {}

        void AddVote(size_t VoterId, EVoteOptions Option) override;

        void StartVote();
        void FinaleVote();

        SUserData GetUser() const { return m_TargetUser; }
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(CClanVote, m_MessageId, m_TargetUser, m_Yes, m_No, m_vVotersIds)
    };

    void OnInit() override;
    void OnConsoleInit() override;

    void OnModuleInit(class CBotCore *pBotCore) override;

    const std::string Name() const override { return "apply_vote_manager"; }

//NOTE: events handler for dpp like form or buttons
    void ButtonClick(CConsole::IResult Result);
    void FormSubmit(CConsole::IResult Result);

private:
    std::map<std::size_t, std::unique_ptr<CClanVote>> m_vpVotes;     
};
