#pragma once
#include <string>
#include <cstddef>
#include <odb/core.hxx>

namespace MY_IM {
    #pragma db object table("channel")
    class Channel {
        public:
            Channel() {}
            Channel(const std::string &cid, const std::string &creator_id):
                _channel_id(cid), _creator_id(creator_id) {}

            std::string channel_id() const { return _channel_id; }
            void channel_id(const std::string &cid) { _channel_id = cid; }

            std::string creator_id() const { return _creator_id; }
            void creator_id(const std::string &uid) { _creator_id = uid; }

        private:
            friend class odb::access;
            #pragma db id auto
            unsigned long _id;
            #pragma db type("varchar(64)") index unique
            std::string _channel_id;
            #pragma db type("varchar(64)")
            std::string _creator_id;
    };
}
