let ws;
const statusDiv = document.getElementById("status");
const ssidSelect = document.getElementById("ssid");

function connectWebSocket() {
  ws = new WebSocket("ws://192.168.4.1/ws");

  ws.onopen = () => {
    console.log("✅ WebSocket connected");
    ws.send("status");
    sendScan();
  };

  ws.onmessage = (event) => {
    const msg = event.data;

    if (msg.startsWith("status:")) {
      const statusText = msg.substring(7);
      statusDiv.textContent = statusText;
      document.getElementById("resetBtn").disabled = !statusText.includes(".");
    } else if (msg.startsWith("scan:")) {
      const list = JSON.parse(msg.substring(5));
      ssidSelect.innerHTML = "Scanning...";
      list.forEach(ssid => {
        const opt = document.createElement("option");
        opt.text = ssid;
        opt.value = ssid;
        ssidSelect.appendChild(opt);
      });
    }
  };

  ws.onclose = () => {
    console.warn("❌ WebSocket disconnected. Retrying...");
    setTimeout(connectWebSocket, 3000);
  };
}

function sendScan() {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send("scan");
  }
}

function sendSubmit() {
  const ssid = ssidSelect.value;
  const password = document.getElementById("password").value;
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(`submit:${ssid},${password}`);
  }
}

function resetCredentials() {
  if (confirm("Reset WiFi settings?")) {
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send("reset");
    } else {
      statusDiv.textContent = "⚠️ WiFi not connected";
    }
  }
}

document.getElementById("wifiForm").addEventListener("submit", function (e) {
  e.preventDefault();
  sendSubmit();
});

connectWebSocket();
