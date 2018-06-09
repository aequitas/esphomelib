//
//  lg.h
//  esphomelib
//
//  Created by Otto Winter on 06.06.18.
//  Copyright © 2018 Otto Winter. All rights reserved.
//

#ifndef ESPHOMELIB_LG_H
#define ESPHOMELIB_LG_H

#include "esphomelib/remote/remote_protocol.h"
#include "esphomelib/defines.h"

#ifdef USE_REMOTE

ESPHOMELIB_NAMESPACE_BEGIN

namespace remote {

class LGTransmitter : public RemoteTransmitter {
 public:
  LGTransmitter(const std::string &name, uint32_t data, uint8_t nbits);

  RemoteTransmitData get_data() override;

 protected:
  uint32_t data_;
  uint8_t nbits_;
};

bool decode_lg(RemoteReceiveData &data, uint32_t *data_, uint8_t *nbits);

class LGDecoder : public RemoteReceiveDecoder {
 public:
  LGDecoder(const std::string &name, uint32_t data, uint8_t nbits);

 protected:
  bool matches(RemoteReceiveData &data) override;

 protected:
  uint32_t data_;
  uint8_t nbits_;
};

class LGDumper : public RemoteReceiveDumper {
 public:
  void dump(RemoteReceiveData &data) override;
};

} // namespace remote

ESPHOMELIB_NAMESPACE_END

#endif //USE_REMOTE

#endif //ESPHOMELIB_LG_H
