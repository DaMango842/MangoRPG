#pragma once

/**
 * @file Logger.h
 * @brief 高级日志系统实现
 *
 * 本文件实现了一个现代化的C++20日志系统，具有以下特性：
 * - 多级别日志（DEBUG, INFO, WARNING, ERROR）
 * - 控制台彩色输出
 * - 文件日志记录
 * - 自动捕获源代码位置信息
 * - 类型安全的格式化输出
 * - 构造/析构自动日志
 * - 线程安全的单例实现
 *
 * @version 1.0
 * @date 2025/8/7
 * @author Mango(划掉)
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <chrono>
#include <format>
#include <typeinfo>
#include <source_location>
#include <functional>
#include <memory>
#include <unordered_map>
#include <cstdio>
#include <concepts>
#include <type_traits>
#include <vector>

#define NOMINMAX
#define NOGDI
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef ERROR
#undef ERROR
#endif

 /**
  * @enum LogLevel
  * @brief 日志级别枚举
  *
  * 定义四种日志级别：
  * - DEBUG: 调试信息，最详细级别
  * - INFO: 常规信息，记录程序运行状态
  * - WARNING: 警告信息，潜在问题
  * - ERROR: 错误信息，需要立即关注的问题
  */
enum class LogLevel {
	DEBUG,    ///< 调试信息
	INFO,     ///< 常规信息
	WARNING,  ///< 警告信息
	ERROR     ///< 错误信息
};

/// 日志级别到ANSI颜色代码的映射
static const std::unordered_map<LogLevel, std::string> log_color = {
	{ LogLevel::DEBUG,   "\x1b[90m" },  // 灰色
	{ LogLevel::INFO,    "\x1b[37m" },  // 白色
	{ LogLevel::WARNING, "\x1b[33m" },  // 黄色
	{ LogLevel::ERROR,   "\x1b[31m" }   // 红色
};

/**
 * @class LoggerCore
 * @brief 日志系统的核心实现类
 *
 * 提供日志记录的核心功能，包括：
 * - 日志消息的构建
 * - 控制台和文件输出
 * - 自动捕获源代码位置
 * - 线程安全的单例模式
 */
class LoggerCore {
public:
	/**
	 * @brief 获取日志器单例实例
	 * @return LoggerCore& 单例引用
	 *
	 * 使用Meyer's单例模式，保证线程安全
	 */
	static LoggerCore& instance() {
		static LoggerCore logger;
		return logger;
	}

	/**
	 * @brief 写入简单日志
	 * @tparam T 内容类型
	 * @param level 日志级别
	 * @param content 日志内容
	 *
	 * 示例：
	 * @code
	 * LoggerCore::instance().write(LogLevel::INFO, "System initialized");
	 * @endcode
	 */
	template<typename T>
	void write(LogLevel level, const T& content) {
		const auto loc = std::source_location::current();
		const std::string fullMsg = buildLogMessage(level, convertToString(content), loc);
		outputLog(level, fullMsg);
	}

	/**
	 * @brief 写入格式化日志
	 * @tparam Args 参数类型包
	 * @param level 日志级别
	 * @param fmt 格式化字符串
	 * @param args 格式化参数
	 *
	 * 使用C++20的std::format风格格式化：
	 *
	 * 示例：
	 * @code
	 * int value = 42;
	 * LoggerCore::instance().write(LogLevel::DEBUG, "Value: {}", value);
	 * @endcode
	 */
	template<typename... Args>
	void write(LogLevel level, const char* fmt, Args&&... args) {
		const auto loc = std::source_location::current();
		try {
			std::string formatted = std::vformat(fmt, std::make_format_args(args...));
			const std::string fullMsg = buildLogMessage(level, formatted, loc);
			outputLog(level, fullMsg);
		}
		catch (const std::exception& e) {
			std::string error = "Format error: ";
			error += e.what();
			error += " | Original: ";
			error += fmt;
			const std::string fullMsg = buildLogMessage(level, error, loc);
			outputLog(level, fullMsg);
		}
	}

	/**
	 * @brief 设置自定义输出处理器
	 * @param handler 自定义处理函数
	 *
	 * 允许重定向日志输出到自定义目标：
	 * @code
	 * LoggerCore::instance().setOutputHandler([](LogLevel level, const std::string& msg) {
	 *     // 发送到网络或GUI
	 * });
	 * @endcode
	 */
	void setOutputHandler(std::function<void(LogLevel, const std::string&)> handler) {
		outputHandler = std::move(handler);
	}

private:
	/// 构造函数（私有，单例模式）
	LoggerCore() {
		logFile.open("log.txt", std::ios::app);
	}

	/// 析构函数（关闭日志文件）
	~LoggerCore() {
		if (logFile.is_open()) {
			logFile.close();
		}
	}

	// 禁止复制和赋值
	LoggerCore(const LoggerCore&) = delete;
	LoggerCore& operator=(const LoggerCore&) = delete;

	/**
	 * @brief 简化函数名
	 * @param fullName 完整函数签名
	 * @return 简化后的函数名
	 *
	 * 移除模板参数、调用约定等冗余信息：
	 * 输入: "void __cdecl LoggerCore::write<const char*>(...)"
	 * 输出: "write"
	 */
	static std::string simplifyFunctionName(const char* fullName) {
		if (!fullName || !*fullName) return "unknown";

		std::string name(fullName);

		// 移除常见调用约定
		const std::vector<std::string> callingConventions = {
			" __cdecl ", " __stdcall ", " __fastcall ", " __vectorcall "
		};

		for (const auto& conv : callingConventions) {
			size_t pos = name.find(conv);
			if (pos != std::string::npos) {
				name.erase(pos, conv.length());
				break;
			}
		}

		// 移除模板参数
		size_t pos = name.find('<');
		if (pos != std::string::npos) {
			size_t depth = 1;
			size_t end = pos + 1;

			while (end < name.length() && depth > 0) {
				if (name[end] == '<') depth++;
				else if (name[end] == '>') depth--;
				end++;
			}

			if (depth == 0) {
				name.erase(pos, end - pos);
			}
		}

		// 移除返回类型
		pos = name.find_last_of(' ');
		if (pos != std::string::npos) {
			name = name.substr(pos + 1);
		}

		// 移除参数列表
		pos = name.find('(');
		if (pos != std::string::npos) {
			name = name.substr(0, pos);
		}

		return name;
	}

	/**
	 * @brief 简化文件名
	 * @param fullPath 完整文件路径
	 * @return 基本文件名
	 *
	 * 从完整路径中提取文件名：
	 * 输入: "D:/Project/Logger.h"
	 * 输出: "Logger.h"
	 */
	static std::string simplifyFileName(const char* fullPath) {
		if (!fullPath || !*fullPath) return "unknown";

		std::string path(fullPath);

		// 查找最后一个目录分隔符
		size_t pos = path.find_last_of("\\/");
		if (pos != std::string::npos) {
			return path.substr(pos + 1);
		}

		return path;
	}

	/**
	 * @brief 构建完整日志消息
	 * @param level 日志级别
	 * @param content 日志内容
	 * @param loc 源代码位置
	 * @return 格式化后的完整日志消息
	 *
	 * 格式: [时间] [级别] [文件:行号 - 函数] 内容
	 */
	std::string buildLogMessage(LogLevel level, const std::string& content,
		const std::source_location& loc) {
		std::ostringstream oss;
		oss << "[" << getCurrentTime() << "]"
			<< " [" << logLevelToString(level) << "]";

		// 添加位置信息（如果可用）
		if (loc.file_name() && *loc.file_name() && loc.function_name() && *loc.function_name()) {
			oss << " [" << simplifyFileName(loc.file_name()) << ":" << loc.line()
				<< " - " << simplifyFunctionName(loc.function_name()) << "]";
		}
		else if (loc.file_name() && *loc.file_name()) {
			oss << " [" << simplifyFileName(loc.file_name()) << ":" << loc.line() << "]";
		}

		oss << " " << content;
		return oss.str();
	}

	/**
	 * @brief 输出日志到目标
	 * @param level 日志级别
	 * @param fullMsg 完整日志消息
	 *
	 * 输出到：
	 * 1. 控制台（带颜色）
	 * 2. 日志文件（log.txt）
	 * 3. 自定义处理器（如果设置）
	 */
	void outputLog(LogLevel level, const std::string& fullMsg) {
		// 使用自定义处理器或默认控制台输出
		if (outputHandler) {
			outputHandler(level, fullMsg);
		}
		else {
			auto it = log_color.find(level);
			if (it != log_color.end()) {
				std::cout << it->second << fullMsg << "\x1b[0m\n";
			}
			else {
				std::cout << fullMsg << '\n';
			}
		}

		// 写入日志文件
		if (logFile.is_open()) {
			logFile << fullMsg << '\n';
			logFile.flush();
		}
	}

	/**
	 * @brief 类型转换到字符串
	 * @tparam T 类型
	 * @param value 要转换的值
	 * @return 字符串表示
	 *
	 * 支持的类型：
	 * - 数值类型（int, float等）
	 * - 字符串类型（std::string, const char*）
	 * - 宽字符串（std::wstring, wchar_t*）
	 * - 其他类型（使用ostringstream转换）
	 */
	template<typename T>
	static std::string convertToString(const T& value) {
		if constexpr (std::is_arithmetic_v<T>) {
			return std::to_string(value);
		}
		else if constexpr (std::is_convertible_v<T, std::string>) {
			return std::string(value);
		}
		else if constexpr (std::is_same_v<T, std::wstring> ||
			std::is_same_v<T, wchar_t*>) {
			return wideToUtf8(value);
		}
		else {
			std::ostringstream oss;
			oss << value;
			return oss.str();
		}
	}

	/**
	 * @brief 宽字符串转UTF-8
	 * @param wstr 宽字符串
	 * @return UTF-8编码的字符串
	 */
	static std::string wideToUtf8(const std::wstring& wstr) {
		if (wstr.empty()) return "";
		int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1,
			nullptr, 0, nullptr, nullptr);
		if (len <= 0) return "";

		std::string result(len - 1, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1,
			result.data(), len, nullptr, nullptr);
		return result;
	}

	/**
	 * @brief 获取当前时间字符串
	 * @return 格式化的时间字符串 (YYYY-MM-DD HH:MM:SS.mmm)
	 */
	static std::string getCurrentTime() {
		std::time_t t = std::time(nullptr);
		std::tm tm;
		localtime_s(&tm, &t);

		// 获取毫秒
		auto now = std::chrono::system_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			now.time_since_epoch()) % 1000;

		char buf[64];
		std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec,
			static_cast<int>(ms.count()));

		return buf;
	}

	/**
	 * @brief 日志级别转字符串
	 * @param level 日志级别
	 * @return 级别字符串 ("DEBUG", "INFO"等)
	 */
	static std::string logLevelToString(LogLevel level) {
		switch (level) {
		case LogLevel::DEBUG:   return "DEBUG";
		case LogLevel::INFO:    return "INFO";
		case LogLevel::WARNING: return "WARNING";
		case LogLevel::ERROR:   return "ERROR";
		default:                return "UNKNOWN";
		}
	}

private:
	std::ofstream logFile; ///< 日志文件输出流
	std::function<void(LogLevel, const std::string&)> outputHandler; ///< 自定义输出处理器
};

// ===================== 日志宏 ===================== //

/**
 * @def LOG(level, ...)
 * @brief 通用日志宏
 * @param level 日志级别
 * @param ... 日志内容或格式化字符串+参数
 */

 /**
  * @def LOG_DEBUG(...)
  * @brief DEBUG级别日志宏
  */

  /**
   * @def LOG_INFO(...)
   * @brief INFO级别日志宏
   */

   /**
	* @def LOG_WARN(...)
	* @brief WARNING级别日志宏
	*/

	/**
	 * @def LOG_ERROR(...)
	 * @brief ERROR级别日志宏
	 */
#define LOG(level, ...) LoggerCore::instance().write(level, __VA_ARGS__)
#define LOG_DEBUG(...)   LOG(LogLevel::DEBUG, __VA_ARGS__)
#define LOG_INFO(...)    LOG(LogLevel::INFO, __VA_ARGS__)
#define LOG_WARN(...)    LOG(LogLevel::WARNING, __VA_ARGS__)
#define LOG_ERROR(...)   LOG(LogLevel::ERROR, __VA_ARGS__)

	 // ===================== 自动日志注入 ===================== //

	 /**
	  * @class LogInjector
	  * @tparam T 要注入日志的类
	  * @brief 自动记录构造和析构的日志注入器
	  *
	  * 在构造和析构时自动记录日志：
	  * @code
	  * class MyClass {
	  *     LogInjector<MyClass> logInjector; // 自动记录构造/析构
	  * };
	  * @endcode
	  */
template<typename T>
class LogInjector {
public:
	/// 构造函数（记录构造日志）
	LogInjector() {
		LOG_DEBUG("[Construct] {}", typeid(T).name());
	}

	/// 析构函数（记录析构日志）
	~LogInjector() {
		LOG_DEBUG("[Destruct] {}", typeid(T).name());
	}

	// 禁用复制和移动
	LogInjector(const LogInjector&) = delete;
	LogInjector(LogInjector&&) = delete;
	LogInjector& operator=(const LogInjector&) = delete;
	LogInjector& operator=(LogInjector&&) = delete;
};

/**
 * @def ENABLE_LOG_INJECTION(ClassName)
 * @brief 启用自动日志注入的宏
 * @param ClassName 要注入日志的类名
 *
 * 在类定义中使用：
 * @code
 * class MyClass {
 *     ENABLE_LOG_INJECTION(MyClass);
 * };
 * @endcode
 *
 * 效果：
 * - 构造时记录: [Construct] MyClass
 * - 析构时记录: [Destruct] MyClass
 */
#define ENABLE_LOG_INJECTION(ClassName) \
private: \
    LogInjector<ClassName> _logInjector
