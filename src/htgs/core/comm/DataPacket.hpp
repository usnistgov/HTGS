//
// Created by tjb3 on 3/6/17.
//

#ifndef HTGS_DATAPACKET_HPP
#define HTGS_DATAPACKET_HPP
#include <string>
#include <bits/shared_ptr.h>

#include <htgs/api/IData.hpp>

namespace htgs {
class DataPacket {
 public:
  DataPacket(std::string originName, std::string originAddr,
             const std::string &destName, std::string destAddr, const std::shared_ptr<IData> &data)
      : destName(destName), originName(originName), destAddr(destAddr), originAddr(originAddr), data(data) {

  }

  const std::string &getDestName() const {
    return destName;
  }
  const std::string &getOriginName() const {
    return originName;
  }
  std::string getDestAddr() const {
    return destAddr;
  }
  std::string getOriginAddr() const {
    return originAddr;
  }
  const std::shared_ptr<IData> &getData() const {
    return data;
  }

 private:
  std::string destName;
  std::string originName;

  std::string destAddr;
  std::string originAddr;


  std::shared_ptr<IData> data;
};
}
#endif //HTGS_DATAPACKET_HPP
