# QShell MCP Control Protocol

QShell can expose a local Streamable HTTP MCP endpoint for controlled terminal automation. The endpoint is disabled by default and, when enabled, listens only on:

```text
http://127.0.0.1:<port>/mcp
```

The implementation follows the MCP 2025-06-18 JSON-RPC lifecycle for `initialize`, `notifications/initialized`, `tools/list`, and `tools/call`. It returns HTTP 405 for GET because QShell does not provide an SSE stream.

## Enable MCP

1. Open `File` -> `Setting`.
2. Enable `Enable MCP`.
3. Keep the default port `8765` or choose another local port.
4. Copy the generated `MCP Token`, or click `Regenerate` to create a new token.
5. Click `OK`.

Changing MCP settings restarts the local endpoint. Disabling MCP releases the listening port.

## Configure Codex

Use the token shown in QShell settings:

```bash
export QSHELL_MCP_TOKEN='<token from QShell settings>'
codex mcp add qshell --url http://127.0.0.1:8765/mcp --bearer-token-env-var QSHELL_MCP_TOKEN
```

## Security

The MCP endpoint is intended for local automation only.

- It binds only to `127.0.0.1`.
- Every request must include `Authorization: Bearer <token>`.
- Requests with an `Origin` header are accepted only from `localhost`, `127.0.0.1`, or `[::1]`.
- Session listing omits passwords, SSH private-key passphrases, and other secret fields.
- The server does not expose arbitrary Lua execution.

## Tools

| Tool | Purpose |
| --- | --- |
| `qshell_get_status` | Return QShell version, tab count, current tab name, and MCP listener state. |
| `qshell_list_sessions` | Return configured session `id`, `name`, `protocol`, and `groupId`. |
| `qshell_open_session_by_id` | Open a configured session by `sessionId`. |
| `qshell_open_session_by_name` | Open a configured session by `sessionName`. |
| `qshell_switch_tab` | Switch to a tab by zero-based `index` or tab `name`. |
| `qshell_next_tab` | Switch to the next tab. |
| `qshell_connect_current` | Connect the current tab if disconnected. |
| `qshell_disconnect_current` | Disconnect the current tab if connected. |
| `qshell_send_text` | Send `text` to the current terminal. Supports `interpretEscapes` for `\r`, `\n`, and `\t`. |
| `qshell_send_key` | Send a named key such as `Enter`, `Tab`, `Ctrl+C`, `Up`, or `F1`. |
| `qshell_get_screen_text` | Return visible text from the current terminal screen. |
| `qshell_get_last_line` | Return the last visible terminal line. |
| `qshell_clear_screen` | Clear the current terminal screen. |
| `qshell_wait_for_string` | Wait for `text` in terminal output until `timeoutMs` or `timeoutSeconds`. |
| `qshell_wait_for_regex` | Wait for `pattern` in terminal output until `timeoutMs` or `timeoutSeconds`. |

All tool results include MCP `content` text and `structuredContent` JSON. Operational failures, such as no current terminal or a timeout, are returned as tool results. JSON-RPC protocol errors, such as unknown methods or malformed requests, are returned as JSON-RPC errors.

## Manual Protocol Check

Replace `$QSHELL_MCP_TOKEN` and the port as needed:

```bash
curl -sS http://127.0.0.1:8765/mcp \
  -H "Authorization: Bearer $QSHELL_MCP_TOKEN" \
  -H "Content-Type: application/json" \
  -H "Accept: application/json, text/event-stream" \
  -d '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-06-18","capabilities":{},"clientInfo":{"name":"curl","version":"1"}}}'
```

Then list tools:

```bash
curl -sS http://127.0.0.1:8765/mcp \
  -H "Authorization: Bearer $QSHELL_MCP_TOKEN" \
  -H "Content-Type: application/json" \
  -H "Accept: application/json, text/event-stream" \
  -d '{"jsonrpc":"2.0","id":2,"method":"tools/list"}'
```

