eServ: Embedded Web server toolkit written in pure C

Features
--------
  * Built-in HTTP daemon along with dynamic content management
  * Multi-threaded HTTP daemon and CGI execution
  * Support POSIX environments

Licensing
---------
  MIT License.  lease check the file LICENSE for details.


Programming Note
----------------
  * namespace: ex

Configurations
--------------
  * file: libeserv/http.h
  * default listen port
    #define  PORT 8000
  * default max threads
    #define EX_MAX_THREADS 10
  * directory www
    static const char *IndexFile = "index.html";
    static const char *RootPath = "www";
