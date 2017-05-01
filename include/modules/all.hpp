//
// Vectorized and parallelized version of libnoise using the zacc SIMD toolkit
// Copyright (C) 2015-2016 Sergej Zuyev (sergej.zuyev - at - zz-systems.net)
//
// Original libnoise: 
// Copyright (C) 2003, 2004 Jason Bevins
// The developer's email is jlbezigvins@gmzigail.com (for great email, take
// off every 'zig'.)
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or (at
// your option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License (COPYING.txt) for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once


#include "modules/generators/primitives.hpp"
#include "modules/generators/billow.hpp"
#include "modules/generators/voronoi.hpp"
#include "modules/generators/perlin.hpp"
#include "modules/generators/ridgedmultifrac.hpp"

#include "modules/modifiers/primitives.hpp"
#include "modules/modifiers/curve.hpp"
#include "modules/modifiers/rotate.hpp"
#include "modules/modifiers/select.hpp"
#include "modules/modifiers/terrace.hpp"
#include "modules/modifiers/turbulence.hpp"
#include "modules/modifiers/displace.hpp"

#include "modules/caching/buffer.hpp"
#include "modules/caching/cache.hpp"