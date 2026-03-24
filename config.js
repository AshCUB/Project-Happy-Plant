// =============================================
//  ESP32 Configuration — update IP if it changes
// =============================================
const ESP32_IP = "10.0.0.165";
const ESP32_URL = `http://${ESP32_IP}`;

async function sendToESP32(endpoint, params = {}) {
  const query = new URLSearchParams(params).toString();
  const url = `${ESP32_URL}${endpoint}${query ? "?" + query : ""}`;
  try {
    const response = await fetch(url, { method: "GET", mode: "no-cors" });
    console.log(`Sent to ESP32: ${url}`);
  } catch (err) {
    console.error(`Failed to reach ESP32: ${err}`);
  }
}