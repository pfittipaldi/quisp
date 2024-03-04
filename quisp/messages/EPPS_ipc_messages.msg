import base_messages;

namespace quisp::messages;

packet EPPSNotificationTimeout {}

packet QNICReport extends Header {	
    int partnerQNICAddr;
    int partnerQNICIndex;
    int destQNICIndex;
    bool corrections;
}