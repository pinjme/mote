#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <syslog.h>
#include <boost/thread/mutex.hpp>

using namespace std;
using boost::mutex;


/**
 * Simple thread safe logger class
 *
 * Logs everything to STDOUT, until open_syslog() is called
 */
class Logger {

public:
	Logger(string name){

		// Start with syslog closed
		log_to_syslog=false;

		// Save name
		this->name=name;

	};

	~Logger(){

		if(log_to_syslog) {

			// Close syslog
			mutex::scoped_lock lock(this->guard);
			closelog();
		}

	};

	void open_syslog(){

		mutex::scoped_lock lock(this->guard);
		openlog(this->name.c_str(),0,LOG_DAEMON);
		log_to_syslog=true;

	};

	void notice(string message) {

		writelog(LOG_NOTICE,message);

	};

	void warning(string message) {

		writelog(LOG_WARNING,message);

	};

	void err(string message) {

		writelog(LOG_ERR,message);

	};


protected:

	bool log_to_syslog;
	mutex guard;
	string name;

	void writelog(int level, string message) {

		mutex::scoped_lock lock(this->guard);

		if(log_to_syslog){

			syslog(level, message.c_str());

		} else {

			switch(level){
				case LOG_ERR:
					cout << name << "error: ";
					break;
				case LOG_WARNING:
					cout << name <<"warning: ";
					break;
				default:
					cout << name <<" ";
					break;
			}

			cout << message << endl;
		}

	}

};