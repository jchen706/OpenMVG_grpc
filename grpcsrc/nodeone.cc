

// // one node can either be worker or a leader



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



// add_executable(
//   nodeone #library name
//   nodeone.cc             #sources
//   gps_distance.h )           #headers
// target_link_libraries(nodeone p4protolib)
// target_link_libraries(nodeone ${OPENMVG_LIBRARIES})
// TARGET_LINK_LIBRARIES(nodeone
//   openMVG_system
//   openMVG_image
//   openMVG_features
//   openMVG_sfm
//   openMVG_exif
// )
// target_include_directories(nodeone PUBLIC ${MAPREDUCE_INCLUDE_DIR})
// #target_include_directories(nodeone PUBLIC ${Boost_INCLUDE_DIR})
// add_dependencies(nodeone p4protolib)
// target_link_libraries(nodeone Boost::thread)
// target_link_libraries(nodeone ${Boost_LIBRARIES} )


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

    // Assembles the client's payload and sends it to the server.
    void SendImage(std::string filename) {
      AsyncClientCall* call = new AsyncClientCall;
        

         std::cout << " here 4" <<std::endl;
        // StartCall initiates the RPC call

        call->response_reader = stub_->AsyncSendImage(&call->context, &cq_,  reinterpret_cast<void*>(Type::CONNECT));
        
        std::cout << " here 5" <<std::endl;
        

        ImageRequest irq;
        irq.set_filename(filename);
        // Call object to store rpc data
        //call->response_reader->
       
        std::cout << " here 1 " <<std::endl;

        try
        {
          /* code */
          grpc::WriteOptions writeOptions;
          (call->response_reader)->Write(irq, writeOptions ,  (void*)2); // E1207 17:35:49.704812226    1537 call_op_set.h:973]          assertion failed: false
          // //   //std::sleep(std::chrono::seconds(1));
          
        }
        catch(const std::exception& e)
        {
          std::cerr << e.what() << '\n';
        }
        
        //call->response_reader->WritesDone((void*) call);
        std::cout << " here 2 " <<std::endl;


		// set deadline on the client call
		// dead line is set to 20 seconds
		// std::chrono::time_point<std::chrono::system_clock> deadline = std::chrono::system_clock::now() +
		// std::chrono::milliseconds(20000);
		
		// call->context.set_deadline(deadline);

    std::cout << " here 3 " <<std::endl;

        // stub_->PrepareAsyncSayHello() creates an RPC object, returning
        // an instance to store in "call" but does not actually start the RPC
        // Because we are using the asynchronous API, we need to hold on to
        // the "call" instance in order to get updates on the ongoing RPC.
        // Data we are sending to the server.
       
         std::cout << " here 4" <<std::endl;
        // StartCall initiates the RPC call
        //call->response_reader->StartCall((void*)call);
        std::cout << " here 5" <<std::endl;
       

      
        // Request that, upon completion of the RPC, "reply" be updated with the
        // server's response; "status" with the indication of whether the operation
        // was successful. Tag the request with the memory address of the call object.

        call->response_reader->Finish(&call->status, (void*)call);

    }



    void ReceiveImage(std::string filename) {

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
    };


    // Out of the passed in Channel comes the stub, stored here, our view of the
    // server's exposed services.
    std::unique_ptr<masterworker::RpcImage::Stub> stub_;

    // The producer-consumer queue we use to communicate asynchronously with the
    // gRPC runtime.
    CompletionQueue cq_;
};



class ServerImpl final {
 public:
  ~ServerImpl() {
    server_->Shutdown();
    // Always shutdown the completion queue after the server.
    cq_->Shutdown();
  }

  // There is no shutdown handling in this code.
  void Run() {
    TaskClient greeter(grpc::CreateChannel( "0.0.0.0:50051", grpc::InsecureChannelCredentials())); // create a channel
	greeter.SendImage("M");
  greeter.AsyncCompleteRpc();
    
    // std::string server_address("0.0.0.0:50052");

    // ServerBuilder builder;
    // // Listen on the given address without any authentication mechanism.
    // builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // // Register "service_" as the instance through which we'll communicate with
    // // clients. In this case it corresponds to an *asynchronous* service.
    // builder.RegisterService(&service_);
    // // Get hold of the completion queue used for the asynchronous communication
    // // with the gRPC runtime.
    // cq_ = builder.AddCompletionQueue();
    // // Finally assemble the server.
    // server_ = builder.BuildAndStart();
    // std::cout << "Server listening on " << server_address << std::endl;

    // // Proceed to the server's main loop.

    // HandleRpcs();
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
      Proceed();
    }

    void Proceed() {
      if (status_ == CREATE) {
        // Make this instance progress to the PROCESS state.
        status_ = PROCESS;

        // As part of the initial CREATE state, we *request* that the system
        // start processing SayHello requests. In this request, "this" acts are
        // the tag uniquely identifying the request (so that different CallData
        // instances can serve different requests concurrently), in this case
        // the memory address of this CallData instance.
        service_->RequestSendImage(&ctx_, &stream, cq_, cq_,
                                  this);

      } else if (status_ == PROCESS) {
        // Spawn a new CallData instance to serve new clients while we process
        // the one for this CallData. The instance will deallocate itself as
        // part of its FINISH state.
        new CallData(service_, cq_);
 
        ImageRequest request_1;
        
        stream.Read(&request_1 , this);

        std::cout << "Read stream filename " << request_1.filename() << std::endl;

        ImageResponse rep;
        rep.set_id("8");
        
        stream.Write(rep, this);

        // The actual processing.
        // std::string prefix("Hello ");
        // reply_.set_message(prefix + request_.name());

        // And we are done! Let the gRPC runtime know we've finished, using the
        // memory address of this instance as the uniquely identifying tag for
        // the eve
        status_ = FINISH;
        stream.Finish(Status::OK, this);
      } else {
        GPR_ASSERT(status_ == FINISH);
        // Once in the FINISH state, deallocate ourselves (CallData).
        delete this;
      }
    }

   private:
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
      GPR_ASSERT(cq_->Next(&tag, &ok));
      GPR_ASSERT(ok);
      if(tag == (void*)1) {
           std::cout << "Client connected." << std::endl;
      }
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


  






  
// }









class AsyncBidiGreeterClient {
  enum class Type {
    READ = 1,
    WRITE = 2,
    CONNECT = 3,
    WRITES_DONE = 4,
    FINISH = 5
  };

 public:
  explicit AsyncBidiGreeterClient(std::shared_ptr<Channel> channel)
      : stub_(RpcImage::NewStub(channel)) {
    grpc_thread_.reset(
        new std::thread(std::bind(&AsyncBidiGreeterClient::GrpcThread, this)));
    stream_ = stub_->AsyncSendImage(&context_, &cq_,
                                   reinterpret_cast<void*>(Type::CONNECT));
  }

  // Similar to the async hello example in greeter_async_client but does not
  // wait for the response. Instead queues up a tag in the completion queue
  // that is notified when the server responds back (or when the stream is
  // closed). Returns false when the stream is requested to be closed.
  bool AsyncSayHello(const std::string& user) {
    if (user == "quit") {
      stream_->WritesDone(reinterpret_cast<void*>(Type::WRITES_DONE));
      return false;
    }

    // Data we are sending to the server.
    ImageRequest request;
    request.set_filename(user);

    // This is important: You can have at most one write or at most one read
    // at any given time. The throttling is performed by gRPC completion
    // queue. If you queue more than one write/read, the stream will crash.
    // Because this stream is bidirectional, you *can* have a single read
    // and a single write request queued for the same stream. Writes and reads
    // are independent of each other in terms of ordering/delivery.
    std::cout << " ** Sending request: " << user << std::endl;
    stream_->Write(request, reinterpret_cast<void*>(Type::WRITE));
    return true;
  }

  ~AsyncBidiGreeterClient() {
    std::cout << "Shutting down client...." << std::endl;
    grpc::Status status;
    cq_.Shutdown();
    grpc_thread_->join();
  }

 private:
  void AsyncHelloRequestNextMessage() {
    std::cout << " ** Got response: " << response_.id() << std::endl;

    // The tag is the link between our thread (main thread) and the completion
    // queue thread. The tag allows the completion queue to fan off
    // notification handlers for the specified read/write requests as they
    // are being processed by gRPC.
    stream_->Read(&response_, reinterpret_cast<void*>(Type::READ));
  }

  // Runs a gRPC completion-queue processing thread. Checks for 'Next' tag
  // and processes them until there are no more (or when the completion queue
  // is shutdown).
  void GrpcThread() {
    while (true) {
      void* got_tag;
      bool ok = false;
      // Block until the next result is available in the completion queue "cq".
      // The return value of Next should always be checked. This return value
      // tells us whether there is any kind of event or the cq_ is shutting
      // down.
      if (!cq_.Next(&got_tag, &ok)) {
        std::cerr << "Client stream closed. Quitting" << std::endl;
        break;
      }

      // It's important to process all tags even if the ok is false. One might
      // want to deallocate memory that has be reinterpret_cast'ed to void*
      // when the tag got initialized. For our example, we cast an int to a
      // void*, so we don't have extra memory management to take care of.
      if (ok) {
        std::cout << std::endl
                  << "**** Processing completion queue tag " << got_tag
                  << std::endl;
        
        switch (static_cast<Type>(reinterpret_cast<size_t>(got_tag))) {
          case Type::READ:
            std::cout << "Read a new message." << std::endl;
            break;
          case Type::WRITE:
            std::cout << "Sending message (async)." << std::endl;
            AsyncHelloRequestNextMessage();
            break;
          case Type::CONNECT:
            std::cout << "Server connected." << std::endl;
            break;
          case Type::WRITES_DONE:
            std::cout << "Server disconnecting." << std::endl;
            break;
          case Type::FINISH:
            std::cout << "Client finish; status = "
                      << (finish_status_.ok() ? "ok" : "cancelled")
                      << std::endl;
             //AsyncBidiGreeterClient::~AsyncBidiGreeterClient();
            break;
          default:
            std::cerr << "Unexpected tag " << got_tag << std::endl;
            GPR_ASSERT(false);
        }
      }
    }
  }

  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context_;

  // The producer-consumer queue we use to communicate asynchronously with the
  // gRPC runtime.
  CompletionQueue cq_;

  // Out of the passed in Channel comes the stub, stored here, our view of the
  // server's exposed services.
  std::unique_ptr<RpcImage::Stub> stub_;

  // The bidirectional, asynchronous stream for sending/receiving messages.
  std::unique_ptr<ClientAsyncReaderWriter<ImageRequest, ImageResponse>> stream_;

  // Allocated protobuf that holds the response. In real clients and servers,
  // the memory management would a bit more complex as the thread that fills
  // in the response should take care of concurrency as well as memory
  // management.
  ImageResponse response_;

  // Thread that notifies the gRPC completion queue tags.
  std::unique_ptr<std::thread> grpc_thread_;

  // Finish status when the client is done with the stream.
  grpc::Status finish_status_ = grpc::Status::OK;
};

int main(int argc, char** argv) {
  AsyncBidiGreeterClient greeter(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));

    // Async RPC call that sends a message and awaits a response.
  greeter.AsyncSayHello("hello");
     
  return 0;
 }






// // Assembles the client's payload, sends it and presents the response back
//   // from the server.
//   std::string SayHello(const std::string& user) {
//     // Data we are sending to the server.
//     HelloRequest request;
//     request.set_name(user);

//     // Container for the data we expect from the server.
//     HelloReply reply;

//     // Context for the client. It could be used to convey extra information to
//     // the server and/or tweak certain RPC behaviors.
//     ClientContext context;

//     // The producer-consumer queue we use to communicate asynchronously with the
//     // gRPC runtime.
//     CompletionQueue cq;

//     // Storage for the status of the RPC upon completion.
//     Status status;

//     // stub_->PrepareAsyncSayHello() creates an RPC object, returning
//     // an instance to store in "call" but does not actually start the RPC
//     // Because we are using the asynchronous API, we need to hold on to
//     // the "call" instance in order to get updates on the ongoing RPC.
//     std::unique_ptr<ClientAsyncResponseReader<HelloReply> > rpc(
//         stub_->PrepareAsyncSayHello(&context, request, &cq));

//     // StartCall initiates the RPC call
//     rpc->StartCall();

//     // Request that, upon completion of the RPC, "reply" be updated with the
//     // server's response; "status" with the indication of whether the operation
//     // was successful. Tag the request with the integer 1.
//     rpc->Finish(&reply, &status, (void*)1);
//     void* got_tag;
//     bool ok = false;
//     // Block until the next result is available in the completion queue "cq".
//     // The return value of Next should always be checked. This return value
//     // tells us whether there is any kind of event or the cq_ is shutting down.
//     GPR_ASSERT(cq.Next(&got_tag, &ok));

//     // Verify that the result from "cq" corresponds, by its tag, our previous
//     // request.
//     GPR_ASSERT(got_tag == (void*)1);
//     // ... and that the request was completed successfully. Note that "ok"
//     // corresponds solely to the request for updates introduced by Finish().
//     GPR_ASSERT(ok);

//     // Act upon the status of the actual RPC.
//     if (status.ok()) {
//       return reply.message();
//     } else {
//       return "RPC failed";
//     }
//   }








