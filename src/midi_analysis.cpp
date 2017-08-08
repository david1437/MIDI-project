#include <iostream>
#include <signal.h>
#include <cstdlib>
#include <chrono>
#include <thread>
#include "RtMidi.h"

bool done;
static void finish(int ignore) { done = true; }

int main() {

	int nBytes;
	double stamp;
	std::vector<unsigned char> message;
	RtMidiIn *midiin = 0;

	try {
		midiin = new RtMidiIn();
	}
	catch(RtMidiError &error) {
		error.printMessage();
		exit( EXIT_FAILURE );
	}

	unsigned int nPorts = midiin->getPortCount();
	std::cout << "There are " << nPorts << " MIDI inputs available." << std::endl;

	std::string portName;
	for(unsigned int i = 0; i < nPorts; i++) {
		try {
			portName = midiin->getPortName(i);
		}
		catch(RtMidiError &error) {
			error.printMessage();
			goto cleanup;
		}
		std::cout << "Input Port #" << i+1 << ": " << portName << std::endl;
	}

	try {
		midiin->openVirtualPort();
	}
	catch(RtMidiError &error) {
		error.printMessage();
		goto cleanup;
	}

	midiin->ignoreTypes(false, false, false);

	done = false;
	(void) signal(SIGINT, finish);

	std::cout << "Reading MIDI from port quit with Ctrl-C." << std::endl;
	while(!done) {
		stamp = midiin->getMessage(&message);
		nBytes = message.size();
		for(int i = 0; i < nBytes; i++)
			std::cout << "Byte " << i << " = " << static_cast<int>(message[i]) << ", ";
		if(nBytes > 0)
			std::cout << "Stamp = " << stamp << std::endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	cleanup:
		delete midiin;

	return 0;

}
