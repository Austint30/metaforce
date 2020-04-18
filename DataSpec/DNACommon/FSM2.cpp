#include "DataSpec/DNACommon/FSM2.hpp"

#include "DataSpec/DNACommon/PAK.hpp"

#include <athena/FileWriter.hpp>
#include <athena/Global.hpp>
#include <athena/IStreamWriter.hpp>

#include <logvisor/logvisor.hpp>

namespace DataSpec::DNAFSM2 {
logvisor::Module LogDNAFSM2("urde::DNAFSM2");

template <class IDType>
template <class Op>
void FSM2<IDType>::Enumerate(typename Op::StreamT& s) {
  Do<Op>(athena::io::PropId{"header"}, header, s);
  if (header.magic != SBIG('FSM2')) {
    LogDNAFSM2.report(logvisor::Fatal, FMT_STRING("Invalid FSM2 magic '{}' expected 'FSM2'"), header.magic);
    return;
  }

  if (header.version == 1) {
    if (!detail)
      detail.reset(new FSMV1);
    Do<Op>(athena::io::PropId{"detail"}, static_cast<FSMV1&>(*detail), s);
  } else if (header.version == 2) {
    if (!detail)
      detail.reset(new FSMV2);
    Do<Op>(athena::io::PropId{"detail"}, static_cast<FSMV2&>(*detail), s);
  } else {
    LogDNAFSM2.report(logvisor::Fatal, FMT_STRING("Invalid FSM2 version '{}'"), header.version);
    return;
  }
}

AT_SPECIALIZE_DNA(FSM2<UniqueID32>)
AT_SPECIALIZE_DNA(FSM2<UniqueID64>)

template <>
std::string_view FSM2<UniqueID32>::DNAType() {
  return "urde::FSM2<UniqueID32>"sv;
}

template <>
std::string_view FSM2<UniqueID64>::DNAType() {
  return "urde::FSM2<UniqueID64>"sv;
}

template struct FSM2<UniqueID32>;
template struct FSM2<UniqueID64>;

template <class IDType>
bool ExtractFSM2(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter writer(outPath.getAbsolutePath());
  if (writer.isOpen()) {
    FSM2<IDType> fsm2;
    fsm2.read(rs);
    athena::io::ToYAMLStream(fsm2, writer);
    return true;
  }
  return false;
}
template bool ExtractFSM2<UniqueID32>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template bool ExtractFSM2<UniqueID64>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteFSM2(const FSM2<IDType>& fsm2, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
  if (w.hasError())
    return false;
  fsm2.write(w);
  int64_t rem = w.position() % 32;
  if (rem)
    for (int64_t i = 0; i < 32 - rem; ++i)
      w.writeUByte(0xff);
  return true;
}
template bool WriteFSM2<UniqueID32>(const FSM2<UniqueID32>& fsm2, const hecl::ProjectPath& outPath);
template bool WriteFSM2<UniqueID64>(const FSM2<UniqueID64>& fsm2, const hecl::ProjectPath& outPath);

} // namespace DataSpec::DNAFSM2
