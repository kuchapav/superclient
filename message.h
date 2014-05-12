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

  virtual ~Message(){}

  MessageType type()
  {
    return type_;
  };

  template <typename Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    ar & type_;
  }

private:
  MessageType type_;
};


std::ostream & operator<<(std::ostream &os, Message& m)
{
  switch(m.type()) {
    case 0: return os << "empty message"; break;
    case 1: return os << "info message"; break;
    case 2: return os << "get_connections message"; break;
    case 3: return os << "pull_connection message"; break;
    case 4: return os << "reconnect message"; break;
    case 5: return os << "terminate message"; break;
    default:  return os << "undefined type: " << m.type(); break;
  }
  return os << m.type();
}


class InfoMessage : public Message
{
public:
  InfoMessage(std::string info_str)
  : Message(info),
    info_(info_str)
  {
  };

  std::string get_info()
  {
    return info_;
  }

  template <typename Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    ar & boost::serialization::base_object<Message>(*this);
    ar & info_;
  }

private:
  std::string info_;
};

std::ostream & operator<<(std::ostream &os, InfoMessage& m)
{
  return os << "info message - Info: " << m.get_info();
}

class ConnectionMessage
{
public:
  ConnectionMessage();
  ~ConnectionMessage();

private:
    
};



class header
{
public:
  header(MessageType type, int size)
  : size_(size),
    type_(type)
  {
  };

  header(Message message)
  : type_(message.type())
  {
  };

private:
  MessageType type_;
  int size_;

};

#endif // MESSAGE_H