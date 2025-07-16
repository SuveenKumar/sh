let ws;
const eventListeners = [];

function initWebSocket() {
  ws = new WebSocket(`ws://${location.hostname}/ws`);

  ws.onopen = () => {
    console.log("✅ WebSocket connected");
    broadcast("open");
  };

  ws.onmessage = (event) => {
    const parts = event.data.split('|');
    const type = parts[0];
    const values = parts.slice(1);
    broadcast("message", { type, values });
  };

  ws.onclose = () => {
    console.warn("❌ WebSocket disconnected. Reconnecting...");
    setTimeout(initWebSocket, 3000);
  };
}

function sendMessage(msg) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(msg);
  } else {
    console.warn("⚠️ WebSocket not ready");
  }
}

function broadcast(event, data) {
  eventListeners.forEach(cb => cb(event, data));
}

function onWebSocketEvent(callback) {
  eventListeners.push(callback);
}

initWebSocket();
