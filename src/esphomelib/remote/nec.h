//
//  nec.h
//  esphomelib
//
//  Created by Otto Winter on 06.06.18.
//  Copyright © 2018 Otto Winter. All rights reserved.
//

#ifndef ESPHOMELIB_NEC_H
#define ESPHOMELIB_NEC_H

#include "esphomelib/remote/remote_protocol.h"
#include "esphomelib/defines.h"

#ifdef USE_REMOTE

ESPHOMELIB_NAMESPACE_BEGIN

namespace remote {

class NECTransmitter : public RemoteTransmitter {
 public:
  NECTransmitter(const std::string &name, uint16_t address, uint16_t command);

  RemoteTransmitData get_data() override;

 protected:
  uint16_t address_;
  uint16_t command_;
};

bool decode_nec(RemoteReceiveData &data, uint16_t *address, uint16_t *command);

class NECDecoder : public RemoteReceiveDecoder {
 public:
  NECDecoder(const std::string &name, uint16_t address, uint16_t command);

  bool matches(RemoteReceiveData &data) override;

 protected:
  uint16_t address_;
  uint16_t command_;
};

class NECDumper : public RemoteReceiveDumper {
 public:
  void dump(RemoteReceiveData &data) override;
};

} // namespace remote

ESPHOMELIB_NAMESPACE_END

#endif //USE_REMOTE

#endif //ESPHOMELIB_NEC_H
