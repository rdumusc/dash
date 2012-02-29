
/* Copyright (c) 2011, EFPL/Blue Brain Project
 *                     Stefan Eilemann <stefan.eilemann@epfl.ch> 
 *
 * This file is part of DASH <https://github.com/BlueBrain/dash>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "Commit.h"

#include "Attribute.h"
#include "Change.h"
#include "Node.h"

#include <dash/Context.h>
#include <dash/Node.h>

namespace dash
{
namespace detail
{

Commit::Commit()
     : changes_( new Changes )
{
}

Commit::~Commit()
{
}

void Commit::add( const Change& change )
{
    changes_->push_back( change );
}

void Commit::apply() const
{
    for( ChangesIter i = changes_->begin(); i != changes_->end(); ++i )
    {
        Change& change = *i;
        switch( change.type )
        {
          case Change::NODE_INSERT:
              if( change.node->isMapped( ))
              {
                  change.context->map( change.child,
                                       dash::Context::getCurrent( ));
                  change.node->insert( change.child );
              }
              else
                  EQINFO << "Ignoring Node::insert change, parent not mapped"
                         << std::endl;
              break;
          case Change::NODE_ERASE:
              if( !change.node->isMapped( ))
                  break;
              change.node->erase( change.child );
              break;

          case Change::ATTRIBUTE_INSERT:
              if( change.node->isMapped( ))
              {
                  change.context->map( change.attribute,
                                       dash::Context::getCurrent( ));
                  change.node->insert( change.attribute );
              }
              else
                  EQINFO << "Ignoring Node::insert change, parent not mapped"
                         << std::endl;

              break;

          case Change::ATTRIBUTE_ERASE:
              if( !change.node->isMapped( ))
                  break;
              change.node->erase( change.attribute );
              break;

          case Change::ATTRIBUTE_CHANGED:
              change.attribute->getImpl()->apply( change );
              break;

          default:
              EQINFO << change << std::endl;
              EQUNIMPLEMENTED;
        }
    }
}

}
}
