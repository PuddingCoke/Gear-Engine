#include<Gear/Utils/Logger/LogColor.h>

namespace Gear::Utils::Logger
{
	bool LogColor::operator!=(const LogColor& color) const
	{
		return code != color.code;
	}

	const LogColor LogColor::black = { L"\033[30m" };

	const LogColor LogColor::red = { L"\033[31m" };

	const LogColor LogColor::green = { L"\033[32m" };

	const LogColor LogColor::yellow = { L"\033[33m" };

	const LogColor LogColor::blue = { L"\033[34m" };

	const LogColor LogColor::magenta = { L"\033[35m" };

	const LogColor LogColor::cyan = { L"\033[36m" };

	const LogColor LogColor::white = { L"\033[37m" };

	const LogColor LogColor::brightBlack = { L"\033[90m" };

	const LogColor LogColor::brightRed = { L"\033[91m" };

	const LogColor LogColor::brightGreen = { L"\033[92m" };

	const LogColor LogColor::brightYellow = { L"\033[93m" };

	const LogColor LogColor::brightBlue = { L"\033[94m" };

	const LogColor LogColor::brightMagenta = { L"\033[95m" };

	const LogColor LogColor::brightCyan = { L"\033[96m" };

	const LogColor LogColor::brightWhite = { L"\033[97m" };

	const LogColor LogColor::defaultColor = LogColor::brightWhite;

	const LogColor LogColor::numericColor = LogColor::brightCyan;

	const LogColor LogColor::timeStampColor = LogColor::brightBlack;

	const LogColor LogColor::threadIdColor = LogColor::cyan;

	const LogColor LogColor::functionNameColor = LogColor::yellow;

	const LogColor LogColor::successColor = LogColor::brightGreen;

	const LogColor LogColor::errorColor = LogColor::brightRed;

	const LogColor LogColor::engineColor = LogColor::brightYellow;

	const LogColor LogColor::userColor = LogColor::brightWhite;
}