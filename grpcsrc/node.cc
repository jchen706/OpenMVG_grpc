

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



// class TaskClient {
//   public:
//     explicit TaskClient(std::shared_ptr<Channel> channel): stub_(RpcImage::NewStub(channel)) {}

//     // Assembles the client's payload and sends it to the server.
//     void SendImage(std::string filename) {
//         // Data we are sending to the server.
         
//         // Call object to store rpc data
//         AsyncClientCall* call = new AsyncClientCall;



// 		// set deadline on the client call
// 		// dead line is set to 20 seconds
// 		std::chrono::time_point<std::chrono::system_clock> deadline = std::chrono::system_clock::now() +
// 		std::chrono::milliseconds(20000);
		
// 		call->context.set_deadline(deadline);

//         // stub_->PrepareAsyncSayHello() creates an RPC object, returning
//         // an instance to store in "call" but does not actually start the RPC
//         // Because we are using the asynchronous API, we need to hold on to
//         // the "call" instance in order to get updates on the ongoing RPC.
//         call->response_reader =
//             stub_->PrepareAsyncSendImage(&call->context, &cq_);

//         // StartCall initiates the RPC call
//         call->response_reader->StartCall((void*)call);

//         // Request that, upon completion of the RPC, "reply" be updated with the
//         // server's response; "status" with the indication of whether the operation
//         // was successful. Tag the request with the memory address of the call object.
//         call->response_reader->Finish(&call->status, (void*)call);

//     }

//     // Loop while listening for completed responses.
//     // Prints out the response from the server.
//     ImageResponse AsyncCompleteRpc() {
//         void* got_tag;
//         bool ok = false;

//         // Block until the next result is available in the completion queue "cq".
//         //int check = 0;
//         while(cq_.Next(&got_tag, &ok)) {
//             // The tag in this example is the memory location of the call object
//             //std::cout << " Complete Rpc Call: " <<  check << std::endl;
             
//             AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

//             // Verify that the request was completed successfully. Note that "ok"
//             // corresponds solely to the request for updates introduced by Finish().
//             GPR_ASSERT(ok);
			
//             if (call->status.ok()) {
//                 //std::cout << "RPC reply: " << call->reply.price() << std::endl;
//                  //check++;
                
//                 ImageResponse ir1;
//                 call->response_reader->Read( &ir1 , (void*)call);
//                 std::cout << " Received message:  " << ir1.id() << std::endl;






//                 // std::cout << "Greeter received: " << call->reply.message() << std::endl;
//             } else {
//                 std::cout << "RPC failed" << std::endl;
// 				std::cout << "Error Message: " << (call->status.error_message()) <<std::endl;
// 				//std::cout << (call->status.error_code()) <<std::endl;

//                  //check++;
// 				// call->reply.set_tasksuccess(0);
//                 //return call->response_reader->Finish();
//             }

//             // Once we're complete, deallocate the call object.
//             delete call;
//         }
           
//         //std::cout << " End Async Rpc Call: " <<  check << std::endl;
//     }

//   private:

//     // struct for keeping state and data information
//     struct AsyncClientCall {
//         // Container for the data we expect from the server.
//         ImageResponse repsonse;

//         // Context for the client. It could be used to convey extra information to
//         // the server and/or tweak certain RPC behaviors.
//         ClientContext context;
		

// 		// with a deadline maybe try exponential backoff 

//         // Storage for the status of the RPC upon completion.
//         Status status;


//         std::unique_ptr<ClientAsyncReaderWriter<ImageRequest, ImageResponse>> response_reader;
//     };

//     // Out of the passed in Channel comes the stub, stored here, our view of the
//     // server's exposed services.
//     std::unique_ptr<masterworker::RpcImage::Stub> stub_;

//     // The producer-consumer queue we use to communicate asynchronously with the
//     // gRPC runtime.
//     CompletionQueue cq_;
// };

//enum class Type { READ = 1, WRITE = 2, CONNECT = 3, DONE = 4, FINISH = 5 };


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
        service_->RequestSendImage(&ctx_, &stream, cq_, cq_, this);
        std::cout << "create after send image process" << std::endl;
      } else if (status_ == PROCESS) {
        // Spawn a new CallData instance to serve new clients while we process
        // the one for this CallData. The instance will deallocate itself as
        // part of its FINISH state.
        new CallData(service_, cq_);


        // if(this == reinterpret_cast<void*>(Type::CONNECT)) {
        //   std::cout << "connection" << std::endl;
          // ImageRequest request_1;
          // stream.Read(&request_1 , reinterpret_cast<void*>(Type::READ));

          // std::cout << "Read stream filename " << request_1.filename() << std::endl;
        // } else if (this == (void*)2) {
        //   std::cout << "write from client" << std::endl;
        // }  else if (this == (void*)3) {
        //   std::cout << "write finish from client" << std::endl;
        // } else if (this == (void*)4) {
        //   std::cout << "read from client" << std::endl;
        // }
        std::cout << "in the process call data" << std::endl;
        // switch (static_cast<Type>(reinterpret_cast<size_t>(this))) {
        //   case Type::READ: {
        //     std::cout << "Read a new message." << std::endl;
        //     grpc::WriteOptions options;
        //     ImageResponse rep;
        //     rep.set_id("9");
        //     stream.Write(rep, options,reinterpret_cast<void*>(Type::WRITE));
        

        //     break;
        //   } case Type::WRITE: {
        //     std::cout << "Sending message (async)." << std::endl;
        //     ImageRequest request_1;
        
        //     stream.Read(&request_1 , reinterpret_cast<void*>(Type::READ));

        //     std::cout << "Read stream filename " << request_1.filename() << std::endl;

        //     break;
        //   }case Type::CONNECT:{
        //     std::cout << "Client connected." << std::endl;

            

        //     ImageRequest request_1;

        //     stream.Read(&request_1 , reinterpret_cast<void*>(Type::READ));

        //     std::cout << "Read stream filename " << request_1.filename() << std::endl;

            

        //     //AsyncWaitForHelloRequest();
        //     break;
        //   }case Type::DONE: {
        //     std::cout << "Server disconnecting." << std::endl;
        //     //is_running_ = false;
        //     break;
        //   }case Type::FINISH:
        //     std::cout << "Server quitting." << std::endl;
        //     break;
        //   default:
        //     std::cerr << "Unexpected tag "  << std::endl;
           
        //     //GPR_ASSERT(false);
        // }

        // std::cout << " call data" << std::endl;

       
    

        
        
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
   
    void* tag;  // uniquely identifies a request.
    bool ok;
    while (true) {
      // Block waiting to read the next event from the completion queue. The
      // event is uniquely identified by its tag, which in this case is the
      // memory address of a CallData instance.
      // The return value of Next should always be checked. This return value
      // tells us whether there is any kind of event or cq_ is shutting down.
      std::cout << " handling server tag  before next" << std::endl;

      if(!cq_->Next(&tag, &ok)) {
        std::cout<<"Client streamin closed"<<std::endl;
        break;
      }

      GPR_ASSERT(ok);
      std::cout << " handling server tag: " <<  tag << std::endl;
      
    // std::cout << tag << std::endl;
    //  switch (static_cast<Type>(reinterpret_cast<size_t>(tag))) {
    //       case Type::READ: {
    //         std::cout << "Read a new message." << std::endl;
    //         // grpc::WriteOptions options;
    //         // ImageResponse rep;
    //         // rep.set_id("9");
    //         // stream.Write(rep, options,reinterpret_cast<void*>(Type::WRITE));
        

    //         break;
    //       } case Type::WRITE: {
    //         std::cout << "Sending message (async)." << std::endl;
    //         // ImageRequest request_1;
        
    //         // stream.Read(&request_1 , reinterpret_cast<void*>(Type::READ));

    //         // std::cout << "Read stream filename " << request_1.filename() << std::endl;

    //         break;
    //       }case Type::CONNECT:{
    //         std::cout << "Client connected." << std::endl;

            

    //         // ImageRequest request_1;

    //         // stream.Read(&request_1 , reinterpret_cast<void*>(Type::READ));

    //         // std::cout << "Read stream filename " << request_1.filename() << std::endl;

            

    //         //AsyncWaitForHelloRequest();
    //         break;
    //       }case Type::DONE: {
    //         std::cout << "Server disconnecting." << std::endl;
    //         //is_running_ = false;
    //         break;
    //       }case Type::FINISH:
    //         std::cout << "Server quitting." << std::endl;
    //         break;
    //       default:
    //         std::cerr << "Unexpected tag "  << std::endl;
           
    //         //GPR_ASSERT(false);
    //     }
      //std::cout << static_cast<Type>(reinterpret_cast<size_t>(tag)) << " hello" << std::endl;
      static_cast<CallData*>(tag)->Proceed();
    }
  }

  std::unique_ptr<ServerCompletionQueue> cq_;
  RpcImage::AsyncService service_;
  std::unique_ptr<Server> server_;
};



// int main(int argc, char** argv) {
 
//   ServerImpl server;
//   server.Run();

//   // TaskClient greeter(grpc::CreateChannel( "0.0.0.0:50052", grpc::InsecureChannelCredentials())); // create a channel
//   // greeter.SendImage("R");  
// }







// NOTE: This is a complex example for an asynchronous, bidirectional streaming
// server. For a simpler example, start with the
// greeter_server/greeter_async_server first.

// Most of the logic is similar to AsyncBidiGreeterClient, so follow that class
// for detailed comments. Two main differences between the server and the client
// are: (a) Server cannot initiate a connection, so it first waits for a
// 'connection'. (b) Server can handle multiple streams at the same time, so
// the completion queue/server have a longer lifetime than the client(s).
class AsyncBidiGreeterServer {
  enum class Type {
    READ = 1,
    WRITE = 2,
    CONNECT = 3,
    WRITES_DONE = 4,
    FINISH = 5,
    DONE=6
  };
 public:
  AsyncBidiGreeterServer() {
    // In general avoid setting up the server in the main thread (specifically,
    // in a constructor-like function such as this). We ignore this in the
    // context of an example.
    std::string server_address("0.0.0.0:50051");

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    cq_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();

    // This initiates a single stream for a single client. To allow multiple
    // clients in different threads to connect, simply 'request' from the
    // different threads. Each stream is independent but can use the same
    // completion queue/context objects.
    // stream_.reset(
    //     new ServerAsyncReaderWriter<ImageResponse, ImageRequest>(&context_));
    // service_.RequestSendImage(&context_, stream_.get(), cq_.get(), cq_.get(),
    //                          reinterpret_cast<void*>(Type::CONNECT));

    // // This is important as the server should know when the client is done.
    // context_.AsyncNotifyWhenDone(reinterpret_cast<void*>(Type::DONE));

    // grpc_thread_.reset(new std::thread(
    //     (std::bind(&AsyncBidiGreeterServer::GrpcThread, this))));

    HandleRpcs();

    std::cout << "Server listening on " << server_address << std::endl;
  }

  void SetResponse(const std::string& response) {
    // if (response == "quit" && IsRunning()) {
    //   stream_->Finish(grpc::Status::CANCELLED,
    //                   reinterpret_cast<void*>(Type::FINISH));
    // }
    response_str_ = response;
  }

  ~AsyncBidiGreeterServer() {
    std::cout << "Shutting down server...." << std::endl;
    server_->Shutdown();
    // Always shutdown the completion queue after the server.
    cq_->Shutdown();
    grpc_thread_->join();
  }

  bool IsRunning() const { return is_running_; }

 private:
  void AsyncWaitForHelloRequest() {
    //if (IsRunning()) {
      // In the case of the server, we wait for a READ first and then write a
      // response. A server cannot initiate a connection so the server has to
      // wait for the client to send a message in order for it to respond back.
      metadata_ = context_.client_metadata();
      auto auth = metadata_.find("custom-header");
      if (auth == metadata_.end()) {
        std::cout << "metadata not found " << std::endl;
      } else {
        std::cout << auth->second << std::endl;
      }
      stream_->Read(&request_, reinterpret_cast<void*>(Type::READ));
    //}
  }

  void AsyncHelloSendResponse() {
    std::cout << " ** Handling request: " << request_.filename() << std::endl;
    ImageResponse response;
    std::cout << " ** Sending response: " << response_str_ << std::endl;
    response.set_id(response_str_);

    stream_->Write(response, reinterpret_cast<void*>(Type::WRITE));
  }

  void GrpcThread() {
    while (true) {
      void* got_tag = nullptr;
      bool ok = false;
      if (!cq_->Next(&got_tag, &ok)) {
        std::cerr << "Server stream closed. Quitting" << std::endl;
        break;
      }

      if (ok) {
        std::cout << std::endl
                  << "**** Processing completion queue tag " << got_tag
                  << std::endl;
        switch (static_cast<Type>(reinterpret_cast<size_t>(got_tag))) {
          case Type::READ:
            std::cout << "Read a new message." << std::endl;
            AsyncHelloSendResponse();
            break;
          case Type::WRITE:
            std::cout << "Sending message (async)." << std::endl;
            AsyncWaitForHelloRequest();
            break;
          case Type::CONNECT:
            std::cout << "Client connected." << std::endl;
            AsyncWaitForHelloRequest();
            break;
          case Type::DONE:
            std::cout << "Server disconnecting." << std::endl;
            is_running_ = false;
            break;
          case Type::FINISH:
            std::cout << "Server quitting." << std::endl;
            break;
          default:
            std::cerr << "Unexpected tag " << got_tag << std::endl;
            GPR_ASSERT(false);
        }
      }
    }
  }



  void HandleRpcs() {

    stream_.reset(
        new ServerAsyncReaderWriter<ImageResponse, ImageRequest>(&context_));
    service_.RequestSendImage(&context_, stream_.get(), cq_.get(), cq_.get(),
                             reinterpret_cast<void*>(Type::CONNECT));
    
    // auto auth = metadata.find("custom-header");
    // if (auth == metadata.end()) {
    //   std::cout << "metadata not found " << std::endl;
    // }

    // This is important as the server should know when the client is done.
    context_.AsyncNotifyWhenDone(reinterpret_cast<void*>(Type::DONE));

    //  metadata  = context_.client_metadata();
            
    // for (auto iter = metadata.begin(); iter != metadata.end(); ++iter) {
    //           std::cout << "Header key: " << iter->first << ", value: " << iter->second;
    // }
    
  

     while (true) {
      void* got_tag = nullptr;
      bool ok = false;
      if (!cq_->Next(&got_tag, &ok)) {
        std::cerr << "Server stream closed. Quitting" << std::endl;
        break;
      }

      if (ok) {
        std::cout << std::endl
                  << "**** Processing completion queue tag " << got_tag
                  << std::endl;
        // auto auth = metadata.find("custom-header");
        // if (auth == metadata.end()) {
        //   std::cout << "metadata not found " << std::endl;
        // } else {
        //   std::cout << " found meta " << std::endl;
        // }
        switch (static_cast<Type>(reinterpret_cast<size_t>(got_tag))) {
          case Type::READ:
            std::cout << "Read a new message." << std::endl;
            AsyncHelloSendResponse();
            break;
          case Type::WRITE:
            std::cout << "Sending message (async)." << std::endl;
            AsyncWaitForHelloRequest();
            break;
          case Type::CONNECT:
            std::cout << "Client connected." << std::endl;
            AsyncWaitForHelloRequest();
            break;
          case Type::DONE:
            std::cout << "Server disconnecting." << std::endl;
            is_running_ = false;
            break;
          case Type::FINISH:
            std::cout << "Server quitting." << std::endl;
            break;
          default:
            std::cerr << "Unexpected tag " << got_tag << std::endl;
            GPR_ASSERT(false);
        }
      }
    }
  }

 private:
  ImageRequest request_;
  std::string response_str_ = "OK";
  ServerContext context_;
  std::unique_ptr<ServerCompletionQueue> cq_;
  RpcImage::AsyncService service_;
  std::unique_ptr<Server> server_;
  std::unique_ptr<ServerAsyncReaderWriter<ImageResponse, ImageRequest>> stream_;
  std::unique_ptr<std::thread> grpc_thread_;
  std::multimap<grpc::string_ref, grpc::string_ref> metadata_; 




  bool is_running_ = true;
};

int main(int argc, char** argv) {
  AsyncBidiGreeterServer server;

  std::string response = "response from server";
  // while (server.IsRunning()) {
  //   std::cout << "Enter next set of responses (type quit to end): ";
  //   std::cin >> response;

  server.SetResponse(response);
  //}

  return 0;
}















