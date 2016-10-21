// --------------------------------------------------------------------------
//  _____                    ________   __
// |  __ \                   |  ___\ \ / /
// | |  \/_ __ ___  ___ _ __ | |_   \ V /          Open Source Tools for
// | | __| '__/ _ \/ _ \ '_ \|  _|  /   \            Automated Algorithmic
// | |_\ \ | |  __/  __/ | | | |   / /^\ \             Currency Trading
//  \____/_|  \___|\___|_| |_\_|   \/   \/
//
// --------------------------------------------------------------------------

// Copyright (C) 2016  Anthony Green <anthony@atgreen.org>
// Distrubuted under the terms of the GPL v3 or later.

// This progam pulls ticks from the A-MQ message bus and records them
// to kairosdb.

#include <cstdlib>
#include <memory>
#include <unordered_map>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>

#include <activemq/library/ActiveMQCPP.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>

#include <json-c/json.h>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/time_duration.hpp>

using namespace decaf::util::concurrent;
using namespace decaf::util;
using namespace decaf::lang;
using namespace activemq;
using namespace cms;
using namespace std;

#include "config.h"
 
class TickListener : public ExceptionListener,
		     public MessageListener,
  		     public Runnable {

private:
  Session *session;
  Connection *connection;
  Destination *destination;
  MessageConsumer *consumer;

  string brokerURI;

  unordered_map <string, FILE *> filemap;

  boost::asio::io_service ios;
  boost::asio::ip::tcp::endpoint endpoint;
  boost::asio::ip::tcp::socket *socket;

public:
  TickListener () :
    brokerURI("tcp://broker-amq-tcp:61616?wireFormat=openwire") {
    bool connected = false;

    while (! connected)
      try
	{
	  socket = new boost::asio::ip::tcp::socket(ios);
	  boost::asio::ip::tcp::resolver res (ios);
	  boost::asio::ip::tcp::resolver::query query ("greenfx-kairosdb", "4242");
	  boost::asio::connect (*socket, res.resolve (query));
	  connected = true;
	}
      catch (boost::system::system_error const& e)
	{
	  std::cout << "WARNING: could not connect : " << e.what() << std::endl;
	  sleep(5);
	}
  }

  virtual void run () {
    try {

      // Create a ConnectionFactory
      std::auto_ptr<ConnectionFactory> 
	connectionFactory(ConnectionFactory::createCMSConnectionFactory(brokerURI));
      
      // Create a Connection
      connection = connectionFactory->createConnection("user", "password");
      connection->start();
      connection->setExceptionListener(this);
      
      session = connection->createSession(Session::AUTO_ACKNOWLEDGE);
      destination = session->createTopic("OANDA.TICK");
      
      consumer = session->createConsumer(destination);
      consumer->setMessageListener(this);

      std::cout << "Listening..." << std::endl;

      // Sleep forever
      while (true)
	sleep(1000);
      
    } catch (CMSException& ex) {
      
      printf (ex.getStackTraceString().c_str());
      exit (1);
      
    }
  }

  virtual void onMessage(const Message *msg)
  {
    json_object *jobj = json_tokener_parse (dynamic_cast<const TextMessage*>(msg)->getText().c_str());
    json_object *tick;

    std::cout << "." << std::endl;
    std::cout << dynamic_cast<const TextMessage*>(msg)->getText() << std::endl;
    
    if (json_object_object_get_ex (jobj, "tick", &jobj))
      {
	json_object *bid, *ask, *instrument, *ttime;
	if (json_object_object_get_ex (jobj, "bid", &bid) &&
	    json_object_object_get_ex (jobj, "ask", &ask) &&
	    json_object_object_get_ex (jobj, "instrument", &instrument) &&
	    json_object_object_get_ex (jobj, "time", &ttime))
	  {
	    string instrument_s = json_object_get_string (instrument);
	    string timestamp_s = json_object_get_string (ttime);

	    timestamp_s[10] = ' ';
	    timestamp_s[19] = 0;
	    boost::posix_time::ptime
	      t(boost::posix_time::time_from_string(timestamp_s));
	    boost::posix_time::ptime
	      epoch(boost::gregorian::date(1970,1,1));
	    boost::posix_time::time_duration diff = t - epoch;
	    
	    // Record the tick in kairosdb
	    char buf[1024];
	    sprintf (buf, "put %s.bid %u %s\n",
		     instrument_s.c_str(),
		     diff.total_seconds (),
		     json_object_get_string (bid));
	    std::cout << buf << std::endl;
	    boost::system::error_code ignored_error;
	    boost::asio::write (*socket,
				boost::asio::buffer (buf),
				boost::asio::transfer_all(), ignored_error);
	    
	    json_object_put (bid);
	    json_object_put (ask);
	    json_object_put (instrument);
	    json_object_put (ttime);
	  }
	else
	  {
	    // We are also leaking memory here - but this should never happen.
	    std::cerr << "ERROR: unrecognized json: " 
		      << dynamic_cast<const TextMessage*>(msg)->getText() << std::endl;
	  }
	json_object_put (tick);
      }

    json_object_put (jobj);
  }

  virtual void onException(const CMSException& ex)
  {
    printf (ex.getStackTraceString().c_str());
    exit (1);
  }
};

int main()
{
  //  std::string ts("2014-05-26T13:58:40Z");
  std::string ts("2014-05-26 13:58:40");
  boost::posix_time::ptime t(boost::posix_time::time_from_string(ts));
  std::cout << t << std::endl;


  std::cout << "tickq-to-kairosdb, Copyright (C) 2016  Anthony Green" << std::endl;
  std::cout << "Program started by User " << getuid() << std::endl;

  activemq::library::ActiveMQCPP::initializeLibrary();

  TickListener tick_listener;
  Thread listener_thread(&tick_listener);
  listener_thread.start();
  listener_thread.join();

  std::cout << "Program ended." << std::endl;

  return 0;
}

