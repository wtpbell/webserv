#ifndef HTTPREQUEST_HPP_
#define	HTTPREQUEST_HPP_

#include <string>
#include "HttpHeaders.hpp"

class HttpRequest
{
	public:
		std::string	method_;
		std::string	requestURI_;
		std::string	protocol_;
		HttpHeaders	header;
		std::string	body;

};

#endif //HTTPREQUEST_HPP_
