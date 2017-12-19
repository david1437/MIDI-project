#include <array>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <numeric>
#include <thread>
#include <utility>
#include <vector>
#include "RtMidi.h"

struct KeyInfo {
        std::string note{};
        long double duration{};
        int velocity{};
        int octave{};
        friend std::ostream &operator<<(std::ostream &os, const KeyInfo &k) {
                os << std::left << std::setw(20) << std::setfill(' ')
                   << k.duration << std::left << std::setw(20)
                   << std::setfill(' ') << k.note << std::left << std::setw(20)
                   << std::setfill(' ') << k.octave << std::left
                   << std::setw(20) << std::setfill(' ') << k.velocity
                   << std::left << std::setw(20) << std::setfill(' ')
                   << std::endl;
                return os;
        }
};

int main() {
        RtMidiIn *midiin{0};

        try {
                midiin = new RtMidiIn();
        } catch (RtMidiError &error) {
                error.printMessage();
                exit(EXIT_FAILURE);
        }

        unsigned int nPorts{midiin->getPortCount()};
        if (!nPorts) {
                std::cout << "No Midi input detected.\nPlease connect a Midi "
                             "device to your computer.\n";
                delete midiin;
                exit(EXIT_FAILURE);
        }
        std::cout << "There are " << nPorts << " MIDI inputs available.\n";

        std::string portName{};
        for (unsigned int i{0}; i < nPorts; ++i) {
                try {
                        portName = midiin->getPortName(i);
                } catch (RtMidiError &error) {
                        error.printMessage();
                        delete midiin;
                        exit(EXIT_FAILURE);
                }
                std::cout << "Input Port #" << i << ": " << portName << '\n';
        }

        unsigned int input{};
        std::cout << "Enter the port number to use: ";
        std::cin >> input;

        try {
                midiin->openPort(input);
        } catch (RtMidiError &error) {
                error.printMessage();
                delete midiin;
                exit(EXIT_FAILURE);
        }

        midiin->ignoreTypes(false, false, false);

        std::cout << "Reading MIDI from port quit with \"exit\".\n";

        std::ofstream ofile{"data.txt"};
        ofile << std::left << std::setw(20) << std::setfill(' ')
              << "Duration (ms)" << std::left << std::setw(20)
              << std::setfill(' ') << "Note" << std::left << std::setw(20)
              << std::setfill(' ') << "Octave" << std::left << std::setw(20)
              << std::setfill(' ') << "Velocity" << std::left << std::setw(20)
              << std::endl;

        std::string notes[12]{"C",  "C#", "D",  "D#", "E",  "F",
                              "F#", "G",  "G#", "A",  "A#", "B"};
        int nBytes{};
        long double stamp{};
        std::vector<unsigned char> message{};
        int unique_id{};
        // if -1 that key has not been initially pressed else holds id that
        // started that note
        int id_of_last_on[11][12]{
            {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
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
        std::vector<KeyInfo> key_press_map{};
        std::array<int, 3> midiData{};
        std::array<int, 3> midiDataPrev{};

        auto response = std::async(std::launch::async, []() {
                std::string c{};
                while (c != "exit") {
                        std::cin >> c;
                }
                return c;
        });

        auto span = std::chrono::milliseconds(5);
        while (response.wait_for(span) == std::future_status::timeout) {
                stamp = midiin->getMessage(&message);
                nBytes = message.size();
                for (int i = 0; i < nBytes; ++i)  // adds message data to vector
                {
                        midiData[i] = static_cast<int>(message[i]);
                }
                if (midiData == midiDataPrev) {
                        continue;
                }
                if (midiData[0] == 144) {  // note pressed
                        KeyInfo k{};
                        k.velocity = midiData[2];
                        if (midiData[1] <=
                            11) {  // note = notes[value] : octave = 0
                                k.note = notes[midiData[1]];
                                k.octave = 0;
                                id_of_last_on[k.octave][midiData[1]] =
                                    unique_id;
                        } else {  // note = notes[value % 12] : octave = value /
                                  // 12
                                k.note = notes[midiData[1] % 12];
                                k.octave = midiData[1] / 12;
                                id_of_last_on[k.octave][midiData[1] % 12] =
                                    unique_id;
                        }
                        key_press_map.push_back(k);
                        ++unique_id;
                } else {  // note released
                        if (midiData[1] <=
                            11) {  // note = notes[value] : octave = 0
                                key_press_map[id_of_last_on[0][midiData[1]]]
                                    .duration = stamp * 1000.0;
                                ofile << key_press_map
                                        [id_of_last_on[0][midiData[1]]];
                        } else {  // note = notes[value % 12] : octave = value /
                                // 12
                                key_press_map[id_of_last_on[midiData[1] / 12]
                                                           [midiData[1] % 12]]
                                    .duration = stamp * 1000.0;
                                ofile << key_press_map
                                        [id_of_last_on[midiData[1] / 12]
                                                      [midiData[1] % 12]];
                        }
                }

                midiDataPrev = midiData;
        }

        delete midiin;
        ofile.close();
        return 0;
}
