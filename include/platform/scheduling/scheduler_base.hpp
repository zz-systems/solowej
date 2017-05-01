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

#include <numeric>
#include <vector>
#include <memory>
#include "zacc.hpp"
#include "util/serializable.h"
#include "modules/module_base.hpp"
#include "system/platform.hpp"

namespace cacophony { namespace platform {
    using namespace zacc;
    using namespace modules;
    using json = nlohmann::json;

    struct scheduler_config : serializable<json>
    {
        vec3<int> dimensions, seam_dimensions;
        vec3<float> scale, offset;
        size_t num_threads;
        bool make_seam;

        virtual void deserialize(const json &source) override
        {
            auto dimensions = source["dimensions"];
            auto scale      = source["scale"];
            auto offset     = source["offset"];

            this->dimensions = this->seam_dimensions = dimensions != nullptr && dimensions.size() >= 3
                                                       ? vec3<int>(dimensions[0].get<int>(), dimensions[1].get<int>(), dimensions[2].get<int>())
                                                       : vec3<int>(128, 128, 1);


            this->scale		 = scale != nullptr && scale.size() >= 3
                                  ? vec3<float>(scale[0].get<float>(), scale[1].get<float>(), scale[2].get<float>())
                                  : vec3<float>(1);

            this->offset = offset != nullptr && offset.size() >= 3
                           ? vec3<float>(offset[0].get<float>(), offset[1].get<float>(), offset[2].get<float>())
                           : vec3<float>(0);


            num_threads = source.value<size_t>("num_threads", -1);
            if(num_threads < 1)
                num_threads = zacc::platform::instance().num_threads();

            this->make_seam     = source.value<bool>("make_seam", true);
            if(make_seam)
            {
                seam_dimensions.x = std::max(seam_dimensions.x - 1, 1);
                seam_dimensions.y = std::max(seam_dimensions.y - 1, 1);
                seam_dimensions.z = std::max(seam_dimensions.z - 1, 1);
            }
        }
    };

    DISPATCHED class alignas(branch::alignment) scheduler_base :
        public serializable<json>
    {

    protected:
        scheduler_config _config;

    public:

        const scheduler_config &config() const
        {
            return _config;
        }

        virtual void deserialize(const json &source) override
        {
            _config << source;

            transform = build_transform(transform);
        }

        virtual float *operator()(const vec3<float> &origin) /*const*/
		{
			auto d = _config.dimensions;
			//auto target = new float[d.x * d.y * d.z];

			zacc::aligned_allocator<float, 32> alloc;

			//float* target;// = alloc.allocate(d.x * d.y * d.z);
			float* target = reinterpret_cast<float*>(aligned_malloc(sizeof(float) * d.x * d.y * d.z, 64));
			this->schedule(target, origin);

			return target;
		}

        virtual void schedule(float *target, const vec3<float> &origin)     /*const*/ = 0;
        virtual void schedule(int *target,   const vec3<float> &origin)     /*const*/ = 0;


        virtual void operator()(float *target, const vec3<float> &origin)   /*const*/
        {
            schedule(target, origin);
        }

        virtual void operator()(int *target, const vec3<float> &origin)   /*const*/
        {
            schedule(target, origin);
        }

        Module<branch>		source;
        Transformer<branch>	transform;

        virtual void set_source(Module<branch> source)
        {
            this->source = source;
        }

        virtual void set_transform(Transformer<branch> transformer)
        {
            this->transform = transformer;
        }
    protected:

        inline vec3<zfloat> build_coords(const int index)
        {
            std::array<int, zfloat::dim> offset;
            std::iota(offset.begin(), offset.end(), 1);

            auto idx = static_cast<zint>(offset) + index;

            vec3<zfloat> result;



            result.x =  idx % _config.dimensions.x;
            result.y = (idx / _config.dimensions.x) % _config.dimensions.y;
            result.z =  idx / (_config.dimensions.x * _config.dimensions.y);

           // auto result = vec3<zfloat>(x + data, x ,x);

            //std::cout << "build_coords(" << index << ") =>" << result << std::endl;

            return result;
        }

        template<typename T>
        inline void stream_result(typename T::scalar_t* stride, size_t x, const T &r) const
        {
            std::copy(r.begin(), r.end(), stride + x);
        }

        template<typename T>
        inline void store_result(typename T::scalar_t* stride, size_t x, const T &r) const
        {
            std::copy(r.begin(), r.end(), stride + x);
        }


        template<typename T>
        void write_result(const vec3<int> &dimensions, typename T::scalar_t *stride, size_t remainder, size_t x, const T &result)
        {
            if (remainder == 0) // All rows aligned on required boundaries -> stream
            {
                stream_result(stride, x, result);
            }
            else if (x < (dimensions.x - remainder)) // Not aligned - but still in the "good" range
            {
                store_result(stride, x, result);
            }
            else // Fill remaining columns
            {
                auto extracted = result.data();

                for (size_t i = 0; i < remainder; i++)
                    stride[x + i] = extracted[i];
            }
        }

        Transformer<branch> build_transform(Transformer<branch> transformer)
        {
            std::cout << "building input transform..." << std::endl;

            std::cout << std::boolalpha << "has seam: " << _config.make_seam << std::endl;

            auto scale              = _config.scale;
            auto offset             = _config.offset;
            auto dimensions         = _config.dimensions;
            auto seam_dimensions    = _config.seam_dimensions;

            std::cout << "scale: " << scale << std::endl;
            std::cout << "offset: " << offset << std::endl;
            std::cout << "dimensions: " << dimensions << std::endl;
            std::cout << "seam_dimensions: " << seam_dimensions << std::endl;
            std::cout << "has transformer:" << std::boolalpha << static_cast<bool>(transformer) << std::endl;

            if (static_cast<bool>(transformer))
            {
                if (scale != vec3<float>(1) && offset != vec3<float>(0))
                {
                    std::cout << "user defined transformer: scaled, biased" << std::endl;
                    return [seam_dimensions, scale, offset, transformer](const auto&c) { return transformer(c) / static_cast<vec3<zfloat>>(seam_dimensions) * static_cast<vec3<zfloat>>(scale) + static_cast<vec3<zfloat>>(offset); };
                }
                if (scale != vec3<float>(1))
                {
                    std::cout << "user defined transformer: scaled" << std::endl;
                    return [seam_dimensions, scale, transformer](const auto&c) { return transformer(c) / static_cast<vec3<zfloat>>(seam_dimensions) * static_cast<vec3<zfloat>>(scale); };
                }
                if (offset != vec3<float>(0))
                {
                    std::cout << "user defined transformer: biased" << std::endl;
                    return [seam_dimensions, offset, transformer](const auto&c) { return transformer(c) / static_cast<vec3<zfloat>>(seam_dimensions) + static_cast<vec3<zfloat>>(offset); };
                }
            }
            else
            {
                if (scale != vec3<float>(1) && offset != vec3<float>(0))
                {
                    std::cout << "default transformer: scaled, biased" << std::endl;
                    return [seam_dimensions, scale, offset](const auto&c) { return c / static_cast<vec3<zfloat>>(seam_dimensions) * static_cast<vec3<zfloat>>(scale) + static_cast<vec3<zfloat>>(offset); };
                }
                if (scale != vec3<float>(1))
                {
                    std::cout << "default transformer: scaled" << std::endl;
                    return [seam_dimensions, scale](const auto&c) { return c / static_cast<vec3<zfloat>>(seam_dimensions) * static_cast<vec3<zfloat>>(scale); };
                }
                if (offset != vec3<float>(0))
                {
                    std::cout << "default transformer: biased" << std::endl;
                    return [seam_dimensions, offset](const auto&c) { return c / static_cast<vec3<zfloat>>(seam_dimensions) + static_cast<vec3<zfloat>>(offset); };
                }
            }

            std::cout << "default transformer" << std::endl;
            return [seam_dimensions](const auto &c)  { return c / static_cast<vec3<zfloat>>(seam_dimensions); };
        }
    };
}}