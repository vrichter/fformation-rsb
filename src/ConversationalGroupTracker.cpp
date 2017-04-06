/********************************************************************
**                                                                 **
** File   : src/ConversationalGroupTracker.cpp                     **
** Authors: Viktor Richter                                         **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
********************************************************************/

#include "ConversationalGroupTracker.h"
#include "Macros.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <mutex>
#include <thread>

static void generate_new_id(ConversationalGroupTracker::Group& to){
  static boost::uuids::basic_random_generator<boost::mt19937> generator;
  static std::mutex mutex;
  static ::google::protobuf::uint32 simple_id = 0;
  std::lock_guard<std::mutex> lock(mutex);
  to.mutable_tracking_info()->set_id(++simple_id);
  to.mutable_tracking_info()->set_persistent_id(boost::uuids::to_string(generator()));
}

static void copy_id(ConversationalGroupTracker::Group& to, const ConversationalGroupTracker::Group& from) {
  if(!to.has_tracking_info()){
    to.set_allocated_tracking_info(rst::tracking::TrackingInfo::default_instance().New());
  }
  to.mutable_tracking_info()->set_id(from.tracking_info().id());
  to.mutable_tracking_info()->set_persistent_id(from.tracking_info().persistent_id());
}

struct GroupMatch {
  double overlap;
  const ConversationalGroupTracker::Group* group;
};

static bool isInGroup(const rst::hri::PersonHypothesis& person, const rst::hri::ConversationalGroup& group){
  size_t missing_id_counter = 0;
  for(auto other : group.persons()){
    if(!person.tracking_info().has_id()){
      WARNING_LOG("Person has no tracking_info.id: " << person.ShortDebugString());
      break;
    }
    if(!other.tracking_info().has_id()){
      ++missing_id_counter;
      continue;
    }
    if(person.tracking_info().id() == other.tracking_info().id()){
        return true;
    }
  }
  if(missing_id_counter > 0){
    WARNING_LOG("Group contains persons without tracking_info.id (" << missing_id_counter << "/" << group.persons_size());
  }
  return false;
}

static double calculateOverlap(const ConversationalGroupTracker::Group& a, const ConversationalGroupTracker::Group& b){
  size_t match = 0;
  for(auto person : a.persons()){
    if(isInGroup(person,b)){
      match++;
    }
  }
  return double(match) / double(std::max(a.persons_size(),b.persons_size()));
}

static GroupMatch findBestMatch(ConversationalGroupTracker::Group& match, ConversationalGroupTracker::GroupsPtr against){
  GroupMatch result{0,nullptr};
  if(against.get() == nullptr){
    return result;
  }
  for(auto &group : against->element()){
    double overlap = calculateOverlap(match,group);
    if(overlap > result.overlap){
      result.overlap = overlap;
      result.group = &group;
    }
  }
  return result;
}

ConversationalGroupTracker::ConversationalGroupTracker(
    const fformation::Options &options) {}

void ConversationalGroupTracker::track(GroupsPtr update) {
  for (int i = 0; i < update->element_size(); ++i) {
    Group* group = update->mutable_element(i);
    auto match = findBestMatch(*group, _current);
    if (match.overlap > 0.5) {
      copy_id(*group,*match.group);
    } else {
      generate_new_id(*group);
    }
  }
  _current = update;
}
