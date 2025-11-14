#ifndef HTTPHEADERS_HPP_
#define	HTTPHEADERS_HPP_

#include <string>
#include <map>

class HttpHeaders
{
	public:
		std::map<std::string, std::string>	field;
		void								set(const std::string& key, const std::string);
		std::string							get(const std::string& key) const;
		bool								has(const std::string& key) const;
};


#endif //HTTPHEADERS_HPP_
