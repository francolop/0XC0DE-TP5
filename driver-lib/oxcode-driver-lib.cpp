#include <fcntl.h>
#include <iostream>
#include <unistd.h>

static int device{};

bool init()
{
	device = open("/dev/oxcode", 0);
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
	const auto ret = write(device, std::to_string(signal_no).data(), 10);
	return ret > 0;
}

bool read_signal_values(int* out, size_t len)
{
	if (!out || len == 0)
		return false;

	char buffer[256];
	if (len > sizeof(buffer))
		len = sizeof(buffer);

	const ssize_t ret = read(device, buffer, len);
	if (ret <= 0)
		return false;

	for (ssize_t i = 0; i < ret; ++i) {
		out[i] = buffer[i] - '0';
	}

	return true;
}
