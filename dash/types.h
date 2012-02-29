
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

/**
 * @file dash/types.h
 *
 * Forward declarations and type definitions for the dash namespace.
 */
#ifndef DASH_TYPES_H
#define DASH_TYPES_H

#include <co/base/refPtr.h>
#include <co/base/types.h>
#include <co/base/uint128_t.h>

#include <vector>
#ifdef _MSC_VER
#  include <memory>
#else
#  include <tr1/memory>
#endif

namespace dash
{

using co::base::uint128_t;
using co::base::RefPtr;

class Commit;
class Context;
template< class, int32_t > class Vector;

class Attribute;
typedef RefPtr< Attribute > AttributePtr; //!< Attribute smart pointer
typedef RefPtr< const Attribute > AttributeConstPtr; //!< Attribute const pointer
typedef std::vector< AttributePtr > Attributes; //!< Vector of AttributePtr
/** Const iterator for Attributes */
typedef Attributes::const_iterator AttributesCIter;
typedef Attributes::iterator AttributesIter; //!< Iterator for Attributes

class Node;
typedef co::base::RefPtr< Node > NodePtr; //!< Node smart pointer
typedef co::base::RefPtr< const Node > NodeConstPtr; //!< Node const pointer
typedef std::vector< NodePtr > Nodes; //!< Vector of NodePtr
typedef Nodes::const_iterator NodesCIter; //!< Const iterator for Nodes
typedef Nodes::iterator NodesIter; //!< Iterator for Nodes

/** @cond INTERNAL */
namespace detail
{
class Commit;
typedef std::tr1::shared_ptr< Commit > CommitPtr;
typedef std::tr1::shared_ptr< const Commit > CommitConstPtr;
}
/** @endcond */

}

#endif // DASH_DETAIL_TYPES_H
