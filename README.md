eServ: Embedded Web server toolkit written in pure C

Origin
------
  * Jserv's eserv https://code.google.com/p/eserv/

Features
--------
  * Built-in HTTP daemon along with dynamic content management
  * Multi-threaded HTTP daemon and CGI execution
  * Support POSIX environments

Licensing
---------
  MIT License.  Please check the file LICENSE for details.
  The demo CGI programs utilize jQuery, which is copyrighted by
  jQuery Foundation and other contributors.  Both eserv and jQuery
  are released under MIT License.

Programming Note
----------------
  * namespace: ex

Configurations
--------------
  * file: libeserv/http.h
  * default listen port
    #define  PORT 80
  * default max threads
    #define EX_MAX_THREADS 10
  * directory www
    static const char *IndexFile = "index.html";
    static const char *RootPath = "www";

Session
-------
  * it works, but it's not GOOD enough.


