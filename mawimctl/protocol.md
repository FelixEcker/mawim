# mawimctl
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

## Flags
**header file:** `mawimctl.h`

| Bit (left to right) | Description
| ------------------- | -----------
| 0                   | When set, disables responses from MaWiM
| 1-7                 | Unused

## API Reference
### mawimctl.h
#### enum mawimctl_cmd_id
This enumerator represents all commands defined by mawimctl. See the Commands
chapter for more information.

#### enum mawimctl_status
This enumerator represents every status with which a mawimctl server can
respond. See the Status chapter for more information.

#### mawimctl_command_t
`typedef struct mawimctl_command {...} mawimctl_command_t`

This structure type represents a command. Besides the data specified in the
Communication chapter, it also has some extra fields.

* `int       sender_fd;` The socket filedescriptor of the sender. Only set Server-side.
* `uint8_t   command_identifier;` The command identifier (See `enum mawimctl_command`)
* `uint8_t   flags;` Flags for command execution (See the flags chapter)
* `uint16_t  data_length;` Length of the command data
* `uint8_t  *data;` Pointer to the command data

#### mawimctl_response_t
`typedef struct mawimctl_response {...} mawimctl_response_t`

This structure type represents a server response to a command.

* `uint8_t   status;` The status of the response (See `enum mawimctl_status`)
* `uint16_t  data_length;` The length of the response data
* `uint8_t  *data;` Pointer to the command data.

### mawimctl_server.h
#### maiwmctl_server_t
`typedef struct mawimctl_server {...} mawimctl_server_t`

This structure type represents a mawimctl server. 

* `char               *sock_path;` Path to the UNIX Socket
* `struct sockaddr_un  sock_name;` Internal Socket Name
* `int                 sock_fd;` File descriptor of the connection socket
* `int                 pending_cmd_count;` Amount of commands currently pending
* `mawimctl_command_t *pending_cmds;` Currently pending commands
