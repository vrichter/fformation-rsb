/********************************************************************
**                                                                 **
** File   : src/RsbClassificator.h                                 **
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

#include <fformation/GroupDetector.h>
#include <rsb/Informer.h>
#include <rsb/Listener.h>
#include <rst/hri/ConversationalGroupCollection.pb.h>

class FrameTransform;
class GroupTracker;

class RsbClassificator {
public:
  RsbClassificator(fformation::GroupDetector::Ptr detector,
                   const std::string &in_scope, const std::string &out_scope);

private:
  void handle(rsb::EventPtr event);

  fformation::GroupDetector::Ptr _detector;
  rsb::ListenerPtr _listener;
  rsb::Informer<rst::hri::ConversationalGroupCollection>::Ptr _informer;
  std::shared_ptr<FrameTransform> _transform;
  std::shared_ptr<GroupTracker> _tracker;
  rsb::HandlerPtr _handler;
};
