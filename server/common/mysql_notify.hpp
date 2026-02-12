#pragma once
#include "mysql.hpp"
#include <sstream>
#include "notify-odb.hxx"

namespace MY_IM {
class NotifyTable {
public:
    using ptr = std::shared_ptr<NotifyTable>;
    NotifyTable(const std::shared_ptr<odb::core::database> &db) : _db(db) {}

    bool insert(Notify &nt) {
        try {
            odb::transaction trans(_db->begin());
            _db->persist(nt);
            trans.commit();
        } catch (std::exception &e) {
            LOG_ERROR("新增离线通知失败 {}:{}！", nt.event_id(), e.what());
            return false;
        }
        return true;
    }

    std::vector<Notify> pending(const std::string &uid, int limit) {
        std::vector<Notify> res;
        if (limit <= 0) limit = 100;
        try {
            odb::transaction trans(_db->begin());
            std::stringstream cond;
            cond << "user_id='" << uid << "' ";
            cond << "and status=" << static_cast<int>(NOTIFY_PENDING) << " ";
            cond << "order by create_time asc limit " << limit;
            odb::result<Notify> r(_db->query<Notify>(cond.str()));
            for (auto i = r.begin(); i != r.end(); ++i) {
                res.push_back(*i);
            }
            trans.commit();
        } catch (std::exception &e) {
            LOG_ERROR("查询离线通知失败 {}:{}！", uid, e.what());
        }
        return res;
    }

    bool markPushed(const std::vector<std::string> &event_ids) {
        if (event_ids.empty()) return true;
        try {
            odb::transaction trans(_db->begin());
            typedef odb::query<Notify> query;
            for (const auto &eid : event_ids) {
                auto obj = _db->query_one<Notify>(query::event_id == eid);
                if (!obj) continue;
                obj->status(NOTIFY_PUSHED);
                _db->update(*obj);
            }
            trans.commit();
        } catch (std::exception &e) {
            LOG_ERROR("更新离线通知推送状态失败:{}！", e.what());
            return false;
        }
        return true;
    }

    bool remove(const std::string &uid, const std::vector<std::string> &event_ids) {
        if (event_ids.empty()) return true;
        try {
            odb::transaction trans(_db->begin());
            typedef odb::query<Notify> query;
            for (const auto &eid : event_ids) {
                _db->erase_query<Notify>(query::event_id == eid && query::user_id == uid);
            }
            trans.commit();
        } catch (std::exception &e) {
            LOG_ERROR("删除离线通知失败 {}:{}！", uid, e.what());
            return false;
        }
        return true;
    }

private:
    std::shared_ptr<odb::core::database> _db;
};
}
