/********************************************************************
**                                                                 **
** File   : src/ConversationalGroupTracker.h                       **
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

#include <boost/shared_ptr.hpp>
#include <fformation/Options.h>
#include <rst/hri/ConversationalGroupCollection.pb.h>

class ConversationalGroupTracker {
public:
  typedef rst::hri::ConversationalGroup Group;
  typedef rst::hri::ConversationalGroupCollection Groups;
  typedef boost::shared_ptr<Groups> GroupsPtr;

  ConversationalGroupTracker(const fformation::Options &options);

  void track(GroupsPtr update);

private:
  GroupsPtr _current;
};
