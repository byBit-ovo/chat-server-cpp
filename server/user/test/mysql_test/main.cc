#include "../../../odb/user.hxx"
#include "user-odb.hxx"
#include "../../../common/mysql.hpp"
#include "../../../common/logger.hpp"
#include <gflags/gflags.h>

DEFINE_bool(log_mode,false,"false: 调试模式, true: 发布模式");
DEFINE_string(log_file,"","发布模式下的日志输出文件");
DEFINE_int32(log_level,0,"发布模式下的日志输出等级");

void insert(MY_IM::UserTable &tb){
	MY_IM::User::ptr user1 = std::make_shared<MY_IM::User>("123","jack","8333");
	MY_IM::User::ptr user2 = std::make_shared<MY_IM::User>("124","jerry","8374");
	MY_IM::User::ptr user3 = std::make_shared<MY_IM::User>("125","tom","87622");
	tb.insert(user1);
	tb.insert(user2);
	tb.insert(user3);
}
std::shared_ptr<MY_IM::User> select_by_id(MY_IM::UserTable &tb)
{
	std::shared_ptr<MY_IM::User> user = tb.select_by_id("125");
	std::cout << user->user_id() << std::endl;
	std::cout << user->nickname() << std::endl;
	std::cout << user->password() << std::endl;
	return user;
}
std::shared_ptr<MY_IM::User> select_by_phone(MY_IM::UserTable &tb)
{
	std::shared_ptr<MY_IM::User> user = tb.select_by_phone("110");
	std::cout << user->user_id() << std::endl;
	std::cout << user->nickname() << std::endl;
	std::cout << user->password() << std::endl;
	std::cout << user->phone() << std::endl;
	return user;
}
void update(MY_IM::UserTable &tb)
{
	MY_IM::User::ptr user = select_by_phone(tb);
	user->description("Have a good day!");
	user->phone("110");
	user->password("this is a new passwd");
	tb.update(user);
}
void select_multi(MY_IM::UserTable &tb)
{
	std::vector<std::string> users_id = {"123","124","125"};
	auto res = tb.select_multi_users(users_id);
	for(auto &user: res)
	{
		std::cout << user.nickname() << std::endl;
	}
}
int main(int argc,char *argv[])
{
	google::ParseCommandLineFlags(&argc,&argv,true);
    init_logger(FLAGS_log_mode,FLAGS_log_file,FLAGS_log_level);
	auto db = MY_IM::ODBFactory::create("root","200533","127.0.0.1","IM","utf8",3306,3);
	MY_IM::UserTable table(db);
	// insert(table);
	// select_by_id(table);
	// update(table);
	select_multi(table);
	return 0;
}