/********************************************************************
**                                                                 **
** File   : src/rsb/RsbClassificator.cpp                           **
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

#include "RsbClassificator.h"
#include <fformation/Exception.h>

// need to turn off warnings for rsb includes
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <rsb/Event.h>
#include <rsb/EventId.h>
#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/Informer.h>
#include <rsb/Listener.h>
#include <rsb/filter/TypeFilter.h>
#include <rst/generic/Dictionary.pb.h>
#include <rst/hri/ConversationalGroupCollection.pb.h>
#include <rst/hri/PersonHypotheses.pb.h>
#include <rsb/converter/ProtocolBufferConverter.h>
#include <rsb/converter/Repository.h>
#pragma GCC diagnostic pop


using fformation::GroupDetector;
using fformation::Timestamp;
using fformation::Person;
using fformation::PersonId;
using fformation::Pose2D;
using fformation::Position2D;
using fformation::RotationRadian;
using fformation::Classification;
using fformation::Observation;

typedef rst::hri::ConversationalGroupCollection OutType;
typedef boost::shared_ptr<OutType> OutTypePtr;
typedef rst::hri::PersonHypotheses InType;
typedef boost::shared_ptr<InType> InTypePtr;
typedef boost::optional<Pose2D> OPose2D;

#define DEBUG_LOG(x)                                                           \
  { std::cerr << __FILE__ << "[" << __LINE__ << "]:\t" x << std::endl; }

#define ASSERT(test, message)                                                  \
  {                                                                            \
    if (!(test)) {                                                             \
      std::stringstream str;                                                   \
      str << message;                                                          \
      throw fformation::Exception(str.str());                                  \
    }                                                                          \
  }

class FrameTransform {
public:
  Pose2D transform(const rst::geometry::Translation &t) const {
    return Pose2D(Position2D(t.x(), t.y()));
  }

  Pose2D transform(const rst::geometry::Translation &t,
                   const rst::geometry::Rotation &r) const {
    if (r.has_frame_id()) {
      DEBUG_LOG("ignoring rotation in frame " << r.frame_id());
    } else {
      DEBUG_LOG("ignoring rotation without frame " << r.frame_id());
    }
    if (t.has_frame_id()) {
      DEBUG_LOG("position from frame " << r.frame_id());
    } else {
      DEBUG_LOG("position with empty frame " << r.frame_id());
    }
    return Pose2D(Position2D(t.x(), t.y()));
  }
};

class GroupTracker {
public:
  void track(OutTypePtr current) {
    DEBUG_LOG("not tracking");
    _last = current;
  }

private:
  OutTypePtr _last;
};

static Timestamp createTimestamp(rsb::EventPtr event) {
  return Timestamp(event->getId().getSequenceNumber());
}

static OPose2D findHeadPose(const rst::hri::PersonHypothesis &person,
                            const FrameTransform &tf) {
  if (person.has_head()) {
    auto location = person.head().shape().transformation();
    return OPose2D(tf.transform(location.translation(), location.rotation()));
  }
  return  OPose2D();
}

static OPose2D findFacePose(const rst::hri::PersonHypothesis &person,
                            const FrameTransform &tf) {
  if (person.has_face() && person.face().has_location()) {
    if (!person.face().has_orientation()) {
      DEBUG_LOG("create from face with position and orentation");
      return OPose2D(
          tf.transform(person.face().location(), person.face().orientation()));
    } else {
      DEBUG_LOG("Create from face without orientation");
      return OPose2D(tf.transform(person.face().location()));
    }
  }
  return OPose2D();
}

static OPose2D findBodyPose(const rst::hri::PersonHypothesis &person,
                            const FrameTransform &tf) {
  if (person.has_face() && person.face().has_location()) {
    if (!person.face().has_orientation()) {
      DEBUG_LOG("create from body with position and orentation");
      return OPose2D(
          tf.transform(person.face().location(), person.face().orientation()));
    } else {
      DEBUG_LOG("Create from body without orientation");
      return OPose2D(tf.transform(person.face().location()));
    }
  }
  return OPose2D();
}

static std::vector<OPose2D>
findPersonsPose(const rst::hri::PersonHypothesis &person,
                const FrameTransform &tf) {
  return std::vector<OPose2D>({
      findHeadPose(person, tf), findFacePose(person, tf),
      findBodyPose(person, tf),
  });
}

static Pose2D findBestPose(const std::vector<OPose2D> &poses) {
  OPose2D best;
  // find the first with position and rotation or at least the first with
  // position
  for (auto pose : poses) {
    if (pose) {
      if (pose.get().rotation()) {
        best = pose;
        break;
      } else if (!best) {
        best = pose;
      }
    }
  }
  if (best) {
    return best.get();
  }
  throw fformation::Exception("No position information found.");
}

static std::vector<Person> createPersons(rsb::EventPtr event,
                                         const FrameTransform &tf) {
  InTypePtr data = boost::static_pointer_cast<InType>(event->getData());
  std::vector<Person> result;
  result.reserve(data->persons_size());
  for (int i = 0; i < data->persons_size(); ++i) {
    try {
      result.push_back(Person(PersonId::from(i), findBestPose(findPersonsPose(
                                                     data->persons(i), tf))));
    } catch (const fformation::Exception &e) {
      std::cerr << "Error while searching for pose of person #" << i
                << " in rsb event . Person: " << data->persons(i).DebugString()
                << ". Error: " << e.what() << std::endl;
    }
  }
  return result;
}

static Observation createObservation(rsb::EventPtr data,
                                     const FrameTransform &tf) {
  return Observation(createTimestamp(data), createPersons(data, tf));
}

static void fillGroup(rst::hri::ConversationalGroup *fill,
                      const fformation::IdGroup &group, InTypePtr &source) {
  for (auto id : group.persons()) {
    int i = id.as<int>();
    ASSERT(i >= 0, "Person id cannot be smaller than 0.");
    ASSERT(i < source->persons_size(),
           "Person id cannot be higher than the amount of persons.");
    auto person = fill->add_persons();
    *person = source->persons(i);
  }
}

OutTypePtr createResultData(const Classification &cl, InTypePtr source) {
  OutTypePtr result(new OutType());
  for (auto group : cl.idGroups()) {
    fillGroup(result->add_element(), group, source);
  }
  return result;
}

rsb::EventPtr createEvent(OutTypePtr data, rsb::EventPtr &from) {
  rsb::EventPtr result(new rsb::Event());
  for (auto cause : from->getCauses()) {
    result->addCause(cause);
  }
  result->addCause(from->getId());
  result->setData(data);
  return result;
}

RsbClassificator::RsbClassificator(GroupDetector::Ptr detector,
                                   const std::string &in_scope,
                                   const std::string &out_scope)
    : _detector(nullptr), _listener(nullptr), _informer(nullptr),
      _transform(new FrameTransform()), _tracker(new GroupTracker) {
  _detector.swap(detector);
  _listener = rsb::getFactory().createListener(in_scope);
  _informer = rsb::getFactory().createInformer<OutType>(out_scope);
  _listener->addFilter(
      rsb::filter::FilterPtr(rsb::filter::TypeFilter::createForType<InType>()));

  rsb::EventFunctionHandler *eh = new rsb::EventFunctionHandler(
      [this](rsb::EventPtr event) { this->handle(event); });
  _handler = rsb::HandlerPtr(eh);
  _listener->addHandler(_handler);
}

void RsbClassificator::handle(rsb::EventPtr event) {
  auto observation = createObservation(event, *_transform);
  auto classification = _detector->detect(observation);
  auto result = createResultData(
      classification, boost::static_pointer_cast<InType>(event->getData()));
  _tracker->track(result);
  auto result_event = createEvent(result, event);
  _informer->publish(result_event);
}
