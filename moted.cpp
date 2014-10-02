#include <v8/v8.h>
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <fstream>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "INIReader.h"

// Fastcache should be in mutable mode
#define FASTCACHE_MUTABLE_DATA
#include "fastcache/Fastcache.h"


using boost::shared_ptr;
using namespace active911;
using namespace v8;
using namespace std;
//using active911::Fastcache;

// Global logging functions, etc
void writelog(int level, string message);
void closeup (int retval);
void fatal(string message);
void log_err(string message);
void log_notice(string message);
void signal_handler(int signum);	// Catch SIGHUP, etc

// Globals
bool daemonized=false;


int main(int argc, char **argv) {

	// Read the config file
	INIReader reader("moted.cfg");
	if (reader.ParseError() < 0) {
		fatal("Can't read moted.cfg");
	}

	// Set config parameters
	int ip_port = reader.GetInteger("moted","ip-port",9090);
	int worker_threads = reader.GetInteger("moted","worker-threads",16);
	string user= reader.Get("moted","user","moted");
	string group= reader.Get("moted","group","moted");
	string pid_file_name= reader.Get("moted","pidfile","/var/run/moted.pid");

	// Daemonize and setup logging 
	if(argc==2 && 0==strcmp(argv[1],"-d")) { 

		// -d set.	Don't daemonize
		writelog(LOG_NOTICE,"-d set.	Not daemonizing.");

	} else {

		if(daemon(0,0)!=0) {

			fatal("Error daemonizing! Exiting...");
		}
		daemonized=true;
		openlog("moted",0,LOG_DAEMON);
	}

	// Install signal handlers
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);


	// Write PID.	 Do this before dropping root in case dir is not writeable by daemon user.
	ofstream pidfile;
	pidfile.open(pid_file_name.c_str());
	pidfile << getpid();
	pidfile.close();
	if(pidfile.fail()){

		fatal("Unable to write pid to \""+pid_file_name+"\"");
	}

	// Drop root
	if (getuid() == 0) {

		// Get uid and gid info 
		struct passwd* p=getpwnam(user.c_str());		// according to getpwnam(3), this should not be free()d
		struct group* g=getgrnam(group.c_str());

		if (p==NULL || g==NULL || setgid(g->gr_gid) != 0 || setuid(p->pw_uid) != 0) {

			fatal("Unable to change user / group to "+user+"/"+group);
		}
	}


	try {


	} catch (exception& e){

		fatal(e.what());
	}

	// Go!
	stringstream ss; 
	ss << "Starting server" << endl;
	ss << "    pid            : " << getpid() << endl; 
	ss << "    worker threads : " << worker_threads << endl;
	// ss << "    mysql poolsize : " << mysql_poolsize << endl;
	ss << endl;
	writelog(LOG_NOTICE, ss.str());


	closeup(0);
	return 0;
}



void fatal(string message) {

	writelog(LOG_ERR, "fatal: " + message);
	closeup(1);
}

void log_notice(string message) {

	writelog(LOG_NOTICE,message);

}

void log_err(string message) {

	writelog(LOG_ERR,message);

}

void writelog(int level, string message) {

	// If we are a daemon, syslog it
	if(daemonized){

		syslog(level, message.c_str());

	} else {

		switch(level){
			case LOG_ERR:
				cout << "error: ";
				break;
			case LOG_WARNING:
				cout << "warning: ";
				break;
		}

		cout << message << endl;
	}
}

void closeup (int retval) {

	// Stop the server

	if(daemonized){

		closelog();
	}

	exit(retval);
}

void signal_handler(int signum){

	if(signum==SIGTERM || signum==SIGINT){

		writelog(LOG_NOTICE,string("caught signal. Closing..."));

		// Stop the server
	 }

}

