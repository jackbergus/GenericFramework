//
// Created by Giacomo Bergami on 05/05/2026.
//

#ifndef GENERALFRAMEWORK_INITNETWORKING_H
#define GENERALFRAMEWORK_INITNETWORKING_H

#include <memory>

class InitNetworking {
    static std::unique_ptr<InitNetworking> self;
    InitNetworking();
    bool good_;

public:

    bool good() const { return good_; }
    static InitNetworking* getInstance() {
        if (!self) {
            std::unique_ptr<InitNetworking> tmp{new InitNetworking()};
            self = std::move(tmp);
        }
        return self.get();
    }


    void close();
};


#endif //GENERALFRAMEWORK_INITNETWORKING_H