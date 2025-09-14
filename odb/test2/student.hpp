#pragma once
#include <string>
#include <cstddef> // std::size_t
#include <boost/date_time/posix_time/posix_time.hpp>
#include <odb/nullable.hxx>
#include <odb/core.hxx>

#pragma db object	
class Student 
{
	private:
		#pragma db id auto
		unsigned long _id;
		#pragma db unique
		std::string _sn;

		std::string _name;
		odb::nullable<unsigned short> _age;
		#pragma db index
		unsigned long _class_id;
};

#pragma db object	
class Classes
{
	private:
		unsigned long _id;
		std::string _name;
};