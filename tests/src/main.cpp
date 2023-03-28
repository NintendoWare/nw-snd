#include <iostream>
#include <nw/snd/bfwav.h>
#include <utils/file_util.h>

int main(int argc, char** argv) {
  std::cout << "[c++] Init Testing" << std::endl;
  const auto file = file::util::ReadAllBytes(argv[1]);
  const auto fwav = nw::snd::Fwav::FromBinary({file.data(), file.size()});

  std::ofstream stream(argv[2], std::ios::binary);
  const auto data = fwav.ToBinary(exio::Endianness::Little);
  stream.write(reinterpret_cast<const char*>(data.data()), data.size());

  for (size_t i = 0; i < fwav.channels.size(); i++) {
    std::cout << fwav.channels[0].sound_data_offset << std::endl;
    std::cout << fwav.channels[0].adpcm_info.coefficients[0] << std::endl;
  }
}