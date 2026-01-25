#pragma once
#include "mysql.hpp"
#include "channel.hxx"
#include "channel-odb.hxx"

namespace MY_IM {
    class ChannelTable {
        public:
            using ptr = std::shared_ptr<ChannelTable>;
            ChannelTable(const std::shared_ptr<odb::core::database> &db):_db(db){}
            bool insert(Channel &ch) {
                try {
                    odb::transaction trans(_db->begin());
                    _db->persist(ch);
                    trans.commit();
                } catch (std::exception &e) {
                    LOG_ERROR("新增频道失败 {}:{}！", ch.channel_id(), e.what());
                    return false;
                }
                return true;
            }
            std::string creator(const std::string &cid) {
                try {
                    odb::transaction trans(_db->begin());
                    typedef odb::query<Channel> query;
                    typedef odb::result<Channel> result;
                    std::shared_ptr<Channel> ch(_db->query_one<Channel>(query::channel_id == cid));
                    trans.commit();
                    if (ch) {
                        return ch->creator_id();
                    }
                } catch (std::exception &e) {
                    LOG_ERROR("获取频道创建者失败 {}:{}！", cid, e.what());
                }
                return "";
            }
        private:
            std::shared_ptr<odb::core::database> _db;
    };
}
