# mawimctl protocol 1.0
MaWiM control - Command line tool for controlling MaWiM

## Communication
All communication between mawimctl and MaWiM occur over a UNIX Domain Socket
located at `/tmp/mawim.control.socket`.

Communication follows the scheme of mawimctl command -> MaWiM Response.

The command format is as follows:
| Offset | Length | Description
| ------ | ------ | -----------
| 0      | 1      | Command Identifier
| 1      | 1      | Flags
| 2      | 1      | Data Length (x)
| 3      | x      | Command Data

Reponses to commands follow this format:
| Offset | Length | Description
| ------ | ------ | -----------
| 0      | 1      | Status
| 1      | 2      | Data Length (x)
| 2      | x      | Response Data

## Commands
**header file:** `mawimctl.h`

| Indentifier | Name
| ----------- | ----
| 0x00        | MAWIMCTL_GET_VERSION
| 0x01        | MAWIMCTL_GET_WORKSPACE
| 0x02        | MAWIMCTL_SET_WORKSPACE
| 0x03        | MAWIMCTL_RELOAD
| 0x04        | MAWIMCTL_CLOSE_FOCUSED
| 0x05        | MAWIMCTL_MOVE_FOCUSED_TO_WORKSPACE

### MAWIMCTL_GET_VERSION
Causes MaWiM to respond with its NULL-terminated, ascii version string.

### MAWIMCTL_GET_WORKSPACE
Causes MaWiM to respond with the number of the currently active workspace.

### MAWIMCTL_SET_WORKSPACE
Causes MaWiM to change the Workspace. The Data Length for this command has to be 1,
with the data containing the number of the workspace to be set.

The workspace number is a 1-based index.

MaWiM may respond with status MAWIMCTL_OK, or MAWIMCTL_NO_SUCH_WORKSPACE.

### MAWIMCTL_RELOAD
Causes MaWiM to reload its config and update the state of all windows.

MaWiM may respond with status MAWIMCTL_OK, MAWIMCTL_CONFIG_MISSING or MAWIMCTL_CONFIG_MALFORMED.

### MAWIMCTL_CLOSE_FOCUSED
Causes MaWiM to close the currently focused window.

MaWiM may respond with status MAWIMCTL_OK, or MAWIMCTL_NO_WINDOW_FOCUSED.

### MAWIMCTL_MOVE_FOCUSED_TO_WORKSPACE
Causes MaWiM to attempt to move the currently focused window to the specified workspace.
The Data Length for this command has to be 1, with the data containing the number of the destination workspace.

MaWiM may respond with MAWIMCTL_OK, MAWIMCTL_NO_WINDOW_FOCUSED, or MAWIMCTL_NO_SUCH_WORKSPACE.

## Status
**header file:** `mawimctl.h`

| Identifier | Name
| ---------- | ----
| 0x00       | MAWIMCTL_OK
| 0x01       | MAWIMCTL_INVALID_COMMAND
| 0x02       | MAWIMCTL_INVALID_DATA_FORMAT
| 0x03       | MAWIMCTL_NO_SUCH_WORKSPACE
| 0x04       | MAWIMCTL_CONFIG_MISSING
| 0x05       | MAWIMCTL_CONFIG_MALFORMED
| 0x06       | MAWIMCTL_NO_WINDOW_FOCUSED
| 0x07       | MAWIMCTL_INTENRAL_ERROR

## Flags
**header file:** `mawimctl.h`

| Bit (left to right) | Description
| ------------------- | -----------
| 0                   | When set, disables responses from MaWiM
| 1-7                 | Unused

## API Reference
### mawimctl.h
#### MACRO MAWIMCTL_DEFAULT_SOCK_LOCATION
```c
#define MAWIMCTL_DEFAULT_SOCK_LOCATION "/tmp/mawim.control.socket"
```

The default location for the mawimctl socket.

#### MACRO MAWIMCTL_FLAG_NO_RESPONSE
```c
#define MAWIMCTL_FLAG_NO_RESPONSE 0b10000000
```

Mask for the NO_RESPONSE flag.

#### MACRO MAWIMCTL_COMMAND_BASESIZE
```c
#define MAWIMCTL_COMMAND_BASESIZE 5
```

Defines the minimum/base size for a command in bytes.

#### MACRO MAWIMCTL_COMMAND_MAXSIZE
```c
#define MAWIMCTL_COMMAND_MAXSIZE MAWIMCTL_COMMAND_BASESIZE + UINT16_MAX
```

Defines the maximum size for a command in bytes.

#### MACRO MAWIMCTL_RESPONSE_BASESIZE
```c
#define MAWIMCTL_RESPONSE_BASESIZE 3
```

Defines the minimum/base size for a response in bytes.

#### MACRO MAWIMCTL_RESPONSE_MAXSIZE
```c
#define MAWIMCTL_RESPONSE_MAXSIZE MAWIMCTL_RESPONSE_BASESIZE + UINT16_MAX
```

Defines the maximum size for a response in bytes.

#### typealias mawimctl_workspaceid_t
```c
typedef uint8_t mawimctl_workspaceid_t
```

This typealias is used for identifying workspaces in MaWiM and mawimctl.

#### enum mawimctl_cmd_id
This enumerator represents all commands defined by mawimctl. See the Commands
chapter for more information.

#### enum mawimctl_status
This enumerator represents every status with which a mawimctl server can
respond. See the Status chapter for more information.

#### mawimctl_command_t
```c
typedef struct mawimctl_command {...} mawimctl_command_t
```

This structure type represents a command. Besides the data specified in the
Communication chapter, it also has some extra fields.

* `int       sender_fd;` The socket filedescriptor of the sender. Only set Server-side.
* `uint8_t   command_identifier;` The command identifier (See `enum mawimctl_command`)
* `uint8_t   flags;` Flags for command execution (See the flags chapter)
* `uint16_t  data_length;` Length of the command data
* `uint8_t  *data;` Pointer to the command data

#### mawimctl_response_t
```c
typedef struct mawimctl_response {...} mawimctl_response_t
```

This structure type represents a server response to a command.

* `uint8_t   status;` The status of the response (See `enum mawimctl_status`)
* `uint16_t  data_length;` The length of the response data
* `uint8_t  *data;` Pointer to the command data.

### mawimctl_server.h
#### MACRO MAWIMCTL_SERVER_CONNECTION_BACKLOG_SIZE
```c
#ifndef MAWIMCTL_SERVER_CONNECTION_BACKLOG_SIZE
#define MAWIMCTL_SERVER_CONNECTION_BACKLOG_SIZE 20
#endif
```

The size for the backlog of unaccepted connections for the server.

#### mawimctl_server_t
```c
typedef struct mawimctl_server {...} mawimctl_server_t
```

This structure type represents a mawimctl server. 

* `char               *sock_path;` Path to the UNIX Socket
* `struct sockaddr_un  sock_name;` Internal Socket Name
* `int                 sock_fd;` File descriptor of the connection socket
* `int                 pending_cmd_count;` Amount of commands currently pending
* `mawimctl_command_t *pending_cmds;` Currently pending commands

#### mawimctl_server_start()
```c
mawimctl_server_t *mawimctl_server_start(char *where);
```

Starts a mawimctl server.

Parameters:
* where - The location in the filesystem where the socket should be created. If NULL, it will fallback to `MAWIMCTL_DEFAULT_SOCK_LOCATION` (see mawimctl.h).

Returns:
* NULL if the server start failed, otherwise a pointer to the newly allocated server structure.

#### mawimctl_server_stop()
```c
void mawimctl_server_stop(mawimctl_server_t *server);
```

Stops a mawimctl server and frees its structure.

Parameters:
* server - Pointer to the server structure which should be stopped.

#### mawimctl_server_update()
```c
void mawimctl_server_update(mawimctl_server_t *server);
```

Updates the server. This includes accepting all new connection requests, reading their commands an queueing them.

Parameters:
* server - Pointer to the server structure which should be updated.

#### mawimctl_server_next_command()
```c
bool mawimctl_server_next_command(mawimctl_server_t *server, mawimctl_command_t *dest_container);
```

Gets the first pending command in the server's queue.

Parameters:
* server - Pointer to the server structure from which's queue the command should be grabbed.
* dest_container - Pointer to the destination for the command.

Returns:
* true if a command was written to dest_container. false otherwise.

#### mawimctl_server_respond()
```c
bool mawimctl_server_respond(mawimctl_server_t *server, int sockfd, mawimctl_response_t response);
```

Sends a response to the specified socket.

Parameters:
* server - The server structure which should be responded from.
* sockfd - The socket file descriptor to which should be responded.
* response - The filled out response which should be sent.

### mawimctl_client.h
#### mawimctl_connection_t
```c
typedef struct mawimctl_connection {...} mawimctl_connection_t;
```

This structure represents a client's connection to a mawimctl server.

* `int                 sock_fd;` The filedescriptor for the server connevtion.
* `char               *sock_path;` The path of the socket in the file system.
* `struct sockaddr_un  sock_name;` Internal Socket Name.

#### mawimctl_client_connect()
```c
mawimctl_connection_t *mawimctl_client_connect(char *where);
```

Establishes a connection to the mawimctl server / MaWiM.

Parameters:
* where - The location of the socket in the filesystem.

Returns:
* NULL if connection failed, otherwise a pointer to the newly created connection structure.

#### mawimctl_client_send_command()
```c
bool mawimctl_client_send_command(mawimctl_connection_t *connection, mawimctl_command_t command);
```

Sends a command to the mawimctl server.

Parameters:
* connection - The connection structure to be used for sending.
* command - The command to be sent.

Returns:
* true if successful, false if not.

#### mawimctl_read_response()
```c
bool mawimctl_read_response(mawimctl_connection_t *connection, mawimctl_response_t *dest);
```

Read a response from the mawimctl server into dest.

Parameters:
* connection - The connection from where the response should be read.
* dest - Pointer to the mawimctl_response destination structure.

Returns:
* true on success, false on failure.