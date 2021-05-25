/** \file RealTimeController.h
 *  \todo clean Clean code when it is simple.
 *  \todo doc Write doxygen documentation.
 *  \authors cldurand,takaakimatsuo
 *  \date 2018/03/19
 *
 *  \brief RealTimeController
 */
#ifndef QUISP_MODULES_REALTIMECONTROLLER_H_
#define QUISP_MODULES_REALTIMECONTROLLER_H_

#include "IRealTimeController.h"
#include "utils/ComponentProvider.h"

using namespace omnetpp;

namespace quisp {
namespace modules {

/** \class RealTimeController RealTimeController.h
 *  \todo Documentation of the class header.
 *
 *  \brief RealTimeController
 */
class RealTimeController : public IRealTimeController {
 private:
  int myAddress;

 protected:
  virtual void initialize() override;
  virtual void handleMessage(cMessage* msg) override;

 public:
  RealTimeController();
  void EmitPhoton(int qnic_index, int qubit_index, QNIC_type qnic_type, int pulse) override;
  void ReInitialize_StationaryQubit(int qnic_index, int qubit_index, QNIC_type qnic_type, bool consumed) override;
  utils::ComponentProvider provider;
};

}  // namespace modules
}  // namespace quisp

#endif /* QUISP_MODULES_REALTIMECONTROLLER_H_ */
