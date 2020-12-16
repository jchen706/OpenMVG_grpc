
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>
#include "masterworker.grpc.pb.h"

#include <thread>

using std::thread;

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerAsyncReaderWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::ClientReader;
using grpc::ClientAsyncReaderWriter;
using grpc::ClientWriter;
using grpc::ClientAsyncWriter;
using grpc::ClientAsyncReader;


// using boost threadpool
///https://stackoverflow.com/questions/19500404/how-to-create-a-thread-pool-using-boost-in-c

#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

boost::asio::io_service ioService;
boost::thread_group threadpool;


using masterworker::ImageRequest;
using masterworker::ImageResponse;
using masterworker::RpcImage;

class TaskClient {

  enum class Type {
    READ = 1,
    WRITE = 2,
    CONNECT = 3,
    WRITES_DONE = 4,
    FINISH = 5
   
  };

  public:
    explicit TaskClient(std::shared_ptr<Channel> channel): stub_(RpcImage::NewStub(channel)) {
        
    }
    void SendFile(std::string filepath) {

        const size_t chunk_size = 4UL << 20;
    
        AsyncClientCall* _call = new AsyncClientCall;
        _call->state_ = START;
        

        // add any type of metadata per image or file or parameters
        _call->context.AddMetadata("custom-header", "99999"); 

        bool finish = false;
        bool wrote_data = false;
        grpc::Status statu = Status::OK;
        ImageResponse irq;

        void* got_tag;
        bool ok = false;

        while(!finish) {

           if(_call->state_  == START) {
                 // call for client to connect to the server
                  
                _call->response_reader = stub_->AsyncSendServerStream(&_call->context, &cq_,  reinterpret_cast<void*>(Type::CONNECT));
                std::cout << " start stub " << std::endl;
            } else if (_call->state_  == CONNECTED) {
                // server is connected
                std::cout << " async server connected" << std::endl;
            }  else if (_call->state_  == WRITE) {
                // server calls for read
                ImageRequest irq;
                irq.set_filename(filename);
                grpc::WriteOptions writeOptions;
                (_call->response_reader)->Write(irq, writeOptions ,  reinterpret_cast<void*>(Type::WRITE));
                std::cout << "Client sends filename: " <<   irq.filename() << std::endl;
                // while(!wrote_data) {
                    
                // }
            } else if (_call->state_ == READ) {
                // server writes to the channel
             
                (_call->response_reader)->Read(&irq,  reinterpret_cast<void*>(Type::READ));
                std::cout << "Server sends response: " <<   irq.id() << std::endl;
                std::cout << irq.id().compare("OK") << std::endl;
                std::cout << irq.id().length() << std::endl;
                if (irq.id().compare("OK") == 0) {
                     (_call->response_reader)->WritesDone(reinterpret_cast<void*>(Type::WRITES_DONE));
                    wrote_data = true;
                }

            } else if (_call->state_ == WRITES_DONE) {
                  //(_call->response_reader)->WritesDone(reinterpret_cast<void*>(Type::WRITES_DONE));

            } else if (_call->state_ == FINISH) {
                 
                 std::cout << "Finish Reading from the server " << std::endl; 
                  (_call->response_reader)->Finish(&statu, got_tag);
                finish = true;
                return;

            }

            std::cout << " before completion queue next " << std::endl;


            // response from the server 



            if(!cq_.Next(&got_tag, &ok)) {
                std::cerr << "Client stream closed. Quitting" << std::endl;
                break;
            }

            std::cout << " after completion queue next " << std::endl;

            if (ok) {
                std::cout << std::endl
                        << "**** Processing completion queue tag " << got_tag
                        << std::endl;
                
                switch (static_cast<Type>(reinterpret_cast<size_t>(got_tag))) {
                case Type::READ:
                    std::cout << "Read a new message." << std::endl;
                    if(!wrote_data) {
                        _call->state_ = READ;
                    } else {
                        _call->state_ = WRITES_DONE;
                    }
                    break;
                case Type::WRITE:
                    std::cout << "Sending message (async). write" << std::endl;
                    _call->state_ = READ;
                    break;
                case Type::CONNECT:
                    std::cout << "Server connected." << std::endl;
                    _call->state_ = WRITE;
                    break;
                case Type::WRITES_DONE:
                    std::cout << "Writes done" << std::endl;
                    _call->state_ = FINISH;
                    break;
                case Type::FINISH:
                    std::cout << "Finish Reading from the server " << std::endl; 
                   (_call->response_reader)->Finish(&statu, got_tag);
                   finish = true;
                    
                    break;
                default:
                    std::cerr << "Unexpected tag " << got_tag << std::endl;
                    GPR_ASSERT(false);
                }

            } else {
                std::cout << " Not Okay" << std::endl;
            }            
        }
    }





    




    // Assembles the client's payload and sends it to the server. (Example of a async bidirectional)
    void SendImage(std::string filename) {
        AsyncClientCall* _call = new AsyncClientCall;
        _call->state_ = START;
        _call->context.AddMetadata("custom-header", "99999"); 

        bool finish = false;
        bool wrote_data = false;
        grpc::Status statu = Status::OK;
        ImageResponse irq;

        void* got_tag;
        bool ok = false;

        while(!finish) {

           if(_call->state_  == START) {
                 // call for client to connect to the server
                  
                _call->response_reader = stub_->AsyncSendImage(&_call->context, &cq_,  reinterpret_cast<void*>(Type::CONNECT));
                std::cout << " start stub " << std::endl;
            } else if (_call->state_  == CONNECTED) {
                // server is connected
                std::cout << " async server connected" << std::endl;
            }  else if (_call->state_  == WRITE) {
                // server calls for read
                ImageRequest irq;
                irq.set_filename(filename);
                grpc::WriteOptions writeOptions;
                (_call->response_reader)->Write(irq, writeOptions ,  reinterpret_cast<void*>(Type::WRITE));
                std::cout << "Client sends filename: " <<   irq.filename() << std::endl;
                // while(!wrote_data) {
                    
                // }
            } else if (_call->state_ == READ) {
                // server writes to the channel
             
                (_call->response_reader)->Read(&irq,  reinterpret_cast<void*>(Type::READ));
                std::cout << "Server sends response: " <<   irq.id() << std::endl;
                std::cout << irq.id().compare("OK") << std::endl;
                std::cout << irq.id().length() << std::endl;
                if (irq.id().compare("OK") == 0) {
                     (_call->response_reader)->WritesDone(reinterpret_cast<void*>(Type::WRITES_DONE));
                    wrote_data = true;
                }

            } else if (_call->state_ == WRITES_DONE) {
                  //(_call->response_reader)->WritesDone(reinterpret_cast<void*>(Type::WRITES_DONE));

            } else if (_call->state_ == FINISH) {
                 
                 std::cout << "Finish Reading from the server " << std::endl; 
                  (_call->response_reader)->Finish(&statu, got_tag);
                finish = true;
                return;

            }

            std::cout << " before completion queue next " << std::endl;


            // response from the server 



            if(!cq_.Next(&got_tag, &ok)) {
                std::cerr << "Client stream closed. Quitting" << std::endl;
                break;
            }

            std::cout << " after completion queue next " << std::endl;

            if (ok) {
                std::cout << std::endl
                        << "**** Processing completion queue tag " << got_tag
                        << std::endl;
                
                switch (static_cast<Type>(reinterpret_cast<size_t>(got_tag))) {
                case Type::READ:
                    std::cout << "Read a new message." << std::endl;
                    if(!wrote_data) {
                        _call->state_ = READ;
                    } else {
                        _call->state_ = WRITES_DONE;
                    }
                    break;
                case Type::WRITE:
                    std::cout << "Sending message (async). write" << std::endl;
                    _call->state_ = READ;
                    break;
                case Type::CONNECT:
                    std::cout << "Server connected." << std::endl;
                    _call->state_ = WRITE;
                    break;
                case Type::WRITES_DONE:
                    std::cout << "Writes done" << std::endl;
                    _call->state_ = FINISH;
                    break;
                case Type::FINISH:
                    std::cout << "Finish Reading from the server " << std::endl; 
                   (_call->response_reader)->Finish(&statu, got_tag);
                   finish = true;
                    
                    break;
                default:
                    std::cerr << "Unexpected tag " << got_tag << std::endl;
                    GPR_ASSERT(false);
                }

            } else {
                std::cout << " Not Okay" << std::endl;
            }            
        }
    }

    
  private:

    // call state
    enum CallType {START ,CONNECTED, READ, WRITE, WRITE_META ,WRITES_DONE, FINISH};

    // struct for keeping state and data information
    struct AsyncClientCall {
        // Container for the data we expect from the server.
        ImageResponse repsonse;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;
		
		// with a deadline maybe try exponential backoff 
        // Storage for the status of the RPC upon completion.
        Status status;

        std::shared_ptr<ClientAsyncReaderWriter<ImageRequest, ImageResponse>> response_reader;


        // Let's implement a tiny state machine with the following states.
        
        CallType state_;  // The current serving state.
    };

    struct AsyncClientUnaryCall {
        // Container for the data we expect from the server.
        ImageResponse repsonse;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;
		
		// with a deadline maybe try exponential backoff 
        // Storage for the status of the RPC upon completion.
        Status status;

        std::shared_ptr<ClientAsyncWriter<ImageRequest, ImageResponse>> response_reader;


        // Let's implement a tiny state machine with the following states.
        
        CallType state_;  // The current serving state.
    };

    // Out of the passed in Channel comes the stub, stored here, our view of the
    // server's exposed services.
    std::unique_ptr<masterworker::RpcImage::Stub> stub_;

    // The producer-consumer queue we use to communicate asynchronously with the
    // gRPC runtime.
    CompletionQueue cq_;
  
};





int main(int argc, char** argv) {
  TaskClient greeter(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));

    // Async RPC call that sends a message and awaits a response.
  greeter.SendImage("hello");
     
  return 0;
 }