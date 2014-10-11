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

		// Logging defaults to stdout
		log_to_syslog=false;
		log_to_buffer=false;

		// Save name
		this->name=name;

	};

	~Logger(){

		mutex::scoped_lock lock(this->guard);

		if(log_to_syslog) {

			// Close syslog
			closelog();
		}

	};

	void open_syslog(){

		mutex::scoped_lock lock(this->guard);

		if(log_to_buffer) {
			err("open_syslog called while logging to buffer! Ignoring.");
			return;
		}

		openlog(this->name.c_str(),0,LOG_DAEMON);
		log_to_syslog=true;

	};

	/**
	 * Redirects all future logging to an in-memory buffer
	 *
	 * This is not reversible
	 */
	void open_buffer() {

		mutex::scoped_lock lock(this->guard);

		if(log_to_syslog) {

			// Close syslog
			closelog();
			log_to_syslog=false;
		}
	
		log_to_buffer=true;

	}

	/**
	 * Fetches the contents of the log buffer, and resets it
	 *
	 * @return std::string
	 */ 
	string get_buffer() {

		mutex::scoped_lock lock(this->guard);
		string s=ss.str();
		ss.str("");
		return s;
	}

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
	bool log_to_buffer;
	mutex guard;
	string name;
	stringstream ss;

	void writelog(int level, string message) {

		mutex::scoped_lock lock(this->guard);

		if(log_to_syslog){

			syslog(level, message.c_str());

		} else if (log_to_buffer) {

			switch(level){
				case LOG_ERR:
					ss << name << "error: ";
					break;
				case LOG_WARNING:
					ss << name <<"warning: ";
					break;
				default:
					ss << name <<" ";
					break;
			}

			ss << message << endl;

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