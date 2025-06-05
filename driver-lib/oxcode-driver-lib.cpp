#include <fcntl.h>
#include <iostream>
#include <unistd.h>

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

bool read_signal_values(int* out)
{
	if (!out)
		return false;

	char buffer[1];

	const ssize_t ret = read(device, buffer, 1);
	if (ret <= 0)
		return false;

	out[0] = buffer[0] - '0';

	return true;
}
