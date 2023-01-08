/** \file StationaryQubit.cc
 *  \authors cldurand,takaakimatsuo
 *  \date 2018/03/14
 *
 *  \brief StationaryQubit
 */
#include "StationaryQubit.h"

#include <PhotonicQubit_m.h>
#include <messages/classical_messages.h>
#include <omnetpp.h>
#include <bitset>
#include <stdexcept>
#include <unordered_set>
#include <unsupported/Eigen/KroneckerProduct>
#include <unsupported/Eigen/MatrixFunctions>
#include <vector>
#include "modules/QNIC/StationaryQubit/QubitId.h"
#include "omnetpp/cexception.h"

using namespace Eigen;

using quisp::messages::PhotonicQubit;
using quisp::modules::qubit_id::QubitId;
using quisp::types::CliffordOperator;
using quisp::types::EigenvalueResult;
using quisp::types::MeasurementOutcome;
using quisp::types::MeasureXResult;
using quisp::types::MeasureYResult;
using quisp::types::MeasureZResult;

namespace quisp {
namespace modules {

Define_Module(StationaryQubit);

StationaryQubit::StationaryQubit() : provider(utils::ComponentProvider{this}) {}

/**
 * \brief Initialize StationaryQubit
 *
 * Omnet called method to initialize objects.
 *
 */
void StationaryQubit::initialize() {
  // read and set parameters
  emission_success_probability = par("emission_success_probability");

  // Get parameters from omnet
  stationary_qubit_address = par("stationary_qubit_address");
  node_address = par("node_address");
  qnic_address = par("qnic_address");
  qnic_type = par("qnic_type");
  qnic_index = par("qnic_index");
  emission_jittering_standard_deviation = par("emission_jittering_standard_deviation").doubleValue();

  /* e^(t/T1) energy relaxation, e^(t/T2) phase relaxation. Want to use only 1/10 of T1 and T2 in general.*/

  // initialize variables for graph state representation tracking
  vertex_operator = CliffordOperator::H;

  backend = provider.getQuantumBackend();
  auto config = prepareBackendQubitConfiguration(par("overwrite_backend_qubit_config").boolValue());
  qubit_ref = backend->getQubit(new QubitId(node_address, qnic_index, qnic_type, stationary_qubit_address), std::move(config));
  if (qubit_ref == nullptr) throw std::runtime_error("qubit_ref nullptr error");
  setFree(false);

  // watch variables to show them in the GUI
  WATCH(emitted_time);
  WATCH(is_busy);
}

std::unique_ptr<IConfiguration> StationaryQubit::prepareBackendQubitConfiguration(bool overwrite) {
  auto conf = backend->getDefaultConfiguration();
  if (!overwrite) return conf;
  if (auto et_conf = dynamic_cast<backend::ErrorTrackingConfiguration *>(conf.get())) {
    et_conf->measurement_x_err_rate = par("x_measurement_error_rate").doubleValue();
    et_conf->measurement_y_err_rate = par("y_measurement_error_rate").doubleValue();
    et_conf->measurement_z_err_rate = par("z_measurement_error_rate").doubleValue();

    et_conf->h_gate_err_rate = par("h_gate_error_rate").doubleValue();
    et_conf->h_gate_x_err_ratio = par("h_gate_x_error_ratio").doubleValue();
    et_conf->h_gate_y_err_ratio = par("h_gate_y_error_ratio").doubleValue();
    et_conf->h_gate_z_err_ratio = par("h_gate_z_error_ratio").doubleValue();

    et_conf->x_gate_err_rate = par("x_gate_error_rate").doubleValue();
    et_conf->x_gate_x_err_ratio = par("x_gate_x_error_ratio").doubleValue();
    et_conf->x_gate_y_err_ratio = par("x_gate_y_error_ratio").doubleValue();
    et_conf->x_gate_z_err_ratio = par("x_gate_z_error_ratio").doubleValue();

    et_conf->z_gate_err_rate = par("z_gate_error_rate").doubleValue();
    et_conf->z_gate_x_err_ratio = par("z_gate_x_error_ratio").doubleValue();
    et_conf->z_gate_y_err_ratio = par("z_gate_y_error_ratio").doubleValue();
    et_conf->z_gate_z_err_ratio = par("z_gate_z_error_ratio").doubleValue();

    et_conf->cnot_gate_err_rate = par("cnot_gate_error_rate").doubleValue();
    et_conf->cnot_gate_iz_err_ratio = par("cnot_gate_iz_error_ratio").doubleValue();
    et_conf->cnot_gate_zi_err_ratio = par("cnot_gate_zi_error_ratio").doubleValue();
    et_conf->cnot_gate_zz_err_ratio = par("cnot_gate_zz_error_ratio").doubleValue();
    et_conf->cnot_gate_ix_err_ratio = par("cnot_gate_ix_error_ratio").doubleValue();
    et_conf->cnot_gate_xi_err_ratio = par("cnot_gate_xi_error_ratio").doubleValue();
    et_conf->cnot_gate_xx_err_ratio = par("cnot_gate_xx_error_ratio").doubleValue();
    et_conf->cnot_gate_iy_err_ratio = par("cnot_gate_iy_error_ratio").doubleValue();
    et_conf->cnot_gate_yi_err_ratio = par("cnot_gate_yi_error_ratio").doubleValue();
    et_conf->cnot_gate_yy_err_ratio = par("cnot_gate_yy_error_ratio").doubleValue();

    et_conf->memory_x_err_rate = par("memory_x_error_rate").doubleValue();
    et_conf->memory_y_err_rate = par("memory_y_error_rate").doubleValue();
    et_conf->memory_z_err_rate = par("memory_z_error_rate").doubleValue();
    et_conf->memory_excitation_rate = par("memory_energy_excitation_rate").doubleValue();
    et_conf->memory_relaxation_rate = par("memory_energy_relaxation_rate").doubleValue();
    et_conf->memory_completely_mixed_rate = par("memory_completely_mixed_rate").doubleValue();
  }
  return conf;
}

void StationaryQubit::finish() {}

/**
 * \brief handle PhotonicQubit generated by StatinoryQubit itself
 *
 * \param msg is the PhotonicQubit message
 */
void StationaryQubit::handleMessage(cMessage *msg) {
  if (!msg->isSelfMessage()) {
    throw cRuntimeError("StationaryQubit::handleMessage: message from outside is not expected");
  }
  bubble("Got a photon!!");
  setBusy();
  double rand = dblrand();
  if (rand < (1 - emission_success_probability)) {
    PhotonicQubit *pk = check_and_cast<PhotonicQubit *>(msg);
    pk->setPhotonLost(true);
    send(pk, "tolens_quantum_port$o");
  } else {
    send(msg, "tolens_quantum_port$o");
  }
}

MeasureXResult StationaryQubit::correlationMeasureX() { return qubit_ref->correlationMeasureX(); }
MeasureYResult StationaryQubit::correlationMeasureY() { return qubit_ref->correlationMeasureY(); }
MeasureZResult StationaryQubit::correlationMeasureZ() { return qubit_ref->correlationMeasureZ(); }
EigenvalueResult StationaryQubit::localMeasureX() { return qubit_ref->localMeasureX(); }
EigenvalueResult StationaryQubit::localMeasureY() {
  error("Not Yet Implemented");
  return EigenvalueResult::PLUS_ONE;
}
EigenvalueResult StationaryQubit::localMeasureZ() { return qubit_ref->localMeasureZ(); }

// Convert X to Z, and Z to X error. Therefore, Y error stays as Y.
void StationaryQubit::Hadamard_gate() { qubit_ref->gateH(); }
void StationaryQubit::Z_gate() { qubit_ref->gateZ(); }
void StationaryQubit::X_gate() { qubit_ref->gateX(); }

void StationaryQubit::CNOT_gate(IStationaryQubit *control_qubit) { qubit_ref->gateCNOT(check_and_cast<StationaryQubit *>(control_qubit)->qubit_ref); }

// This is invoked whenever a photon is emitted out from this particular qubit.
void StationaryQubit::setBusy() {
  is_busy = true;
  emitted_time = simTime();
  if (hasGUI()) {
    getDisplayString().setTagArg("i", 1, "red");
  }
}

// Re-initialization of this stationary qubit
// This is called at the beginning of the simulation (in initialization() above), and whenever it is reinitialized via the RealTimeController.
void StationaryQubit::setFree(bool consumed) {
  qubit_ref->setFree();
  is_busy = false;
  locked = false;
  locked_ruleset_id = -1;
  locked_rule_id = -1;
  action_index = -1;
  emitted_time = -1;

  EV_DEBUG << "Freeing this qubit! " << this << " at qnode: " << node_address << " qnic_type: " << qnic_type << " qnic_index: " << qnic_index << "\n";
  if (hasGUI()) {
    if (consumed) {
      bubble("Consumed!");
      getDisplayString().setTagArg("i", 1, "yellow");
    } else {
      bubble("Failed to entangle!");
      getDisplayString().setTagArg("i", 1, "blue");
    }
  }
}

backends::IQubit *StationaryQubit::getEntangledPartner() const { return qubit_ref->getEntangledPartner(); }
void StationaryQubit::assertEntangledPartnerValid() { qubit_ref->assertEntangledPartnerValid(); }

/*To avoid disturbing this qubit.*/
void StationaryQubit::Lock(unsigned long rs_id, int rule_id, int action_id) {
  if (rs_id == -1 || rule_id == -1 || action_id == -1) {
    error("ruleset_id || rule_id || action_id == -1");
  }
  locked = true;
  locked_ruleset_id = rs_id;  // Used to identify what this qubit is locked for.
  locked_rule_id = rule_id;
  action_index = action_id;
  if (hasGUI()) {
    bubble("Locked!");
    getDisplayString().setTagArg("i", 1, "purple");
  }
}

void StationaryQubit::Unlock() {
  locked = false;
  locked_ruleset_id = -1;  // Used to identify what this qubit is locked for.
  locked_rule_id = -1;
  action_index = -1;
  if (hasGUI()) {
    bubble("Unlocked!");
    getDisplayString().setTagArg("i", 1, "pink");
  }
}

bool StationaryQubit::isLocked() { return locked; }

/**
 * \brief Generate photon entangled with the memory
 * \warning Shouldn't we destroy a possibly existing photon object before? <- No, I dont think so...
 */
PhotonicQubit *StationaryQubit::generateEntangledPhoton() {
  Enter_Method("generateEntangledPhoton()");
  auto *photon = new PhotonicQubit("Photon");
  // To simulate the actual physical entangled partner, not what the system thinks!!! we need this.

  // This photon is entangled with.... node_address = node's index
  photon->setNodeEntangledWith(node_address);

  // qnic_address != qnic_index. qnic_index is not unique because there are 3 types.
  photon->setQNICEntangledWith(qnic_address);

  // stationary_qubit_address = StationaryQubit's index
  photon->setStationaryQubitEntangledWith(stationary_qubit_address);
  photon->setQNICtypeEntangledWith(qnic_type);
  photon->setEntangled_with(this);
  return photon;
}

/**
 * \brief Emit photon
 *
 * \param pulse: 0 for nothing, 1 for first, 2 for last, 3 for first and last
 *
 * The stationary qubit shouldn't be already busy.
 */
void StationaryQubit::emitPhoton(int pulse) {
  Enter_Method("emitPhoton()");
  if (is_busy) {
    error("Requested a photon emission to a busy qubit... this should not happen!");
    return;
  }
  PhotonicQubit *pk = generateEntangledPhoton();
  if (pulse & STATIONARYQUBIT_PULSE_BEGIN) pk->setFirst(true);
  if (pulse & STATIONARYQUBIT_PULSE_END) pk->setLast(true);
  if (pulse & STATIONARYQUBIT_PULSE_BOUND) pk->setKind(3);
  float jitter_timing = normal(0, emission_jittering_standard_deviation);
  float abso = fabs(jitter_timing);
  scheduleAt(simTime() + abso, pk);  // cannot send back in time, so only positive lag
}

// This gets direcltly invoked when darkcount happened in BellStateAnalyzer.cc.
[[deprecated]] void StationaryQubit::setCompletelyMixedDensityMatrix() { qubit_ref->setCompletelyMixedDensityMatrix(); }

void StationaryQubit::setEntangledPartnerInfo(IStationaryQubit *partner) {
  // When BSA succeeds, this method gets invoked to store entangled partner information.
  // This will also be sent classically to the partner node afterwards.
  qubit_ref->setEntangledPartner(partner->getBackendQubitRef());
}

backends::IQubit *StationaryQubit::getBackendQubitRef() const { return qubit_ref; }
int StationaryQubit::getPartnerStationaryQubitAddress() const {
  auto *partner_qubit_ref = qubit_ref->getEntangledPartner();
  auto *partner_id = dynamic_cast<const QubitId *const>(partner_qubit_ref->getId());
  if (partner_id == nullptr) cRuntimeError("StationaryQubit::getPartnerStationaryQubitAddress: null partner backend qubit id cast");
  return partner_id->qubit_addr;
}

/* Add another X error. If an X error already exists, then they cancel out */
[[deprecated]] void StationaryQubit::addXerror() { qubit_ref->addErrorX(); }

/* Add another Z error. If an Z error already exists, then they cancel out */
[[deprecated]] void StationaryQubit::addZerror() { qubit_ref->addErrorZ(); }

// Only tracks error propagation. If two booleans (Alice and Bob) agree (truetrue or falsefalse), keep the purified ebit.
bool StationaryQubit::Xpurify(IStationaryQubit *resource_qubit /*Controlled*/) { return qubit_ref->purifyX(check_and_cast<StationaryQubit *>(resource_qubit)->qubit_ref); }

bool StationaryQubit::Zpurify(IStationaryQubit *resource_qubit /*Target*/) { return qubit_ref->purifyZ(check_and_cast<StationaryQubit *>(resource_qubit)->qubit_ref); }

MeasurementOutcome StationaryQubit::measure_density_independent() { return qubit_ref->measureDensityIndependent(); }

// protected internal graph backend functions

void StationaryQubit::applyClifford(types::CliffordOperator op) { this->vertex_operator = clifford_application_lookup[(int)op][(int)(this->vertex_operator)]; }

void StationaryQubit::applyRightClifford(types::CliffordOperator op) { this->vertex_operator = clifford_application_lookup[(int)(this->vertex_operator)][(int)op]; }

bool StationaryQubit::isNeighbor(StationaryQubit *another_qubit) { return this->neighbors.find(another_qubit) != this->neighbors.end(); }

void StationaryQubit::addEdge(StationaryQubit *another_qubit) {
  if (another_qubit == this) error("adding edge to self is not allowed");
  this->neighbors.insert(another_qubit);
  another_qubit->neighbors.insert(this);
}

void StationaryQubit::deleteEdge(StationaryQubit *another_qubit) {
  this->neighbors.erase(another_qubit);
  another_qubit->neighbors.erase(this);
}

void StationaryQubit::toggleEdge(StationaryQubit *another_qubit) {
  if (this->isNeighbor(another_qubit)) {
    this->deleteEdge(another_qubit);
  } else {
    this->addEdge(another_qubit);
  }
}

void StationaryQubit::removeAllEdges() {
  for (auto *v : this->neighbors) {
    v->neighbors.erase(this);
  }
  this->neighbors.clear();
}

void StationaryQubit::localComplement() {
  auto it_end = this->neighbors.end();
  for (auto it_u = this->neighbors.begin(); it_u != it_end; it_u++) {
    auto it_v = std::next(it_u);
    for (; it_v != it_end; it_v++) {
      (*it_u)->toggleEdge(*it_v);
    }
  }
  for (auto *v : this->neighbors) {
    v->applyRightClifford(CliffordOperator::S);
  }
  this->applyRightClifford(CliffordOperator::RX_INV);
}

void StationaryQubit::removeVertexOperation(StationaryQubit *qubit_to_avoid) {
  if (this->neighbors.empty() || this->vertex_operator == types::CliffordOperator::Id) {
    return;
  }
  auto *swapping_partner_temp = qubit_to_avoid;
  for (auto *v : this->neighbors) {
    if (v != qubit_to_avoid) {
      swapping_partner_temp = v;
      break;
    }
  }
  auto swapping_partner = swapping_partner_temp;
  std::string decomposition_string = decomposition_table[(int)this->vertex_operator];
  for (int i = decomposition_string.size() - 1; i >= 0; i--) {
    if (decomposition_string[i] == 'V') {
      this->localComplement();
    } else {
      // 'U'
      swapping_partner->localComplement();
    }
  }
}

void StationaryQubit::applyPureCZ(StationaryQubit *another_qubit) {
  auto *aq = another_qubit;
  this->removeVertexOperation(aq);
  aq->removeVertexOperation(this);
  this->removeVertexOperation(aq);

  bool has_edge = this->isNeighbor(aq);
  int current_vop = (int)(this->vertex_operator);
  int aq_vop = (int)(aq->vertex_operator);
  this->vertex_operator = controlled_Z_lookup_node_1[has_edge][current_vop][aq_vop];
  aq->vertex_operator = controlled_Z_lookup_node_2[has_edge][current_vop][aq_vop];
  if (has_edge != controlled_Z_lookup_edge[has_edge][current_vop][aq_vop]) {
    this->toggleEdge(aq);
  }
}

EigenvalueResult StationaryQubit::graphMeasureZ() {
  auto vop = this->vertex_operator;
  auto result = EigenvalueResult::PLUS_ONE;
  if (this->neighbors.empty()) {
    switch (vop) {
      case CliffordOperator::H:
      case CliffordOperator::RY_INV:
      case CliffordOperator::S_INV_RY_INV:
      case CliffordOperator::S_RY_INV:
        break;
      case CliffordOperator::RY:
      case CliffordOperator::S_INV_RY:
      case CliffordOperator::S_RY:
      case CliffordOperator::Z_RY:
        result = EigenvalueResult::MINUS_ONE;
        break;
      default:
        result = (dblrand() < 0.5) ? EigenvalueResult::PLUS_ONE : EigenvalueResult::MINUS_ONE;
    }
  } else {
    this->removeVertexOperation(this);  // nothing to be avoided
    result = (dblrand() < 0.5) ? EigenvalueResult::PLUS_ONE : EigenvalueResult::MINUS_ONE;
    this->removeAllEdges();
  }
  this->vertex_operator = (result == EigenvalueResult::PLUS_ONE) ? CliffordOperator::H : CliffordOperator::RY;
  return result;
}

// public member functions

void StationaryQubit::cnotGate(IStationaryQubit *control_qubit) {
  // apply memory error
  this->applyClifford(CliffordOperator::H);  // use apply Clifford for pure operation
  this->applyPureCZ((StationaryQubit *)control_qubit);
  this->applyClifford(CliffordOperator::H);
  // apply CNOT error
}

void StationaryQubit::hadamardGate() {
  // apply memory error
  this->applyClifford(CliffordOperator::H);
  // apply single qubit gate error
}
void StationaryQubit::zGate() {
  // apply memory error
  this->applyClifford(CliffordOperator::Z);
  // apply single qubit gate error
}
void StationaryQubit::xGate() {
  // apply memory error
  this->applyClifford(CliffordOperator::X);
  // apply single qubit gate error
}
void StationaryQubit::sGate() {
  // apply memory error
  this->applyClifford(CliffordOperator::S);
  // apply single qubit gate error
}
void StationaryQubit::sdgGate() {
  // apply memory error
  this->applyClifford(CliffordOperator::S_INV);
  // apply single qubit gate error
}
void StationaryQubit::excite() {
  auto result = this->measureZ();
  if (result == EigenvalueResult::PLUS_ONE) {
    this->applyClifford(CliffordOperator::X);
  }
}
void StationaryQubit::relax() {
  auto result = this->measureZ();
  if (result == EigenvalueResult::MINUS_ONE) {
    this->applyClifford(CliffordOperator::X);
  }
}

EigenvalueResult StationaryQubit::measureX() {
  this->hadamardGate();
  return this->measureZ();
}

EigenvalueResult StationaryQubit::measureY() {
  this->sdgGate();
  this->hadamardGate();
  return this->measureZ();
}

EigenvalueResult StationaryQubit::measureZ() {
  // apply memory error
  auto result = this->graphMeasureZ();
  // apply measurement error
  return result;
}

// initialize static variables
CliffordOperator StationaryQubit::clifford_application_lookup[24][24] =
#include "clifford_application_lookup.tbl"
    ;

bool StationaryQubit::controlled_Z_lookup_edge[2][24][24] =
#include "cz_lookup_edge.tbl"
    ;

CliffordOperator StationaryQubit::controlled_Z_lookup_node_1[2][24][24] =
#include "cz_lookup_node_1.tbl"
    ;

CliffordOperator StationaryQubit::controlled_Z_lookup_node_2[2][24][24] =
#include "cz_lookup_node_2.tbl"
    ;

std::string StationaryQubit::decomposition_table[24] = {
    "", "VV", "UUVV", "UU", "VVV", "V", "VUU", "UUV", "UVUUU", "UUUVU", "UVVVU", "UVU", "U", "UUU", "VVU", "UVV", "UVVV", "UV", "UVUU", "UUUV", "VVVU", "VU", "VUUU", "UUVU",
};

}  // namespace modules
}  // namespace quisp
