CMAKE_MINIMUM_REQUIRED( VERSION 3.1 )

PROJECT( SimpleServer CXX )

SET( CMAKE_CXX_STANDARD 11 )

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic" )

ADD_EXECUTABLE( qotd
  qotd.cc
  ClientSocket.cc
  Server.cc
)
