<h1>CumulusMQ</h1>

CumulusMQ is an event-driven job queue designed to efficiently handle messaging tasks through a TCP interface. This project aims to facilitate seamless communication between various components in a distributed system.

<h2>Features</h2>
<li>Event-driven architecture</li>
<li>TCP-based interaction</li>
<li>Flexible message handling</li>


<h2>Protocol</h2>

CumulusMQ utilizes a custom TCP protocol designed for efficient message handling. The protocol allows clients to subscribe, unsubscribe, publish, and retrieve messages from the queue. Each message follows a specific structure to ensure reliable communication and processing. The protocol fields include message length, message type, topic length, topic, body length, and message body, facilitating structured and organized message transmission.

| Message Length | Message Type | Topic Length | Message Body | Body Length | Message Body |  
|----------------|--------------|-------------|---------------|-------------|--------------|
| 4 bytes        | 1 byte       | 4 bytes     | m bytes       | 4 bytes     | n bytes      | 


| Message Type Value | Action      |
|--------------------|-------------|
| 0x01               | Subscribe   |
| 0x02               | Unsubscribe |
| 0x03               | Publish     |
| 0x04               | Retrieve    |

<h2>Building CumulusMQ</h2>

<h4>Prerequisites</h4>

- CMake: Version 3.5 or higher
- C++ Compiler: Clang 18+ or any compiler that supports C++17
- Zig: 0.13.0

<h4>Steps to Build</h4>

<h5>Building with CMake</h5>

1. Clone the repository:

```sh

git clone https://github.com/vidurkhanal/CumulusMQ.git
cd CumulusMQ
```

2. Create a build directory:

It is a good practice to create a separate directory for building the project.

```sh

mkdir build
cd build
```

3. Run CMake to configure the project:

```sh

cmake ..
```

This will generate the necessary build files in the build directory.

4. Build the project:

```sh

cmake --build .
```



<h5>Building with Zig</h5>

1. Clone the repository:

```sh

git clone https://github.com/vidurkhanal/CumulusMQ.git
cd CumulusMQ
```

2. Run zig-build
   
```sh

zig build
```
This will compile the source files and create the executable.

<h4>Running the Executable</h4>

Once the build process completes successfully, you can run the generated executable. The executable will be located in the build directory:

```sh

./CumulusMQ
```

Ensure that you have the necessary permissions to execute the file. If not, you might need to adjust the permissions using chmod.

<h3>Troubleshooting</h3>

- **CMake Version Issues**: Ensure you have CMake version 3.5 or higher.
- **Compiler Support**: Make sure you are using Clang 19 or any compiler that supports C++17.
- **Zig Version Issues**: Ensure you have Zig version 0.13.0 or higher.
- **Dependencies**: Ensure all dependencies and libraries are correctly installed and accessible.

<h2>Usage</h2>

Here’s an example of how to format a message to publish a job to a topic:

1. Calculate the lengths:
   - Topic: "example_topic" (13 bytes)
   - Message Body: "Hello, World!" (13 bytes)
2. Construct the message:
   
| Message Length | Message Type | Topic Length | Message Body | Body Length | Message Body |  
|----------------|--------------|-------------|---------------|-------------|--------------|
| 0x00000021 (33 bytes)        | 0x03 (Publish) | 0x0000000D (13 bytes)  | example_topic       | 0x0000000D (13 bytes)  | Hello, World! | 

3. Send the message over TCP:
Use your preferred TCP client to connect and send the byte stream.

<h2>Contributing</h2>
Contributions are welcome! Please submit a pull request or open an issue to discuss what you would like to change.

<h2>License</h2>
This project is licensed under the MIT License - see the LICENSE file for details.




