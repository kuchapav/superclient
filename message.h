#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <boost/serialization/base_object.hpp>

enum MessageType
{
  info, get_connections, pull_connection, reconnect, terminate
};

class Message
{
public:
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