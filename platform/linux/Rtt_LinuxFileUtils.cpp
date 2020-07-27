#include "Rtt_LinuxFileUtils.h"
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "Core/Rtt_Assert.h"

namespace Rtt
{
	const char *LinuxFileUtils::GetStartupPath(std::string *exeFileName)
	{
		static char buf[PATH_MAX + 1];
		ssize_t count = readlink("/proc/self/exe", buf, PATH_MAX);
		buf[count] = 0;

		// remove file name
		char *filename = strrchr(buf, '/');
		Rtt_ASSERT(filename);

		if (exeFileName)
		{
			*exeFileName = filename + 1;
		}

		*filename = 0;

		return buf;
	}

	const char *LinuxFileUtils::GetHomePath()
	{
		const char *homeDir = NULL;

		if ((homeDir = getenv("HOME")) == NULL)
		{
			homeDir = getpwuid(getuid())->pw_dir;
		}

		return homeDir;
	}
};
