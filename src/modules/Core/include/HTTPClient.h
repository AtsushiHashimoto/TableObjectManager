/*!
 * @file HTTPClient.h
 * @author kitchen
 * @date Date Created: 2015/Feb/07
 * @date Last Change:2015/Feb/07.
 */
#ifndef __SKL_HTTP_CLIENT_H__
#define __SKL_HTTP_CLIENT_H__

#include <boost/asio.hpp>
#include "skl.h"

namespace skl{

/*!
 * @class HTTPClient
 * @brief HTTPClient get/post requestを送るクラス(redirectには対応していない簡単なもの)
 */
class HTTPClient{
	class tcp_client;
	public:
		HTTPClient(const std::string& host,int port=80);
		virtual ~HTTPClient();
		bool get(const std::string& path, const std::string& query="");
		bool post(const std::string& path, const std::string& query);

		inline unsigned int status_code()const{return tcp_base_->status_code();}
		inline const std::string& read()const{return tcp_base_->read();}
		inline std::map<std::string,std::string> header()const{return tcp_base_->header();}
	protected:
		void makeRequest(boost::asio::streambuf& buf, const std::string& path, const std::string& query="")const;
		bool UrlEncode(std::string& str)const;

		std::string host_;
		int port_;
		boost::asio::io_service io_service_;


		typedef boost::shared_ptr<tcp_client> tcp_client_ptr;
		tcp_client_ptr tcp_base_;
	public:
		class tcp_client{
		public:
			tcp_client(boost::asio::io_service& io_service)
				: io_service_(io_service), socket_(io_service_), resolver_(io_service_) { }

			void write(const std::string& host,int port, boost::asio::streambuf *request){
				request_ = request;
				boost::asio::ip::tcp::resolver::query query(host, boost::lexical_cast<std::string>(port));
				resolver_.async_resolve(
					query,
					boost::bind(&tcp_client::handle_resolve, this, boost::asio::placeholders::error, boost::asio::placeholders::iterator)
					);
			}

			inline unsigned int status_code()const{return status_code_;}
			inline const std::string& read()const{return response_;}
			inline const std::map<std::string,std::string>& header()const{return header_;}

			inline boost::asio::io_service& io_service(){return this->io_service_;}
		protected:
			std::map<std::string,std::string> header_;
		private:
			//async_resolve後ハンドル
			void handle_resolve(const boost::system::error_code& err,boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
			{
				if(!err)
				{
					//正常
					boost::asio::ip::tcp::endpoint endpoint= *endpoint_iterator;
					socket_.async_connect(
						endpoint,
						boost::bind(&tcp_client::handle_connect, this, boost::asio::placeholders::error, ++endpoint_iterator)
						);

				}
				else throw std::invalid_argument("Error: "+err.message());
			}

			//async_connect後ハンドル
			void handle_connect(const boost::system::error_code& err,boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
			{
				if(!err)
				{
					//正常接続
					boost::asio::async_write(
						socket_,
						*request_,
						boost::bind(&tcp_client::handle_write_request, this, boost::asio::placeholders::error)
						);
				}
				else if(endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
				{
					//接続ミス
					socket_.close();
					boost::asio::ip::tcp::endpoint endpoint= *endpoint_iterator;
					socket_.async_connect(
						endpoint,
						boost::bind(&tcp_client::handle_connect, this, boost::asio::placeholders::error, ++endpoint_iterator)
						);

				}
				else throw std::invalid_argument("Error: "+err.message());
			}

			//async_write後ハンドル
			void handle_write_request(const boost::system::error_code& err)
			{
				if(!err)
				{
					//正常送信
					boost::asio::async_read_until(
						socket_,
						response_stream_,
						"\r\n",
						boost::bind(&tcp_client::handle_read_status, this, boost::asio::placeholders::error)
						);
				}
				else throw std::invalid_argument("Error: "+err.message());
			}

			//async_read_until後ハンドル
			void handle_read_status(const boost::system::error_code& err)
			{
				if(!err)
				{
					//正常送信
					std::istream response_stream(&response_stream_);
					//レスポンスチェック
					std::string http_version;
					response_stream >> http_version;
					response_stream >> status_code_;
					std::string status_message;
					response_stream >> status_message;

					if (!response_stream || http_version.substr(0, 5) != "HTTP/")
					{
						throw std::invalid_argument("Invalid response");
					}

					boost::asio::async_read_until(
						socket_,
						response_stream_,
						"\r\n\r\n",
						boost::bind(&tcp_client::handle_read_header, this, boost::asio::placeholders::error)
						);
				}
				else throw std::invalid_argument("Error: "+err.message());
			}

			//handle_read_status後ハンドル
			void handle_read_header(const boost::system::error_code& err)
			{
				if(!err)
				{
					//正常送信
					std::istream response_stream(&response_stream_);
					std::string header;
					std::vector<std::string> buf;
					while (std::getline(response_stream, header)){
						if(header!="\r")break;
					}
					while (header != "\r"){
						buf = skl::split_strip(header,":",2);
						header_[buf[0]] = buf[1];
						//std::cout << "header[" << buf[0] << "] = " << buf[1] << std::endl; 
						std::getline(response_stream, header);
					}
					if (response_stream_.size() > 0)
						response_ = static_cast<std::string>(boost::asio::buffer_cast<const char*>(response_stream_.data()));

					boost::asio::async_read(
						socket_,
						response_stream_,
						boost::asio::transfer_at_least(1),
						boost::bind(&tcp_client::handle_read_content, this, boost::asio::placeholders::error)
						);      
				}
				else throw std::invalid_argument("Error: "+err.message());
			}

			//handle_read_header後ハンドル
			void handle_read_content(const boost::system::error_code& err)
			{
				if(!err)
				{
					response_ = static_cast<std::string>(boost::asio::buffer_cast<const char*>(response_stream_.data()));

					boost::asio::async_read(
						socket_,
						response_stream_,
						boost::asio::transfer_at_least(1),
						boost::bind(&tcp_client::handle_read_content,
						this,
						boost::asio::placeholders::error)
						);
				}
				else if (err != boost::asio::error::eof)
				{
					std::cerr << "Error: " << err << "\n";
				}
			}

			boost::asio::io_service& io_service_;
			boost::asio::streambuf *request_;
			boost::asio::streambuf response_stream_;
			std::string response_;
			unsigned int status_code_;
			boost::asio::ip::tcp::socket socket_;
			boost::asio::ip::tcp::resolver resolver_;
		};
/*
		int main(){
			try
			{
				boost::asio::io_service io_service;

				boost::asio::streambuf buffer;
				std::ostream ss(&buffer);
				boost::asio::streambuf response;

				ss << "GET / HTTP/1.1\r\n";
				ss << "Host: godai.e-whs.net\r\n";
				ss << "Accept: *//*\r\n";  // to uncomment, // -> /
				ss << "Connection: close\r\n\r\n";
				ss << std::flush;

				tcp_client tcp(io_service);
				tcp.write("godai.e-whs.net",&buffer);
				io_service.run();

				std::string A;
				tcp.read(A);

			}
			catch (std::exception& e)
			{
				std::cout << "Exception: " << e.what() << "\n";
			}

			return 0;
		}
*/
};



} // skl

#endif // __SKL_HTTP_CLIENT_H__

