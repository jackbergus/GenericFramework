// Controller.h
// This file is part of GeneralFramework
//
// Copyright (C)  2026 - gyankos
//
// narcissus is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  narcissus is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with narcissus. If not, see <http://www.gnu.org/licenses/>.

#ifndef GENERALFRAMEWORK_CONTROLLER_H
#define GENERALFRAMEWORK_CONTROLLER_H

#include <cstdint>
#include <wchar.h>
#include <jackbergus/framework/structures/ModelStatus.h>
#include <jackbergus/framework/structures/ControllerRequest.h>
#include <jackbergus/framework/Model.h>
#include <jackbergus/framework/View.h>


namespace jackbergus {
    namespace framework {
        class Controller {
            Model model;
            View view;
        public:

            enum ControllerReqStatus : uint8_t {
                MODEL_WAS_UPDATED_UPFRONT = 0b00000001,
                VIEW_WAS_UPDATED_UPFRONT = 0b00000010,
            };

            /**
             *  8
             * @param request Request forwarded by either a user, or from anything handling the request
             * @return        Actions being taken as a consequence of the implementation of the system
             * 
             */
            uint8_t recvControllerRequest(const ControllerRequest& request) {
                ModelStatus reply;
                uint8_t result = 0;
                if (model.updateModelUponRequest(request, reply)) {
                    result |= MODEL_WAS_UPDATED_UPFRONT;
                    if (view.renderModelUpdate(request, reply)) {
                        result |= VIEW_WAS_UPDATED_UPFRONT;
                    }
                }
                return result;
            }
        };
    } // framework
} // jackbergus

#endif //GENERALFRAMEWORK_CONTROLLER_H
