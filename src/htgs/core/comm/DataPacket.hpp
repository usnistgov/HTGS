// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.
//
// Created by tjb3 on 3/6/17.
//

/**
 * @file DataPacket.hpp
 * @author Timothy Blattner
 * @date March 6, 2017
 *
 * @brief Implements the data packet that is used by the TaskGraphCommunicator.
 *
 */

#ifndef HTGS_DATAPACKET_HPP
#define HTGS_DATAPACKET_HPP
#include <string>
#include <memory>

#include <htgs/api/IData.hpp>

namespace htgs {

/**
 * @class DataPacket DataPacket.hpp <htgs/core/comm/DataPacket.hpp>
 *
 * @brief Implements a data packet that is transmitted to the TaskGraphCommunicator.
 *
 * A data packet stores any type of IData that is to be sent to some destination HTGS address
 * with an endpoint name. The data packet is used to store this meta data for the TaskGraphCommunicator.
 *
 * The TaskGraphCommunicator passes the IData held in the data packet to an end point identified by the HTGS address
 * and task name. This retrieves the input connector, which is used to transmit the IData.
 *
 * @note When using DataPacket, the IData is dynamically cast to the input connector's end point type. It is important to
 * have the IData type to match the input connector's type.
 *
 */
class DataPacket {
 public:
  /**
   * Constructs a data packet
   * @param originName the origin task name
   * @param originAddr the origin task address
   * @param destName the destination task name
   * @param destAddr the destination task address
   * @param data the data that is to be transmitted
   */
  DataPacket(std::string originName, std::string originAddr,
             const std::string &destName, std::string destAddr, const std::shared_ptr<IData> &data)
      : destName(destName), originName(originName), destAddr(destAddr), originAddr(originAddr), data(data) {

  }

  /**
   * Gets the destination task name
   * @return the destination task name
   */
  const std::string &getDestName() const {
    return destName;
  }

  /**
   * Gets the origin task name
   * @return the origin task name
   */
  const std::string &getOriginName() const {
    return originName;
  }

  /**
   * Gets the destination task address
   * @return the destination task address
   */
  std::string getDestAddr() const {
    return destAddr;
  }

  /**
   * Gets the origin task address
   * @return the origin task address
   */
  std::string getOriginAddr() const {
    return originAddr;
  }

  /**
   * Gets the data associated with the data packet
   * @return the data
   */
  const std::shared_ptr<IData> &getData() const {
    return data;
  }

 private:
  std::string destName; //!< The destination task name
  std::string originName; //!< The origin task name

  std::string destAddr; //!< The destination task address
  std::string originAddr; //!< The origin task address


  std::shared_ptr<IData> data; //!< The data
};
}
#endif //HTGS_DATAPACKET_HPP
