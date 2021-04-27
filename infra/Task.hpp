#ifndef ANALYSISTREE_INFRA_TASK_HPP_
#define ANALYSISTREE_INFRA_TASK_HPP_

#include <set>

#include "Configuration.hpp"
#include "Constants.hpp"
#include "EventHeader.hpp"

#include "Chain.hpp"
#include "Cuts.hpp"

class TChain;
class TTree;
class TFile;

namespace AnalysisTree {

class Configuration;
class DataHeader;

class Task {

 public:
  Task() = default;
  virtual ~Task() = default;

  virtual void Init() = 0;
  virtual void Exec() = 0;
  virtual void Finish() = 0;

  void PreInit();

  void SetInConfiguration(const Configuration* config) { config_ = config; }
  void SetDataHeader(const DataHeader* data_header) { data_header_ = data_header; }

  void SetInputBranchNames(const std::set<std::string>& br) { in_branches_ = br; }

  ANALYSISTREE_ATTR_NODISCARD const std::set<std::string>& GetInputBranchNames() const { return in_branches_; }

  ANALYSISTREE_ATTR_NODISCARD bool IsGoodEvent(const EventHeader& event_header) const {
    return event_cuts_ ? event_cuts_->Apply(event_header) : true;
  }

  ANALYSISTREE_ATTR_NODISCARD bool IsGoodEvent(const Chain& t) const {
    if(!event_cuts_) return true;
    auto br_name = event_cuts_->GetBranches().begin();
    auto* eh = ANALYSISTREE_UTILS_GET<EventHeader*>(t.GetPointerToBranch(*br_name));
    return event_cuts_->Apply(*eh);
  }

  void SetEventCuts(Cuts* cuts){
    if(cuts->GetBranches().size() != 1){
      throw std::runtime_error("Event cuts on only 1 branch are allowed at the moment!");
    }
    event_cuts_ = cuts;
  }

 protected:
  const Configuration* config_{nullptr};
  const DataHeader* data_header_{nullptr};

  Cuts* event_cuts_{nullptr};

  std::set<std::string> in_branches_{};
  bool is_init_{false};

  ClassDef(Task, 1);
};

}// namespace AnalysisTree

#endif//ANALYSISTREE_INFRA_TASK_HPP_
