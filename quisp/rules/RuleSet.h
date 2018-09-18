/** \file RuleSet.h
 *  \authors cldurand
 *  \date 2018/06/25
 *
 *  \brief RuleSet
 */
#ifndef QUISP_RULES_RULESET_H_
#define QUISP_RULES_RULESET_H_

#include "Rule.h"
#include <omnetpp.h>

namespace quisp {
namespace rules {

/** \class RuleSet RuleSet.h
 *
 * \brief Set of rules for the RuleEngine.
 */

class RuleSet {
    public:
      std::list<pRule> rules;
      int owner;

      RuleSet(int o) { owner = o; }
      void addRule(Rule * r) { rules.push_back(pRule(r)); };

class RuleSet : public std::list<pRule> {
    private:
        int owner;
    public:
        RuleSet(int o) : std::list<pRule> () { owner = o; }
        void addRule(Rule * r) { push_back(pRule(r)); };
        void addRule(pRule& r) { push_back(pRule(std::move(r))); };

};

} // namespace rules
} // namespace quisp

#endif//QUISP_RULES_RULESET_H_
