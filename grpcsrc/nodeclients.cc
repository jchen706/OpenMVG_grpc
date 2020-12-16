
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
        _call = new AsyncClientCall;
        _call->state_ = START;
    }


    // Assembles the client's payload and sends it to the server.
    void SendImage(std::string filename) {
        bool finish = false;

        if(_call->state_  == START) {
                 // call for client to connect to the server
                _call->response_reader = stub_->AsyncSendImage(&_call->context, &cq_,  reinterpret_cast<void*>(Type::CONNECT));    
            } else if (_call->state_  == CONNECTED) {
                // server is connected
            } else if (_call->state_  == READ) {
                // server calls for read
                ImageRequest irq;
                irq.set_filename(filename);
                grpc::WriteOptions writeOptions;
                (_call->response_reader)->Write(irq, writeOptions ,  reinterpret_cast<void*>(Type::WRITE));
                std::cout << "Client sends filename: " <<   irq.filename() << std::endl;
            } else if (_call->state_ == WRITE) {
                // server writes to the channel
                ImageResponse irq;
                (_call->response_reader)->Read(&irq,  reinterpret_cast<void*>(Type::READ));
                std::cout << "Server sends response: " <<   irq.id() << std::endl;


            } else if (_call->state_ == WRITES_DONE) {
                  (_call->response_reader)->WritesDone(reinterpret_cast<void*>(Type::WRITES_DONE));
                
            }


            // response from the server 

            void* got_tag;
            bool ok = false;


            if(!cq_.Next(&got_tag, &ok)) {
                std::cerr << "Client stream closed. Quitting" << std::endl;
                break;
            }

            if (ok) {
                std::cout << std::endl
                        << "**** Processing completion queue tag " << got_tag
                        << std::endl;
                
                switch (static_cast<Type>(reinterpret_cast<size_t>(got_tag))) {
                case Type::READ:
                    std::cout << "Read a new message." << std::endl;
                    _call->state_ = READ;
                    break;
                case Type::WRITE:
                    std::cout << "Sending message (async)." << std::endl;
                    _call->state_ = WRITE;
                    break;
                case Type::CONNECT:
                    std::cout << "Server connected." << std::endl;
                    _call->state_ = CONNECTED;
                    break;
                case Type::WRITES_DONE:
                    std::cout << "Server disconnecting." << std::endl;
                    break;
                case Type::FINISH:
                    // std::cout << "Client finish; status = "
                    //         << (finish_status_.ok() ? "ok" : "cancelled")
                    //         << std::endl;
                    //AsyncBidiGreeterClient::~AsyncBidiGreeterClient();
                    break;
                default:
                    std::cerr << "Unexpected tag " << got_tag << std::endl;
                    GPR_ASSERT(false);
                }


            
        }
    }

    // Loop while listening for completed responses.
    // Prints out the response from the server.
    ImageResponse AsyncCompleteRpc() {
        void* got_tag;
        bool ok = false;

        // Block until the next result is available in the completion queue "cq".
        //int check = 0;
        while(cq_.Next(&got_tag, &ok)) {
            // The tag in this example is the memory location of the call object
            //std::cout << " Complete Rpc Call: " <<  check << std::endl;
             
            AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

            // Verify that the request was completed successfully. Note that "ok"
            // corresponds solely to the request for updates introduced by Finish().
            GPR_ASSERT(ok);
			
            if (call->status.ok()) {
                //std::cout << "RPC reply: " << call->reply.price() << std::endl;
                 //check++;

                 switch (static_cast<Type>(reinterpret_cast<long>(got_tag))) {
                      case Type::READ:
                        std::cout << "Read a new message." << std::endl;
                        AsyncSendMessage();
                        break;
                      case Type::WRITE:
                        std::cout << "Sending message (async)." << std::endl;
                        //AsyncHelloRequestNextMessage();
                        break;
                      case Type::CONNECT:
                        std::cout << "Server connected." << std::endl;
                        break;
                      case Type::WRITES_DONE:
                        std::cout << "Server disconnecting." << std::endl;
                        break;
                      case Type::FINISH:
                        // std::cout << "Client finish; status = "
                        //           << (finish_status_.ok() ? "ok" : "cancelled")
                        //           << std::endl;
                        // context_.TryCancel();
                        // cq_.Shutdown();
                        delete call;
                        break;
                      default:
                        std::cerr << "Unexpected tag " << got_tag << std::endl;
                        GPR_ASSERT(false);
                }
                
                // ImageResponse ir1;
                // call->response_reader->Read( &ir1 , (void*)call);
                
                // std::cout << " Received message:  " << ir1.id() << std::endl;



              std::cout << " here 1 222" <<std::endl;
              // call->response_reader->WritesDone( reinterpret_cast<void*>(Type::WRITES_DONE));



                // std::cout << "Greeter received: " << call->reply.message() << std::endl;
            } else {
                std::cout << "RPC failed" << std::endl;
                std::cout << "Error Message: " << (call->status.error_message()) <<std::endl;
                std::cout << (call->status.error_code()) <<std::endl;
                delete call;
                 //check++;
				// call->reply.set_tasksuccess(0);
                //return call->response_reader->Finish();
            }

            // Once we're complete, deallocate the call object.
            
        }
           
        //std::cout << " End Async Rpc Call: " <<  check << std::endl;
    }

  private:

    // call state
    enum CallType {START ,CONNECTED, READ, WRITE, WRITES_DONE, FINISH};

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

    AsyncClientCall* _call;

    // Out of the passed in Channel comes the stub, stored here, our view of the
    // server's exposed services.
    std::unique_ptr<masterworker::RpcImage::Stub> stub_;

    // The producer-consumer queue we use to communicate asynchronously with the
    // gRPC runtime.
    CompletionQueue cq_;
  
};