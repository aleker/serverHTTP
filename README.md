# HTTP Server
Simple HTTP server with FCGI application.

## Additional libraries and packages
* libconfig 1.5-3 for C language (with libconfig.h header)

[libconfig github](https://github.com/hyperrealm/libconfig/blob/master/README)

[libconfig documentation](http://www.hyperrealm.com/libconfig/libconfig_manual.html#Using-the-Library-from-a-C-Program)

* fcgi 2.4.0-12

## Compiling
Project include CMakeLists.txt.

If problems occured, please use makefile. Instructions:

Go to path:
```
cd /serwerHTTP
make
```
## Settings
You can use config file config.cfg (which is in serverHTTP/bin directory) to set certain arguments.

## Running programs
Go to path /bin:
```
cd /serwerHTTP/bin
```
Run FCGI Application:
```
./fcgi
```
Run HTTP server with sudo privilege and type ip address and port respectively e.g.:
```
sudo ./serverHTTP 0.0.0.0 80
```

## Testing
* GET

To test the GET method, use the Internet brower. Go to address 0.0.0.0 and you should see a simple "Hello world" message.
Please make sure the server and fcgi application are running. You can also add additional parameters after the /, eg "0.0.0.0/test".

* POST

To test the POST method, you can use curl. It should be already installed on your computer system. If not, you can download it [here](https://curl.haxx.se/download.html).
Examples of usage:
 * Simple POST method.
 
The simplest POST method gets the similiar "Hello world" response.
 ```
 curl -d "your_text_here" 0.0.0.0:80
 ```
 
 * Sending a txt file with POST.

Sending the txt file with curl will be forwarded to the fcgi application and then, the fcgi app will create a new txt file
with the same name and save the sent content.
  ```
  curl -F "file=@path_to_txt_file" 0.0.0.0:80
  ```
    
  * Sending file with a different extenstion with POST
  
Sending the file with a different extension (eg jpg or pdf files) will get a "Wrong file format" response.
  ```
  curl -F "file=@path_to_a_different_file" 0.0.0.0:80
  ```
  
