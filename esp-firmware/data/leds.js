const ledCount = 4;

for (let i = 0; i < ledCount; i++) {
  const checkbox = document.getElementById(`led${i}`);
  checkbox.addEventListener("change", () => {
    const value = checkbox.checked ? 1 : 0;
    sendMessage(`TogglePin|${i}|${value}`);
  });
}

onWebSocketEvent((event, data) => {
  if (event === "message" && data.type === "PinsState") {
    const pinStates = data.values[0];
    [...pinStates].forEach((val, i) => {
      const checkbox = document.getElementById(`led${i}`);
      checkbox.checked = val === "1";
    });
  }
});
