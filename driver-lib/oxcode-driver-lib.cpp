#include <fcntl.h>
#include <iostream>
#include <unistd.h>

extern "C"{

static int device{};

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
	close(device);
}

bool switch_signal(const uint signal_no)
{
	if (signal_no > 1)
	{
		return false;
	}
	const auto ret = write(device, std::to_string(signal_no).data(), 1);
	return ret >= 0;
}

int read_signal_values()
{
	char buffer[1];

	if (read(device, buffer, 1) <= 0)
		return -1;
	return buffer[0] - '0';
}
}