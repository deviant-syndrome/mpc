#pragma once

#include <tl/expected.hpp>

#include <memory>
#include <vector>
#include <string>

namespace mpc::sampler { class Sound; class Program; }
namespace mpc::sequencer { class Sequence; }
namespace mpc::file::wav { class WavFile; }

namespace mpc::disk {

class MpcFile;

using mpc_io_error_msg = std::string;

}

using file_or_error      = tl::expected<std::shared_ptr<mpc::disk::MpcFile>, mpc::disk::mpc_io_error_msg>;
using sound_or_error     = tl::expected<std::shared_ptr<mpc::sampler::Sound>, mpc::disk::mpc_io_error_msg>;
using program_or_error   = tl::expected<std::shared_ptr<mpc::sampler::Program>, mpc::disk::mpc_io_error_msg>;
using sequence_or_error  = tl::expected<std::shared_ptr<mpc::sequencer::Sequence>, mpc::disk::mpc_io_error_msg>;
using sequences_or_error = tl::expected<std::vector<std::shared_ptr<mpc::sequencer::Sequence>>, mpc::disk::mpc_io_error_msg>;
using void_or_error      = tl::expected<void, mpc::disk::mpc_io_error_msg>;
using wav_or_error       = tl::expected<std::shared_ptr<mpc::file::wav::WavFile>, mpc::disk::mpc_io_error_msg>;
