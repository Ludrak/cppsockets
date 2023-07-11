#pragma once 

#include <iostream>

# define LOG_DEBUG(category, log_message) \
	std::cout << "[DEBUG] " << log_message << std::endl;
# define LOG_INFO(category, log_message) \
	std::cout << "[INFO] " << log_message << std::endl;
# define LOG_WARN(category, log_message) \
	std::cout << "[WARNING] " << log_message << std::endl;
# define LOG_ERROR(category, log_message) \
	std::cout << "[ERROR] " << log_message << std::endl;
# define LOG_CRITICAL(category, log_message) \
	std::cout << "[CRITICAL] " << log_message << std::endl;
