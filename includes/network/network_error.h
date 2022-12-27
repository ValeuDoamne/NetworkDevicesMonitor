#include <exception>
#include <string>

namespace net{
	class network_error : public std::exception {
		std::string error;
	public:
		network_error(std::string e)
			:error(e) { }
		void add(const std::string& e) { this->error+=e; }
		virtual const char *what() const throw() { return error.c_str();}
	};
}
