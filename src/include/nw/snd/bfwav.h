#include <vector>

#include <exio/binary_reader.h>
#include <exio/error.h>
#include <exio/types.h>
#include <exio/util/magic_utils.h>

namespace nw::snd {

namespace bfwav {

constexpr auto Magic = exio::util::MakeMagic("FWAV");
constexpr auto InfoMagic = exio::util::MakeMagic("INFO");
constexpr auto DataMagic = exio::util::MakeMagic("DATA");

enum class Encoding : u8 { PCM8, PCM16, ADPCM, IMAADPCM };

struct Section {
  u16 flag;
  u16 padding;
  u32 offset;
  u32 size;
  EXIO_DEFINE_FIELDS(Section, flag, padding, offset, size);
};

struct Header {
  std::array<char, 4> magic;
  u16 bom;
  u16 header_size;
  u32 version;
  u32 file_size;
  u16 num_sections;
  u16 padding;
  Section info_section;
  Section data_section;
  EXIO_DEFINE_FIELDS(Header, magic, bom, header_size, version, file_size, num_sections, padding,
                     info_section, data_section);
};

struct ReferenceTable {
  u32 num_entries;
  EXIO_DEFINE_FIELDS(ReferenceTable, num_entries);
};

struct ReferenceTableEntry {
  u16 flag;
  u16 padding;
  u32 offset;
  EXIO_DEFINE_FIELDS(ReferenceTableEntry, flag, padding, offset);
};

struct ChannelInfo {
  ReferenceTableEntry sound_data_entry;
  ReferenceTableEntry adpcm_entry;
  u32 reserved;
  EXIO_DEFINE_FIELDS(ChannelInfo, sound_data_entry, adpcm_entry, reserved);
};

struct ADPCMInfo {
  u16 coefficients;
  u16 pred_scale;
  u16 yn_1;
  u16 yn_2;
  u16 loop_pred_scale;
  u16 loop_yn_1;
  u16 loop_yn_2;
  u16 padding;
  EXIO_DEFINE_FIELDS(ADPCMInfo, coefficients, pred_scale, yn_1, yn_2, loop_pred_scale, loop_yn_1,
                     loop_yn_2, padding);
};

struct Info {
  std::array<char, 4> magic;
  u32 size;
  Encoding encoding;
  u8 is_loop;
  u16 padding;
  u32 smaple_rate;
  u32 loop_start;
  u32 loop_end;
  u32 loop_start_frame;
  EXIO_DEFINE_FIELDS(Info, magic, size, encoding, is_loop, padding, smaple_rate, loop_start,
                     loop_end, loop_start_frame)
};

struct Data {
  std::array<char, 4> magic;
  u32 size;
  EXIO_DEFINE_FIELDS(Data, magic, size);
};

}  // namespace bfwav

class Fwav {
public:
  static Fwav FromBinary(tcb::span<const u8> data);
  Fwav(exio::BinaryReader reader, size_t offset);
  tcb::span<const u8> data;
  std::vector<bfwav::ChannelInfo> channels;

private:
  bfwav::Info m_info;
};

}  // namespace nw::snd