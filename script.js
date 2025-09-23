// === SINRICPRO INTEGRATION ===
// Ganti dengan endpoint dan API Key SinricPro Anda
const SINRICPRO_API_URL = "https://api.sinric.pro/v1/devices/{DEVICE_ID}/actions";
const SINRICPRO_STATUS_URL = "https://api.sinric.pro/v1/devices/{DEVICE_ID}/state";
const SINRICPRO_API_KEY = "YOUR_API_KEY";

function sendSwitchToSinricPro(state) {
	fetch(SINRICPRO_API_URL, {
		method: "POST",
		headers: {
			"Content-Type": "application/json",
			"Authorization": SINRICPRO_API_KEY
		},
		body: JSON.stringify({
			action: "setPowerState",
			value: state ? "On" : "Off"
		})
	});
}

function sendDimmerToSinricPro(value) {
	fetch(SINRICPRO_API_URL, {
		method: "POST",
		headers: {
			"Content-Type": "application/json",
			"Authorization": SINRICPRO_API_KEY
		},
		body: JSON.stringify({
			action: "setBrightness",
			value: value
		})
	});
}

function getDeviceStatus() {
	fetch(SINRICPRO_STATUS_URL, {
		method: "GET",
		headers: {
			"Authorization": SINRICPRO_API_KEY
		}
	})
	.then(res => res.json())
	.then(data => {
		// Update tampilan web sesuai status
		const isOn = data.powerState === "On";
		const brightness = data.brightness;
		const sw = document.getElementById("switch1");
		const status = document.getElementById("status1");
		if (sw && status) {
			sw.checked = isOn;
			status.textContent = isOn ? "ON" : "OFF";
			status.style.color = isOn ? "#00e676" : "#f3f3f3";
			status.style.background = isOn ? "#23272f" : "#181c24";
		}
		const brightnessInput = document.getElementById("brightness");
		const brightnessValue = document.getElementById("brightnessValue");
		if (brightnessInput && brightnessValue) {
			brightnessInput.value = brightness;
			brightnessValue.textContent = brightness + "%";
		}
	});
}

// Polling status setiap 5 detik
setInterval(getDeviceStatus, 5000);

// Event handler untuk saklar dan dimmer
const sw1 = document.getElementById("switch1");
if (sw1) {
	sw1.addEventListener("change", function(e) {
		sendSwitchToSinricPro(e.target.checked);
	});
}
const brightnessInput = document.getElementById("brightness");
if (brightnessInput) {
	brightnessInput.addEventListener("input", function(e) {
		sendDimmerToSinricPro(e.target.value);
	});
}
// Navbar toggle interaktivitas
const navbarToggle = document.getElementById('navbarToggle');
const navbarMenu = document.querySelector('.navbar-menu');
navbarToggle.addEventListener('click', () => {
	navbarMenu.classList.toggle('active');
});

// SPA Navigation
const navLinks = document.querySelectorAll('.nav-link[data-section]');
const sections = document.querySelectorAll('.page-section');

function showSection(sectionId) {
	sections.forEach(sec => {
		sec.style.display = sec.id === sectionId ? 'block' : 'none';
	});
	navLinks.forEach(link => {
		if (link.getAttribute('data-section') === sectionId) {
			link.classList.add('active');
		} else {
			link.classList.remove('active');
		}
	});
}

navLinks.forEach(link => {
	link.addEventListener('click', function(e) {
		e.preventDefault();
		const sectionId = this.getAttribute('data-section');
		showSection(sectionId);
	});
});

// Tampilkan Home saat pertama kali
showSection('home');

// Simulasi interaktivitas
function sendToRobotDyn(type, value) {
	// Placeholder: komunikasi ke modul RobotDyn
	console.log(`Kirim ke RobotDyn: ${type} = ${value}`);
}

// Saklar
for (let i = 1; i <= 4; i++) {
	const sw = document.getElementById(`switch${i}`);
	const status = document.getElementById(`status${i}`);
	if (sw && status) {
		sw.addEventListener('change', (e) => {
			sendToRobotDyn(`saklar${i}`, e.target.checked);
			status.textContent = e.target.checked ? 'ON' : 'OFF';
			status.style.color = e.target.checked ? '#00e676' : '#f3f3f3';
			status.style.background = e.target.checked ? '#23272f' : '#181c24';
		});
		// Set initial status
		status.textContent = sw.checked ? 'ON' : 'OFF';
		status.style.color = sw.checked ? '#00e676' : '#f3f3f3';
		status.style.background = sw.checked ? '#23272f' : '#181c24';
	}
}

// Toggle
const toggleSwitch = document.getElementById('toggleSwitch');
if (toggleSwitch) {
	toggleSwitch.addEventListener('change', (e) => {
		sendToRobotDyn('toggle', e.target.checked);
	});
}

// Slider kecerahan
const brightness = document.getElementById('brightness');
const brightnessValue = document.getElementById('brightnessValue');
if (brightness && brightnessValue) {
	brightness.addEventListener('input', (e) => {
		brightnessValue.textContent = `${e.target.value}%`;
		sendToRobotDyn('brightness', e.target.value);
	});
}

// Sensor PIR (simulasi deteksi gerakan)
const pirIndicator = document.getElementById('pirIndicator');
let pirDetected = false;
function simulatePir() {
	pirDetected = !pirDetected;
	if (pirIndicator) {
		if (pirDetected) {
			pirIndicator.classList.remove('pir-off');
			pirIndicator.classList.add('pir-on');
			pirIndicator.innerHTML = '<i class="pir-icon"></i> Terdeteksi';
		} else {
			pirIndicator.classList.remove('pir-on');
			pirIndicator.classList.add('pir-off');
			pirIndicator.innerHTML = '<i class="pir-icon"></i> Tidak Terdeteksi';
		}
		sendToRobotDyn('pir', pirDetected);
	}
}
// Simulasi PIR: ubah status setiap 3 detik
setInterval(simulatePir, 3000);
