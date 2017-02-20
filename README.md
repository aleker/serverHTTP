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
You have to remember that fcgi must be running before you start server HTTP.
