#include "Action.h"

namespace quisp::rules {

Action::Action(int partner_addr, QNIC_type qnic_type, int qnic_id) {
  partner_address.push_back(partner_addr);
  qnic_types.push_back(qnic_type);
  qnic_ids.push_back(qnic_id);
};

Purification::Purification(PurType purification_type, int partner_addr, QNIC_type qnic_type, int qnic_id)
    : Action(partner_addr, qnic_type, qnic_id), purification_type(purification_type) {}

json Purification::serialize_json() {
  json purification_json;
  purification_json["type"] = "purification";
  purification_json["options"]["purification_type"] = purification_type;
  purification_json["options"]["partner_address"] = partner_address;
  purification_json["options"]["qnic_type"] = qnic_types;
  purification_json["options"]["qnic_id"] = qnic_ids;
  return purification_json;
}

EntanglementSwapping::EntanglementSwapping(std::vector<int> partner_addr, std::vector<QNIC_type> qnic_type, std::vector<int> qnic_id) : Action(partner_addr, qnic_type, qnic_id) {}

json EntanglementSwapping::serialize_json() {
  json swapping_json;
  swapping_json["type"] = "swapping";
  swapping_json["options"]["partner_address"] = partner_address;  // should be two
  swapping_json["options"]["qnic_type"] = qnic_types;
  swapping_json["options"]["qnic_id"] = qnic_ids;
  return swapping_json;
}

Wait::Wait(int partner_addr, QNIC_type qnic_type, int qnic_id) : Action(partner_addr, qnic_type, qnic_id) {}

json Wait::serialize_json() {
  json wait_json;
  wait_json["type"] = "wait";
  wait_json["options"]["partner_address"] = partner_address;  // should be two
  wait_json["options"]["qnic_type"] = qnic_types;
  wait_json["options"]["qnic_id"] = qnic_ids;
  return wait_json;
}

Tomography::Tomography(int num_measurement, int partner_addr, QNIC_type qnic_type, int qnic_id) : Action(partner_addr, qnic_type, qnic_id), num_measurement(num_measurement) {}

json Tomography::serialize_json() {
  json tomography_json;
  tomography_json["type"] = "tomography";
  tomography_json["options"]["num_measure"] = num_measurement;
  tomography_json["options"]["partner_address"] = partner_address;
  tomography_json["options"]["qnic_type"] = qnic_types;
  tomography_json["options"]["qnic_id"] = qnic_ids;
  return tomography_json;
}

}  // namespace quisp::rules