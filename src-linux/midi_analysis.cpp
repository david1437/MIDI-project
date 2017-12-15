#include <iostream>
#include <signal.h>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <fstream>
#include <iomanip>
#include <map>
#include <vector>
#include <algorithm>
#include <iterator>
#include <utility>
#include "RtMidi.h"

struct KeyInfo {
	std::string note {};
	long double duration {};
	int velocity {};
	int octave {};
	friend std::ostream& operator<<(std::ostream& os, const std::pair<int, KeyInfo>& k) {
		os << std::left << std::setw(20) << std::setfill(' ') 
		   << k.second.duration << std::left << std::setw(20) << std::setfill(' ') 
		   << k.second.note << std::left << std::setw(20) << std::setfill(' ') 
		   << k.second.octave << std::left << std::setw(20) << std::setfill(' ') 
		   << k.second.velocity 
	           << std::endl;
		return os;
	}
};

bool done;
static void finish(int ignore) { done = true; }

int main() {

	RtMidiIn *midiin {0};

	try {
		midiin = new RtMidiIn();
	}
	catch(RtMidiError &error) {
		error.printMessage();
		exit( EXIT_FAILURE );
	}

	unsigned int nPorts {midiin->getPortCount()};
	if(!nPorts) {
		std::cout << "No Midi input detected.\nPlease connect a Midi device to your computer.\n";
		delete midiin;
		exit( EXIT_FAILURE );
	}
	std::cout << "There are " << nPorts << " MIDI inputs available.\n";

	std::string portName{};
	for(unsigned int i {0}; i < nPorts; ++i) {
		try {
			portName = midiin->getPortName(i);
		}
		catch(RtMidiError &error) {
			error.printMessage();
			delete midiin;
			exit( EXIT_FAILURE );
		}
		std::cout << "Input Port #" << i << ": " << portName << '\n';
	}

	unsigned int input {};
	std::cout << "Enter the port number to use: ";
	std::cin >> input;

	try {
		midiin->openPort(input);
	}
	catch (RtMidiError &error) {
		error.printMessage();
		delete midiin;
		exit( EXIT_FAILURE );
	}

	midiin->ignoreTypes(false, false, false);

	done = false;
	(void) signal(SIGINT, finish);

	std::cout << "Reading MIDI from port quit with Ctrl-C.\n";

	std::ofstream ofile{"data.txt"};
	ofile << std::left << std::setw(20) << std::setfill(' ') 
		<< "Duration" << std::left << std::setw(20) << std::setfill(' ') 
		<< "Note" << std::left << std::setw(20) << std::setfill(' ') 
		<< "Octave" << std::left << std::setw(20) << std::setfill(' ') 
		<< "Velocity" 
		<< std::endl;

	std::string notes[12] { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	int nBytes {};
	long double stamp {};
	std::vector<unsigned char> message {};
	int unique_id {};
	// if -1 that key has not been initially pressed else holds id that started that note
	int id_of_last_on[11][12] {{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
				   {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
				   {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
				   {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
				   {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
				   {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
				   {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
				   {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
				   {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
				   {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
				   {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};
	std::map<int, KeyInfo> key_press_map {};
	int midiData[3] {};

	while (!done) {
		stamp = midiin->getMessage(&message);
		nBytes = message.size();
		for (int i = 0; i < nBytes; ++i) //adds message data to vector
		{
			midiData[i] = static_cast<int>(message[i]);
		}
		if(midiData[0] == 144) { //note pressed
			KeyInfo k {};
			k.velocity = midiData[2];
			if(midiData[1] <= 11) { //note = notes[value] : octave = 0
				k.note = notes[midiData[1]];
				k.octave = 0;
				id_of_last_on[k.octave][midiData[1]] = unique_id;
			}
			else { //note = notes[value % 12] : octave = value / 12
				k.note = notes[midiData[1] % 12];
				k.octave = midiData[1] / 12;
				id_of_last_on[k.octave][midiData[1] % 12] = unique_id;
			}
			key_press_map[unique_id++] = k;
		}
		else { //note released
			if(midiData[1] <= 11) { //note = notes[value] : octave = 0
				key_press_map[id_of_last_on[0][midiData[1]]].duration = stamp * 1000.0;
			}
			else { //note = notes[value % 12] : octave = value / 12
				key_press_map[id_of_last_on[midiData[1] / 12][midiData[1] % 12]].duration = stamp * 1000.0;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(5));

	}

	std::copy(std::begin(key_press_map), std::end(key_press_map), std::ostream_iterator<KeyInfo>(ofile));

	delete midiin;
	ofile.close();
	return 0;

}
