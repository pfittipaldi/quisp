/** \file VisibilityChecker.h
 *
 *  \brief VisibilityChecker
 */
#pragma once

#include <omnetpp.h>
#include "omnetpp/simtime.h"
#include "channels/FSChannel.h"
#include "messages/gatedqueue_messages_m.h"

using namespace omnetpp;
using namespace quisp::messages;

/** \class VisibilityChecker VisibilityChecker.cc
 *
 *  \brief VisibilityChecker: Crude, duty-cycle based model for the satellite orbiting in and out of sight.
 */
namespace quisp::modules::Satellite {
class VisibilityChecker : public cSimpleModule {
 public:
  VisibilityChecker();
  ~VisibilityChecker();

 protected:
  virtual void initialize() override;
  virtual void handleMessage(cMessage *msg) override;
  void toggleVisibility();
  double orbital_period;
  double vis_start_coeff;
  double vis_end_coeff;
};
}