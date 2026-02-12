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
        size_t m_Id = 0;
        int m_Yes = 0;
        int m_No = 0;
        std::vector<std::pair<size_t, EVoteOptions>> m_vVotersIds;
    public:
        virtual void SetId(size_t Id) { m_Id = Id; };
        virtual size_t Id() { return m_Id; };

        virtual void AddVote(size_t VoterId, EVoteOptions Option) = 0;
    };
private:
};

class CApplyVoteManager : public IVoteManager
{
public:
    class CClanVote : public IVote
    {
    private:
        friend class CApplyVoteManager;
        size_t m_MessageId;
        SUserData m_TargetUser;
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

    ~CApplyVoteManager() override;

    void OnInit() override;
    void OnConsoleInit() override;

//NOTE: events handler for dpp like form or buttons
    void ButtonClick(CConsole::IResult Result);
    void FormSubmit(CConsole::IResult Result);

private:
    std::map<std::size_t, std::unique_ptr<CClanVote>> m_vpVotes;     
};
