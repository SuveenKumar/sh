const ledCount = 4;

function changeDeviceName() {
  const name = document.getElementById("deviceNameInput").value.trim();
  if (name) {
    sendMessage(`ChangeName|${name}`);
  }
}

function createDevicePanel(deviceId, name) {
  const container = document.getElementById("device-panels");

  const panel = document.createElement("div");
  panel.className = "panel";
  panel.id = `panel-${deviceId}`;

  const title = document.createElement("h3");
  title.textContent = name || `Device ${deviceId}`;
  panel.appendChild(title);

  for (let i = 0; i < ledCount; i++) {
    const ledDiv = document.createElement("div");
    ledDiv.className = "led";

    // Editable label
    const label = document.createElement("label");
    label.contentEditable = true;
    label.textContent = `LED ${i + 1}`;
    label.className = "editable-label";
    label.setAttribute("data-default", `LED ${i + 1}`); // Optional fallback

    const switchLabel = document.createElement("label");
    switchLabel.className = "switch";

    const input = document.createElement("input");
    input.type = "checkbox";
    input.id = `led-${deviceId}-${i}`;
    input.addEventListener("change", () => {
      const value = input.checked ? 1 : 0;
      sendMessage(`TogglePin|${deviceId}|${i}|${value}`);
    });

    const slider = document.createElement("span");
    slider.className = "slider";

    switchLabel.appendChild(input);
    switchLabel.appendChild(slider);

    ledDiv.appendChild(label);
    ledDiv.appendChild(switchLabel);

    panel.appendChild(ledDiv);
  }

  container.appendChild(panel);
}


onWebSocketEvent((event, data) => {
  if (event === "message") {
    if (data.type === "PinsState") {
      const deviceId = data.values[0];
      const pinStates = data.values[1];

      // Check if panel for this device exists; if not, create it
      if (!document.getElementById(`panel-${deviceId}`)) {
        createDevicePanel(deviceId, `Device ${deviceId}`);
      }

      // Update LED states
      [...pinStates].forEach((val, i) => {
        const checkbox = document.getElementById(`led-${deviceId}-${i}`);
        if (checkbox) checkbox.checked = val === "1";
      });
    }
    if (data.type === "NameChanged") {
      alert("✅ Device Name changed successfully!");
    }
    
    if (data.type === "DeviceName") {
      const deviceName = data.values[0];
      document.getElementById("deviceNameInput").value = deviceName;
    }
  }
});
