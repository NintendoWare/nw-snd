#include "nw/snd/bfwav.h"

namespace nw::snd {

namespace bfwav {

ADPCMInfo ADPCMInfo::Read(exio::BinaryReader reader, size_t offset) {
  ADPCMInfo result = *reader.Read<ADPCMInfo>(offset);
  reader.Seek(offset);
  for (size_t i = 0; i < 16; i++) {
    result.coefficients[i] = *reader.Read<u16>();
  }

  reader.Seek(offset + sizeof(ADPCMInfo));
  return result;
}

void ADPCMInfo::Write(exio::BinaryWriter* writer) const {
  const auto offset = writer->Tell();
  writer->Write(*this);
  writer->Seek(offset);
  for (size_t i = 0; i < 16; i++) {
    writer->Write(this->coefficients[i]);
  }

  writer->Seek(offset + sizeof(ADPCMInfo));
}

}  // namespace bfwav

Fwav Fwav::FromBinary(tcb::span<const u8> data) {
  return Fwav({data, exio::Endianness::Big}, 0);
}

std::vector<u8> Fwav::ToBinary(exio::Endianness endian) const {
  exio::BinaryWriter writer{endian};
  Write(&writer);
  return writer.Finalize();
}

Fwav::Fwav(exio::BinaryReader reader, size_t offset) {
  m_reader = {reader.span(),
              exio::ByteOrderMarkToEndianness(reader.Read<bfwav::Header>().value().bom)};

  const auto header = *m_reader.Read<bfwav::Header>();
  if (header.magic != bfwav::Magic) {
    throw exio::InvalidDataError("Invalid FWAV magic");
  }
  if (header.version != 0x10200) {
    throw exio::InvalidDataError("Only version 1.2.0 is supported");
  }
  if (header.file_size != m_reader.span().size()) {
    throw exio::InvalidDataError("The specified file size does not match the input buffer");
  }
  if (header.num_sections != 2) {
    throw exio::InvalidDataError("The parser expected 2 sections but found " +
                                 std::to_string(header.num_sections));
  }

  m_info = *m_reader.Read<bfwav::Info>(header.info_section.offset);
  if (m_info.magic != bfwav::InfoMagic) {
    throw exio::InvalidDataError("Invalid INFO magic");
  }

  const auto reference_offset = m_reader.Tell();
  const auto reference_table = *m_reader.Read<bfwav::ReferenceTable>();
  channels.resize(reference_table.num_entries);
  for (size_t i = 0; i < channels.size(); i++) {
    const auto entry = *m_reader.Read<bfwav::ReferenceTableEntry>();
    if (entry.flag != 0x7100) {
      throw exio::InvalidDataError("The parser expected a channel info flag but found " +
                                   std::to_string(entry.flag));
    }

    const auto channel_info = *m_reader.Read<bfwav::ChannelInfo>(reference_offset + entry.offset);
    if (channel_info.sound_data_entry.flag != 0x1F00) {
      throw exio::InvalidDataError("The parser expected a sound data flag but found " +
                                   std::to_string(channel_info.sound_data_entry.flag));
    }
    if (channel_info.adpcm_entry.flag != 0x0300) {
      throw exio::InvalidDataError("The parser expected a ADPCM flag but found " +
                                   std::to_string(channel_info.adpcm_entry.flag));
    }

    bfwav::Channel channel{};
    channel.sound_data_offset = channel_info.sound_data_entry.offset;
    channel.adpcm_info = bfwav::ADPCMInfo::Read(m_reader, reference_offset + entry.offset +
                                                              channel_info.adpcm_entry.offset);
    channels[i] = channel;
  }

  const auto data_header = *m_reader.Read<bfwav::Data>(header.data_section.offset);
  if (data_header.magic != bfwav::DataMagic) {
    throw exio::InvalidDataError("Invalid DATA magic");
  }

  data = m_reader.span().subspan(header.data_section.offset, data_header.size);
}

void Fwav::Write(exio::BinaryWriter* writer) const {
  writer->Seek(0x40);  // Header size & padding - not sizeof(bfwav::Header)

  writer->Write(m_info);

  // Write reference table
  writer->Write<u32>(channels.size());
  for (size_t i = 0; i < channels.size(); i++) {
    bfwav::ReferenceTableEntry entry{};
    entry.flag = 0x7100;
    entry.offset = sizeof(bfwav::ReferenceTable) + sizeof(bfwav::ReferenceTableEntry) * (i + 1) +
                   sizeof(bfwav::ChannelInfo) * i;
    writer->Write(entry);
  }

  for (const auto channel : channels) {
    bfwav::ReferenceTableEntry sound_data_entry{};
    sound_data_entry.flag = 0x1F00;
    sound_data_entry.offset = 0x18;

    bfwav::ReferenceTableEntry adpcm_info_entry{};
    adpcm_info_entry.flag = 0x0300;
    adpcm_info_entry.offset = sizeof(bfwav::ChannelInfo);

    bfwav::ChannelInfo channel_info{};
    channel_info.sound_data_entry = sound_data_entry;
    channel_info.adpcm_entry = adpcm_info_entry;

    writer->Write(channel_info);
    channel.adpcm_info.Write(writer);
  }

  writer->AlignUp(32);

  bfwav::Section info_section{};
  info_section.flag = 0x7000;
  info_section.offset = 0x40;
  info_section.size = 0x80;

  bfwav::Section data_section{};
  data_section.flag = 0x7001;
  data_section.offset = writer->Tell();
  data_section.size = data.size();

  bfwav::Data data_header{};
  data_header.magic = bfwav::DataMagic;
  data_header.size = data.size();
  writer->Write(data_header);
  writer->WriteBytes(data.subspan(0x08, data.size() - 0x08));

  bfwav::Header header{};
  header.magic = bfwav::Magic;
  header.bom = 0xFEFF;
  header.header_size = 0x40;
  header.version = 0x10200;
  header.file_size = writer->Tell();
  header.num_sections = 2;
  header.info_section = info_section;
  header.data_section = data_section;
  writer->Seek(0);
  writer->Write(header);
}

}  // namespace nw::snd