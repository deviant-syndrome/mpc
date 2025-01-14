#pragma once

#include <file/all/AllParser.hpp>

#include <vector>
#include <utility>
#include <string>

namespace mpc::sequencer {
class Song;
}

namespace mpc::file::all {
class Song
{
    
public:
    static const int LENGTH = 528;
    
private:
    static const int NAME_OFFSET = 0;

    /**
     * Each step in a song is 2 bytes. The first byte is the sequence index (0 - 99),
     * the second byte is the number of repeats.
     */
    static const int FIRST_STEP_OFFSET = NAME_OFFSET + AllParser::NAME_LENGTH;
    static const int STEPS_LENGTH = 500;
    static const int STEP_LENGTH = 2;
    static const int STEP_COUNT = 250;
    static const int IS_USED_OFFSET = FIRST_STEP_OFFSET + (STEP_COUNT * STEP_LENGTH);

public:
    std::string name;
    std::vector<std::pair<uint8_t, uint8_t>> steps;
    bool isUsed = false;

    std::vector<char> saveBytes = std::vector<char>(LENGTH);
    
public:
    std::string getName();
    
public:
    std::vector<char>& getBytes();
    
    Song(const std::vector<char>& loadBytes);
    Song(mpc::sequencer::Song* mpcSong);
    std::vector<std::pair<uint8_t, uint8_t>> getSteps();
    bool getIsUsed();
};
}
