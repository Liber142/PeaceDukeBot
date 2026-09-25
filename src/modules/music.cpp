#include "music.h"

#include <engine/config.h>

void CMusic::OnInit()
{
	dpp::slashcommand Command = dpp::slashcommand(
		"join",
		"join",
		Bot()->me.id);

	Bot()->guild_command_create(Command, Config()->DEFAULT_GUILD_ID, [this](const dpp::confirmation_callback_t &Callback) {
		if(Callback.is_error())
		{
			CLogger::Error(Name(), Callback.get_error().human_readable);
			return;
		}
	});
}

void CMusic::OnConsoleInit()
{
	Console()->Register("join", {}, SLASH_COMMAND, [this](CConsole::IResult Result) { Test(Result); }, "");
}

void CMusic::Test(CConsole::IResult &Result)
{
	if(const auto *Command = dynamic_cast<const dpp::slashcommand_t *>(Result.m_Event))
	{
		dpp::guild *g = dpp::find_guild(Config()->DEFAULT_GUILD_ID);
		g->connect_member_voice(*Command->owner, Command->command.get_issuing_user().id);

		Bot()->on_voice_ready([this](const dpp::voice_ready_t &Event) {
			std::vector<uint8_t> Pcmdata = DecodeMP3(std::string("/home/liber/Music/godfather.mp3"));

			dpp::voiceconn *Voiceconn = Event.from()->get_voice(Config()->DEFAULT_GUILD_ID);
			if(Voiceconn && Voiceconn->voiceclient && Voiceconn->voiceclient->is_ready())
			{
				/* Stream the already decoded MP3 file. This passes the PCM data to the library to be encoded to OPUS */
				Voiceconn->voiceclient->send_audio_raw((uint16_t *)Pcmdata.data(), Pcmdata.size());
			}
		});
	}
}

std::vector<uint8_t> CMusic::DecodeMP3(const std::string FilePath) const
{
	/* This will hold the decoded MP3.
	 * The D++ library expects PCM format, which are raw sound
	 * data, 2 channel stereo, 16 bit signed 48000Hz.
	 */
	std::vector<uint8_t> Pcmdata;

	mpg123_init();

	int Err = 0;
	unsigned char *Buffer;
	size_t BufferSize, Done;
	int Channels, Encoding;
	long Rate;

	/* Note it is important to force the frequency to 48000 for Discord compatibility */
	mpg123_handle *pMh = mpg123_new(NULL, &Err);
	mpg123_param(pMh, MPG123_FORCE_RATE, 48000, 48000.0);

	/* Decode entire file into a vector. You could do this on the fly, but if you do that
	 * you may get timing issues if your CPU is busy at the time and you are streaming to
	 * a lot of channels/guilds.
	 */
	BufferSize = mpg123_outblock(pMh);
	Buffer = new unsigned char[BufferSize];

	/* Note: In a real world bot, this should have some error logging */
	mpg123_open(pMh, FilePath.c_str());
	mpg123_getformat(pMh, &Rate, &Channels, &Encoding);

	unsigned int Counter = 0;
	for(int TotalBtyes = 0; mpg123_read(pMh, Buffer, BufferSize, &Done) == MPG123_OK;)
	{
		for(auto i = 0; i < BufferSize; i++)
		{
			Pcmdata.push_back(Buffer[i]);
		}
		Counter += BufferSize;
		TotalBtyes += Done;
	}
	delete Buffer;
	mpg123_close(pMh);
	mpg123_delete(pMh);

	return Pcmdata;
}
