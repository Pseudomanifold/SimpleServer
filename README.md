# SimpleServer

This repository contains [my](http://bastian.rieck.ru) implementation of
a simple TCP/IP server in C++. I started fiddling with this to learn
more about UNIX network programming.

See [my blog](http://bastian.rieck.ru/blog/posts/2015/sockets_ordeal_cxx11) for
additional information and comments.

## How do I use the API?

The sources are meant to become an API when they grow up. At present,
there is only an implementation of a *Quote of the Day* service. See
`qotd.cc` for more details.

## What's cool about it?

Good question. I wanted to encapsulate the C API for socket programming
and make it more modern. So far, I did not really succeed, I guess.
There is one cool thing at the moment: Requests are handled by
specifying an arbitrary function object (yay, C++11). This function is
then called asynchronously (yay, C++11 again!) whenever a new client is
accepted. You can then send stuff to the client or whatever...

## How do I run my own QOTD server?

That at least I may answer! After cloning this repository, please run
the following commands:

    $ mkdir build
    $ cd build
    $ cmake ../
    $ make
    $ ./qotd ../Quotes.txt

You can exit the server with CTRL+C. You can connect to your server like
this:

    $ nc localhost 1041

## Why is the QOTD server not running under port 17?

Because most Linux distributions disallow opening ports below 1024
without root access or at least configuration changes. While I think
that I am a trustworthy person, I do not presume that you want to
compile this as root...

Thus, the QOTD server runs under port 1041, which is 1024+17 and hence
quite clever.

## Licence

The code is released under an MIT licence.
