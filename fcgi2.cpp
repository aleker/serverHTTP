#include <fcgicc.h>
#include <algorithm>

int handle_request(FastCGIRequest& request) {
    // This is always the first event to occur.  It occurs when the
    // server receives all parameters.  There may be more data coming on the
    // standard input stream.

    if (request.params.count("REQUEST_URI"))
        return 0;  // OK, continue processing
    else
        return 1;  // stop processing and return error code
}

int handle_data(FastCGIRequest& request) {
    // This event occurs when data is received on the standard input stream.
    // A simple string is used to hold the input stream, so it is the
    // responsibility of the application to remember which data it has
    // processed. The application may modify it; new data will be appended
    // to it by the server. The same goes for the output and error streams:
    // the application should append data to them; the server will remove
    // all sent data from them.

    std::transform(request.in.begin(), request.in.end(),
                   std::back_inserter(request.err),
                   std::bind1st(std::plus<char>(), 1));
    request.in.clear();  // don't process it again
    return 0;  // still OK
}

class Application {
public:
    int handle_complete(FastCGIRequest& request) {
        // The event handler can also be a class member function. This
        // event occurs when the parameters and standard input streams are
        // both closed, and thus the request is complete.

        request.out.append("Content-Type: text/plain\r\n\r\n");
        request.out.append("You requested: ");
        request.out.append(request.params[std::string("REQUEST_URI")]);
        return 0;
    }
};

int main(int argc, char** argv) {
    FastCGIServer server; // Instantiate a server

// Set up our request handlers
    server.request_handler(&handle_request);
    server.data_handler(&handle_data);
    server.complete_handler(application, &Application::handle_complete);

    server.listen(7000);        // Listen on a TCP port
    server.listen(7001);        // ... or on two
    server.listen("./socket");  // ... and also on a local doman socket

    server.process(100); // Process some data, but don't wait more
    // than 100 ms for it to arrive

    server.process(); // Process some data with no timeout

    server.process_forever(); // Process everything
}

