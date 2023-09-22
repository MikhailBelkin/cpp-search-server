#include "server.h"
#include "process_queries.h"


namespace RequestTCPServer {

	std::string remove_esc(std::string& s) {
		std::string result;
		for (auto c : s) {
			if (c >= 0x20) {
				result += c;
			}
		}
		return result;
	}

	void Session::start() {
		std::string init_str = "Session id:" + std::to_string(session_id_) + "\n\rSearch server 1.0. Enter keyword\n\r";

		boost::asio::async_write(socket_, boost::asio::buffer(init_str.data(), init_str.size()),
			boost::bind(&Session::write_handler, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));


		socket_.async_read_some(boost::asio::buffer(buffer_.data(), buffer_.size()), boost::bind(&Session::read_handler, this,

			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));




	}

	void Session::write_handler(const boost::system::error_code& error, size_t bytes_transefered) {
		if (!error) {
			//socket_.async_read_some( buffer_, boost::bind(&session::read_handler, this,

			if (!requests_.empty() && requests_.back() == "quit") {
				socket_.shutdown(boost::asio::socket_base::shutdown_both);
				socket_.close();
			}



			socket_.async_read_some(boost::asio::buffer(buffer_.data(), bytes_transefered), boost::bind(&Session::read_handler, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));

		}
		return;
	}

	void Session::read_handler(const boost::system::error_code& error, size_t bytes_transefered) {
		if (!error) {
			input_log.push_back(buffer_.data()[0]);
			if (input_log.back() == '\n') {
				if (input_log.size() > 1) {
					requests_.push_back(std::string(remove_esc(input_log)));

					std::string result = "Request was:"s + requests_.back() + "\n\r"s;

					if (!requests_.empty() && requests_.back() == "show") {
						for (auto str : requests_) {
							result += str + "\n\r"s;
						}

					}
					else {

						auto documents_arr = ss_.FindTopDocuments(requests_.back());
						if (documents_arr.empty()) {
							result += "Nothing found for this"s + "\r\n"s;
						}
						else {

							result += std::to_string(documents_arr.size())+ " Documents found:"s + "\r\n"s+ "\r\n"s;
							int count_doc = 0;
							for (Document& doc : documents_arr) {
								result += std::to_string(count_doc++) + 
									" document is:"s + "\r\n"s + static_cast<std::string>(ss_.GetRawDocument(doc.id))+"\r\n"s
									+"Rating: "s+std::to_string(doc.rating) + " Relevance: "s+std::to_string(doc.relevance)+"\r\n"s+ "\r\n"s;
								

							}
						}


					}

					boost::asio::async_write(socket_, boost::asio::buffer(result.data(), result.size()), boost::bind(&Session::write_handler, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));


				}
				input_log.clear();

				return;

			}
			else {
				socket_.async_read_some(boost::asio::buffer(buffer_.data(), bytes_transefered), boost::bind(&Session::read_handler, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));


			}
		}
		return;
	}



	void Server::start_accept()
	{
		Session* new_session = new Session(ss_, io_service_, session_count);
		session_count++;
		acceptor_.async_accept(new_session->GetSocket(),
			boost::bind(&Server::handle_accept, this, new_session,
				boost::asio::placeholders::error));
	}


	void Server::handle_accept(Session* new_session,
		const boost::system::error_code& error)
	{
		if (!error)
		{
			new_session->start();
		}
		else
		{
			delete new_session;
		}

		start_accept();
	}


}