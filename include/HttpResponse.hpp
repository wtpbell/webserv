#ifndef HTTPRESPONSE_HPP_
#define	HTTPRESPONSE_HPP_

#include <string>
#include "HttpHeaders.hpp"

class HttpResponse
{
	public:
		int			statusCode;
		std::string	statusMessage;
		std::string	protocol_;
		HttpHeaders	headers;
		std::string	body;
};

#endif //HTTPRESPONSE_HPP_
