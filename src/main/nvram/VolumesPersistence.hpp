#pragma once

#include <map>
#include <string>

#include <disk/Volume.hpp>

namespace mpc { class Mpc; }

namespace mpc::nvram {
class VolumesPersistence {
public:
    static void save(mpc::Mpc&);
    static std::map<std::string, mpc::disk::MountMode> getPersistedConfigs();
    static std::string getPersistedActiveUUID();
    
};
}
