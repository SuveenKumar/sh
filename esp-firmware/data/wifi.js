const ssidSelect = document.getElementById("ssid");
const passwordInput = document.getElementById("password");
const statusDiv = document.getElementById("status");
const resetBtn = document.getElementById("resetBtn");

document.getElementById("scanBtn").addEventListener("click", () => {
  sendMessage("Scan");
});

document.getElementById("wifiForm").addEventListener("submit", (e) => {
  e.preventDefault();
  const ssid = ssidSelect.value;
  const pass = passwordInput.value;
  sendMessage(`Submit|${ssid}|${pass}`);
});

resetBtn.addEventListener("click", () => {
  if (confirm("Reset WiFi settings?")) {
    sendMessage("Reset");
  }
});

onWebSocketEvent((event, data) => {
  if (event === "message") {
    if (data.type === "Status") {
      const [connected, msg] = data.values;
      statusDiv.textContent = msg;
      resetBtn.disabled = connected !== "1";
    } else if (data.type === "ScanResults") {
      ssidSelect.innerHTML = "";
      data.values.forEach(ssid => {
        const opt = document.createElement("option");
        opt.value = opt.text = ssid;
        ssidSelect.appendChild(opt);
      });
    }
  }
});