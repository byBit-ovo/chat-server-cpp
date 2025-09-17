#pragma once
#include <string>
#include <cstddef> // std::size_t
#include <boost/date_time/posix_time/posix_time.hpp>
#include <odb/nullable.hxx>
#include <odb/core.hxx>


#pragma db object		// map this class to a table in database
class Student 
{
	public:
		Student(){}
		Student(unsigned long sn, const std::string &name, unsigned short age, unsigned long cid):
            _sn(sn), _name(name), _age(age), _class_id(cid){}
        void sn(unsigned long num) { _sn = num; }
        unsigned long sn() { return _sn; }

        void name(const std::string &name) { _name = name; }
        std::string name() { return _name; }

        void age(unsigned short num) { _age = num; }
        odb::nullable<unsigned short> age() { return _age; }

        void class_id(unsigned long cid) { _class_id = cid; }
        unsigned long class_id() { return _class_id; }
	private:
		friend class odb::access;
		#pragma db id auto  		//primary key && auto increment
		unsigned long _id;
		#pragma db unique			//create unique index on this member
		unsigned long _sn;
		std::string _name;
		odb::nullable<unsigned short> _age;
		#pragma db index		// create index on this member
		unsigned long _class_id;
};


#pragma db object	
class Classes
{
	public:
		Classes() {}
        Classes(const std::string &name,unsigned long id) : _name(name),_id(id){}
        void name(const std::string &name) { _name = name; }
        std::string name() { return _name; }
	private:
		friend class odb::access;
		#pragma db id
		unsigned long _id;
		std::string _name;
};


//查询所有的学生信息，并显示班级名称
#pragma db view object(Student)\
                object(Classes = classes : Student::_class_id == classes::_id)\
                query((?))
struct classes_student {
    #pragma db column(Student::_id)
    unsigned long id;
    #pragma db column(Student::_sn)
    unsigned long sn;
    #pragma db column(Student::_name)
    std::string name;
    #pragma db column(Student::_age)
    odb::nullable<unsigned short>age;
    #pragma db column(classes::_name)
    std::string class_name;
};


// 只查询学生姓名  ,   (?)  外部调用时传入的过滤条件
#pragma db view query("select name from Student" + (?))
struct all_name {
    std::string name;
};

//odb -d mysql --std c++11 --generate-query --generate-schema --profile boost/date-time student.hxx

// git config --global user.name "xunyiovo"
// git config --global user.email "3302371508@qq.com"
