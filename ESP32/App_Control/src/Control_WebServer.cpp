#include "Control_WebServer.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "config.h"

#define WS_SERVER_PORT 81
static WebServer server(WEB_SERVER_PORT);
static WebSocketsServer wsServer(WS_SERVER_PORT);
static Car_Controller *g_car = nullptr;

// ==================== HTML / CSS / JS Giao diện RC Controller Chuẩn Opcode CAN ====================
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

// ==================== XỬ LÝ WEBSOCKET STREAMING THỜI GIAN THỰC ====================
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

// ==================== Khởi tạo Web Server ====================
void Control_WebServer_Init(Car_Controller &car) {
    g_car = &car;

    // 1. Khởi động WiFi Station (Bắt sóng WiFi ngoài / Hotspot)
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    Serial.println("\n[WIFI STA] Dang khoi tao ket noi WiFi...");
    Serial.printf("       SSID : %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // 2. Khởi động WebSocket Server thời gian thực (Port 81)
    wsServer.begin();
    wsServer.onEvent(onWebSocketEvent);
    Serial.printf("[WS] WebSocket Server da san sang tren port %d (Latency 1-2ms)!\n", WS_SERVER_PORT);

    // 3. Định tuyến URL
    // Trang chủ giao diện Web
    server.on("/", HTTP_GET, []() {
        server.send_P(200, "text/html", INDEX_HTML);
    });

    // API thực thi lệnh Opcode: /api/cmd?code=0x041&seq=123 hoặc /api/cmd?action=forward
    static uint32_t s_last_cmd_seq = 0;
    server.on("/api/cmd", HTTP_GET, []() {
        if (!g_car) {
            server.send(500, "application/json", "{\"ok\":false,\"err\":\"Car controller not ready\"}");
            return;
        }

        // Kiem tra thu tu goi tin: loai bo cac goi cu bi tre do lag mang WiFi
        if (server.hasArg("seq")) {
            uint32_t seq = (uint32_t)server.arg("seq").toInt();
            if (seq < s_last_cmd_seq && (s_last_cmd_seq - seq) < 1000000000UL) {
                server.send(200, "application/json", "{\"ok\":false,\"stale\":true}");
                return;
            }
            s_last_cmd_seq = seq;
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
    Serial.printf("[WEB] Web Server da khoi dong tren cong %d (Dang doi WiFi cap IP)...\n\n", WEB_SERVER_PORT);
}

void Control_WebServer_Loop() {
    server.handleClient();
    wsServer.loop();
}
