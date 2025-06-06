#include <fcntl.h>
#include <iostream>
#include <unistd.h>

extern "C"{

static int device = -1;

bool init()
{
	device = open("/dev/oxcode", O_RDWR);
	if (device < 0) {
		return false;
	}
	return true;
}

void cleanup()
{
	if (device >= 0)
	{
		close(device);
	}
}

bool switch_signal(const uint signal_no)
{
	if (device < 0  || signal_no > 1)
	{
		return false;
	}
	const std::string value = std::to_string(signal_no);
	const auto ret = write(device, value.c_str(), 1);
	return ret >= 0;
}

int read_signal_values()
{
	if (device < 0)
	{
		return false;
	}
	char buffer[1];

	if (read(device, buffer, 1) <= 0)
		return -1;
	return buffer[0] - '0';
}
}