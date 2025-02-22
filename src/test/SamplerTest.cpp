#include "catch2/catch_test_macros.hpp"

#include "Mpc.hpp"
#include "disk/AbstractDisk.hpp"
#include "disk/Volume.hpp"
#include "lcdgui/ScreenComponent.hpp"
#include "mpc_fs.hpp"
#include "disk/SoundLoader.hpp"

#include <thread>

#include <cmrc/cmrc.hpp>
#include <string_view>

CMRC_DECLARE(mpctest);

using namespace mpc;
using namespace mpc::disk;

void prepareSamplerResources(mpc::Mpc& mpc)
{
    auto tmpDocsPath = fs::temp_directory_path();
    tmpDocsPath.concat("/VMPC2000XL-test/");
    fs::remove_all(tmpDocsPath);
    fs::create_directories(tmpDocsPath);
    auto disk = mpc.getDisk();

    disk->getVolume().localDirectoryPath = tmpDocsPath;
    disk->initRoot();
    disk->initFiles();

    auto fs = cmrc::mpctest::get_filesystem();

    for (auto &&entry: fs.iterate_directory("test/Sampler"))
    {
        auto file = fs.open("test/Sampler/" + entry.filename());
        char *data = (char *) std::string_view(file.begin(), file.end() - file.begin()).data();
        auto newFile = disk->newFile(entry.filename());
        std::vector<char> dataVec(data, data + file.size());
        newFile->setFileData(dataVec);
    }

    disk->initFiles();
}

TEST_CASE("Sort sounds by memory index", "[sampler]")
{
    mpc::Mpc mpc;
    mpc.init(1, 5);

    prepareSamplerResources(mpc);

    auto sampler = mpc.getSampler();

    while(sampler->getSoundSortingTypeName() != "MEMORY")
    {
        sampler->switchToNextSoundSortType();
    }

    REQUIRE(sampler->getSoundSortingTypeName() == "MEMORY");

    const bool shouldBeConverted = false;

    SoundLoaderResult r;
    SoundLoader soundLoader(mpc, false);

    for (int i = 3; i > 0; i--)
    {
        auto f = mpc.getDisk()->getFile("SOUND" + std::to_string(i) + ".SND");
        auto s = sampler->addSound();
        soundLoader.loadSound(f, r, s, shouldBeConverted);
    }

    REQUIRE(sampler->getSoundCount() == 3);

    auto sortedSounds = sampler->getSounds();

    for (int i = 0; i < 3; i++)
    {
        auto s = sortedSounds[i];
        REQUIRE(s->getName() == "sound" + std::to_string(3 - i));
    }

    mpc.getLayeredScreen()->openScreen("trim");
    auto controls = mpc.getActiveControls();
    auto soundName = [&](){ return controls->findChild<lcdgui::Field>("snd")->getText(); };
    REQUIRE(soundName() == "sound3          (ST)");
    controls->turnWheel(1);
    REQUIRE(soundName() == "sound2          (ST)");
    controls->turnWheel(1);
    REQUIRE(soundName() == "sound1          (ST)");
}

TEST_CASE("Sort sounds by name", "[sampler]")
{
    mpc::Mpc mpc;
    mpc.init(1, 5);

    prepareSamplerResources(mpc);

    auto sampler = mpc.getSampler();

    while(sampler->getSoundSortingTypeName() != "NAME")
    {
        sampler->switchToNextSoundSortType();
    }

    REQUIRE(sampler->getSoundSortingTypeName() == "NAME");

    const bool shouldBeConverted = false;

    SoundLoaderResult r;
    SoundLoader soundLoader(mpc, false);

    for (int i = 3; i > 0; i--)
    {
        auto f = mpc.getDisk()->getFile("SOUND" + std::to_string(i) + ".SND");
        auto s = sampler->addSound();
        soundLoader.loadSound(f, r, s, shouldBeConverted);
    }

    REQUIRE(sampler->getSoundCount() == 3);

    auto sortedSounds = sampler->getSortedSounds();

    for (int i = 0; i < 3; i++)
    {
        auto s = sortedSounds[i];
        REQUIRE(s.first->getName() == "sound" + std::to_string(i + 1));
    }

    mpc.getLayeredScreen()->openScreen("trim");
    auto controls = mpc.getActiveControls();
    auto soundName = [&](){ return controls->findChild<lcdgui::Field>("snd")->getText(); };
    REQUIRE(soundName() == "sound1          (ST)");
    controls->turnWheel(1);
    REQUIRE(soundName() == "sound2          (ST)");
    controls->turnWheel(1);
    REQUIRE(soundName() == "sound3          (ST)");
}

TEST_CASE("Sort sounds by size", "[sampler]")
{
    mpc::Mpc mpc;
    mpc.init(1, 5);

    prepareSamplerResources(mpc);

    auto sampler = mpc.getSampler();

    while(sampler->getSoundSortingTypeName() != "SIZE")
    {
        sampler->switchToNextSoundSortType();
    }

    REQUIRE(sampler->getSoundSortingTypeName() == "SIZE");

    const bool shouldBeConverted = false;

    SoundLoaderResult r;
    SoundLoader soundLoader(mpc, false);

    for (int i = 3; i > 0; i--)
    {
        auto f = mpc.getDisk()->getFile("SOUND" + std::to_string(i) + ".SND");
        auto s = sampler->addSound();
        soundLoader.loadSound(f, r, s, shouldBeConverted);
    }

    REQUIRE(sampler->getSoundCount() == 3);

    std::vector<int> expected{ 2, 3, 1 };

    auto sortedSounds = sampler->getSortedSounds();

    for (int i = 0; i < 3; i++)
    {
        auto s = sortedSounds[i];
        REQUIRE(s.first->getName() == "sound" + std::to_string(expected[i]));
    }

    mpc.getLayeredScreen()->openScreen("trim");
    auto controls = mpc.getActiveControls();
    auto soundName = [&](){ return controls->findChild<lcdgui::Field>("snd")->getText(); };
    REQUIRE(soundName() == "sound2          (ST)");
    controls->turnWheel(1);
    REQUIRE(soundName() == "sound3          (ST)");
    controls->turnWheel(1);
    REQUIRE(soundName() == "sound1          (ST)");
}

TEST_CASE("Switch sort and retain correct sound index", "[sampler]")
{
    mpc::Mpc mpc;
    mpc.init(1, 5);

    prepareSamplerResources(mpc);

    auto sampler = mpc.getSampler();

    const bool shouldBeConverted = false;

    SoundLoaderResult r;
    SoundLoader soundLoader(mpc, false);

    for (int i = 3; i > 0; i--)
    {
        auto f = mpc.getDisk()->getFile("SOUND" + std::to_string(i) + ".SND");
        auto s = sampler->addSound();
        soundLoader.loadSound(f, r, s, shouldBeConverted);
    }

    REQUIRE(sampler->getSoundCount() == 3);

    mpc.getLayeredScreen()->openScreen("trim");
    auto controls = mpc.getActiveControls();
    auto soundName = [&](){ return controls->findChild<lcdgui::Field>("snd")->getText(); };
    REQUIRE(soundName() == "sound3          (ST)");
    controls->turnWheel(1);
    REQUIRE(soundName() == "sound2          (ST)");

    while(sampler->getSoundSortingTypeName() != "SIZE")
    {
        sampler->switchToNextSoundSortType();
    }

    mpc.getLayeredScreen()->openScreen("main");
    mpc.getLayeredScreen()->openScreen("trim");

    REQUIRE(soundName() == "sound2          (ST)");
    controls->turnWheel(1);
    REQUIRE(soundName() == "sound3          (ST)");
    controls->turnWheel(1);
    REQUIRE(soundName() == "sound1          (ST)");
}

TEST_CASE("Sort does not corrupt note parameter sound indices", "[sampler]")
{
    mpc::Mpc mpc;
    mpc.init(1, 5);

    prepareSamplerResources(mpc);

    auto sampler = mpc.getSampler();

    const bool shouldBeConverted = false;

    SoundLoaderResult r;
    SoundLoader soundLoader(mpc, false);

    for (int i = 0; i < 3; i++)
    {
        auto f = mpc.getDisk()->getFile("SOUND" + std::to_string(i + 1) + ".SND");
        auto s = sampler->addSound();
        soundLoader.loadSound(f, r, s, shouldBeConverted);
    }

    mpc.getLayeredScreen()->openScreen("program-assign");
    auto controls = mpc.getActiveControls();
    mpc.getLayeredScreen()->setFocus("snd");

    REQUIRE(sampler->getSoundSortingTypeName() == "MEMORY");

    auto soundName = [&](){ return controls->findChild<lcdgui::Field>("snd")->getText(); };
    REQUIRE(soundName() == "OFF");
    controls->turnWheel(1);
    REQUIRE(soundName() == "sound1");
    controls->turnWheel(1);
    REQUIRE(soundName() == "sound2");
    controls->turnWheel(1);
    REQUIRE(soundName() == "sound3");

    controls->turnWheel(-1);
    REQUIRE(soundName() == "sound2");

    while(sampler->getSoundSortingTypeName() != "SIZE")
    {
        sampler->switchToNextSoundSortType();
    }

    std::vector<int> expected{ 2, 3, 1 };

    auto sortedSounds = sampler->getSortedSounds();

    for (int i = 0; i < 3; i++)
    {
        REQUIRE(soundName() == "sound" + std::to_string(expected[i]));
        controls->turnWheel(1);
    }
}
