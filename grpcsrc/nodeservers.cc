

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


  void HandleRpcs() {

    stream_.reset(
        new ServerAsyncReaderWriter<ImageResponse, ImageRequest>(&context_));
    service_.RequestSendImage(&context_, stream_.get(), cq_.get(), cq_.get(),
                             reinterpret_cast<void*>(Type::CONNECT));

    // This is important as the server should know when the client is done.
    context_.AsyncNotifyWhenDone(reinterpret_cast<void*>(Type::DONE));



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

 private:
  ImageRequest request_;
  std::string response_str_ = "OK";
  ServerContext context_;
  std::unique_ptr<ServerCompletionQueue> cq_;
  RpcImage::AsyncService service_;
  std::unique_ptr<Server> server_;
  std::unique_ptr<ServerAsyncReaderWriter<ImageResponse, ImageRequest>> stream_;
  std::unique_ptr<std::thread> grpc_thread_;




  bool is_running_ = true;
};

int main(int argc, char** argv) {
  AsyncBidiGreeterServer server;


  return 0;
}
