#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

#include "message.h"


class connection
{
public:
  connection(boost::asio::io_service& io_service)
    : socket_(io_service)
  {
  }

  boost::asio::ip::tcp::socket& socket()
  {
    return socket_;
  }

  template <typename T, typename Handler>
  void async_write(const T& t, Handler handler)
  {
    std::ostringstream archive_stream;
    boost::archive::text_oarchive archive(archive_stream);
    archive << t;
    outbound_data_ = archive_stream.str();

    std::ostringstream header_stream;
    header_stream << std::setw(header_length)
      << std::hex << outbound_data_.size();
    if (!header_stream || header_stream.str().size() != header_length)
    {
      boost::system::error_code error(boost::asio::error::invalid_argument);
      socket_.get_io_service().post(boost::bind(handler, error));
      return;
    }
    outbound_header_ = header_stream.str();

    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(outbound_header_));
    buffers.push_back(boost::asio::buffer(outbound_data_));
    boost::asio::async_write(socket_, buffers, handler);
  }

  template <typename T, typename Handler>
  void async_read(T& t, Handler handler)
  {
    void (connection::*f)(
        const boost::system::error_code&,
        T&, boost::tuple<Handler>)
      = &connection::handle_read_header<T, Handler>;
    boost::asio::async_read(socket_, boost::asio::buffer(inbound_header_),
        boost::bind(f,
          this, boost::asio::placeholders::error, boost::ref(t),
          boost::make_tuple(handler)));
  }

  template <typename T, typename Handler>
  void handle_read_header(const boost::system::error_code& e,
      T& t, boost::tuple<Handler> handler)
  {
    if (e)
    {
      boost::get<0>(handler)(e);
    }
    else
    {
      std::istringstream is(std::string(inbound_header_, header_length));
      std::size_t inbound_data_size = 0;
      if (!(is >> std::hex >> inbound_data_size))
      {
        boost::system::error_code error(boost::asio::error::invalid_argument);
        boost::get<0>(handler)(error);
        return;
      }

      inbound_data_.resize(inbound_data_size);
      void (connection::*f)(
          const boost::system::error_code&,
          T&, boost::tuple<Handler>)
        = &connection::handle_read_data<T, Handler>;
      boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
        boost::bind(f, this,
          boost::asio::placeholders::error, boost::ref(t), handler));
    }
  }

  template <typename T, typename Handler>
  void handle_read_data(const boost::system::error_code& e,
      T& t, boost::tuple<Handler> handler)
  {
    if (e)
    {
      boost::get<0>(handler)(e);
    }
    else
    {
      try
      {
        std::string archive_data(&inbound_data_[0], inbound_data_.size());
        std::istringstream archive_stream(archive_data);
        boost::archive::text_iarchive archive(archive_stream);
        archive >> t;
      }
      catch (std::exception& e)
      {
        boost::system::error_code error(boost::asio::error::invalid_argument);
        boost::get<0>(handler)(error);
        return;
      }

      boost::get<0>(handler)(e);
    }
  }

private:
  boost::asio::ip::tcp::socket socket_;
  enum { header_length = 8 };
  std::string outbound_header_;
  std::string outbound_data_;

  char inbound_header_[header_length];
  std::vector<char> inbound_data_;
};

typedef boost::shared_ptr<connection> connection_ptr;

class superclient
{
public:
  superclient(boost::asio::io_service& io_service,
          boost::asio::ip::tcp::endpoint& endpoint)
  : acceptor_(io_service, endpoint)
  {
    connection_ptr new_conn(new connection(acceptor_.get_io_service()));
    acceptor_.async_accept(new_conn->socket(),
        boost::bind(&superclient::handle_accept, this,
          boost::asio::placeholders::error, new_conn));
    
  }

private:
  void handle_accept(const boost::system::error_code& e, connection_ptr conn)
  {
    if (!e)
    {
      std::cout << "Povedlo se pripojit: " << conn->socket().local_endpoint() << std::endl;

      conn->async_read(data_,
          boost::bind(&superclient::handle_read, this,
            boost::asio::placeholders::error, conn));

      InfoMessage msg("Vitejte!");
      std::cout << "Vytvorena nova zprava: " << msg << std::endl;

      conn->async_write(msg,
          boost::bind(&superclient::handle_write, this,
            boost::asio::placeholders::error));
    }

    connection_ptr new_conn(new connection(acceptor_.get_io_service()));
    acceptor_.async_accept(new_conn->socket(),
        boost::bind(&superclient::handle_accept, this,
          boost::asio::placeholders::error, new_conn));
  }

  void handle_write(const boost::system::error_code& e)
  {
    if (!e) std::cout << "povedlo se zapsat nejaka data" << std::endl;
  };

  void handle_read(const boost::system::error_code& e, connection_ptr conn)
  {
    if (!e)
    {
      std::cout << "povedlo se precist nejaka data:" << data_.type() << std::endl;
      conn->async_read(data_,
          boost::bind(&superclient::handle_read, this,
            boost::asio::placeholders::error, conn));
    }
    else
    {
      std::cout << "Odpojil se: " << conn->socket().local_endpoint() << std::endl;
      conn->socket().close();
    }
  };

  boost::asio::ip::tcp::acceptor acceptor_;
  Message data_;
};

int main(int argc, char const *argv[])
{
  if (argc < 2)
    {
      std::cerr << "Usage: superclient <port>\n";
      return 1;
    }

  boost::asio::io_service io_service;
  std::cout << "io_service created" << std::endl;

  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), atoi(argv[1]));
  std::cout << "endpoint created: " << endpoint << std::endl;

  superclient net(io_service, endpoint);
  std::cout << "superclient created" << std::endl;

  io_service.run();
  
  return 0;
}