#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <iostream>
#include <boost/serialization/base_object.hpp>

enum MessageType
{
  empty, info, get_connections, pull_connection, reconnect, terminate
};

class Message
{
public:
  Message(){};

  Message(MessageType type)
  : type_(type)
  {
  };

  Message(MessageType type, std::string data)
  : type_(type),
    data_(data)
  {
  };

  Message(std::string data)
  : type_(info),
    data_(data)
  {
  };

  virtual ~Message(){}

  MessageType type()
  {
    return type_;
  };

  std::string get_data()
  {
    return data_;
  }

  template <typename Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    ar & type_;
    ar & data_;
  }

private:
  MessageType type_;
  std::string data_; //zde jsou zaserializovana data (pokud nejaka takova jsou, jinak prazdny string)
};


std::ostream & operator<<(std::ostream &os, Message& m)
{
  switch(m.type()) {
    case 0: return os << "empty message"; break;
    case 1: return os << "info message - Info: " << m.get_data(); break;
    case 2: return os << "get_connections message"; break;
    case 3: return os << "pull_connection message"; break;
    case 4: return os << "reconnect message"; break;
    case 5: return os << "terminate message"; break;
    default:  return os << "undefined type: " << m.type(); break;
  }
  return os << m.type();
}

#endif // MESSAGE_H