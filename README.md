# SimpleWebServer

A rudimentary HTTP server written in C. It listens on a specified port for incoming client requests. Upon receiving a request, the server processes it and sends back a corresponding response.

## Features
- Utilizes the `select` system call to handle multiple clients.
- Supports basic error handling (e.g., 404 Not Found, 500 Internal Server Error).
- Processes GET requests and responds with appropriate content.

## Dependencies
- Standard C libraries
- `wrapsock.h` and `ws_helpers.h` for additional socket and HTTP utilities.

## How to Run
1. Clone this repository:
git clone https://github.com/duankunh/SimpleWebServer.git

2. Navigate to the repository directory:

cd SimpleWebServer

markdown

3. Compile the server:

gcc -o wserver wserver.c -Wall

csharp

4. Run the server on a specified port (e.g., port 8080):

./wserver 8080

csharp

5. In a web browser or using a tool like `curl`, make a request to `http://localhost:8080/`

## Limitations
- The server is designed for educational purposes and is not production-ready.
- Only handles a limited set of HTTP requests.

## Contributing
Feel free to fork this repository and make improvements or extend the functionality.

## License
MIT License
