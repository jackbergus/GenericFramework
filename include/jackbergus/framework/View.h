// View.h
// This file is part of GeneralFramework
//
// Copyright (C)  2026 - Giacomo Bergami
//
// GeneralFramework is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  GeneralFramework is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with GeneralFramework. If not, see <http://www.gnu.org/licenses/>.

#ifndef GENERALFRAMEWORK_VIEW_H
#define GENERALFRAMEWORK_VIEW_H

#include <jackbergus/framework/structures/ModelStatus.h>
#include <jackbergus/framework/structures/ControllerRequest.h>

namespace jackbergus {
    namespace framework {
        class View {

        public:
            /**
             *
             * @param initiated_request Request initiating the visualization request
             * @param values_to_render  Required visualization depending on the request that was initiated
             * @return
             */
            bool renderModelUpdate(const ControllerRequest& initiated_request, const ModelStatus& values_to_render) {
                return true;
            }

        };
    } // framework
} // jackbergus

#endif //GENERALFRAMEWORK_VIEW_H
