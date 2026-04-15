# Soporte de IA

Puede optar por utilizar su modelo de IA favorito (LLM) para ayudarle con sus notas.
Hay soporte integrado para **[Groq](https://groq.com/)** (porque puede obtener claves gratuitas de la API) y **[OpenAI](https://openai.com/)**,
pero la API interna está diseñada para poder extenderse fácilmente a otros modelos de IA con scripts.
Hay varios scripts de integración de backend de IA disponibles en el repositorio de scripts para que pueda comenzar.

::: tip
La **API de finalización de OpenAI** se implementó para utilizarse en **scripts**.
De esta manera puede hacer lo que quiera y experimentar con ella.
:::

- Hay una **barra de herramientas de IA** y una sección en el **menú principal Editar**, en donde puede activar
  el servicio activado y seleccionar el backend y el modelo de IA
- Puede agregar una clave de la API de [OpenAI](https://openai.com/) y [Groq](https://groq.com/) en la nueva **configuración de IA**
  - Busque más **integraciones de backend de IA**, como _Ollama_ y _llama.cpp_ en el repositorio de scripts
- Utilice la nueva orden de secuencias de órdenes "script.aiComplete(prompt)" de acuerdo con su propia imaginación
  - Para más información, por favor revise la
    [documentación de secuencias de órdenes](../scripting/methods-and-objects.md#use-a-completion-prompt-on-the-currently-selected-ai-model)
- En el **repositorio de scripts**, como primer paso, hay un script de
  [finalización automática de IA](https://github.com/qownnotes/scripts/tree/master/ai-autocompletion),
  que utiliza el modelo de IA seleccionado para **completar automáticamente la selección actual** en la edición del texto de la nota,
  y un script de [herramienta de texto de IA](https://github.com/qownnotes/scripts/tree/master/ai-text-tool),
  que utiliza el modelo de IA seleccionado para **procesar la selección actual** en la edición del texto de la nota,
  por ejemplo para resumirlo, traducirlo o corregir errores tipográficos y gramaticales

![ai-settings](/img/editor/ai-settings.webp)

## MCP Server

QOwnNotes includes a built-in **MCP (Model Context Protocol) server** that allows external AI agents
to access your notes over HTTP with Server-Sent Events (SSE) transport.

You can enable and configure the MCP server in the **AI / MCP server** settings page.

- **Enable/disable** the MCP server
- **Configure the port** (default: 22226)
- **Bearer token authentication** — a security token is auto-generated and must be included as a
  `Bearer` token in the `Authorization` header of all requests

### Available MCP Tools

The MCP server exposes the following tools to AI agents:

- **`search_notes`** — Full-text search across all notes in the current note folder.
  Accepts a `query` string parameter and an optional `limit` (default 20).
  Returns matching note names and a short preview of each.
- **`fetch_note`** — Retrieve the full content of a single note by `name` or `id`.
  Devuelve el texto, nombre de archivo e identificador de la nota.

### Conectar un agente de IA

AI agents connect to the MCP server using the HTTP+SSE transport:

1. **SSE endpoint**: `GET http://localhost:22226/sse`  
   Opens a Server-Sent Events stream. The server sends an `endpoint` event containing
   the URL for sending JSON-RPC messages.
2. **Message endpoint**: `POST http://localhost:22226/message?sessionId=<id>`  
   Send MCP JSON-RPC requests here. The response is delivered over the SSE stream.

Todas las solicitudes deben incluir el encabezado:

```
Authorization: Bearer <your-security-token>
```

### Example `curl` Requests

Primero, establezca el token y puerto:

```bash
export TOKEN="su-token-mcp"
export PORT=22226
```

Open the SSE stream in one terminal:

```bash
curl -N \
  -H "Accept: text/event-stream" \
  -H "Authorization: Bearer $TOKEN" \
  "http://localhost:$PORT/sse"
```

The server will send an `endpoint` event containing a URL like:

```text
event: endpoint
data: http://localhost:22226/message?sessionId=3d8c7b0e-...
```

Use the `sessionId` from that event in the following requests. The `POST` request itself returns
`202 Accepted`; the JSON-RPC response is delivered over the SSE stream.

Initialize the MCP session:

```bash
curl \
  -X POST \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  --data '{
    "jsonrpc": "2.0",
    "id": 1,
    "method": "initialize",
    "params": {}
  }' \
  "http://localhost:$PORT/message?sessionId=3d8c7b0e-..."
```

List the available MCP tools:

```bash
curl \
  -X POST \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  --data '{
    "jsonrpc": "2.0",
    "id": 2,
    "method": "tools/list",
    "params": {}
  }' \
  "http://localhost:$PORT/message?sessionId=3d8c7b0e-..."
```

Search notes:

```bash
curl \
  -X POST \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  --data '{
    "jsonrpc": "2.0",
    "id": 3,
    "method": "tools/call",
    "params": {
      "name": "search_notes",
      "arguments": {
        "query": "meeting notes",
        "limit": 5
      }
    }
  }' \
  "http://localhost:$PORT/message?sessionId=3d8c7b0e-..."
```

Fetch a note by name:

```bash
curl \
  -X POST \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  --data '{
    "jsonrpc": "2.0",
    "id": 4,
    "method": "tools/call",
    "params": {
      "name": "fetch_note",
      "arguments": {
        "name": "Daily Journal"
      }
    }
  }' \
  "http://localhost:$PORT/message?sessionId=3d8c7b0e-..."
```

Fetch a note by ID:

```bash
curl \
  -X POST \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  --data '{
    "jsonrpc": "2.0",
    "id": 5,
    "method": "tools/call",
    "params": {
      "name": "fetch_note",
      "arguments": {
        "id": 123
      }
    }
  }' \
  "http://localhost:$PORT/message?sessionId=3d8c7b0e-..."
```
