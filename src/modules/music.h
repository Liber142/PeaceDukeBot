#pragma once
#include "module.h"

#include <engine/console.h>

#include <mpg123.h>
#include <out123.h>

class CMusic : public IModule
{
public:
	void OnInit() override;
	void OnConsoleInit() override;

	const std::string Name() const override { return "CMusic"; }

private:
	void Test(CConsole::IResult &Result);
	std::vector<uint8_t> DecodeMP3(const std::string FilePath) const;
	int Download(const std::string &Url) const;

	std::string m_TmpForTestPath;
	std::vector<uint8_t> m_vBuffer;
};
