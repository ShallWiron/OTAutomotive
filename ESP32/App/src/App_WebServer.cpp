#include "App_WebServer.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <SD.h>
#include <Preferences.h>
#include <vector>
#include "config.h"

#define WS_SERVER_PORT 81
static WebServer server(WEB_SERVER_PORT);
static WebSocketsServer wsServer(WS_SERVER_PORT);
static Car_Controller *g_car = nullptr;
static OTA_Protocol *g_ota = nullptr;
static Preferences g_prefs;
static File g_uploadFile;

static String g_alertMessage = "";
static String g_alertType = ""; // "success", "error", "info"
static bool g_isFlashing = false;
static String g_appSourceInfo = "Gateway NVS (Bấm 'Truy vấn' để đồng bộ qua CAN)";

// ==================== HTML / CSS / JS Giao diện Điều khiển Xe RC ====================
static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Robot Controller - CAN Opcode</title>
    <style>
        :root {
            --bg-metal: #181d24;
            --plate-bg: #232a35;
            --plate-border: #323b49;
            --orange-neon: #ff9100;
            --orange-glow: rgba(255, 145, 0, 0.45);
            --blue-neon: #00e5ff;
            --blue-glow: rgba(0, 229, 255, 0.45);
            --red-estop: #ff1744;
            --btn-metal: #2e3644;
            --btn-metal-active: #1a202a;
            --font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
        }

        * {
            box-sizing: border-box;
            user-select: none;
            -webkit-user-select: none;
            margin: 0;
            padding: 0;
            touch-action: manipulation;
        }

        body {
            background-color: var(--bg-metal);
            font-family: var(--font-family);
            color: #ffffff;
            display: flex;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
            padding: 12px;
        }

        .rc-chassis {
            background: linear-gradient(145deg, #202631, #161b22);
            border: 2px solid var(--plate-border);
            border-radius: 18px;
            box-shadow: 0 12px 36px rgba(0,0,0,0.65), inset 0 1px 1px rgba(255,255,255,0.1);
            width: 100%;
            max-width: 760px;
            padding: 16px;
            display: flex;
            flex-direction: column;
            gap: 16px;
        }

        .top-toolbar {
            display: flex;
            align-items: center;
            justify-content: space-between;
            background: var(--plate-bg);
            border: 1px solid var(--plate-border);
            border-radius: 12px;
            padding: 10px 14px;
            gap: 8px;
            flex-wrap: wrap;
        }

        .tool-btn {
            background: var(--btn-metal);
            border: 1px solid #455266;
            color: #e0e6ed;
            padding: 8px 12px;
            border-radius: 8px;
            font-size: 0.9rem;
            font-weight: 700;
            cursor: pointer;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            gap: 2px;
            min-width: 58px;
            transition: all 0.12s ease;
            box-shadow: 0 3px 6px rgba(0,0,0,0.3);
            touch-action: none;
            -webkit-touch-callout: none;
            -webkit-user-select: none;
            user-select: none;
        }
        .tool-btn:active {
            transform: translateY(2px);
            box-shadow: none;
        }
        .tool-btn.active-red {
            border-color: var(--red-estop);
            color: #ff8a80;
            box-shadow: 0 0 10px rgba(255, 23, 68, 0.4);
        }
        .tool-btn.active-blue {
            border-color: var(--blue-neon);
            color: var(--blue-neon);
            box-shadow: 0 0 10px var(--blue-glow);
        }
        .tool-btn.active-orange {
            border-color: var(--orange-neon);
            color: var(--orange-neon);
            box-shadow: 0 0 10px var(--orange-glow);
        }
        .tool-btn .sub-tag {
            font-size: 0.65rem;
            color: #8fa0b5;
            text-transform: uppercase;
        }

        .status-badge-box {
            display: flex;
            flex-direction: column;
            align-items: flex-end;
            margin-left: auto;
        }
        .status-badge-title {
            font-size: 0.72rem;
            font-weight: 700;
            color: #90a4ae;
            letter-spacing: 0.5px;
        }
        .status-badge-val {
            font-size: 0.95rem;
            font-weight: 800;
            color: var(--orange-neon);
            text-shadow: 0 0 8px var(--orange-glow);
        }

        .main-deck {
            display: flex;
            align-items: center;
            justify-content: space-around;
            gap: 14px;
            padding: 10px 0;
        }

        .metal-panel {
            background: var(--plate-bg);
            border: 1px solid var(--plate-border);
            border-radius: 16px;
            padding: 16px;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 16px;
            box-shadow: inset 0 2px 4px rgba(0,0,0,0.4);
        }
        .left-vertical {
            flex-direction: column;
        }
        .right-horizontal {
            flex-direction: row;
        }

        .circle-btn {
            width: 84px;
            height: 84px;
            border-radius: 50%;
            background: linear-gradient(135deg, #374253, #222a36);
            border: 2px solid #52637a;
            color: #ffffff;
            font-weight: 800;
            font-size: 1.15rem;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            box-shadow: 0 6px 14px rgba(0,0,0,0.5), inset 0 1px 2px rgba(255,255,255,0.25);
            transition: all 0.08s ease;
            position: relative;
            touch-action: none;
            -webkit-touch-callout: none;
            -webkit-user-select: none;
            user-select: none;
        }
        .circle-btn:active, .circle-btn.pressed {
            background: linear-gradient(135deg, #1f2733, #151a22);
            border-color: var(--blue-neon);
            box-shadow: 0 0 16px var(--blue-glow), inset 0 2px 6px rgba(0,0,0,0.6);
            transform: scale(0.96);
        }
        .circle-btn .btn-arrow {
            font-size: 2rem;
            line-height: 1;
            color: #00e5ff;
        }
        .circle-btn .btn-letter {
            font-size: 0.72rem;
            color: #b0bec5;
            letter-spacing: 0.5px;
            margin-top: 2px;
        }

        .center-rosette-panel {
            background: #191f28;
            border: 2px solid #2e3846;
            border-radius: 14px;
            padding: 12px;
            display: flex;
            align-items: center;
            justify-content: center;
            box-shadow: inset 0 3px 8px rgba(0,0,0,0.6);
        }

        .rosette-grid {
            display: grid;
            grid-template-columns: 50px 50px 50px;
            grid-template-rows: 50px 50px 50px;
            gap: 7px;
        }

        .dir-arrow {
            width: 50px;
            height: 50px;
            background: #252e3c;
            border: 1px solid #3d4a5d;
            border-radius: 9px;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            font-size: 1.15rem;
            color: #8fa0b5;
            transition: all 0.08s ease;
            cursor: pointer;
            touch-action: none;
            user-select: none;
            -webkit-user-select: none;
            box-shadow: 0 2px 5px rgba(0,0,0,0.3);
        }
        .dir-arrow:active, .dir-arrow.pressed {
            background: #ff9100;
            color: #12161c;
            border-color: #ffab40;
            box-shadow: 0 0 14px var(--orange-glow);
            transform: scale(0.92);
        }
        .dir-arrow.lit {
            background: #ff9100;
            color: #12161c;
            border-color: #ffab40;
            box-shadow: 0 0 14px var(--orange-glow);
            font-weight: 800;
        }
        .dir-arrow.lit-center {
            background: #37474f;
            color: #00e5ff;
            border-color: var(--blue-neon);
        }
        .diag-label {
            font-size: 0.58rem;
            font-weight: 700;
            margin-top: 1px;
        }

        .footer-formulas {
            background: var(--plate-bg);
            border: 1px solid var(--plate-border);
            border-radius: 10px;
            padding: 8px 14px;
            display: flex;
            align-items: center;
            justify-content: space-around;
            flex-wrap: wrap;
            gap: 8px;
        }
        .formula-item {
            font-size: 0.74rem;
            font-weight: 700;
            color: #90a4ae;
        }
        .formula-item span.res {
            color: var(--orange-neon);
            font-size: 0.8rem;
        }

        @media (max-width: 600px) {
            .circle-btn { width: 72px; height: 72px; }
            .circle-btn .btn-arrow { font-size: 1.6rem; }
            .rosette-grid { grid-template-columns: 38px 38px 38px; grid-template-rows: 38px 38px 38px; gap: 4px; }
            .dir-arrow { width: 38px; height: 38px; font-size: 0.95rem; }
            .tool-btn { min-width: 50px; padding: 6px 8px; font-size: 0.8rem; }
        }
    </style>
</head>
<body>
    <div class="rc-chassis">
        <!-- 1. TOP TOOLBAR -->
        <div class="top-toolbar">
            <!-- Emergency Stop -->
            <button class="tool-btn active-red" onclick="triggerEStop()" title="Dừng khẩn cấp toàn bộ xe">
                <span>🔴</span>
                <span class="sub-tag">E-STOP</span>
            </button>

            <!-- Mode Switch -->
            <button id="btnMode" class="tool-btn active-blue" onclick="toggleMode()" title="Chuyển chế độ Bánh xe / Tay gắp">
                <span id="icoMode">🚗</span>
                <span id="lblModeTag" class="sub-tag">BÁNH XE</span>
            </button>

            <!-- Gripper Switch -->
            <button id="btnGrip" class="tool-btn" onclick="toggleGripper()" title="Mở / Đóng Kẹp">
                <span>🦾</span>
                <span id="lblGripTag" class="sub-tag">KẸP ĐÓNG</span>
            </button>

            <!-- Settings / OTA Bootloader -->
            <button class="tool-btn" onclick="triggerOTA()" title="Kích hoạt Bootloader nạp OTA">
                <span>⚙️</span>
                <span class="sub-tag">OTA</span>
            </button>

            <!-- Status Indicator Box -->
            <div class="status-badge-box">
                <span id="lblModeStatus" class="status-badge-title">MODE: BÁNH XE</span>
                <span id="txtCmdStatus" class="status-badge-val">DỪNG [0x040]</span>
                <span id="lblWsStatus" style="font-size:0.65rem; font-weight:800; color:#ff1744; letter-spacing:0.5px;">WS: ĐANG KẾT NỐI...</span>
            </div>
        </div>

        <!-- 2. MAIN CONTROLLER DECK -->
        <div class="main-deck">
            <!-- LEFT PANEL: TIẾN & LÙI -->
            <div class="metal-panel left-vertical">
                <button id="btnF" class="circle-btn" data-key="F">
                    <span class="btn-arrow">&#9650;</span>
                    <span id="txtF" class="btn-letter">TIẾN</span>
                </button>
                <button id="btnB" class="circle-btn" data-key="B">
                    <span class="btn-arrow">&#9660;</span>
                    <span id="txtB" class="btn-letter">LÙI</span>
                </button>
            </div>

            <!-- CENTER ROSETTE: 8-DIRECTION COMPASS DISPLAY -->
            <div class="center-rosette-panel">
                <div class="rosette-grid">
                    <!-- Row 1: G (F+L), F, I (F+R) -->
                    <button type="button" id="dir_G" class="dir-arrow" title="Tiến Trái">&#9698;<span class="diag-label">L.FWD</span></button>
                    <button type="button" id="dir_F" class="dir-arrow" title="Tiến">&#9650;<span class="diag-label">FWD</span></button>
                    <button type="button" id="dir_I" class="dir-arrow" title="Tiến Phải">&#9699;<span class="diag-label">R.FWD</span></button>

                    <!-- Row 2: L, S, R -->
                    <button type="button" id="dir_L" class="dir-arrow" title="Quay Trái">&#9664;<span class="diag-label">LEFT</span></button>
                    <button type="button" id="dir_S" class="dir-arrow lit-center" title="Dừng Xe">&#9632;<span class="diag-label">STOP</span></button>
                    <button type="button" id="dir_R" class="dir-arrow" title="Quay Phải">&#9654;<span class="diag-label">RIGHT</span></button>

                    <!-- Row 3: H (B+L), B, J (B+R) -->
                    <button type="button" id="dir_H" class="dir-arrow" title="Lùi Trái">&#9700;<span class="diag-label">L.REV</span></button>
                    <button type="button" id="dir_B" class="dir-arrow" title="Lùi">&#9660;<span class="diag-label">REV</span></button>
                    <button type="button" id="dir_J" class="dir-arrow" title="Lùi Phải">&#9701;<span class="diag-label">R.REV</span></button>
                </div>
            </div>

            <!-- RIGHT PANEL: TRÁI & PHẢI -->
            <div class="metal-panel right-horizontal">
                <button id="btnL" class="circle-btn" data-key="L">
                    <span class="btn-arrow">&#9664;</span>
                    <span id="txtL" class="btn-letter">TRÁI</span>
                </button>
                <button id="btnR" class="circle-btn" data-key="R">
                    <span class="btn-arrow">&#9654;</span>
                    <span id="txtR" class="btn-letter">PHẢI</span>
                </button>
            </div>
        </div>

        <!-- 3. FOOTER INFO -->
        <div class="footer-formulas">
            <div class="formula-item">Giao thức: <span class="res">CAN 11-bit ID</span></div>
            <div class="formula-item">Bánh xe: <span class="res">0x040 - 0x048</span></div>
            <div class="formula-item">Tay gắp: <span class="res">0x080 - 0x086</span></div>
            <div class="formula-item"><a href="/ota" style="color:var(--blue-neon); text-decoration:none; font-weight:bold;">⚡ TRANG NẠP OTA &rarr;</a></div>
        </div>
    </div>

    <script>
        // Bảng tra mã lệnh CAN ID chuẩn hóa (Opcodes)
        const CMD = {
            ESTOP:        "0x000",
            MODE_MOTOR:   "0x001",
            MODE_SERVO:   "0x002",
            MOTOR_STOP:   "0x040",
            MOTOR_FWD:    "0x041",
            MOTOR_REV:    "0x042",
            MOTOR_LEFT:   "0x043",
            MOTOR_RIGHT:  "0x044",
            MOTOR_L_FWD:  "0x045",
            MOTOR_L_REV:  "0x046",
            MOTOR_R_FWD:  "0x047",
            MOTOR_R_REV:  "0x048",
            SERVO_STOP:   "0x080",
            SERVO_UP:     "0x081",
            SERVO_DOWN:   "0x082",
            SERVO_EXT:    "0x083",
            SERVO_RET:    "0x084",
            GRIP_OPEN:    "0x085",
            GRIP_CLOSE:   "0x086"
        };

        let currentMode = "MOTOR";
        let gripOpen = false;

        let f_pressed = false;
        let b_pressed = false;
        let l_pressed = false;
        let r_pressed = false;

        let activeDirectCmd = null;
        let lastSentCode = CMD.MOTOR_STOP;
        let heartbeatInterval = null;
        let cmdSeq = 0;
        let isSending = false;
        let pendingStop = false;

        let ws = null;
        let wsConnected = false;

        function connectWebSocket() {
            const proto = (window.location.protocol === 'https:') ? 'wss:' : 'ws:';
            const wsUrl = `${proto}//${window.location.hostname}:81/`;

            ws = new WebSocket(wsUrl);

            ws.onopen = () => {
                wsConnected = true;
                console.log("[WS] Da ket noi WebSocket port 81");
                const b = document.getElementById('lblWsStatus');
                if (b) { b.innerText = "WS: LIVE (1-2ms)"; b.style.color = "#00e5ff"; }
            };

            ws.onclose = () => {
                wsConnected = false;
                console.warn("[WS] Mat ket noi WebSocket, dang ket noi lai...");
                const b = document.getElementById('lblWsStatus');
                if (b) { b.innerText = "WS: RECONNECTING..."; b.style.color = "#ff1744"; }
                setTimeout(connectWebSocket, 1000);
            };

            ws.onerror = (e) => {
                console.error("[WS] Loi:", e);
                ws.close();
            };
        }

        function sendCmd(code) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(code);
            } else {
                fetch('/api/cmd?code=' + encodeURIComponent(code), { cache: 'no-store', keepalive: true }).catch(() => {});
            }
        }

        function evaluateCommandCode() {
            if (activeDirectCmd) {
                return activeDirectCmd;
            }

            if (currentMode === "MOTOR") {
                if (f_pressed && l_pressed) return CMD.MOTOR_L_FWD;
                if (f_pressed && r_pressed) return CMD.MOTOR_R_FWD;
                if (b_pressed && l_pressed) return CMD.MOTOR_L_REV;
                if (b_pressed && r_pressed) return CMD.MOTOR_R_REV;
                if (f_pressed) return CMD.MOTOR_FWD;
                if (b_pressed) return CMD.MOTOR_REV;
                if (l_pressed) return CMD.MOTOR_LEFT;
                if (r_pressed) return CMD.MOTOR_RIGHT;
                return CMD.MOTOR_STOP;
            } else {
                if (f_pressed) return CMD.SERVO_UP;
                if (b_pressed) return CMD.SERVO_DOWN;
                if (l_pressed) return CMD.SERVO_EXT;
                if (r_pressed) return CMD.SERVO_RET;
                return CMD.SERVO_STOP;
            }
        }

        function updateMotion() {
            const code = evaluateCommandCode();
            highlightRosette(code);

            const isStop = (code === CMD.MOTOR_STOP || code === CMD.SERVO_STOP);

            if (code !== lastSentCode) {
                lastSentCode = code;
                dispatchCode(code);

                if (heartbeatInterval) {
                    clearInterval(heartbeatInterval);
                    heartbeatInterval = null;
                }

                if (!isStop) {
                    // WebSocket heartbeat 100ms: sieu nhe, truyen thong 1-2ms khong bao gio lag
                    heartbeatInterval = setInterval(() => {
                        sendCmd(lastSentCode);
                    }, 100);
                }
            }
        }

        function dispatchCode(code) {
            const desc = getCommandDesc(code);
            document.getElementById('txtCmdStatus').innerText = desc + " [" + code + "]";
            sendCmd(code);
        }

        function getCommandDesc(code) {
            switch(code) {
                case CMD.MOTOR_FWD:    return "TIẾN";
                case CMD.MOTOR_REV:    return "LÙI";
                case CMD.MOTOR_LEFT:   return "QUAY TRÁI";
                case CMD.MOTOR_RIGHT:  return "QUAY PHẢI";
                case CMD.MOTOR_L_FWD:  return "TIẾN TRÁI";
                case CMD.MOTOR_R_FWD:  return "TIẾN PHẢI";
                case CMD.MOTOR_L_REV:  return "LÙI TRÁI";
                case CMD.MOTOR_R_REV:  return "LÙI PHẢI";
                case CMD.MOTOR_STOP:   return "DỪNG XE";

                case CMD.SERVO_UP:     return "NÂNG TAY";
                case CMD.SERVO_DOWN:   return "HẠ TAY";
                case CMD.SERVO_EXT:    return "VƯƠN TAY";
                case CMD.SERVO_RET:    return "THU TAY";
                case CMD.SERVO_STOP:   return "DỪNG TAY";
                case CMD.GRIP_OPEN:    return "MỞ KẸP";
                case CMD.GRIP_CLOSE:   return "ĐÓNG KẸP";
                case CMD.ESTOP:        return "E-STOP";
                default:               return "LỆNH " + code;
            }
        }

        function highlightRosette(code) {
            document.querySelectorAll('.dir-arrow').forEach(el => {
                el.classList.remove('lit');
                el.classList.remove('lit-center');
            });

            if (code === CMD.MOTOR_STOP || code === CMD.SERVO_STOP) {
                document.getElementById('dir_S').classList.add('lit-center');
            } else {
                let id = null;
                if (code === CMD.MOTOR_FWD || code === CMD.SERVO_UP) id = 'dir_F';
                else if (code === CMD.MOTOR_REV || code === CMD.SERVO_DOWN) id = 'dir_B';
                else if (code === CMD.MOTOR_LEFT || code === CMD.SERVO_EXT) id = 'dir_L';
                else if (code === CMD.MOTOR_RIGHT || code === CMD.SERVO_RET) id = 'dir_R';
                else if (code === CMD.MOTOR_L_FWD) id = 'dir_G';
                else if (code === CMD.MOTOR_R_FWD) id = 'dir_I';
                else if (code === CMD.MOTOR_L_REV) id = 'dir_H';
                else if (code === CMD.MOTOR_R_REV) id = 'dir_J';
                if (id) {
                    const target = document.getElementById(id);
                    if (target) target.classList.add('lit');
                }
            }
        }

        function toggleMode() {
            if (currentMode === "MOTOR") {
                currentMode = "SERVO";
                sendCmd(CMD.MODE_SERVO);
                document.getElementById('lblModeStatus').innerText = "MODE: TAY GẮP";
                document.getElementById('icoMode').innerText = "🦾";
                document.getElementById('lblModeTag').innerText = "TAY GẮP";
                document.getElementById('btnMode').className = "tool-btn active-orange";
                document.getElementById('txtF').innerText = "NÂNG";
                document.getElementById('txtB').innerText = "HẠ";
                document.getElementById('txtL').innerText = "VƯƠN";
                document.getElementById('txtR').innerText = "THU";
            } else {
                currentMode = "MOTOR";
                sendCmd(CMD.MODE_MOTOR);
                document.getElementById('lblModeStatus').innerText = "MODE: BÁNH XE";
                document.getElementById('icoMode').innerText = "🚗";
                document.getElementById('lblModeTag').innerText = "BÁNH XE";
                document.getElementById('btnMode').className = "tool-btn active-blue";
                document.getElementById('txtF').innerText = "TIẾN";
                document.getElementById('txtB').innerText = "LÙI";
                document.getElementById('txtL').innerText = "TRÁI";
                document.getElementById('txtR').innerText = "PHẢI";
            }
            f_pressed = b_pressed = l_pressed = r_pressed = false;
            updateMotion();
        }

        function toggleGripper() {
            gripOpen = !gripOpen;
            sendCmd(gripOpen ? CMD.GRIP_OPEN : CMD.GRIP_CLOSE);
            const btn = document.getElementById('btnGrip');
            btn.className = "tool-btn " + (gripOpen ? "active-orange" : "");
            document.getElementById('lblGripTag').innerText = gripOpen ? "KẸP MỞ" : "KẸP ĐÓNG";
        }

        function triggerEStop() {
            f_pressed = b_pressed = l_pressed = r_pressed = false;
            document.querySelectorAll('.circle-btn').forEach(b => b.classList.remove('pressed'));
            if (heartbeatInterval) {
                clearInterval(heartbeatInterval);
                heartbeatInterval = null;
            }
            lastSentCode = CMD.ESTOP;
            sendCmd(CMD.ESTOP);
            highlightRosette(CMD.ESTOP);
            document.getElementById('txtCmdStatus').innerText = "E-STOP [0x000]";
        }

        function triggerOTA() {
            if (confirm("Kích hoạt Bootloader STM32 (CAN 0x600) và mở trang Nạp OTA?")) {
                sendCmd("0x600");
                setTimeout(() => {
                    window.location.href = "/ota";
                }, 300);
            }
        }

        function bindButton(btnId, setFlag) {
            const btn = document.getElementById(btnId);
            if (!btn) return;

            const onPress = (e) => {
                e.preventDefault();
                if (e.pointerType === 'mouse' && e.button !== 0) return;
                btn.classList.add('pressed');
                activeDirectCmd = null;
                setFlag(true);
                updateMotion();
            };

            const onRelease = (e) => {
                e.preventDefault();
                btn.classList.remove('pressed');
                setFlag(false);
                updateMotion();
            };

            if (window.PointerEvent) {
                btn.addEventListener('pointerdown', onPress);
                btn.addEventListener('pointerup', onRelease);
                btn.addEventListener('pointercancel', onRelease);
                btn.addEventListener('pointerleave', (e) => {
                    if (btn.classList.contains('pressed')) onRelease(e);
                });
            } else {
                btn.addEventListener('touchstart', onPress, { passive: false });
                btn.addEventListener('touchend', onRelease, { passive: false });
                btn.addEventListener('touchcancel', onRelease, { passive: false });
                btn.addEventListener('mousedown', onPress);
                btn.addEventListener('mouseup', onRelease);
                btn.addEventListener('mouseleave', (e) => {
                    if (btn.classList.contains('pressed')) onRelease(e);
                });
            }
        }

        function bindRosetteButton(btnId, getCmdFunc) {
            const btn = document.getElementById(btnId);
            if (!btn) return;

            const onPress = (e) => {
                e.preventDefault();
                if (e.pointerType === 'mouse' && e.button !== 0) return;
                btn.classList.add('pressed');
                activeDirectCmd = getCmdFunc();
                updateMotion();
            };

            const onRelease = (e) => {
                e.preventDefault();
                btn.classList.remove('pressed');
                if (activeDirectCmd === getCmdFunc()) {
                    activeDirectCmd = null;
                }
                updateMotion();
            };

            if (window.PointerEvent) {
                btn.addEventListener('pointerdown', onPress);
                btn.addEventListener('pointerup', onRelease);
                btn.addEventListener('pointercancel', onRelease);
                btn.addEventListener('pointerleave', (e) => {
                    if (btn.classList.contains('pressed')) onRelease(e);
                });
            } else {
                btn.addEventListener('touchstart', onPress, { passive: false });
                btn.addEventListener('touchend', onRelease, { passive: false });
                btn.addEventListener('touchcancel', onRelease, { passive: false });
                btn.addEventListener('mousedown', onPress);
                btn.addEventListener('mouseup', onRelease);
                btn.addEventListener('mouseleave', (e) => {
                    if (btn.classList.contains('pressed')) onRelease(e);
                });
            }
        }

        bindButton('btnF', val => f_pressed = val);
        bindButton('btnB', val => b_pressed = val);
        bindButton('btnL', val => l_pressed = val);
        bindButton('btnR', val => r_pressed = val);

        bindRosetteButton('dir_F', () => (currentMode === "MOTOR" ? CMD.MOTOR_FWD : CMD.SERVO_UP));
        bindRosetteButton('dir_B', () => (currentMode === "MOTOR" ? CMD.MOTOR_REV : CMD.SERVO_DOWN));
        bindRosetteButton('dir_L', () => (currentMode === "MOTOR" ? CMD.MOTOR_LEFT : CMD.SERVO_EXT));
        bindRosetteButton('dir_R', () => (currentMode === "MOTOR" ? CMD.MOTOR_RIGHT : CMD.SERVO_RET));
        bindRosetteButton('dir_S', () => (currentMode === "MOTOR" ? CMD.MOTOR_STOP : CMD.SERVO_STOP));

        bindRosetteButton('dir_G', () => CMD.MOTOR_L_FWD);
        bindRosetteButton('dir_I', () => CMD.MOTOR_R_FWD);
        bindRosetteButton('dir_H', () => CMD.MOTOR_L_REV);
        bindRosetteButton('dir_J', () => CMD.MOTOR_R_REV);

        function resetAllMotion() {
            if (f_pressed || b_pressed || l_pressed || r_pressed || activeDirectCmd) {
                f_pressed = b_pressed = l_pressed = r_pressed = false;
                activeDirectCmd = null;
                document.querySelectorAll('.circle-btn, .dir-arrow').forEach(b => b.classList.remove('pressed'));
                updateMotion();
            }
        }

        window.addEventListener('pointerup', resetAllMotion);
        window.addEventListener('pointercancel', resetAllMotion);
        window.addEventListener('mouseup', resetAllMotion);
        window.addEventListener('touchend', resetAllMotion);
        window.addEventListener('touchcancel', resetAllMotion);
        window.addEventListener('blur', resetAllMotion);

        // Bàn phím máy tính (WASD & Mũi tên)
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
            if (k === 'c' || k === 'g') { toggleGripper(); return; }

            if (changed) updateMotion();
        });

        window.addEventListener('keyup', e => {
            const k = e.key.toLowerCase();
            let changed = false;

            if (k === 'w' || k === 'arrowup')    { f_pressed = false; document.getElementById('btnF').classList.remove('pressed'); changed = true; }
            if (k === 's' || k === 'arrowdown')  { b_pressed = false; document.getElementById('btnB').classList.remove('pressed'); changed = true; }
            if (k === 'a' || k === 'arrowleft')  { l_pressed = false; document.getElementById('btnL').classList.remove('pressed'); changed = true; }
            if (k === 'd' || k === 'arrowright') { r_pressed = false; document.getElementById('btnR').classList.remove('pressed'); changed = true; }

            if (changed) updateMotion();
        });

        // Khoi tao WebSocket port 81 ngay khi tai trang
        connectWebSocket();
    </script>
</body>
</html>
)rawliteral";

// ==================== CÁC HÀM XỬ LÝ TRANG NẠP OTA ====================
struct SdFileInfo {
    String name;
    size_t size;
    bool isBin;
};

// Hàm đảm bảo thẻ nhớ SD luôn sẵn sàng (tự mount lại nếu vừa cắm thẻ)
static bool ensureSdCard() {
    if (SD.cardType() == CARD_NONE) {
        SD.end();
        return SD.begin(SD_CS_PIN);
    }
    return true;
}

// Quét danh sách file trên thẻ nhớ SD
static std::vector<SdFileInfo> getSdFileList() {
    std::vector<SdFileInfo> files;
    if (!ensureSdCard()) {
        return files;
    }

    File root = SD.open("/");
    if (!root || !root.isDirectory()) {
        return files;
    }

    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String fname = String(file.name());
            if (!fname.startsWith("/")) {
                fname = "/" + fname;
            }
            bool isBin = fname.endsWith(".bin") || fname.endsWith(".BIN");
            files.push_back({ fname, file.size(), isBin });
        }
        file = root.openNextFile();
    }
    root.close(); // Đóng directory tránh leak file descriptor
    return files;
}

static String buildHtmlPage() {
    uint8_t activeApp = g_prefs.getUChar("active_app", 1);
    String lastFile = g_prefs.getString("last_file", "Chưa có");
    uint32_t lastSize = g_prefs.getUInt("last_size", 0);
    String lastStatus = g_prefs.getString("last_status", "Sẵn sàng");

    std::vector<SdFileInfo> fileList = getSdFileList();
    int binCount = 0;
    for (const auto& f : fileList) {
        if (f.isBin) binCount++;
    }

    String html = "<!DOCTYPE html><html lang='vi'><head><meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>ESP32 CAN OTA Gateway</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 15px; background: #f4f6f8; color: #333; line-height: 1.5; }";
    html += ".container { max-width: 800px; margin: 0 auto; background: #fff; padding: 20px; border-radius: 6px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }";
    html += "h2 { color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 8px; margin-top: 0; }";
    html += "fieldset { border: 1px solid #ccc; border-radius: 4px; padding: 12px 15px; margin-bottom: 16px; background: #fafafa; }";
    html += "legend { font-weight: bold; color: #2980b9; padding: 0 5px; }";
    html += "table { width: 100%; border-collapse: collapse; margin-top: 8px; background: #fff; }";
    html += "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }";
    html += "th { background-color: #eee; }";
    html += "button, input[type='submit'] { background: #3498db; color: #fff; border: none; padding: 8px 14px; border-radius: 4px; cursor: pointer; font-size: 14px; }";
    html += "button:hover, input[type='submit']:hover { background: #2980b9; }";
    html += "button:disabled, input[type='submit']:disabled { background: #bbb; cursor: not-allowed; }";
    html += ".btn-danger { background: #e74c3c; }";
    html += ".btn-danger:hover { background: #c0392b; }";
    html += ".btn-flash { background: #27ae60; font-weight: bold; font-size: 15px; padding: 10px 18px; }";
    html += ".btn-flash:hover { background: #219150; }";
    html += ".alert { padding: 10px 14px; border-radius: 4px; margin-bottom: 16px; font-weight: bold; }";
    html += ".alert-success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }";
    html += ".alert-error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }";
    html += ".alert-info { background: #d1ecf1; color: #0c5460; border: 1px solid #bee5eb; }";
    html += ".badge { display: inline-block; padding: 4px 10px; border-radius: 12px; font-weight: bold; font-size: 13px; }";
    html += ".badge-app1 { background: #3498db; color: #fff; }";
    html += ".badge-app2 { background: #e67e22; color: #fff; }";
    html += "</style>";
    html += "<script>";
    html += "function onFlashSubmit() {";
    html += "  if (!confirm('Xác nhận bắt đầu nạp Firmware qua mạng CAN Bus sang STM32?\\nQuá trình sẽ mất khoảng 5-10 giây.')) return false;";
    html += "  var btn = document.getElementById('btnFlash');";
    html += "  if (btn) { btn.disabled = true; btn.innerText = 'Đang nạp OTA qua CAN Bus... Vui lòng đợi!'; btn.style.background = '#888'; }";
    html += "  return true;";
    html += "}";
    html += "</script>";
    html += "</head><body>";

    html += "<div class='container'>";
    html += "<div style='display:flex; justify-content:space-between; align-items:center; margin-bottom:14px;'>";
    html += "<a href='/' style='text-decoration:none; background:#2c3e50; color:#fff; padding:8px 16px; border-radius:5px; font-weight:bold; font-size:13px; display:inline-flex; align-items:center; gap:6px;'>🎮 VỀ TRANG ĐIỀU KHIỂN XE</a>";
    html += "<span style='color:#7f8c8d; font-size:13px; font-weight:bold;'>Dual-Bank Gateway</span>";
    html += "</div>";
    
    html += "<h2>BỘ ĐIỀU KHIỂN CAN OTA GATEWAY (ESP32 - STM32)</h2>";

    // Khối thông báo Alert
    if (g_alertMessage.length() > 0) {
        String cls = "alert-info";
        if (g_alertType == "success") cls = "alert-success";
        else if (g_alertType == "error") cls = "alert-error";
        html += "<div class='alert " + cls + "'>" + g_alertMessage + "</div>";
        g_alertMessage = ""; // Reset sau khi hiển thị
        g_alertType = "";
    }

    // 1. Khối thông tin hệ thống & STM32
    html += "<fieldset><legend>1. Thông tin hệ thống & STM32 ECU</legend>";
    if (WiFi.status() == WL_CONNECTED) {
        html += "<p><b>Trạng thái WiFi:</b> <span style='color:green; font-weight:bold;'>Đã kết nối</span> (SSID: <b>" + String(WIFI_SSID) + "</b>, RSSI: " + String(WiFi.RSSI()) + " dBm)</p>";
        html += "<p><b>Địa chỉ IP:</b> <a href='http://" + WiFi.localIP().toString() + "/ota' target='_blank'>http://" + WiFi.localIP().toString() + "/ota</a></p>";
    } else {
        html += "<p><b>Trạng thái WiFi:</b> <span style='color:red; font-weight:bold;'>Chưa kết nối / Đang kết nối lại...</span> (SSID: " + String(WIFI_SSID) + ")</p>";
    }
    html += "<p><b>Thẻ nhớ SD:</b> " + String(SD.cardType() != CARD_NONE ? "Đã sẵn sàng" : "<span style='color:red;'>Chưa nhận thẻ nhớ!</span>") + "</p>";
    html += "<hr style='border:0; border-top:1px dashed #ccc;'>";
    html += "<p><b>Phân vùng App hiện tại của STM32:</b> ";
    if (activeApp == 1) {
        html += "<span class='badge badge-app1'>APP 1 (0x08004000)</span>";
    } else if (activeApp == 2) {
        html += "<span class='badge badge-app2'>APP 2 (0x08012000)</span>";
    } else {
        html += "<span>Chưa xác định (Mặc định: App 1)</span>";
    }
    html += "</p>";
    html += "<p style='font-size:13px; color:#555;'><i>Nguồn xác thực: " + g_appSourceInfo + "</i></p>";
    html += "<p><b>Lần nạp gần nhất:</b> " + lastFile + " (" + String(lastSize) + " Bytes) - Trạng thái: " + lastStatus + "</p>";
    html += "<form method='POST' action='/query' style='margin-top:8px;'>";
    html += "<button type='submit'>Truy vấn App từ STM32 qua CAN</button>";
    html += "</form>";
    html += "</fieldset>";

    // 2. Tải file lên thẻ nhớ SD
    html += "<fieldset><legend>2. Tải file Firmware (.bin) lên thẻ nhớ SD</legend>";
    html += "<form method='POST' action='/upload' enctype='multipart/form-data'>";
    html += "<input type='file' name='firmware' accept='.bin' required style='margin-right:10px;'>";
    html += "<button type='submit'>Tải lên SD</button>";
    html += "</form>";
    html += "</fieldset>";

    // 3. Danh sách file trong thẻ nhớ SD
    html += "<fieldset><legend>3. Danh sách file trong thẻ nhớ SD (" + String(fileList.size()) + " files)</legend>";
    if (fileList.empty()) {
        html += "<p><i>Chưa có file nào trên thẻ nhớ SD hoặc chưa cắm thẻ nhớ. Vui lòng kiểm tra thẻ nhớ và tải file .bin lên ở Mục 2.</i></p>";
    } else {
        html += "<table><thead><tr><th>Tên file</th><th>Kích thước</th><th>Định dạng</th><th>Hành động</th></tr></thead><tbody>";
        for (const auto& f : fileList) {
            html += "<tr>";
            html += "<td>" + f.name + "</td>";
            html += "<td>" + String(f.size) + " bytes</td>";
            html += "<td>" + String(f.isBin ? "<b style='color:green;'>Firmware .bin</b>" : "File khác") + "</td>";
            html += "<td>";
            html += "<form method='POST' action='/delete' style='display:inline;' onsubmit='return confirm(\"Xác nhận xóa file " + f.name + " trên thẻ nhớ?\");'>";
            html += "<input type='hidden' name='filename' value='" + f.name + "'>";
            html += "<button type='submit' class='btn-danger' style='padding:4px 8px; font-size:12px;'>Xóa</button>";
            html += "</form>";
            html += "</td>";
            html += "</tr>";
        }
        html += "</tbody></table>";
    }
    html += "</fieldset>";

    // 4. Chọn file và phân vùng để Flash OTA
    html += "<fieldset><legend>4. Chọn file & Phân vùng để Flash OTA qua CAN</legend>";
    if (binCount == 0) {
        html += "<p style='color:#c0392b;'><i>Không tìm thấy file firmware (.bin) nào trên thẻ nhớ SD. Vui lòng tải file .bin ở Mục 2 để kích hoạt tính năng nạp.</i></p>";
    } else {
        html += "<form method='POST' action='/flash' onsubmit='return onFlashSubmit();'>";
        html += "<div style='margin-bottom:12px;'>";
        html += "<label><b>Chọn file firmware (.bin):</b></label><br>";
        html += "<select name='filename' style='padding:6px; min-width:260px; margin-top:4px;'>";
        for (const auto& f : fileList) {
            if (f.isBin) {
                html += "<option value='" + f.name + "'>" + f.name + " (" + String(f.size) + " bytes)</option>";
            }
        }
        html += "</select>";
        html += "</div>";

        // Gợi ý phân vùng đích theo kiến trúc Dual-Bank
        uint8_t targetSuggestion = (activeApp == 1) ? 2 : 1;

        html += "<div style='margin-bottom:12px;'>";
        html += "<label><b>Chọn phân vùng đích trên STM32:</b></label><br>";
        html += "<label style='margin-right:20px;'>";
        html += "<input type='radio' name='app_id' value='1' " + String(targetSuggestion == 1 ? "checked" : "") + "> ";
        html += "<b>App 1</b> (Địa chỉ: 0x08004000, 56KB)";
        html += "</label><br>";
        html += "<label>";
        html += "<input type='radio' name='app_id' value='2' " + String(targetSuggestion == 2 ? "checked" : "") + "> ";
        html += "<b>App 2</b> (Địa chỉ: 0x08012000, 56KB)";
        html += "</label>";
        html += "<p style='font-size:12px; color:#666; margin:4px 0 0 0;'>";
        html += "* Gợi ý Dual-Bank: Hiện STM32 đang là App " + String(activeApp) + ", hệ thống tự động chọn phân vùng dự phòng App " + String(targetSuggestion) + ".";
        html += "</p>";
        html += "</div>";

        html += "<div style='margin-bottom:15px;'>";
        html += "<label>";
        html += "<input type='checkbox' name='crc_err' value='1'> ";
        html += "Giả lập lỗi CRC ở Chunk #2 (Blackbox Test kiểm thử cơ chế Fail-safe Rollback)";
        html += "</label>";
        html += "</div>";

        html += "<button type='submit' id='btnFlash' class='btn-flash' " + String(g_isFlashing ? "disabled" : "") + ">";
        html += g_isFlashing ? "ĐANG NẠP OTA... XIN ĐỢI!" : "BẮT ĐẦU FLASH OTA QUA CAN";
        html += "</button>";
        html += "</form>";
    }
    html += "</fieldset>";

    html += "</div></body></html>";
    return html;
}

static void handleRoot() {
    server.send(200, "text/html", buildHtmlPage());
}

static void handleUploadFile() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        ensureSdCard();

        // Lấy tên file gốc (cắt bỏ đường dẫn từ phía client nếu có)
        String filename = upload.filename;
        int lastSlash = filename.lastIndexOf('/');
        if (lastSlash >= 0) filename = filename.substring(lastSlash + 1);
        int lastBackslash = filename.lastIndexOf('\\');
        if (lastBackslash >= 0) filename = filename.substring(lastBackslash + 1);
        filename.trim();
        if (filename.length() == 0) filename = "firmware.bin";
        if (!filename.startsWith("/")) filename = "/" + filename;

        Serial.printf("[HTTP] Bat dau tai file len SD: %s\n", filename.c_str());

        if (SD.exists(filename)) {
            SD.remove(filename);
        }

        g_uploadFile = SD.open(filename, FILE_WRITE);
        if (!g_uploadFile) {
            Serial.printf("[HTTP ERR] Khong the tao file tren SD: %s\n", filename.c_str());
            g_alertMessage = "Khong the tao file tren the nho SD! Kiem tra the nho.";
            g_alertType = "error";
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (g_uploadFile) {
            g_uploadFile.write(upload.buf, upload.currentSize);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (g_uploadFile) {
            g_uploadFile.close();
            Serial.printf("[HTTP] Tai file len SD hoan tat: %s (%u bytes)\n",
                          upload.filename.c_str(), upload.totalSize);
            g_alertMessage = "Tai len the nho thanh cong: " + upload.filename + " (" + String(upload.totalSize) + " bytes)";
            g_alertType = "success";
        }
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        if (g_uploadFile) {
            g_uploadFile.close();
            Serial.println("[HTTP ERR] Upload bi huy!");
        }
        g_alertMessage = "Qua trinh tai file bi huy!";
        g_alertType = "error";
    }
}

static void handleUploadDone() {
    server.sendHeader("Location", "/ota");
    server.send(303);
}

static void handleFlash() {
    if (g_isFlashing) {
        g_alertMessage = "He thong dang thuc hien nap OTA, vui long doi!";
        g_alertType = "error";
        server.sendHeader("Location", "/ota");
        server.send(303);
        return;
    }

    if (!server.hasArg("filename") || !server.hasArg("app_id")) {
        g_alertMessage = "Thieu thong tin file hoac phan vung dich!";
        g_alertType = "error";
        server.sendHeader("Location", "/ota");
        server.send(303);
        return;
    }

    String filename = server.arg("filename");
    int appId = server.arg("app_id").toInt();
    bool injectCrc = server.hasArg("crc_err");

    if (appId != 1 && appId != 2) {
        g_alertMessage = "Phan vung khong hop le (chi chap nhan App 1 hoac App 2)!";
        g_alertType = "error";
        server.sendHeader("Location", "/ota");
        server.send(303);
        return;
    }

    ensureSdCard();

    if (!SD.exists(filename)) {
        g_alertMessage = "File " + filename + " khong ton tai tren the nho SD!";
        g_alertType = "error";
        server.sendHeader("Location", "/ota");
        server.send(303);
        return;
    }

    File file = SD.open(filename, FILE_READ);
    if (!file) {
        g_alertMessage = "Khong the mo file: " + filename;
        g_alertType = "error";
        server.sendHeader("Location", "/ota");
        server.send(303);
        return;
    }

    size_t fileSize = file.size();
    g_ota->setInjectCrcError(injectCrc);

    Serial.printf("[WEB] Kich hoat flash file %s vao App %d qua CAN (CRC Error Test=%s)...\n",
                  filename.c_str(), appId, injectCrc ? "ON" : "OFF");

    g_isFlashing = true;
    bool ok = g_ota->flashFromFile(file, (uint8_t)appId);
    file.close();
    g_isFlashing = false;

    if (ok) {
        g_prefs.putUChar("active_app", (uint8_t)appId);
        g_prefs.putString("last_file", filename);
        g_prefs.putUInt("last_size", (uint32_t)fileSize);
        g_prefs.putString("last_status", "Thanh cong (100%)");
        g_appSourceInfo = "Vua nap thanh cong qua CAN vao App " + String(appId);

        g_alertMessage = "FLASH THANH CONG! Da nap " + filename + " vao App " + String(appId) + ". STM32 da boot vao firmware moi!";
        g_alertType = "success";
    } else {
        String err = g_ota->getLastError();
        if (err.isEmpty()) err = "Loi truyen CAN hoac verify firmware that bai!";
        g_prefs.putString("last_status", "That bai: " + err);

        g_alertMessage = "FLASH THAT BAI: " + err;
        g_alertType = "error";
    }

    server.sendHeader("Location", "/ota");
    server.send(303);
}

static void handleQuery() {
    Serial.println("[WEB] Gui lenh truy van App status sang STM32 qua CAN...");
    uint8_t stmApp = g_ota->queryCurrentApp(400);

    if (stmApp == 1 || stmApp == 2) {
        g_prefs.putUChar("active_app", stmApp);
        g_appSourceInfo = "Xac thuc truc tiep tu STM32 qua CAN Bus (500 kbps)";
        g_alertMessage = "STM32 phan hoi truc tiep qua CAN: Dang hoat dong o App " + String(stmApp);
        g_alertType = "success";
    } else {
        g_appSourceInfo = "Gateway NVS (STM32 khong phan hoi qua CAN luc nay)";
        g_alertMessage = "STM32 khong phan hoi qua CAN (Co the dang ban chay xe hoac chua bat). Hien thi theo bo nho Gateway.";
        g_alertType = "info";
    }

    server.sendHeader("Location", "/ota");
    server.send(303);
}

static void handleDelete() {
    if (server.hasArg("filename")) {
        ensureSdCard();
        String filename = server.arg("filename");
        if (SD.exists(filename)) {
            SD.remove(filename);
            g_alertMessage = "Da xoa file: " + filename;
            g_alertType = "info";
        }
    }
    server.sendHeader("Location", "/ota");
    server.send(303);
}



static void handleOtaRoot() {
    ensureSdCard();
    server.send(200, "text/html", buildHtmlPage());
}

// ==================== CÁC HÀM XỬ LÝ TRANG ĐIỀU KHIỂN XE ====================
static void handleControlRoot() {
    server.send_P(200, "text/html", INDEX_HTML);
}

static uint32_t s_last_app_cmd_seq = 0;
static void handleApiCmd() {
    if (!g_car) {
        server.send(500, "application/json", "{\"ok\":false,\"err\":\"Car controller not ready\"}");
        return;
    }

    // Kiem tra thu tu goi tin: loai bo cac goi cu bi tre do lag mang WiFi
    if (server.hasArg("seq")) {
        uint32_t seq = (uint32_t)server.arg("seq").toInt();
        if (seq < s_last_app_cmd_seq && (s_last_app_cmd_seq - seq) < 1000000000UL) {
            server.send(200, "application/json", "{\"ok\":false,\"stale\":true}");
            return;
        }
        s_last_app_cmd_seq = seq;
    }

    bool ok = false;
    if (server.hasArg("code")) {
        String codeStr = server.arg("code");
        uint16_t id = (uint16_t)strtoul(codeStr.c_str(), nullptr, 0);
        ok = g_car->executeCanId(id);
    } else if (server.hasArg("action")) {
        String act = server.arg("action");
        ok = g_car->executeCommand(act);
    } else {
        server.send(400, "application/json", "{\"ok\":false,\"err\":\"Missing code or action\"}");
        return;
    }

    String json = "{\"ok\":" + String(ok ? "true" : "false") + ",\"action\":\"" + g_car->getLastAction() + "\"}";
    server.send(200, "application/json", json);
}

static void handleApiStatus() {
    if (!g_car) {
        server.send(500, "application/json", "{}");
        return;
    }
    server.send(200, "application/json", g_car->getStatusJson());
}

// ==================== CÁC HÀM XỬ LÝ WEBSOCKET (REAL-TIME 2MS) ====================
static void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WS] Client #%u ngat ket noi -> Tu dong DUNG XE vi an toan!\n", num);
            if (g_car) g_car->stop();
            break;

        case WStype_CONNECTED: {
            IPAddress ip = wsServer.remoteIP(num);
            Serial.printf("[WS] Client #%u ket noi tu IP: %s\n", num, ip.toString().c_str());
            wsServer.sendTXT(num, "{\"status\":\"CONNECTED\"}");
            break;
        }

        case WStype_TEXT: {
            if (!g_car || length == 0) return;
            char buf[64];
            size_t copyLen = length < (sizeof(buf) - 1) ? length : (sizeof(buf) - 1);
            memcpy(buf, payload, copyLen);
            buf[copyLen] = '\0';

            char *p = buf;
            while (*p == ' ' || *p == '\t') p++;

            if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
                uint16_t id = (uint16_t)strtoul(p, nullptr, 16);
                g_car->executeCanId(id);
            } else {
                g_car->executeCommand(String(p));
            }
            break;
        }

        default:
            break;
    }
}

// ==================== KHỞI TẠO VÀ LOOP WEBSERVER HỢP NHẤT ====================
void App_WebServer_Init(Car_Controller &car, OTA_Protocol &ota) {
    g_car = &car;
    g_ota = &ota;
    g_prefs.begin("ota_gw", false);

    // 1. Cấu hình Wi-Fi: Chế độ Station (Bắt sóng WiFi ngoài / Hotspot)
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    Serial.println("\n[WIFI STA] Dang khoi tao ket noi WiFi...");
    Serial.printf("  -> Ket noi vao SSID: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // 2. Khởi động WebSocket Server thời gian thực (Port 81)
    wsServer.begin();
    wsServer.onEvent(onWebSocketEvent);
    Serial.printf("[WS] WebSocket Server da san sang tren port %d (Latency 1-2ms)!\n", WS_SERVER_PORT);

    // 3. Định tuyến các URL giao diện & API
    // Giao diện Lái xe
    server.on("/", HTTP_GET, handleControlRoot);
    server.on("/control", HTTP_GET, handleControlRoot);
    server.on("/api/cmd", HTTP_GET, handleApiCmd);
    server.on("/api/status", HTTP_GET, handleApiStatus);

    // Giao diện Nạp OTA & API OTA
    server.on("/ota", HTTP_GET, handleOtaRoot);
    server.on("/upload", HTTP_POST, handleUploadDone, handleUploadFile);
    server.on("/flash", HTTP_POST, handleFlash);
    server.on("/query", HTTP_POST, handleQuery);
    server.on("/delete", HTTP_POST, handleDelete);

    // Khởi động server HTTP
    server.begin();
    Serial.printf("[HTTP] Web Server hop nhat da khoi dong tren port %d!\n", WEB_SERVER_PORT);
    Serial.println(">> Web san sang (Vui long doi WiFi ket noi de lay dia chi IP)...");
}

void App_WebServer_Loop() {
    server.handleClient();
    wsServer.loop();
}
