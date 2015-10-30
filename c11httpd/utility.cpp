/**
 * Utility functions.
 *
 * Copyright (c) 2015 Alex Jin (toalexjin@hotmail.com)
 */

#include "c11httpd/utility.h"
#include <ctime>


namespace c11httpd {


void utility_t::response_date(char* str) {
	auto now = std::time(0);
	struct tm now_tm;
	struct tm* now_tm_ptr;
	const char* format = "%a, %d %b %Y %H:%M:%S GMT";

	now_tm_ptr = gmtime_r(&now, &now_tm);
	if (now_tm_ptr == 0) {
		str[0] = 0;
		return;
	}

	const auto result = strftime(str, response_date_len, format, now_tm_ptr);
	if (result == 0 || result >= response_date_len) {
		str[0] = 0;
		return;
	}
}


} // namespace c11httpd.

