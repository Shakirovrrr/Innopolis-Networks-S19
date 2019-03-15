# `socket()`

Creates a new connection socket.

The call might be nonblocking if the `SOCK_NONBLOCK` flag applied by bitwise or to `type` arg.

## Return value

Returns the socket file descriptor or `-1` in case of failure.

## Args

- `int domain` - Communication domain, specifies the thing with which we're going to connect. Values predefined in `<sys/socket.h>`, for example, `AF_INET` and `AF_INET6` for IPv4 and IPv6 respectively or `AF_LOCAL` for local communication.
- `int type` - Communication type, or how we're going to transfer data. For example, `SOCK_STREAM` means TCP connection, `SOCK_DGRAM` - UDP.
- `int protocol` - WHich protocol we're going to use. Usually left as `0` (default protocol) but something different might be retrieved with `protocols()`.


# `accept()`

Accepts new request from queue and creates a new socket to connect to client.

This call blocks the execution while connecting to the client.

## Return value

Returns the new socket file descriptor or `-1` in case of failure.

## Args

- `int sockfd` - Listening socket which we created with `socket()`.
- `struct sockaddr *addr` - Pointer to structure where the client address will be written.
- `int addrlen` - Pointer to integer. The `addr` structure size will be written here.

# `select()`

Waits for some of file descriptors to become unlocked for operations.

## Return value

Returns the number of file descriptors, `-1` on failure or `0` on timeout.

## Args

- `int nfds` - Highest number of all provided FDs in all FD sets.
- `fd_set *readfds` - File descriptors to wait for available to read.
- `fd_set *writefds` - File descriptors to wait for available to write.
- `fd_set *exceptfds` - File descriptors to wait for available to something different.
- `struct timeval *timeout` - Timeout for waiting.

# `bind()`

Binds a socket with specified address.

## Return value

Returns `0` on success and `-1` on failure.

## Args

- `int sockfd` - FD of the socket we want to bind with address.
- `const struct sockaddr *addr` - Structure with address.
- `socklen_t addrlen` - Size of the address structure.
