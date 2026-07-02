#include<Gear/Utils/Logger/LogColor.h>

namespace Gear::Utils::Logger
{
	bool LogColor::operator!=(const LogColor& color) const
	{
		return code != color.code;
	}

	const LogColor LogColor::black = { "\033[30m" };

	const LogColor LogColor::red = { "\033[31m" };

	const LogColor LogColor::green = { "\033[32m" };

	const LogColor LogColor::yellow = { "\033[33m" };

	const LogColor LogColor::blue = { "\033[34m" };

	const LogColor LogColor::magenta = { "\033[35m" };

	const LogColor LogColor::cyan = { "\033[36m" };

	const LogColor LogColor::white = { "\033[37m" };

	const LogColor LogColor::brightBlack = { "\033[90m" };

	const LogColor LogColor::brightRed = { "\033[91m" };

	const LogColor LogColor::brightGreen = { "\033[92m" };

	const LogColor LogColor::brightYellow = { "\033[93m" };

	const LogColor LogColor::brightBlue = { "\033[94m" };

	const LogColor LogColor::brightMagenta = { "\033[95m" };

	const LogColor LogColor::brightCyan = { "\033[96m" };

	const LogColor LogColor::brightWhite = { "\033[97m" };

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
