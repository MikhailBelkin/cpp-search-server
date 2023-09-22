#pragma once
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>

#include "search_server.h"


namespace RequestTCPServer {

	std::string remove_esc(std::string& s);

	class Session {
	public:
		Session(SearchServer&s, boost::asio::io_service& ios, int session_count) :ss_(s), socket_(ios), session_id_(session_count), buffer_(1024) {}

		boost::asio::ip::tcp::socket& GetSocket() {
			return socket_;
		}

		void start();

		void write_handler(const boost::system::error_code& error, size_t bytes_transefered);

		void read_handler(const boost::system::error_code& error, size_t bytes_transefered);

	private:

		boost::asio::ip::tcp::socket  socket_;
		std::vector<char> buffer_;
		int session_id_;
		std::string input_log;
		std::vector<std::string> requests_;
		SearchServer& ss_;
	};



	class Server {
	public:
		Server( SearchServer&s,   boost::asio::io_context& ios, short port) : ss_(s), io_service_(ios),
			acceptor_(ios, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
		{
			start_accept();

		}

		void start_accept();


		void handle_accept(Session* new_session,
			const boost::system::error_code& error);


	private:
		boost::asio::io_context& io_service_;
		boost::asio::ip::tcp::acceptor acceptor_;
		int session_count = 0;
		SearchServer& ss_;
	};


}