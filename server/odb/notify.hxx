#pragma once
#include <string>
#include <cstddef>
#include <odb/nullable.hxx>
#include <odb/core.hxx>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace MY_IM {

enum NotifyStatus : unsigned char {
    NOTIFY_PENDING = 0,
    NOTIFY_PUSHED = 1
};

#pragma db object table("notify")
class Notify {
public:
    Notify() {}
    Notify(const std::string& eid,
           const std::string& uid,
           unsigned char ntype,
           const std::string& payload,
           unsigned char status = NOTIFY_PENDING)
        : _event_id(eid), _user_id(uid), _notify_type(ntype),
          _payload(payload), _status(status),
          _create_time(boost::posix_time::second_clock::universal_time()) {}

    std::string event_id() const { return _event_id; }
    void event_id(const std::string& v) { _event_id = v; }

    std::string user_id() const { return _user_id; }
    void user_id(const std::string& v) { _user_id = v; }

    unsigned char notify_type() const { return _notify_type; }
    void notify_type(unsigned char v) { _notify_type = v; }

    std::string payload() const { return _payload; }
    void payload(const std::string& v) { _payload = v; }

    unsigned char status() const { return _status; }
    void status(unsigned char v) { _status = v; }

    boost::posix_time::ptime create_time() const { return _create_time; }
    void create_time(const boost::posix_time::ptime& v) { _create_time = v; }

private:
    friend class odb::access;
#pragma db id auto
    unsigned long _id;
#pragma db type("varchar(64)") index unique
    std::string _event_id;
#pragma db type("varchar(64)") index
    std::string _user_id;
    unsigned char _notify_type;
#pragma db type("longtext")
    std::string _payload;
    unsigned char _status;
#pragma db type("TIMESTAMP")
    boost::posix_time::ptime _create_time;
};

}
