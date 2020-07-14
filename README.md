# Simple Chatroom

Clear hierarchy for easy maintenances.

- `common.h`: Definitions for communication.
- `linker.c`: Linking btw client/server.
- `client.c`/`client.h`
- `server.c`/`server.h`

You can also read my report for usage and implementation overviews.

## How to run

### Installation: `make`

#### Environment: Standard Linux, require `cowsay` and `curl` installed.

The makefile will create three folders, `bin/`, `asset/`, and `file/`.

- `bin/` is where the compiled binary files are.
- `asset/` and `file/` are folders for storage when trying to launch a chat server.
- Launch server: `bin/chat_server [PORT]`
- Launch client: `bin/chat_client [SERVER_IP] [PORT]`
- Uninstall: `make clean`
