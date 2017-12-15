#include <iostream>
#include <signal.h>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <fstream>
#include <iomanip>
#include <map>
#include <vector>
#include "RtMidi.h"

bool done;
static void finish(int ignore) { done = true; }

int main() {

	std::ofstream ofile;
	ofile.open("data.txt");
	ofile << std::left << std::setw(20) << std::setfill(' ') 
		<< "Delta Time" << std::left << std::setw(20) << std::setfill(' ') 
		<< "Key Press" << std::left << std::setw(20) << std::setfill(' ') 
		<< "Note" << std::left << std::setw(20) << std::setfill(' ') 
		<< "Octave" << std::left << std::setw(20) << std::setfill(' ') 
		<< "Velocity" 
		<< std::endl;

	std::string notes[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	int nBytes;
	long double stamp;
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
		std::cout << "Input Port #" << i << ": " << portName << std::endl;
	}

	unsigned int input;
	std::cout << "Enter the port number to use: ";
	std::cin >> input;

	try {
		midiin->openPort(input);
	}
	catch (RtMidiError &error) {
		error.printMessage();
		goto cleanup;
	}

	midiin->ignoreTypes(false, false, false);

	done = false;
	(void) signal(SIGINT, finish);

	std::cout << "Reading MIDI from port quit with Ctrl-C." << std::endl;

	while (!done) {
		stamp = midiin->getMessage(&message);
		nBytes = message.size();
		if (nBytes > 0)
			ofile << std::left << std::setw(20) << std::setfill(' ') << stamp * 1000;
		for (int i = 0; i < nBytes; i++) 
		{
			int value = static_cast<int>(message[i]);
			if (i == 0)
			{
				if (value == 144)
					ofile << std::left << std::setw(20) << std::setfill(' ') << "On";
				else
					ofile << std::left << std::setw(20) << std::setfill(' ') << "Off";
			}
			else if (i == 1) 
			{
				if (value <= 11) {
					ofile << std::left << std::setw(20) << std::setfill(' ') << notes[value] << std::setw(20) << "0";
				}
				else {
					ofile << std::left << std::setw(20) << std::setfill(' ') << notes[value % 12] << std::setw(20) << value / 12;
				}
			}
			else if (i == 2)
			{
				ofile << std::left << std::setw(20) << std::setfill(' ') << value << std::endl; 
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	cleanup:
		delete midiin;
		ofile.close();

	return 0;

}
