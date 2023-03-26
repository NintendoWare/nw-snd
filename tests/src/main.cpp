#include <iostream>
#include <nw/snd/bfwav.h>
#include <utils/file_util.h>

int main(int argc, char** argv) {
  std::cout << "[c++] Init Testing" << std::endl;
  const auto file = file::util::ReadAllBytes(argv[1]);
  const auto fwav = nw::snd::Fwav::FromBinary({file.data(), file.size()});

  std::ofstream stream(std::string(argv[1]) + ".data");
  const auto data = fwav.data;
  stream.write(reinterpret_cast<const char*>(data.begin()), data.size());

  for (size_t i = 0; i < fwav.channels.size(); i++) {
    std::cout << fwav.channels[0].adpcm_entry.flag << std::endl;
    std::cout << fwav.channels[0].sound_data_entry.flag << std::endl;
  }
}