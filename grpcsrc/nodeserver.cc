

// one node can either be worker or a leader



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

// GRPC_TRACE=ap
// GRPC_VERBOSITY=debug

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


class ServerImpl final {
   enum class Type {
    READ = 1,
    WRITE = 2,
    CONNECT = 3,
    WRITES_DONE = 4,
    FINISH = 5,
    DONE=6
  };

 public:
  ~ServerImpl() {
    server_->Shutdown();
    // Always shutdown the completion queue after the server.
    cq_->Shutdown();
  }

  // There is no shutdown handling in this code.
  void Run() {
    std::string server_address("0.0.0.0:50051");

    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service_" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *asynchronous* service.
    builder.RegisterService(&service_);
    // Get hold of the completion queue used for the asynchronous communication
    // with the gRPC runtime.
    cq_ = builder.AddCompletionQueue();
    // Finally assemble the server.
    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    // Proceed to the server's main loop.
    HandleRpcs();
  }

 private:
  // Class encompasing the state and logic needed to serve a request.
  class CallData {
   public:
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    CallData(RpcImage::AsyncService* service, ServerCompletionQueue* cq)
        : service_(service), cq_(cq), stream(&ctx_), status_(CREATE) {
      // Invoke the serving logic right away.

    
    
    }

    void Proceed() {
      if (status_ == CREATE) {
        // Make this instance progress to the PROCESS state.
        status_ = PROCESS;
        std::cout << "create process" << std::endl;

        // As part of the initial CREATE state, we *request* that the system
        // start processing SayHello requests. In this request, "this" acts are
        // the tag uniquely identifying the request (so that different CallData
        // instances can serve different requests concurrently), in this case
        // the memory address of this CallData instance.
        service_->RequestSendImage(&ctx_, &stream, cq_, cq_,
                                  reinterpret_cast<void*>(Type::CONNECT));
        std::cout << "create after send image process" << std::endl;
      } else if (status_ == PROCESS) {
        // Spawn a new CallData instance to serve new clients while we process
        // the one for this CallData. The instance will deallocate itself as
        // part of its FINISH state.
        new CallData(service_, cq_);


        // if(this == reinterpret_cast<void*>(Type::CONNECT)) {
        //   std::cout << "connection" << std::endl;
        //   ImageRequest request_1;
        //   stream.Read(&request_1 , reinterpret_cast<void*>(Type::READ));

        //   std::cout << "Read stream filename " << request_1.filename() << std::endl;
        // } else if (this == (void*)2) {
        //   std::cout << "write from client" << std::endl;
        // }  else if (this == (void*)3) {
        //   std::cout << "write finish from client" << std::endl;
        // } else if (this == (void*)4) {
        //   std::cout << "read from client" << std::endl;
        // }

        std::cout << "in the process call data" << this  << std::endl;
        switch (static_cast<Type>(reinterpret_cast<size_t>(this))) {
          case Type::READ: {
            std::cout << "Read a new message." << std::endl;
            grpc::WriteOptions options;
            ImageResponse rep;
            rep.set_id("9");
            stream.Write(rep, options,reinterpret_cast<void*>(Type::WRITE));
        

            break;
          } case Type::WRITE: {
            std::cout << "Sending message (async)." << std::endl;
            ImageRequest request_1;
        
            stream.Read(&request_1 , reinterpret_cast<void*>(Type::READ));

            std::cout << "Read stream filename " << request_1.filename() << std::endl;

            break;
          }case Type::CONNECT:{
            std::cout << "Client connected." << std::endl;

            

            ImageRequest request_1;

            stream.Read(&request_1 , reinterpret_cast<void*>(Type::READ));

            std::cout << "Read stream filename " << request_1.filename() << std::endl;

            

            //AsyncWaitForHelloRequest();
            break;
          }case Type::DONE: {
            std::cout << "Server disconnecting." << std::endl;
            //is_running_ = false;
            break;
          }case Type::FINISH:
            std::cout << "Server quitting." << std::endl;
            break;
          default:
            std::cerr << "Unexpected tag "  << std::endl;
           
            //GPR_ASSERT(false);
        }

        std::cout << " call data" << std::endl;

       
    

        
        
        status_ = FINISH;
        
        
        stream.Finish(Status::OK, this);

     
        

        // The actual processing.
        // std::string prefix("Hello ");
        // reply_.set_message(prefix + request_.name());

        // And we are done! Let the gRPC runtime know we've finished, using the
        // memory address of this instance as the uniquely identifying tag for
        // the event.
       
      } else {
        GPR_ASSERT(status_ == FINISH);
        // Once in the FINISH state, deallocate ourselves (CallData).
        delete this;
      }
    }


   

   private:


    // void AsyncSendResponse(ServerAsyncReaderWriter<ImageResponse, ImageRequest> stream) {
    //   ImageResponse rep;
    //   rep.set_id(7);
    
    //   std::cout << " ** Sending response: " << 7 << std::endl;
    //   //response.set_message(response_str_);
    //   stream->Write(rep, reinterpret_cast<void*>(Type::WRITE));
    // }
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    RpcImage::AsyncService* service_;
    // The producer-consumer queue where for asynchronous server notifications.
    ServerCompletionQueue* cq_;
    // Context for the rpc, allowing to tweak aspects of it such as the use
    // of compression, authentication, as well as to send metadata back to the
    // client.
    ServerContext ctx_;



    // What we get from the client.
    ImageRequest request_;
    // What we send back to the client.
    ImageResponse reply_;

    // The means to get back to the client.
    ServerAsyncReaderWriter<ImageResponse, ImageRequest> stream;

    // Let's implement a tiny state machine with the following states.
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;  // The current serving state.
  };

  // This can be run in multiple threads if needed.
  void HandleRpcs() {
    // Spawn a new CallData instance to serve new clients.
    new CallData(&service_, cq_.get());
    std::cout << "new call instance in handle rpcs" << std::endl;
    void* tag;  // uniquely identifies a request.
    bool ok;
    while (true) {
      // Block waiting to read the next event from the completion queue. The
      // event is uniquely identified by its tag, which in this case is the
      // memory address of a CallData instance.
      // The return value of Next should always be checked. This return value
      // tells us whether there is any kind of event or cq_ is shutting down.
      GPR_ASSERT(cq_->Next(&tag, &ok));
      GPR_ASSERT(ok);
      

     
      //std::cout << static_cast<Type>(reinterpret_cast<size_t>(tag)) << " hello" << std::endl;
      static_cast<CallData*>(tag)->Proceed();
    }
  }

  std::unique_ptr<ServerCompletionQueue> cq_;
  RpcImage::AsyncService service_;
  std::unique_ptr<Server> server_;
};



int main(int argc, char** argv) {
 
  ServerImpl server;
  server.Run();

  // TaskClient greeter(grpc::CreateChannel( "0.0.0.0:50052", grpc::InsecureChannelCredentials())); // create a channel
  // greeter.SendImage("R");  
}
