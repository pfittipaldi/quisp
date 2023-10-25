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
using namespace messages;

/** \class VisibilityChecker VisibilityChecker.cc
 *
 *  \brief VisibilityChecker: Crude, duty-cycle based model for the satellite orbiting in and out of sight.
 */
class VisibilityChecker : public cSimpleModule {
 public:
  VisibilityChecker();
  ~VisibilityChecker();

 protected:
  virtual void initialize() override;
  virtual void handleMessage(cMessage *msg) override;
  void toggleVisibility();
  double orbital_period;
  double up_time;
  double down_time;
  bool is_satellite;
  bool is_visible;
  double next_check_time;
};
