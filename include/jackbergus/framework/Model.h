// Model.h
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

#ifndef GENERALFRAMEWORK_MODEL_H
#define GENERALFRAMEWORK_MODEL_H

#include <jackbergus/framework/structures/ControllerRequest.h>
#include <jackbergus/framework/structures/ModelStatus.h>

namespace jackbergus {
    namespace framework {
        class Model {
        public:
            /**
             *
             * @param request Request handled by the controller
             * @param status  Retrieving the status, while replying to the request
             * @return Returs true if the request was satisfactorily managed and false otherwise. In that case, I cannot consider the value from status
             */
            bool updateModelUponRequest(const ControllerRequest& request, ModelStatus& status) {
                return false;
            }
        };
    } // framework
} // jackbergus

#endif //GENERALFRAMEWORK_MODEL_H
