// FROM: http://chriswu.me/blog/getting-request-uri-and-content-in-c-plus-plus-fcgi/

#include <stdlib.h>
#include <iostream>
#include <string>
#include <fcgi_stdio.h>
#include "fcgio.h"
#include "ConfigFile.h"
#include <fstream>
#include <string.h>

using namespace std;

// Maximum bytes
const unsigned long STDIN_MAX = 10000000;

string get_request_content(const FCGX_Request &request) {
    char *content_length_str = FCGX_GetParam("CONTENT_LENGTH", request.envp);
    unsigned long content_length = STDIN_MAX;

    if (content_length_str) {
        content_length = strtol(content_length_str, &content_length_str, 10);
        if (*content_length_str) {
            cerr << "Can't Parse 'CONTENT_LENGTH='"
                 << FCGX_GetParam("CONTENT_LENGTH", request.envp)
                 << "'. Consuming stdin up to " << STDIN_MAX << endl;
        }

        if (content_length > STDIN_MAX) {
            content_length = STDIN_MAX;
        }
    } else {
        // Do not read from stdin if CONTENT_LENGTH is missing
        content_length = 0;
    }

    char *content_buffer = new char[content_length];
    cin.read(content_buffer, content_length);
    content_length = cin.gcount();
    do cin.ignore(1024); while (cin.gcount() == 1024);

    string content(content_buffer, content_length);
    delete[] content_buffer;
    return content;
}

string getFilename(const FCGX_Request &request) {
    const char *contentDisposition = FCGX_GetParam("HTTP_CONTENT_DISPOSITION", request.envp);
    if (contentDisposition) {
        string content(contentDisposition);
        int index = (int) content.find("filename=");
        string filename;
        if (index != -1) {
            index += 10;
            try {
                while (contentDisposition[index] != '\"') {
                    filename.push_back(contentDisposition[index]);
                    index++;
                }
            }
            catch (exception &e) {
                perror("Error reading filename.");
                return "";
            }
            return filename;
        }
    }
    return "";
}

int main(void) {
    // Backup the stdio streambufs
    streambuf *cin_streambuf = cin.rdbuf();
    streambuf *cout_streambuf = cout.rdbuf();
    streambuf *cerr_streambuf = cerr.rdbuf();

    FCGX_Request request;

    FCGX_Init();
    int port;
    string ip = "";
    if (ConfigFile::getConfigFile().readFCGI(&ip, &port) == -1) return -1;
    string s = ":";
    s.append(to_string(port));
    char const *port_char = s.c_str();
    int sock = FCGX_OpenSocket(port_char, 1024);
    FCGX_InitRequest(&request, sock, 0);

    while (FCGX_Accept_r(&request) >= 0) {
        fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);

        cin.rdbuf(&cin_fcgi_streambuf);
        cout.rdbuf(&cout_fcgi_streambuf);
        cerr.rdbuf(&cerr_fcgi_streambuf);

        // get the request URI
        const char *uri = FCGX_GetParam("REQUEST_URI", request.envp);
        string content = get_request_content(request);
        if (content.length() == 0) {
            content = "";
        }
        string filename = getFilename(request);
        if (filename.length() > 0) {
            if (filename.substr(filename.length() - 4, 4) == ".txt") {
                ofstream myfile;
                myfile.open(filename.c_str());
                myfile << content << "\n";
                myfile.close();
                // TODO różne headery
                cout << "HTTP/1.1 200 OK\r\n"
                     << "Content-type: text/html\r\n"
                     << "\r\n"
                     << "<html>\n"
                     << "  <head>\n"
                     << "    <title>POST request!</title>\n"
                     << "  </head>\n"
                     << "  <body>\n"
                     << "    <h1>FILE SAVED to " << filename.c_str() << "/h1>\n"
                     << "  </body>\n"
                     << "</html>\n"
                     << "\r\n";
            } else {
                cout << "HTTP/1.1 200 OK\r\n"
                     << "Content-type: text/html\r\n"
                     << "\r\n"
                     << "<html>\n"
                     << "  <head>\n"
                     << "    <title>POST request!</title>\n"
                     << "  </head>\n"
                     << "  <body>\n"
                     << "    <h1>Wrong file extension./h1>\n"
                     << "  </body>\n"
                     << "</html>\r\n"
                     << "\r\n";
            }
        } else {
            cout << "HTTP/1.1 200 OK\r\n"
                 << "Content-type: text/html\r\n"
                 << "\r\n"
                 << "<html>\n"
                 << "  <head>\n"
                 << "    <title>Hello, World!</title>\n"
                 << "  </head>\n"
                 << "  <body>\n"
                 << "    <h1>Hello " << " from " << uri << " !</h1>\n"
                 << "  </body>\n"
                 << "</html>\n"
                 << "\r\n";
        }
    }

// restore stdio streambufs
    cin.rdbuf(cin_streambuf);
    cout.rdbuf(cout_streambuf);
    cerr.rdbuf(cerr_streambuf);

    return 0;

}

