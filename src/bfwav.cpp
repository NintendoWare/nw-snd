#include "nw/snd/bfwav.h"

namespace nw::snd {

Fwav Fwav::FromBinary(tcb::span<const u8> data) {
  return Fwav({data, exio::Endianness::Big}, 0);
}

Fwav::Fwav(exio::BinaryReader reader, size_t offset) {
  reader = {reader.span(),
            exio::ByteOrderMarkToEndianness(reader.Read<bfwav::Header>().value().bom)};

  const auto header = *reader.Read<bfwav::Header>();
  if (header.magic != bfwav::Magic) {
    throw exio::InvalidDataError("Invalid FWAV magic");
  }
  if (header.version != 0x10200) {
    throw exio::InvalidDataError("Only version 1.2.0 is supported");
  }
  if (header.file_size != reader.span().size()) {
    throw exio::InvalidDataError("The specified file size does not match the input buffer");
  }
  if (header.num_sections != 2) {
    throw exio::InvalidDataError("The parser expected 2 sections but found " +
                                 std::to_string(header.num_sections));
  }

  const auto info_section = *reader.Read<bfwav::Info>(header.info_section.offset);
  if (info_section.magic != bfwav::InfoMagic) {
    throw exio::InvalidDataError("Invalid INFO magic");
  }

  const auto reference_offset = reader.Tell();
  const auto reference_table = *reader.Read<bfwav::ReferenceTable>();
  channels.resize(reference_table.num_entries);
  for (size_t i = 0; i < channels.size(); i++) {
    const auto entry = *reader.Read<bfwav::ReferenceTableEntry>();
    if (entry.flag != 0x7100) {
      throw exio::InvalidDataError("The parser expected a channel info flag but found " +
                                   std::to_string(entry.flag));
    }

    channels[i] = *reader.Read<bfwav::ChannelInfo>(reference_offset + entry.offset);
  }

  const auto data_header = *reader.Read<bfwav::Data>(header.data_section.offset);
  if (data_header.magic != bfwav::DataMagic) {
    throw exio::InvalidDataError("Invalid DATA magic");
  }

  data = reader.span().subspan(header.data_section.offset, data_header.size);
}

}  // namespace nw::snd