/**
	curl-asio: wrapper for integrating libcurl with boost.asio applications
	Copyright (c) 2013 Oliver Kuckertz <oliver.kuckertz@mologie.de>
	See COPYING for license information.

	Integration of libcurl's multi interface with Boost.Asio
*/

#pragma once

#include "config.h"
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include "initialization.h"
#include "native.h"
#include "socket_info.h"

namespace curl
{
	class easy;

	class CURLASIO_API multi :
			public boost::noncopyable
	{
	public:
		multi(boost::asio::io_service &io_service);

		~multi();

		boost::asio::io_service &get_io_service()
		{
			return io_service_;
		}

		native::CURLM *native_handle()
		{
			return handle_;
		}

		void add(easy *easy_handle);

		void remove(easy *easy_handle);

		void socket_register(std::shared_ptr<socket_info> si);

		void socket_cleanup(native::curl_socket_t s);

	private:
		using socket_info_ptr = std::shared_ptr<socket_info>;

		void add_handle(native::CURL *native_easy);

		void remove_handle(native::CURL *native_easy);

		void assign(native::curl_socket_t sockfd, void *user_data);

		void socket_action(native::curl_socket_t s, int event_bitmask);

		using socket_function_t = int (*)(native::CURL *native_easy, native::curl_socket_t s, int what, void *userp, void *socketp);

		void set_socket_function(socket_function_t socket_function);

		void set_socket_data(void *socket_data);

		using timer_function_t = int (*)(native::CURLM *native_multi, long timeout_ms, void *userp);

		void set_timer_function(timer_function_t timer_function);

		void set_timer_data(void *timer_data);

		void monitor_socket(socket_info_ptr si, int action);

		void process_messages();

		bool still_running();

		void start_read_op(socket_info_ptr si);

		void handle_socket_read(const boost::system::error_code &err, socket_info_ptr si);

		void start_write_op(socket_info_ptr si);

		void handle_socket_write(const boost::system::error_code &err, socket_info_ptr si);

		void handle_timeout(const boost::system::error_code &err);

		using socket_type = boost::asio::ip::tcp::socket;
		using socket_map_type = std::map<socket_type::native_handle_type, socket_info_ptr>;
		socket_map_type sockets_;

		socket_info_ptr get_socket_from_native(native::curl_socket_t native_socket);

		static int socket(native::CURL *native_easy, native::curl_socket_t s, int what, void *userp, void *socketp);

		static int timer(native::CURLM *native_multi, long timeout_ms, void *userp);

		using easy_set_type = std::unordered_set<easy *>;

		boost::asio::io_service &io_service_;
		initialization::ptr initref_;
		native::CURLM *handle_;
		easy_set_type               easy_handles_;
		boost::asio::deadline_timer timeout_;
		int                         still_running_;
	};
}
