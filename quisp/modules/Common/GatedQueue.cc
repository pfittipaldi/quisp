
#include "GatedQueue.h"


namespace quisp::modules {

Define_Module(GatedQueue);



void GatedQueue::initialize() {
   Queue::initialize();
   is_busy = false;
}

void GatedQueue::startTransmitting(cMessage *msg) {

  if (BSMTimingNotification* btn = dynamic_cast<BSMTimingNotification*>(msg)) {
      if (btn->getFirstPhotonEmitTime() < simTime()) btn->setFirstPhotonEmitTime(simTime()+next_check_time);
  }


  EV_INFO << "Starting transmission of " << msg << endl;
  is_busy = true;
  int64_t num_bytes = check_and_cast<cPacket *>(msg)->getByteLength();
  send(msg, "line$o");  // inout gate's output
  emit(tx_bytes_signal, (long)num_bytes);

  // Schedule an event for the time when last bit will leave the gate.
  cChannel* chl = gate("line$o")->getTransmissionChannel();
  simtime_t transmission_finish_time = chl->getTransmissionFinishTime();


  EV_INFO << "Transmission will end in " << transmission_finish_time << "\n";

  if (simTime() != SIMTIME_ZERO) {
      EV << "Error";
  }

  // pass end_transmission_event when it ends
  scheduleAt(transmission_finish_time, end_transmission_event);
}

void GatedQueue::handleMessage(cMessage *msg)
{
    if (hasGUI()) {
        bubble("GatedQueue received a message!\n");
      }

    if (auto vco = dynamic_cast<VisCheckOutcome *>(msg)) {
           if (vco->getNext_check_time() == 0) {
               pending_vcr = false;
               msg = (cMessage *)queue.pop();
               emit(queuing_time_signal, simTime() - msg->getTimestamp());
               emit(qlen_signal, queue.getLength());
               startTransmitting(msg);
           } else {
               VisCheckRetry* retry = new VisCheckRetry();
               next_check_time = vco->getNext_check_time();
               scheduleAfter(next_check_time,retry);
           }
          delete vco;
           return;
        }

    if (dynamic_cast<VisCheckRetry *>(msg)) {
        VisCheckRequest* vis_check = new VisCheckRequest();
        vis_check->setOut_gate(gate("line$o")->getNextGate()->getName());
        vis_check->setIndex(gate("line$o")->getNextGate()->getIndex());
        send(vis_check,"to_vc");
        return;
    }

    if (msg->arrivedOn("line$i")) {
        emit(rx_bytes_signal, (long)check_and_cast<cPacket *>(msg)->getByteLength());
        send(msg, "out");
        return;
      }




    if (msg == end_transmission_event) {  // update busy status
        // Transmission finished, we can start next one.
        EV_INFO << "Transmission finished.\n";
        is_busy = false;

        if (queue.isEmpty()) {
          emit(busy_signal, false);
          return;
        }

        if (!is_busy and !queue.isEmpty()) {
        is_busy = true;
        VisCheckRequest* vis_check = new VisCheckRequest();
        vis_check->setOut_gate(gate("line$o")->getNextGate()->getName());
        vis_check->setIndex(gate("line$o")->getNextGate()->getIndex());
        send(vis_check,"to_vc");
        }
    return;
    }

    if (frame_capacity && queue.getLength() >= frame_capacity) {
          EV_INFO << "Received " << msg << " but transmitter busy and queue full: discarding\n";
          emit(drop_signal, (long)check_and_cast<cPacket *>(msg)->getByteLength());
          delete msg;
          return;
        }

    EV_INFO << "Received " << msg << ": queuing up\n";
    msg->setTimestamp();
    queue.insert(msg);
    emit(qlen_signal, queue.getLength());

    if (!is_busy and !queue.isEmpty() and !pending_vcr) {
    pending_vcr = true;
    VisCheckRequest* vis_check = new VisCheckRequest();
    vis_check->setOut_gate(gate("line$o")->getNextGate()->getName());
    vis_check->setIndex(gate("line$o")->getNextGate()->getIndex());
    send(vis_check,"to_vc");
    }
}

} //namespace