# cpp-search-server
Search server backend and fomt end. Supprot parallel requests

New version of serarch server contains simple frontend:

Custom TCP Server based on boost::asio. It is using acync tcp/ip operation. So it's support many connection(>10K).
Usage search-server <port number>
Be able to use telnet or terminal as client
You need just type key words and server show relevance documents.
"show" - command shows all typed requests
"quit" - exit from terminal and close connetion


Build with C++17
Using Boost C++ library
