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
#include "connection.h"

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
      std::cout << "Povedlo se pripojit: " << conn->socket().remote_endpoint() << std::endl;

      conn->async_read(data_,
          boost::bind(&superclient::handle_read, this,
            boost::asio::placeholders::error, conn));

      Message msg("Vitejte!");
      // std::cout << "Vytvorena nova zprava: " << msg << std::endl;

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
    // if (!e) std::cout << "povedlo se zapsat nejaka data" << std::endl;
  };

  void handle_read(const boost::system::error_code& e, connection_ptr conn)
  {
    if (!e)
    {
      std::cout << conn->socket().remote_endpoint() << ": " << data_ << std::endl;
      
      conn->async_read(data_,
          boost::bind(&superclient::handle_read, this,
            boost::asio::placeholders::error, conn));
    }
    else
    {
      std::cout << "Odpojil se: " << conn->socket().remote_endpoint() << std::endl;
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
  // std::cout << "io_service created" << std::endl;

  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), atoi(argv[1]));
  // std::cout << "endpoint created: " << endpoint << std::endl;

  superclient net(io_service, endpoint);
  // std::cout << "superclient created" << std::endl;

  io_service.run();
  
  return 0;
}