#include <omnetpp.h>
#include <omnetpp/csimulation.h>
#include <stdexcept>
#include "omnetpp/clcg32.h"

using namespace omnetpp;
using namespace omnetpp::common;
namespace quisp_test {
class StaticEnv : public cEnvir {
 protected:
  void unsupported() const { throw std::runtime_error("StaticEnv: Unsupported method called"); }
  virtual void alert(const char *msg) override { ::printf("\n<!> %s\n\n", msg); }
  virtual bool askYesNo(const char *msg) override {
    unsupported();
    return false;
  }

 public:
  // constructor, destructor
  StaticEnv() {}
  virtual ~StaticEnv() {}

  // eventlog callback interface
  virtual void objectDeleted(cObject *object) override {}
  virtual void simulationEvent(cEvent *event) override {}
  virtual void messageScheduled(cMessage *msg) override {}
  virtual void messageCancelled(cMessage *msg) override {}
  virtual void beginSend(cMessage *msg) override {}
  virtual void messageSendDirect(cMessage *msg, cGate *toGate, simtime_t propagationDelay, simtime_t transmissionDelay) override {}
  virtual void messageSendHop(cMessage *msg, cGate *srcGate) override {}
  virtual void messageSendHop(cMessage *msg, cGate *srcGate, simtime_t propagationDelay, simtime_t transmissionDelay, bool discard) override {}
  virtual void endSend(cMessage *msg) override {}
  virtual void messageCreated(cMessage *msg) override {}
  virtual void messageCloned(cMessage *msg, cMessage *clone) override {}
  virtual void messageDeleted(cMessage *msg) override {}
  virtual void moduleReparented(cModule *module, cModule *oldparent, int oldId) override {}
  virtual void componentMethodBegin(cComponent *from, cComponent *to, const char *methodFmt, va_list va, bool silent) override {}
  virtual void componentMethodEnd() override {}
  virtual void moduleCreated(cModule *newmodule) override {}
  virtual void moduleDeleted(cModule *module) override {}
  virtual void gateCreated(cGate *newgate) override {}
  virtual void gateDeleted(cGate *gate) override {}
  virtual void connectionCreated(cGate *srcgate) override {}
  virtual void connectionDeleted(cGate *srcgate) override {}
  virtual void displayStringChanged(cComponent *component) override {}
  virtual void undisposedObject(cObject *obj) override;
  virtual void log(cLogEntry *entry) override {}

  // configuration, model parameters
  virtual void preconfigure(cComponent *component) override {}
  virtual void configure(cComponent *component) override {}
  virtual void readParameter(cPar *parameter) override { unsupported(); }
  virtual bool isModuleLocal(cModule *parentmod, const char *modname, int index) override { return true; }
  virtual cXMLElement *getXMLDocument(const char *filename, const char *xpath = nullptr) override {
    unsupported();
    return nullptr;
  }
  virtual cXMLElement *getParsedXMLString(const char *content, const char *xpath = nullptr) override {
    unsupported();
    return nullptr;
  }
  virtual void forgetXMLDocument(const char *filename) override {}
  virtual void forgetParsedXMLString(const char *content) override {}
  virtual void flushXMLDocumentCache() override {}
  virtual void flushXMLParsedContentCache() override {}
  virtual unsigned getExtraStackForEnvir() const override { return 0; }
  virtual cConfiguration *getConfig() override {
    unsupported();
    return nullptr;
  }
  virtual std::string resolveResourcePath(const char *fileName, cComponentType *context) override { return ""; }
  virtual bool isGUI() const override { return false; }
  virtual bool isExpressMode() const override { return false; }

  // UI functions (see also protected ones)
  virtual void bubble(cComponent *component, const char *text) override {}
  virtual std::string gets(const char *prompt, const char *defaultreply = nullptr) override {
    unsupported();
    return "";
  }
  virtual cEnvir &flush() {
    ::fflush(stdout);
    return *this;
  }

  // RNGs
  virtual int getNumRNGs() const override { return 0; }
  virtual cRNG *getRNG(int k) override {
    auto *rng = new omnetpp::cLCG32();
    rng->initialize(0, 0, 0, 0, 0, 0);
    return rng;
    // return nullptr;
  }

  // output vectors
  virtual void *registerOutputVector(const char *modulename, const char *vectorname) override { return nullptr; }
  virtual void deregisterOutputVector(void *vechandle) override {}
  virtual void setVectorAttribute(void *vechandle, const char *name, const char *value) override {}
  virtual bool recordInOutputVector(void *vechandle, simtime_t t, double value) override { return false; }

  // output scalars
  virtual void recordScalar(cComponent *component, const char *name, double value, opp_string_map *attributes = nullptr) override {}
  virtual void recordStatistic(cComponent *component, const char *name, cStatistic *statistic, opp_string_map *attributes = nullptr) override {}

  virtual void addResultRecorders(cComponent *component, simsignal_t signal, const char *statisticName, cProperty *statisticTemplateProperty) override {}

  // snapshot file
  virtual std::ostream *getStreamForSnapshot() override {
    unsupported();
    return nullptr;
  }
  virtual void releaseStreamForSnapshot(std::ostream *os) override { unsupported(); }

  // misc
  virtual int getArgCount() const override {
    unsupported();
    return 0;
  }
  virtual char **getArgVector() const override {
    unsupported();
    return nullptr;
  }
  virtual int getParsimProcId() const override { return 0; }
  virtual int getParsimNumPartitions() const override { return 1; }
  virtual unsigned long getUniqueNumber() override {
    unsupported();
    return 0;
  }
  virtual bool idle() override { return false; }
  virtual void refOsgNode(osg::Node *scene) override {}
  virtual void unrefOsgNode(osg::Node *scene) override {}
  virtual bool ensureDebugger(cRuntimeError *) override { return false; }

  virtual void getImageSize(const char *imageName, double &outWidth, double &outHeight) override { unsupported(); }
  virtual void getTextExtent(const cFigure::Font &font, const char *text, double &outWidth, double &outHeight, double &outAscent) override { unsupported(); }
  virtual void appendToImagePath(const char *directory) override { unsupported(); }
  virtual void loadImage(const char *fileName, const char *imageName = nullptr) override { unsupported(); }
  virtual cFigure::Rectangle getSubmoduleBounds(const cModule *submodule) override { return cFigure::Rectangle(NAN, NAN, NAN, NAN); }
  virtual double getZoomLevel(const cModule *module) override { return NAN; }
  virtual double getAnimationTime() const override { return 0; }
  virtual double getAnimationSpeed() const override { return 0; }
  virtual double getRemainingAnimationHoldTime() const override { return 0; }

  // lifecycle listeners
  virtual void addLifecycleListener(cISimulationLifecycleListener *listener) override {}
  virtual void removeLifecycleListener(cISimulationLifecycleListener *listener) override {}
  virtual void notifyLifecycleListeners(SimulationLifecycleEventType eventType, cObject *details) override {}
};

void StaticEnv::undisposedObject(cObject *obj) {
  if (!cStaticFlag::insideMain()) {
    ::printf(
        "<!> WARNING: global object variable (DISCOURAGED) detected: "
        "(%s)'%s' at %p\n",
        obj->getClassName(), obj->getFullPath().c_str(), obj);
  }
}
}  // namespace quisp_test