#pragma once

#ifdef WEBSOCKETSERVER_USE_TLS
#include "websocketpp/config/asio.hpp"
#else
#include "websocketpp/config/asio_no_tls.hpp"
#endif

#include "websocketpp/server.hpp"
#include <set>
#include <vector>
#include <sstream>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include "User.h"

// Thanks to https://searchcode.com/codesearch/view/14575470/ for inspiration

extern boost::shared_ptr<Logger> logger;


#ifdef WEBSOCKETSERVER_USE_TLS
typedef websocketpp::server<websocketpp::config::asio_tls> server_t;
typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;
#else
typedef websocketpp::server<websocketpp::config::asio> server_t;
#endif

typedef websocketpp::message_buffer::message<websocketpp::message_buffer::alloc::con_msg_manager> message_type;
typedef websocketpp::message_buffer::alloc::con_msg_manager<message_type> con_msg_man_type;

con_msg_man_type::ptr manager(new con_msg_man_type());

using boost::shared_ptr;
using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;



class WebsocketServer {

    /**
     * A request from a single client (user)
     *
     * This will be queued and handled by a worker
     */
    class Request {

    public:
        Request(connection_hdl hdl, shared_ptr<User> user, server_t::message_ptr msg){

            this->hdl=hdl;
            this->user=user;
            this->msg=msg;

        }
        connection_hdl hdl;
        shared_ptr<User> user;
        server_t::message_ptr msg;
    };


private:
    typedef std::set<connection_hdl> con_list;      
    boost::mutex con_list_guard;
    server_t server;
    con_list m_connections;
    std::map<connection_hdl,shared_ptr<User> > users;
    boost::mutex users_guard;
    con_msg_man_type::ptr manager;

    // And a thread pool and worker caste
    boost::thread_group threadpool;
    shared_ptr<boost::detail::atomic_count>worker_run;
    std::queue<shared_ptr<Request> > requests;
    boost::mutex requests_guard;
    boost::condition_variable requests_cond;

public:
    WebsocketServer() {

        // Logging
//        server.set_access_channels(websocketpp::log::alevel::all);
//        server.clear_access_channels(websocketpp::log::alevel::frame_payload);

        server.clear_access_channels(websocketpp::log::alevel::all);

        server.init_asio();
                
        server.set_open_handler(bind(&WebsocketServer::on_open,this,::_1));
        server.set_close_handler(bind(&WebsocketServer::on_close,this,::_1));
        server.set_message_handler(bind(&WebsocketServer::on_message,this,::_1,::_2));
        #ifdef WEBSOCKETSERVER_USE_TLS
        server.set_tls_init_handler(bind(&WebsocketServer::on_tls_init,this,::_1));
        #endif

         this->manager=con_msg_man_type::ptr(new con_msg_man_type());


        // Set up the thread group
        this->worker_run=shared_ptr<boost::detail::atomic_count>(new boost::detail::atomic_count(1));
        for(int n=0; n<5; n++) {
    
            boost::thread* t = new boost::thread(&WebsocketServer::worker_thread, this);
            this->threadpool.add_thread(t);
        }

    }
    
    /**
     * Handle requests
     * 
     * This function is run by the several worker threads, to handle reuqests as they come in
     */

    void worker_thread() {


        boost::thread::id id=boost::this_thread::get_id();
        std::stringstream msg;
        msg << "Worker thread " << id << " starting";
        logger->notice(msg.str());

        while(*this->worker_run) {

            try {

                // Get a request
                boost::unique_lock<boost::mutex> lock(requests_guard);
                while(this->requests.empty()) {

                    this->requests_cond.wait(lock);      // Unlocks the lock until woken
                }
                shared_ptr<Request> r = this->requests.front();
                this->requests.pop();
                lock.unlock();

                // Handle the request
                msg.str("");
                msg << "Thread " << id << " awake and handling: " << r->msg->get_payload();
                logger->notice(msg.str());
                //boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
                // Only accept text messages
                // if(msg->get_opcode()==websocketpp::frame::opcode::BINARY)
                //     return;


                // // Was this a command?
                // if(msg->get_payload() == "where"){

                //     server.send(hdl, user->get_velocity(), websocketpp::frame::opcode::TEXT);
                //     return;
                // }

                // Get the message


            } catch(boost::thread_interrupted& e) {

                // We were asked to leave?             
                std::stringstream msg;
                msg << "Worker thread " << id << " interrupted";
                logger->notice(msg.str());
                return;

            } catch(std::exception& e) {

                logger->err("Unknown exception in worker thread");
            }
        }

        // End             
        msg.str("");
        msg << "Worker thread " << id << " quitting";
        logger->notice(msg.str());

    }

    #ifdef WEBSOCKETSERVER_USE_TLS
    context_ptr on_tls_init(websocketpp::connection_hdl hdl) {

        context_ptr ctx(new boost::asio::ssl::context(boost::asio::ssl::context::tlsv1));

        try {
            ctx->set_options(boost::asio::ssl::context::default_workarounds |
                             boost::asio::ssl::context::no_sslv2 |
                             boost::asio::ssl::context::single_dh_use);
            //ctx->set_password_callback(bind(&get_password));
            ctx->use_certificate_chain_file("server.crt");
            ctx->use_private_key_file("server.key", boost::asio::ssl::context::pem);
        } catch (std::exception& e) {

            std::stringstream msg;
            msg << "Exception on TLS init: " << e.what();
            logger->notice(msg.str());

        }

        return ctx;        
    }
    #endif

    void on_open(connection_hdl hdl) {

        logger->notice("New connection");

        // Add a new connection to our list
        {
            boost::mutex::scoped_lock con_list_lock(con_list_guard);
            m_connections.insert(hdl);
        }

        // Add a new user
        shared_ptr<User> user=shared_ptr<User>(new User());
        {
            boost::mutex::scoped_lock users_lock(users_guard);
            users.insert(std::pair<connection_hdl, shared_ptr<User> >(hdl, user));
        }

        // Send welcome message
        std::string welcome="JOIN";
        broadcast_string(welcome);
    }
    
    void on_close(connection_hdl hdl) {


        // Remove the connection
        {
            boost::mutex::scoped_lock con_list_lock(con_list_guard);
            m_connections.erase(hdl);
        }

        // Get the user who left
        shared_ptr<User>user;
        {
            boost::mutex::scoped_lock users_lock(users_guard);
            user=users[hdl];
        }

        // Send goodbye message
        std::string goodbye="SCAT";
        broadcast_string(goodbye);

        // Erase connection handle
        users.erase(hdl);
    }
   
    void on_message(connection_hdl hdl, server_t::message_ptr msg) {

        // Get the user who sent this message
        shared_ptr<User>user;
        {
            boost::mutex::scoped_lock lock(users_guard);
            user=users[hdl];
        }

        // Create a Request, and queue it for processing
        shared_ptr<Request> r = shared_ptr<Request>(new Request(hdl, user, msg));
        {
            boost::mutex::scoped_lock requests_list_lock(requests_guard);
            this->requests.push(r);
            std::stringstream msg;
            msg << "Queue contains " << this->requests.size() << " requests";
            logger->notice(msg.str());
        }
        this->requests_cond.notify_one();        // Let someone know there is something in the queue

        // Only accept text messages
        // if(msg->get_opcode()==websocketpp::frame::opcode::BINARY)
        //     return;


        // // Was this a command?
        // if(msg->get_payload() == "where"){

        //     server.send(hdl, user->get_velocity(), websocketpp::frame::opcode::TEXT);
        //     return;
        // }

        // Get the message
//        std::string message = msg->get_payload();

        // Broadcast it
//        broadcast_string(message);
    }

    void broadcast_payload(void* payload, size_t len){

        boost::mutex::scoped_lock lock(con_list_guard);

        // Create a message
        message_type::ptr msg = this->manager->get_message(websocketpp::frame::opcode::BINARY,len);
        msg->set_payload((const void*)payload, len);


        // Send to all connections
        for(con_list::iterator it=m_connections.begin(); it!=m_connections.end(); ++it){
            
           server.send(*it, msg);         
        }

    }

    void broadcast_string(string str){

        // Log the message
        logger->notice(str);


        message_type::ptr msg = this->manager->get_message(websocketpp::frame::opcode::TEXT, str.length());
        msg->set_payload(str);

        // Send to all connections
        boost::mutex::scoped_lock lock(con_list_guard);
        for(con_list::iterator it=m_connections.begin(); it!=m_connections.end(); ++it){
            
           server.send(*it, msg);         
        }
    }

    void run(uint16_t port) {
        server.listen(port);
        server.start_accept();
        server.run();
    }

    void stop() {
        broadcast_string("*** THE CAVE IS CLOSING ***");
        --(*this->worker_run);
        logger->notice("Waiting 1000ms for all worker threads to quit...");
        boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
        this->threadpool.interrupt_all();
        this->threadpool.join_all();
        server.stop();
    }

};

