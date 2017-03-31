//
// Created by tjb3 on 3/31/17.
//

#ifndef HTGS_SIMPLECUDADATA_H
#define HTGS_SIMPLECUDADATA_H

#include <htgs/types/Types.hpp>
#include <htgs/api/IData.hpp>

class SimpleCudaData : public htgs::IData {
 public:
  const htgs::m_data_t<double> &getCudaData() const {
    return cudaData;
  }
  void setCudaData(const htgs::m_data_t<double> &cudaData) {
    this->cudaData = cudaData;
  }

 private:
  htgs::m_data_t<double> cudaData;


};

#endif //HTGS_SIMPLECUDADATA_H
