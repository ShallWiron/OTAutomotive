#include "Control_WebServer.h"
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"

static WebServer server(WEB_SERVER_PORT);
static Car_Controller *g_car = nullptr;

// ==================== HTML / CSS / JS Giao diện Bluetooth RC Controller Chuẩn Mẫu ====================
static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Bluetooth RC & Arm Controller</title>
    <style>
        :root {
            --bg-metal: #181d24;
            --plate-bg: #232a35;
            --btn-metal-grad: radial-gradient(circle, #455263 0%, #1f2530 70%, #151921 100%);
            --btn-active-grad: radial-gradient(circle, #0288d1 0%, #01579b 70%, #002f6c 100%);
            --cyan-neon: #00e5ff;
            --orange-neon: #ff9100;
            --red-neon: #ff1744;
            --text-silver: #d1d8e0;
        }
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            -webkit-tap-highlight-color: transparent;
            user-select: none;
            -webkit-user-select: none;
            touch-action: none;
        }
        body {
            background: #0d1117;
            color: var(--text-silver);
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
            padding: 8px;
            overflow: hidden;
        }
        .rc-chassis {
            width: 100%;
            max-width: 780px;
            background: linear-gradient(180deg, #2a3342 0%, #171d26 100%);
            border: 3px solid #3b4657;
            border-radius: 16px;
            padding: 10px 14px 14px 14px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.8), inset 0 1px 0 rgba(255,255,255,0.15);
            display: flex;
            flex-direction: column;
            gap: 10px;
        }

        /* 1. TOP TOOLBAR */
        .top-toolbar {
            display: flex;
            align-items: center;
            justify-content: space-between;
            background: #141922;
            border: 2px solid #283344;
            border-radius: 10px;
            padding: 6px 10px;
            box-shadow: inset 0 2px 6px rgba(0,0,0,0.6);
        }
        .tool-btn {
            background: linear-gradient(180deg, #374354 0%, #202834 100%);
            border: 1px solid #4a5a70;
            color: #cfd8dc;
            border-radius: 8px;
            width: 48px;
            height: 44px;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            font-size: 0.95rem;
            cursor: pointer;
            box-shadow: 0 3px 6px rgba(0,0,0,0.4);
            position: relative;
        }
        .tool-btn .sub-tag {
            font-size: 0.55rem;
            font-weight: 800;
            color: var(--orange-neon);
            margin-top: 1px;
            letter-spacing: 0.5px;
        }
        .tool-btn:active { transform: scale(0.95); }
        .tool-btn.active-red {
            background: radial-gradient(circle, #ff5252 0%, #b71c1c 100%);
            border-color: #ff8a80;
            color: #fff;
            box-shadow: 0 0 12px rgba(255,23,68,0.7);
        }
        .tool-btn.active-blue {
            background: radial-gradient(circle, #40c4ff 0%, #0091ea 100%);
            border-color: #80d8ff;
            color: #fff;
            box-shadow: 0 0 12px rgba(0,229,255,0.7);
        }
        .tool-btn.active-orange {
            background: radial-gradient(circle, #ffa726 0%, #e65100 100%);
            border-color: #ffcc80;
            color: #fff;
            box-shadow: 0 0 12px rgba(255,145,0,0.7);
        }
        .status-badge-box {
            display: flex;
            flex-direction: column;
            align-items: center;
            padding: 0 6px;
        }
        .status-badge-title { font-size: 0.6rem; color: #78909c; font-weight: 700; text-transform: uppercase; }
        .status-badge-val { font-size: 0.85rem; font-weight: 800; color: var(--cyan-neon); letter-spacing: 0.5px; }

        /* 2. MAIN CONTROLLER BODY (3 BLOCKS: LEFT (F/B), CENTER (ROSETTE), RIGHT (L/R)) */
        .main-deck {
            display: grid;
            grid-template-columns: 1fr 1.3fr 1fr;
            gap: 12px;
            align-items: center;
        }

        /* METALLIC CONTROL BUTTON PANELS */
        .metal-panel {
            background: linear-gradient(135deg, #2c3645 0%, #1a212b 100%);
            border: 2px solid #3d4a5d;
            border-radius: 14px;
            padding: 10px;
            box-shadow: inset 0 2px 4px rgba(255,255,255,0.1), 0 6px 14px rgba(0,0,0,0.5);
            display: flex;
            justify-content: center;
            align-items: center;
        }
        .left-vertical {
            flex-direction: column;
            gap: 14px;
        }
        .right-horizontal {
            flex-direction: row;
            gap: 14px;
        }

        /* BIG ROUND CHROME BUTTONS */
        .circle-btn {
            width: 86px;
            height: 86px;
            border-radius: 50%;
            background: var(--btn-metal-grad);
            border: 4px solid #4f5f74;
            box-shadow: 0 6px 12px rgba(0,0,0,0.7), inset 0 3px 6px rgba(255,255,255,0.25);
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            transition: transform 0.06s, box-shadow 0.06s;
            position: relative;
        }
        .circle-btn .btn-arrow {
            font-size: 1.9rem;
            color: var(--cyan-neon);
            text-shadow: 0 0 10px rgba(0,229,255,0.8);
            line-height: 1;
            pointer-events: none;
        }
        .circle-btn .btn-letter {
            font-size: 0.85rem;
            font-weight: 900;
            color: var(--orange-neon);
            text-shadow: 0 0 6px rgba(255,145,0,0.6);
            margin-top: 2px;
            pointer-events: none;
        }
        .circle-btn:active, .circle-btn.pressed {
            background: var(--btn-active-grad);
            border-color: #00e5ff;
            transform: scale(0.93);
            box-shadow: 0 0 22px rgba(0,229,255,0.9), inset 0 2px 6px rgba(0,0,0,0.8);
        }
        .circle-btn:active .btn-arrow, .circle-btn.pressed .btn-arrow {
            color: #ffffff;
            text-shadow: 0 0 16px #ffffff;
        }

        /* CENTER ROSETTE (8-DIRECTION COMPASS) */
        .center-rosette-panel {
            background: #151a22;
            border: 2px solid #2d3848;
            border-radius: 14px;
            padding: 10px;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            box-shadow: inset 0 2px 8px rgba(0,0,0,0.7);
        }
        .rosette-grid {
            display: grid;
            grid-template-columns: 46px 46px 46px;
            grid-template-rows: 46px 46px 46px;
            gap: 6px;
            align-items: center;
            justify-content: center;
        }
        .dir-arrow {
            width: 46px;
            height: 46px;
            border-radius: 8px;
            background: #1f2733;
            border: 1px solid #323f52;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            font-size: 1.1rem;
            color: #4b5d73;
            transition: all 0.1s;
        }
        .dir-arrow .diag-label {
            font-size: 0.55rem;
            font-weight: 800;
            line-height: 1;
            margin-top: 1px;
        }
        .dir-arrow.lit {
            background: radial-gradient(circle, #00e5ff 0%, #01579b 100%);
            border-color: #80d8ff;
            color: #ffffff;
            box-shadow: 0 0 16px rgba(0,229,255,0.9);
            transform: scale(1.06);
        }
        .dir-arrow.lit-center {
            background: radial-gradient(circle, #ff5252 0%, #b71c1c 100%);
            border-color: #ff8a80;
            color: #ffffff;
            box-shadow: 0 0 14px rgba(255,23,68,0.8);
        }

        /* 3. FOOTER COMBINATION GUIDE */
        .footer-formulas {
            display: grid;
            grid-template-columns: 1fr 1fr 1fr 1fr;
            gap: 6px;
            background: #131820;
            border: 1px solid #263140;
            border-radius: 8px;
            padding: 6px 8px;
            text-align: center;
        }
        .formula-item {
            font-size: 0.72rem;
            font-weight: 800;
            color: #90a4ae;
        }
        .formula-item span.res {
            color: var(--orange-neon);
            font-size: 0.8rem;
        }

        /* RESPONSIVE FOR SMALL SCREENS */
        @media (max-width: 600px) {
            .circle-btn { width: 72px; height: 72px; }
            .circle-btn .btn-arrow { font-size: 1.6rem; }
            .rosette-grid { grid-template-columns: 38px 38px 38px; grid-template-rows: 38px 38px 38px; gap: 4px; }
            .dir-arrow { width: 38px; height: 38px; font-size: 0.95rem; }
            .tool-btn { width: 42px; height: 40px; font-size: 0.85rem; }
        }
    </style>
</head>
<body>
    <div class="rc-chassis">
        <!-- 1. TOP TOOLBAR -->
        <div class="top-toolbar">
            <!-- Emergency Stop -->
            <button class="tool-btn active-red" onclick="triggerEStop()" title="Dừng khẩn cấp">
                <span>🔴</span>
                <span class="sub-tag">STOP</span>
            </button>

            <!-- Gripper W - w -->
            <button id="btnGrip" class="tool-btn" onclick="toggleGripper()" title="Mở / Đóng Kẹp">
                <span>🚗</span>
                <span class="sub-tag">W - w</span>
            </button>

            <!-- Light U - u -->
            <button id="btnLight" class="tool-btn" onclick="toggleLight()" title="Bật / Tắt Đèn">
                <span>💡</span>
                <span class="sub-tag">U - u</span>
            </button>

            <!-- Horn / Buzzer V - v -->
            <button id="btnHorn" class="tool-btn" onmousedown="hornPress()" onmouseup="hornRelease()" ontouchstart="hornPress()" ontouchend="hornRelease()" title="Còi Bíp">
                <span>📢</span>
                <span class="sub-tag">V - v</span>
            </button>

            <!-- Mode X - x -->
            <button id="btnMode" class="tool-btn active-blue" onclick="toggleMode()" title="Chuyển chế độ Lái xe / Tay gắp">
                <span id="icoMode">🚗</span>
                <span class="sub-tag">X - x</span>
            </button>

            <!-- Settings / OTA Bootloader -->
            <button class="tool-btn" onclick="triggerOTA()" title="Nạp OTA Bootloader">
                <span>⚙️</span>
                <span class="sub-tag">OTA</span>
            </button>

            <!-- Status Indicator Box -->
            <div class="status-badge-box">
                <span id="lblModeStatus" class="status-badge-title">MODE: BÁNH XE</span>
                <span id="txtCmdStatus" class="status-badge-val">STOP [S]</span>
            </div>
        </div>

        <!-- 2. MAIN CONTROLLER DECK -->
        <div class="main-deck">
            <!-- LEFT PANEL: F (UP) & B (DOWN) -->
            <div class="metal-panel left-vertical">
                <button id="btnF" class="circle-btn" data-key="F">
                    <span class="btn-arrow">&#9650;</span>
                    <span id="txtF" class="btn-letter">F</span>
                </button>
                <button id="btnB" class="circle-btn" data-key="B">
                    <span class="btn-arrow">&#9660;</span>
                    <span id="txtB" class="btn-letter">B</span>
                </button>
            </div>

            <!-- CENTER ROSETTE: 8-DIRECTION COMPASS DISPLAY -->
            <div class="center-rosette-panel">
                <div class="rosette-grid">
                    <!-- Row 1: G (F+L), F, I (F+R) -->
                    <div id="dir_G" class="dir-arrow">&#9698;<span class="diag-label">G</span></div>
                    <div id="dir_F" class="dir-arrow">&#9650;<span class="diag-label">F</span></div>
                    <div id="dir_I" class="dir-arrow">&#9699;<span class="diag-label">I</span></div>

                    <!-- Row 2: L, S, R -->
                    <div id="dir_L" class="dir-arrow">&#9664;<span class="diag-label">L</span></div>
                    <div id="dir_S" class="dir-arrow lit-center">&#9632;<span class="diag-label">S</span></div>
                    <div id="dir_R" class="dir-arrow">&#9654;<span class="diag-label">R</span></div>

                    <!-- Row 3: H (B+L), B, J (B+R) -->
                    <div id="dir_H" class="dir-arrow">&#9700;<span class="diag-label">H</span></div>
                    <div id="dir_B" class="dir-arrow">&#9660;<span class="diag-label">B</span></div>
                    <div id="dir_J" class="dir-arrow">&#9701;<span class="diag-label">J</span></div>
                </div>
            </div>

            <!-- RIGHT PANEL: L (LEFT) & R (RIGHT) -->
            <div class="metal-panel right-horizontal">
                <button id="btnL" class="circle-btn" data-key="L">
                    <span class="btn-arrow">&#9664;</span>
                    <span id="txtL" class="btn-letter">L</span>
                </button>
                <button id="btnR" class="circle-btn" data-key="R">
                    <span class="btn-arrow">&#9654;</span>
                    <span id="txtR" class="btn-letter">R</span>
                </button>
            </div>
        </div>

        <!-- 3. COMBINATION FORMULA FOOTER (EXACT MATCH USER PHOTO) -->
        <div class="footer-formulas">
            <div class="formula-item">F + L = <span class="res">G</span> (Tiến Trái)</div>
            <div class="formula-item">F + R = <span class="res">I</span> (Tiến Phải)</div>
            <div class="formula-item">B + L = <span class="res">H</span> (Lùi Trái)</div>
            <div class="formula-item">B + R = <span class="res">J</span> (Lùi Phải)</div>
        </div>
    </div>

    <script>
        // State variables
        let currentMode = "MOTOR"; // "MOTOR" ('x') or "SERVO" ('X')
        let gripOpen = false;      // W (open) / w (close)
        let lightOn = false;       // u (on) / U (off)
        let hornActive = false;    // V (on) / v (off)

        // Movement keys currently pressed
        let f_pressed = false;
        let b_pressed = false;
        let l_pressed = false;
        let r_pressed = false;

        let lastSentCommand = "S";
        let heartbeatInterval = null;

        // Fast API Sender
        function sendApi(cmd) {
            fetch('/api/cmd?action=' + encodeURIComponent(cmd))
                .catch(e => console.error(e));
        }

        // Evaluate combined command from buttons pressed
        function evaluateCommand() {
            if (currentMode === "MOTOR") {
                if (f_pressed && l_pressed) return "G";
                if (f_pressed && r_pressed) return "I";
                if (b_pressed && l_pressed) return "H";
                if (b_pressed && r_pressed) return "J";
                if (f_pressed) return "F";
                if (b_pressed) return "B";
                if (l_pressed) return "L";
                if (r_pressed) return "R";
                return "S";
            } else {
                // SERVO MODE
                if (f_pressed) return "F"; // Arm Up
                if (b_pressed) return "B"; // Arm Down
                if (l_pressed) return "L"; // Arm Extend
                if (r_pressed) return "R"; // Arm Retract
                return "S"; // Arm Stop
            }
        }

        // Update Rosette and Send Command
        function updateMotion() {
            const cmd = evaluateCommand();
            highlightRosette(cmd);

            // If command changed or we need to send
            if (cmd !== lastSentCommand) {
                lastSentCommand = cmd;
                dispatchCommand(cmd);

                // Start or clear keepalive
                if (heartbeatInterval) clearInterval(heartbeatInterval);
                if (cmd !== "S") {
                    heartbeatInterval = setInterval(() => {
                        dispatchCommand(lastSentCommand);
                    }, 120);
                } else {
                    heartbeatInterval = null;
                }
            }
        }

        // Send evaluated command to ESP32 backend
        function dispatchCommand(cmd) {
            const desc = getCommandDesc(cmd);
            document.getElementById('txtCmdStatus').innerText = desc + " [" + cmd + "]";

            if (currentMode === "MOTOR") {
                switch(cmd) {
                    case "F": sendApi("forward"); break;
                    case "B": sendApi("backward"); break;
                    case "L": sendApi("spin_left"); break;
                    case "R": sendApi("spin_right"); break;
                    case "G": sendApi("left_fwd"); break;
                    case "I": sendApi("right_fwd"); break;
                    case "H": sendApi("left_rev"); break;
                    case "J": sendApi("right_rev"); break;
                    default:  sendApi("stop"); break;
                }
            } else {
                // SERVO MODE
                switch(cmd) {
                    case "F": sendApi("arm_up"); break;
                    case "B": sendApi("arm_down"); break;
                    case "L": sendApi("arm_extend"); break;
                    case "R": sendApi("arm_retract"); break;
                    default:  sendApi("arm_stop"); break;
                }
            }
        }

        function getCommandDesc(cmd) {
            if (currentMode === "MOTOR") {
                switch(cmd) {
                    case "F": return "TIẾN";
                    case "B": return "LÙI";
                    case "L": return "QUAY TRÁI";
                    case "R": return "QUAY PHẢI";
                    case "G": return "TIẾN TRÁI";
                    case "I": return "TIẾN PHẢI";
                    case "H": return "LÙI TRÁI";
                    case "J": return "LÙI PHẢI";
                    default:  return "DỪNG";
                }
            } else {
                switch(cmd) {
                    case "F": return "NÂNG TAY";
                    case "B": return "HẠ TAY";
                    case "L": return "DUỖI TAY";
                    case "R": return "THU TAY";
                    default:  return "DỪNG TAY";
                }
            }
        }

        // Highlight active direction arrow in center rosette
        function highlightRosette(cmd) {
            document.querySelectorAll('.dir-arrow').forEach(el => {
                el.classList.remove('lit');
                el.classList.remove('lit-center');
            });

            if (cmd === "S") {
                document.getElementById('dir_S').classList.add('lit-center');
            } else {
                const target = document.getElementById('dir_' + cmd);
                if (target) target.classList.add('lit');
            }
        }

        // Toggle Mode X - x
        function toggleMode() {
            if (currentMode === "MOTOR") {
                currentMode = "SERVO";
                sendApi("mode_servo"); // 'X'
                document.getElementById('lblModeStatus').innerText = "MODE: TAY GẮP";
                document.getElementById('icoMode').innerText = "🦾";
                document.getElementById('btnMode').className = "tool-btn active-orange";
                document.getElementById('txtF').innerText = "NÂNG";
                document.getElementById('txtB').innerText = "HẠ";
                document.getElementById('txtL').innerText = "DUỖI";
                document.getElementById('txtR').innerText = "THU";
            } else {
                currentMode = "MOTOR";
                sendApi("mode_motor"); // 'x'
                document.getElementById('lblModeStatus').innerText = "MODE: BÁNH XE";
                document.getElementById('icoMode').innerText = "🚗";
                document.getElementById('btnMode').className = "tool-btn active-blue";
                document.getElementById('txtF').innerText = "F";
                document.getElementById('txtB').innerText = "B";
                document.getElementById('txtL').innerText = "L";
                document.getElementById('txtR').innerText = "R";
            }
            // Reset motion state
            f_pressed = b_pressed = l_pressed = r_pressed = false;
            updateMotion();
        }

        // Toggle Gripper W - w
        function toggleGripper() {
            gripOpen = !gripOpen;
            sendApi(gripOpen ? "grip_open" : "grip_close");
            const btn = document.getElementById('btnGrip');
            btn.className = "tool-btn " + (gripOpen ? "active-blue" : "");
        }

        // Toggle Light U - u
        function toggleLight() {
            lightOn = !lightOn;
            sendApi(lightOn ? "led_on" : "led_off");
            const btn = document.getElementById('btnLight');
            btn.className = "tool-btn " + (lightOn ? "active-orange" : "");
        }

        // Horn / Buzzer V - v
        function hornPress() {
            sendApi("buzzer");
            document.getElementById('btnHorn').classList.add('active-orange');
        }
        function hornRelease() {
            document.getElementById('btnHorn').classList.remove('active-orange');
        }

        // Emergency Stop
        function triggerEStop() {
            f_pressed = b_pressed = l_pressed = r_pressed = false;
            document.querySelectorAll('.circle-btn').forEach(b => b.classList.remove('pressed'));
            sendApi("emergency_stop");
            lastSentCommand = "S";
            if (heartbeatInterval) clearInterval(heartbeatInterval);
            highlightRosette("S");
            document.getElementById('txtCmdStatus').innerText = "ESTOP [0x000]";
        }

        // OTA Bootloader Trigger
        function triggerOTA() {
            if (confirm("Chuyển STM32 sang Bootloader để chuẩn bị nạp OTA?")) {
                sendApi("bootloader");
            }
        }

        // MULTI-TOUCH BUTTON BINDING (SUPPORTS 2 FINGERS SIMULTANEOUSLY)
        function bindButton(btnId, setFlag) {
            const btn = document.getElementById(btnId);

            const onPress = (e) => {
                e.preventDefault();
                btn.classList.add('pressed');
                setFlag(true);
                updateMotion();
            };

            const onRelease = (e) => {
                e.preventDefault();
                btn.classList.remove('pressed');
                setFlag(false);
                updateMotion();
            };

            // Touch events (Mobile multi-touch)
            btn.addEventListener('touchstart', onPress, { passive: false });
            btn.addEventListener('touchend', onRelease, { passive: false });
            btn.addEventListener('touchcancel', onRelease, { passive: false });

            // Mouse events (PC testing)
            btn.addEventListener('mousedown', onPress);
            btn.addEventListener('mouseup', onRelease);
            btn.addEventListener('mouseleave', onRelease);
        }

        bindButton('btnF', val => { f_pressed = val; });
        bindButton('btnB', val => { b_pressed = val; });
        bindButton('btnL', val => { l_pressed = val; });
        bindButton('btnR', val => { r_pressed = val; });

        // Global mouseup release safety
        window.addEventListener('mouseup', () => {
            if (f_pressed || b_pressed || l_pressed || r_pressed) {
                f_pressed = b_pressed = l_pressed = r_pressed = false;
                document.querySelectorAll('.circle-btn').forEach(b => b.classList.remove('pressed'));
                updateMotion();
            }
        });

        // PC KEYBOARD (WASD & ARROW KEYS WITH 2-KEY SIMULTANEOUS COMBINATIONS)
        window.addEventListener('keydown', e => {
            if (e.repeat) return;
            const k = e.key.toLowerCase();
            let changed = false;

            if (k === 'w' || k === 'arrowup')    { f_pressed = true; document.getElementById('btnF').classList.add('pressed'); changed = true; }
            if (k === 's' || k === 'arrowdown')  { b_pressed = true; document.getElementById('btnB').classList.add('pressed'); changed = true; }
            if (k === 'a' || k === 'arrowleft')  { l_pressed = true; document.getElementById('btnL').classList.add('pressed'); changed = true; }
            if (k === 'd' || k === 'arrowright') { r_pressed = true; document.getElementById('btnR').classList.add('pressed'); changed = true; }

            if (k === ' ') { triggerEStop(); return; }
            if (k === 'x') { toggleMode(); return; }
            if (k === 'w' && e.ctrlKey) { toggleGripper(); return; }
            if (k === 'u') { toggleLight(); return; }
            if (k === 'v') { hornPress(); return; }

            if (changed) updateMotion();
        });

        window.addEventListener('keyup', e => {
            const k = e.key.toLowerCase();
            let changed = false;

            if (k === 'w' || k === 'arrowup')    { f_pressed = false; document.getElementById('btnF').classList.remove('pressed'); changed = true; }
            if (k === 's' || k === 'arrowdown')  { b_pressed = false; document.getElementById('btnB').classList.remove('pressed'); changed = true; }
            if (k === 'a' || k === 'arrowleft')  { l_pressed = false; document.getElementById('btnL').classList.remove('pressed'); changed = true; }
            if (k === 'd' || k === 'arrowright') { r_pressed = false; document.getElementById('btnR').classList.remove('pressed'); changed = true; }
            if (k === 'v') { hornRelease(); }

            if (changed) updateMotion();
        });
    </script>
</body>
</html>
)rawliteral";

// ==================== Khởi tạo Web Server ====================
void Control_WebServer_Init(Car_Controller &car) {
    g_car = &car;

    // 1. Khởi động WiFi SoftAP
    WiFi.mode(WIFI_AP);
    bool apOk = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
    if (apOk) {
        Serial.println("\n[WIFI] Da phat mang WiFi SoftAP thanh cong!");
        Serial.printf("       SSID : %s\n", WIFI_AP_SSID);
        Serial.printf("       PASS : %s\n", WIFI_AP_PASSWORD);
        Serial.printf("       IP   : %s\n", WiFi.softAPIP().toString().c_str());
    } else {
        Serial.println("[WIFI ERR] Khong the tao SoftAP!");
    }

    // 2. Định tuyến URL
    // Trang chủ giao diện Web
    server.on("/", HTTP_GET, []() {
        server.send_P(200, "text/html", INDEX_HTML);
    });

    // API thực thi lệnh: /api/cmd?action=forward
    server.on("/api/cmd", HTTP_GET, []() {
        if (!g_car) {
            server.send(500, "application/json", "{\"ok\":false,\"err\":\"Car controller not ready\"}");
            return;
        }

        if (server.hasArg("action")) {
            String act = server.arg("action");
            bool ok = g_car->executeCommand(act);
            String json = "{\"ok\":" + String(ok ? "true" : "false") + ",\"action\":\"" + g_car->getLastAction() + "\"}";
            server.send(200, "application/json", json);
        } else {
            server.send(400, "application/json", "{\"ok\":false,\"err\":\"Missing action\"}");
        }
    });

    // API lấy trạng thái JSON: /api/status
    server.on("/api/status", HTTP_GET, []() {
        if (!g_car) {
            server.send(500, "application/json", "{}");
            return;
        }
        server.send(200, "application/json", g_car->getStatusJson());
    });

    // Khởi động server HTTP
    server.begin();
    Serial.printf("[WEB] Web Server da khoi dong tren cong %d (http://%s)\n\n",
                  WEB_SERVER_PORT, WiFi.softAPIP().toString().c_str());
}

void Control_WebServer_Loop() {
    server.handleClient();
}
