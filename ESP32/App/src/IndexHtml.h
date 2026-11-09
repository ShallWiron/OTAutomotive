#ifndef INDEX_HTML_H
#define INDEX_HTML_H

#include <pgmspace.h>

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en" data-theme="dark">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Automotive ECU Gateway | ESP32-STM32 CAN</title>
    <style>
        :root {
            /* Dark Industrial Theme (Default) */
            --bg-body: #09090b;
            --panel-bg: #121215;
            --panel-header: #18181c;
            --panel-border: #32323c;
            --panel-border-subtle: #202026;
            --text-main: #d4d4d8;
            --text-heading: #f4f4f6;
            --text-muted: #8b8b94;
            --btn-bg: #1b1b20;
            --btn-border: #42424e;
            --btn-hover: #26262e;
            --btn-primary-bg: #f4f4f6;
            --btn-primary-text: #09090b;
            --btn-primary-border: #ffffff;
            --btn-primary-hover: #ffffff;
            --code-bg: #0d0d10;
            --badge-bg: #1a1a1f;
            --badge-border: #383844;
            --badge-text: #e0e0e6;
            --badge-active-bg: #2d2d38;
            --badge-active-border: #656578;
            --badge-active-text: #ffffff;
            --badge-target-bg: #18181e;
            --badge-target-border: #383846;
            --badge-target-text: #9696a2;
            --danger-bg: #281012;
            --danger-border: #682024;
            --danger-text: #f87171;
            --danger-hover: #3d1418;
            --danger-active-bg: #dc2626;
            --danger-active-text: #ffffff;
            --accent-amber-bg: #2b1f0c;
            --accent-amber-border: #785012;
            --accent-amber-text: #fbbf24;
            --progress-bar-bg: #0d0d10;
            --progress-bar-fill: #f4f4f6;
            --log-line-muted: #787884;
            --log-line-info: #b8b8c2;
            --log-line-success: #4ade80;
            --log-line-warn: #fbbf24;
            --font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
            --font-mono: SFMono-Regular, Consolas, "Liberation Mono", Menlo, monospace;
        }

        [data-theme="light"] {
            /* Light Industrial Theme */
            --bg-body: #f4f4f6;
            --panel-bg: #ffffff;
            --panel-header: #f8f8fa;
            --panel-border: #c8c8d2;
            --panel-border-subtle: #e4e4eb;
            --text-main: #333338;
            --text-heading: #09090b;
            --text-muted: #70707c;
            --btn-bg: #ffffff;
            --btn-border: #b0b0bc;
            --btn-hover: #eaeaef;
            --btn-primary-bg: #09090b;
            --btn-primary-text: #ffffff;
            --btn-primary-border: #09090b;
            --btn-primary-hover: #1f1f24;
            --code-bg: #f0f0f4;
            --badge-bg: #f0f0f4;
            --badge-border: #c0c0cc;
            --badge-text: #222228;
            --badge-active-bg: #09090b;
            --badge-active-border: #09090b;
            --badge-active-text: #ffffff;
            --badge-target-bg: #f4f4f8;
            --badge-target-border: #d0d0dc;
            --badge-target-text: #70707c;
            --danger-bg: #fef2f2;
            --danger-border: #f87171;
            --danger-text: #b91c1c;
            --danger-hover: #fee2e2;
            --danger-active-bg: #b91c1c;
            --danger-active-text: #ffffff;
            --accent-amber-bg: #fef3c7;
            --accent-amber-border: #d97706;
            --accent-amber-text: #92400e;
            --progress-bar-bg: #e4e4eb;
            --progress-bar-fill: #09090b;
            --log-line-muted: #70707c;
            --log-line-info: #333338;
            --log-line-success: #15803d;
            --log-line-warn: #b45309;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            border-radius: 0 !important; /* Strict squared styling */
        }

        body {
            background-color: var(--bg-body);
            font-family: var(--font-family);
            color: var(--text-main);
            font-size: 13px;
            line-height: 1.5;
            padding-bottom: 30px;
            transition: background-color 0.1s ease, color 0.1s ease;
        }

        /* NAVBAR */
        .navbar {
            background-color: var(--panel-header);
            border-bottom: 2px solid var(--panel-border);
            padding: 10px 24px;
            display: flex;
            align-items: center;
            justify-content: space-between;
        }

        .navbar-brand {
            display: flex;
            align-items: center;
            gap: 12px;
            color: var(--text-heading);
            font-size: 13px;
            font-weight: 800;
            letter-spacing: 0.8px;
            text-decoration: none;
            text-transform: uppercase;
            font-family: var(--font-mono);
        }

        .navbar-brand .subtext {
            font-size: 11px;
            font-weight: 500;
            color: var(--text-muted);
            letter-spacing: 0.2px;
        }

        .navbar-actions {
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .navbar-status {
            display: flex;
            align-items: center;
            gap: 8px;
            font-size: 11px;
            font-family: var(--font-mono);
        }

        .status-pill {
            display: inline-flex;
            align-items: center;
            gap: 6px;
            padding: 4px 8px;
            background: var(--code-bg);
            border: 1px solid var(--panel-border);
            color: var(--text-muted);
            font-weight: 700;
            letter-spacing: 0.3px;
        }

        .status-pill.active {
            color: var(--text-heading);
            border-color: var(--btn-border);
        }

        .indicator {
            width: 7px;
            height: 7px;
            background-color: var(--text-muted);
            display: inline-block;
        }

        .indicator.online {
            background-color: var(--log-line-success);
        }

        .indicator.offline {
            background-color: var(--danger-text);
        }

        /* CONTAINER */
        .container {
            max-width: 1060px;
            margin: 18px auto 0 auto;
            padding: 0 16px;
            display: flex;
            flex-direction: column;
            gap: 16px;
        }

        /* NAV TABS */
        .nav-tabs {
            display: flex;
            border-bottom: 2px solid var(--panel-border);
            gap: 4px;
        }

        .nav-tab-item {
            padding: 9px 20px;
            border: 1px solid transparent;
            border-bottom: none;
            background: transparent;
            color: var(--text-muted);
            font-size: 12px;
            font-weight: 800;
            letter-spacing: 0.6px;
            text-transform: uppercase;
            cursor: pointer;
            font-family: var(--font-mono);
            transition: all 0.1s ease;
        }

        .nav-tab-item:hover {
            color: var(--text-heading);
            background: var(--btn-hover);
        }

        .nav-tab-item.active {
            color: var(--text-heading);
            background: var(--panel-bg);
            border: 1px solid var(--panel-border);
            border-bottom: 2px solid var(--panel-bg);
            margin-bottom: -2px;
        }

        /* PANELS */
        .panel {
            background-color: var(--panel-bg);
            border: 1px solid var(--panel-border);
            overflow: hidden;
        }

        .panel-heading {
            background-color: var(--panel-header);
            border-bottom: 1px solid var(--panel-border);
            padding: 9px 14px;
            display: flex;
            align-items: center;
            justify-content: space-between;
        }

        .panel-title {
            font-size: 12px;
            font-weight: 800;
            color: var(--text-heading);
            text-transform: uppercase;
            letter-spacing: 0.6px;
            font-family: var(--font-mono);
        }

        .panel-body {
            padding: 14px;
        }

        .row-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 14px;
        }

        /* SOURCE SEGMENTED SWITCH */
        .source-switch {
            display: flex;
            border: 1px solid var(--panel-border);
            background: var(--code-bg);
            padding: 2px;
            margin-bottom: 12px;
            gap: 2px;
        }

        .btn-source {
            flex: 1;
            padding: 7px 10px;
            background: transparent;
            border: 1px solid transparent;
            color: var(--text-muted);
            font-size: 11px;
            font-weight: 800;
            font-family: var(--font-mono);
            cursor: pointer;
            transition: all 0.1s;
            text-transform: uppercase;
            letter-spacing: 0.4px;
            text-align: center;
        }

        .btn-source.active {
            background: var(--panel-bg);
            color: var(--text-heading);
            border: 1px solid var(--btn-border);
        }

        /* DATA TABLE */
        .stat-table {
            width: 100%;
            border-collapse: collapse;
        }

        .stat-table tr {
            border-bottom: 1px solid var(--panel-border-subtle);
        }

        .stat-table tr:last-child {
            border-bottom: none;
        }

        .stat-table td {
            padding: 7px 4px;
            vertical-align: middle;
        }

        .stat-label {
            color: var(--text-muted);
            font-weight: 600;
            font-size: 11.5px;
            font-family: var(--font-mono);
            text-transform: uppercase;
            letter-spacing: 0.2px;
            width: 44%;
        }

        .stat-value {
            text-align: right;
            font-weight: 700;
            color: var(--text-heading);
            font-family: var(--font-mono);
            font-size: 12px;
        }

        /* BADGES */
        .badge {
            display: inline-block;
            padding: 3px 8px;
            font-size: 11px;
            font-weight: 800;
            font-family: var(--font-mono);
            letter-spacing: 0.4px;
            text-transform: uppercase;
            background: var(--badge-bg);
            border: 1px solid var(--badge-border);
            color: var(--badge-text);
        }

        .badge-active {
            background: var(--badge-active-bg);
            border-color: var(--badge-active-border);
            color: var(--badge-active-text);
        }

        .badge-target {
            background: var(--badge-target-bg);
            border-color: var(--badge-target-border);
            color: var(--badge-target-text);
        }

        .badge-amber {
            background: var(--accent-amber-bg);
            border-color: var(--accent-amber-border);
            color: var(--accent-amber-text);
        }

        .badge-danger {
            background: var(--danger-bg);
            border-color: var(--danger-border);
            color: var(--danger-text);
        }

        /* BUTTONS */
        .btn {
            display: inline-flex;
            align-items: center;
            justify-content: center;
            padding: 6px 12px;
            font-size: 11px;
            font-weight: 700;
            cursor: pointer;
            border: 1px solid var(--btn-border);
            background-color: var(--btn-bg);
            color: var(--text-heading);
            text-decoration: none;
            font-family: var(--font-mono);
            text-transform: uppercase;
            letter-spacing: 0.4px;
            transition: all 0.08s ease;
            user-select: none;
        }

        .btn:hover {
            background-color: var(--btn-hover);
            border-color: var(--text-heading);
        }

        .btn:active, .btn.active {
            background-color: var(--btn-primary-bg);
            color: var(--btn-primary-text);
            border-color: var(--btn-primary-border);
        }

        .btn-primary {
            background-color: var(--btn-primary-bg);
            color: var(--btn-primary-text);
            border-color: var(--btn-primary-border);
            font-weight: 800;
        }

        .btn-primary:hover {
            background-color: var(--btn-primary-hover);
            border-color: var(--btn-primary-hover);
        }

        .btn-danger {
            background-color: var(--danger-bg);
            color: var(--danger-text);
            border-color: var(--danger-border);
            font-weight: 800;
        }

        .btn-danger:hover {
            background-color: var(--danger-hover);
            border-color: var(--danger-text);
        }

        .btn-danger:active, .btn-danger.active {
            background-color: var(--danger-active-bg);
            color: var(--danger-active-text);
            border-color: var(--danger-active-bg);
        }

        .btn:disabled {
            opacity: 0.3;
            cursor: not-allowed;
            pointer-events: none;
        }

        /* FORM CONTROLS */
        .form-group {
            display: flex;
            flex-direction: column;
            gap: 6px;
            margin-bottom: 10px;
        }

        .form-label {
            font-size: 11px;
            font-weight: 700;
            color: var(--text-muted);
            text-transform: uppercase;
            letter-spacing: 0.3px;
            font-family: var(--font-mono);
        }

        .form-control-file {
            border: 1px solid var(--panel-border);
            background: var(--code-bg);
            padding: 7px 10px;
            font-size: 11.5px;
            color: var(--text-main);
            width: 100%;
            font-family: var(--font-mono);
        }

        select.form-control-file {
            cursor: pointer;
        }

        /* PROGRESS BAR */
        .progress-container {
            display: flex;
            flex-direction: column;
            gap: 6px;
        }

        .progress-header {
            display: flex;
            justify-content: space-between;
            font-size: 11px;
            font-family: var(--font-mono);
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 0.3px;
        }

        .progress-bar {
            height: 12px;
            background-color: var(--progress-bar-bg);
            overflow: hidden;
            border: 1px solid var(--panel-border);
        }

        .progress-bar-fill {
            height: 100%;
            background-color: var(--progress-bar-fill);
            width: 0%;
            transition: width 0.1s ease;
        }

        /* CONSOLE */
        .log-box {
            background-color: var(--code-bg);
            border: 1px solid var(--panel-border);
            padding: 9px 12px;
            height: 150px;
            overflow-y: auto;
            font-family: var(--font-mono);
            font-size: 11px;
            display: flex;
            flex-direction: column;
            gap: 3px;
        }

        .log-line { color: var(--log-line-muted); }
        .log-line.info { color: var(--log-line-info); }
        .log-line.success { color: var(--log-line-success); }
        .log-line.warn { color: var(--log-line-warn); }
        .log-line.err { color: var(--danger-text); }

        /* ==================== CONTROL TAB STYLING ==================== */
        .control-toolbar {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 8px;
            margin-bottom: 14px;
        }

        .btn-ctrl {
            padding: 9px 12px;
            font-size: 11.5px;
            font-weight: 800;
            letter-spacing: 0.5px;
            white-space: nowrap;
            text-align: center;
        }

        .control-grid {
            display: grid;
            grid-template-columns: 310px 1fr;
            gap: 16px;
        }

        .dpad-card {
            background: var(--code-bg);
            border: 1px solid var(--panel-border);
            padding: 14px;
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 12px;
        }

        .dpad-matrix {
            display: grid;
            grid-template-columns: 88px 88px 88px;
            grid-template-rows: 62px 62px 62px;
            gap: 6px;
            justify-content: center;
            user-select: none;
            -webkit-user-select: none;
        }

        .btn-pad {
            background: var(--panel-header);
            border: 1px solid var(--btn-border);
            color: var(--text-heading);
            font-size: 11.5px;
            font-weight: 800;
            font-family: var(--font-mono);
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            user-select: none;
            -webkit-user-select: none;
            touch-action: none;
            transition: all 0.05s ease;
            letter-spacing: 0.4px;
        }

        .btn-pad .pad-key {
            font-size: 9px;
            color: var(--text-muted);
            margin-top: 2px;
            font-weight: 700;
        }

        .btn-pad:active, .btn-pad.active {
            background: var(--text-heading);
            border-color: var(--text-heading);
            color: var(--bg-body);
        }

        .btn-pad:active .pad-key, .btn-pad.active .pad-key {
            color: var(--bg-body);
        }

        .btn-pad.pad-stop {
            background: var(--danger-bg);
            border-color: var(--danger-border);
            color: var(--danger-text);
        }

        .btn-pad.pad-stop:active, .btn-pad.pad-stop.active {
            background: var(--danger-active-bg);
            border-color: var(--danger-active-bg);
            color: var(--danger-active-text);
        }

        .btn-pad.pad-stop:active .pad-key, .btn-pad.pad-stop.active .pad-key {
            color: var(--danger-active-text);
        }

        .btn-pad.pad-disabled {
            opacity: 0.22;
            cursor: not-allowed;
            pointer-events: none;
            background: var(--code-bg);
            border-color: var(--panel-border-subtle);
        }

        .heartbeat-strip {
            width: 100%;
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 5px 8px;
            background: var(--panel-header);
            border: 1px solid var(--panel-border);
            font-family: var(--font-mono);
            font-size: 10.5px;
            color: var(--text-muted);
            font-weight: 700;
        }

        .heartbeat-dot {
            width: 8px;
            height: 8px;
            display: inline-block;
            background: var(--text-muted);
        }

        .heartbeat-dot.pulsing {
            background: var(--log-line-success);
        }

        @media (max-width: 900px) {
            .row-grid, .control-grid { grid-template-columns: 1fr; }
            .control-toolbar { grid-template-columns: repeat(2, 1fr); }
            .navbar { flex-direction: column; gap: 8px; align-items: flex-start; }
            .navbar-actions { width: 100%; justify-content: space-between; }
            .navbar-status { flex-wrap: wrap; }
        }
    </style>
</head>
<body>

    <!-- NAVBAR -->
    <header class="navbar">
        <a href="#" class="navbar-brand">
            <span>ECU GATEWAY</span>
            <span class="subtext">ESP32 / STM32 CAN BUS (500 KBPS)</span>
        </a>
        <div class="navbar-actions">
            <div class="navbar-status">
                <div class="status-pill active">
                    <span class="indicator online"></span>
                    <span>CAN: 500 KBPS</span>
                </div>
                <div class="status-pill">
                    <span>NET: AP MODE</span>
                </div>
                <div class="status-pill" id="pillWsStatus">
                    <span class="indicator offline" id="dotWs"></span>
                    <span id="txtWsStatus">WS: CONNECTING</span>
                </div>
            </div>
            <button class="btn" id="btnThemeToggle" onclick="toggleTheme()" style="padding:4px 10px; font-size:11px;">
                THEME: DARK
            </button>
        </div>
    </header>

    <!-- MAIN CONTAINER -->
    <main class="container">

        <!-- NAV TABS -->
        <nav class="nav-tabs">
            <button id="tabBtnOta" class="nav-tab-item active" onclick="switchTab('ota')">
                FIRMWARE OTA
            </button>
            <button id="tabBtnControl" class="nav-tab-item" onclick="switchTab('control')">
                VEHICLE & ROBOT CONTROL
            </button>
        </nav>

        <!-- ==================== TAB 1: FIRMWARE OTA ==================== -->
        <section id="tabOta" class="row-grid">

            <!-- LEFT: ECU STATUS & DUAL-BANK -->
            <div class="panel">
                <div class="panel-heading">
                    <span class="panel-title">ECU TELEMETRY & DUAL-BANK</span>
                    <button class="btn" onclick="queryActiveApp()">QUERY ECU</button>
                </div>
                <div class="panel-body">
                    <table class="stat-table">
                        <tr>
                            <td class="stat-label">Target MCU</td>
                            <td class="stat-value">STM32F103 (128 KB)</td>
                        </tr>
                        <tr>
                            <td class="stat-label">Flash Architecture</td>
                            <td class="stat-value">Dual-Bank Rollback</td>
                        </tr>
                        <tr>
                            <td class="stat-label">Active Partition</td>
                            <td class="stat-value">
                                <span id="lblActiveApp" class="badge badge-active">APP 1 (0x08004000)</span>
                            </td>
                        </tr>
                        <tr>
                            <td class="stat-label">Target Partition</td>
                            <td class="stat-value">
                                <span id="lblTargetApp" class="badge badge-target">APP 2 (0x08012000)</span>
                            </td>
                        </tr>
                        <tr>
                            <td class="stat-label">Chunk Buffer</td>
                            <td class="stat-value">256 Bytes (32 frames)</td>
                        </tr>
                        <tr>
                            <td class="stat-label">Vehicle Safety State</td>
                            <td class="stat-value">
                                <span class="badge badge-active">STANDBY (SAFE)</span>
                            </td>
                        </tr>
                    </table>

                    <div style="margin-top:14px; padding-top:10px; border-top:1px solid var(--panel-border-subtle); font-size:11px; font-family:var(--font-mono); color:var(--text-muted);">
                        Option Bytes (User Data 0) switches partitions after Global CRC16 Modbus check.
                    </div>
                </div>
            </div>

            <!-- RIGHT: FIRMWARE SOURCE & UPLOAD -->
            <div class="panel">
                <div class="panel-heading">
                    <span class="panel-title">FIRMWARE SOURCE & FLASHING</span>
                </div>
                <div class="panel-body">

                    <!-- SOURCE SELECTOR (RAM BUFFER VS SD CARD) -->
                    <div class="source-switch">
                        <button type="button" class="btn-source active" id="btnSourceRam" onclick="setOtaSource('ram')">
                            DIRECT RAM BUFFER
                        </button>
                        <button type="button" class="btn-source" id="btnSourceSd" onclick="setOtaSource('sd')">
                            MICROSD STORAGE
                        </button>
                    </div>

                    <!-- PANE A: DIRECT RAM UPLOAD -->
                    <div id="paneSourceRam">
                        <div class="form-group">
                            <label class="form-label">SELECT LOCAL BINARY IMAGE (.BIN):</label>
                            <div>
                                <input type="file" id="fileInput" class="form-control-file" accept=".bin" onchange="onFileSelected()">
                            </div>
                        </div>
                    </div>

                    <!-- PANE B: MICROSD CARD STORAGE -->
                    <div id="paneSourceSd" style="display:none; flex-direction:column; gap:10px;">
                        <!-- SECTION B1: SELECT EXISTING FROM SD -->
                        <div class="form-group" style="margin-bottom:8px;">
                            <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:4px;">
                                <label class="form-label" style="margin-bottom:0;">STORED IMAGES ON MICROSD:</label>
                                <div style="display:flex; gap:6px;">
                                    <button type="button" class="btn" onclick="rescanSdCard()" style="padding:2px 8px; font-size:10px;">
                                        RESCAN SD
                                    </button>
                                    <button type="button" class="btn btn-danger" onclick="deleteSelectedSdFile()" style="padding:2px 8px; font-size:10px;">
                                        DELETE
                                    </button>
                                </div>
                            </div>
                            <select id="selSdFile" class="form-control-file" onchange="onSdFileChanged()">
                                <option value="">-- No .bin files scanned yet --</option>
                            </select>
                        </div>

                        <!-- SECTION B2: UPLOAD NEW TO SD -->
                        <div style="border-top:1px solid var(--panel-border-subtle); padding-top:10px;">
                            <label class="form-label">UPLOAD NEW IMAGE TO MICROSD CARD:</label>
                            <div style="display:flex; gap:8px;">
                                <input type="file" id="fileSdUploadInput" class="form-control-file" accept=".bin">
                                <button type="button" class="btn" id="btnUploadToSd" onclick="uploadFileToSd()" style="white-space:nowrap;">
                                    SAVE TO SD
                                </button>
                            </div>
                        </div>
                    </div>

                    <!-- SELECTED FILE DETAILS -->
                    <table class="stat-table" id="fileInfoTable" style="margin-bottom:12px; margin-top:8px; display:none;">
                        <tr>
                            <td class="stat-label">Source Mode</td>
                            <td class="stat-value" id="lblFileSource">RAM Buffer (Direct)</td>
                        </tr>
                        <tr>
                            <td class="stat-label">Filename</td>
                            <td class="stat-value" id="lblFileName">-</td>
                        </tr>
                        <tr>
                            <td class="stat-label">Filesize</td>
                            <td class="stat-value" id="lblFileSize">-</td>
                        </tr>
                    </table>

                    <!-- TARGET PARTITION SELECTOR -->
                    <div style="margin-top:10px; margin-bottom:10px; padding:10px 12px; border:1px solid var(--panel-border); background:var(--code-bg);">
                        <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:6px;">
                            <label class="form-label" style="margin-bottom:0; font-weight:700; color:var(--text-heading);">
                                TARGET STM32 PARTITION:
                            </label>
                            <span id="lblPartitionHint" style="font-size:11px; font-family:var(--font-mono); color:var(--text-muted);">(DUAL-BANK SUGGESTION)</span>
                        </div>
                        <div style="display:flex; gap:16px; align-items:center; flex-wrap:wrap;">
                            <label style="display:inline-flex; align-items:center; gap:6px; cursor:pointer; font-size:12px; font-family:var(--font-mono); color:var(--text-heading);">
                                <input type="radio" name="targetPartition" id="radApp1" value="1" onchange="onPartitionRadioChanged(1)">
                                <span><b>APP 1</b> <span style="color:var(--text-muted); font-size:11px;">(0x08004000, 56KB)</span></span>
                            </label>
                            <label style="display:inline-flex; align-items:center; gap:6px; cursor:pointer; font-size:12px; font-family:var(--font-mono); color:var(--text-heading);">
                                <input type="radio" name="targetPartition" id="radApp2" value="2" checked onchange="onPartitionRadioChanged(2)">
                                <span><b>APP 2</b> <span style="color:var(--text-muted); font-size:11px;">(0x08012000, 56KB)</span></span>
                            </label>
                        </div>
                    </div>

                    <div style="display:flex; align-items:center; justify-content:space-between; margin-top:12px;">
                        <label style="font-size:11px; font-family:var(--font-mono); color:var(--text-muted); cursor:pointer; display:inline-flex; align-items:center; gap:6px;">
                            <input type="checkbox" id="chkInjectCrc"> INJECT CRC ERROR (CHUNK #2)
                        </label>
                        <button id="btnFlash" class="btn btn-primary" onclick="startOtaFlashing()" disabled>
                            FLASH FIRMWARE
                        </button>
                    </div>
                </div>
            </div>

            <!-- FULL WIDTH: PROGRESS & CONSOLE -->
            <div class="panel" style="grid-column: 1 / -1;">
                <div class="panel-heading">
                    <span class="panel-title">FLASH PROGRESS & SYSTEM CONSOLE</span>
                    <button class="btn" onclick="clearLogs()">CLEAR CONSOLE</button>
                </div>
                <div class="panel-body" style="display:flex; flex-direction:column; gap:10px;">
                    <div class="progress-container">
                        <div class="progress-header">
                            <span id="lblProgressStep">STANDBY / READY</span>
                            <span id="lblProgressPct">0%</span>
                        </div>
                        <div class="progress-bar">
                            <div id="progressBarFill" class="progress-bar-fill"></div>
                        </div>
                    </div>

                    <div id="logConsole" class="log-box">
                        <div class="log-line info">[READY] System initialized. Dual-source OTA ready (RAM Buffer / MicroSD).</div>
                    </div>
                </div>
            </div>

        </section>

        <!-- ==================== TAB 2: VEHICLE & ROBOT CONTROL ==================== -->
        <section id="tabControl" class="panel" style="display: none;">
            <div class="panel-heading">
                <span class="panel-title">VEHICLE & ROBOTIC ARM ACTUATOR DECK</span>
                <span id="lblOpcode" style="font-family:var(--font-mono); font-size:12px; color:var(--text-heading); font-weight:800;">
                    STANDBY [0x040]
                </span>
            </div>
            <div class="panel-body">

                <!-- TOP CONTROL TOOLBAR -->
                <div class="control-toolbar">
                    <button class="btn btn-ctrl" id="btnToggleMode" onclick="toggleVehicleMode()">
                        MODE: WHEEL DRIVE [M]
                    </button>
                    <button class="btn btn-ctrl" id="btnHorn">
                        HORN [H]
                    </button>
                    <button class="btn btn-ctrl" id="btnToggleLed" onclick="toggleLedState()">
                        LED PC13: OFF [L]
                    </button>
                    <button class="btn btn-danger btn-ctrl" id="btnEmergencyStop" onclick="triggerEStop()">
                        EMERGENCY STOP [SPACE]
                    </button>
                </div>

                <div class="control-grid">

                    <!-- LEFT: D-PAD MATRIX -->
                    <div class="dpad-card">
                        <div class="dpad-matrix">
                            <!-- ROW 1 -->
                            <button class="btn-pad" id="padNW">
                                <span class="pad-label" id="lblPadNW">NW</span>
                                <span class="pad-key" id="keyPadNW">Q</span>
                            </button>
                            <button class="btn-pad" id="padFWD">
                                <span class="pad-label" id="lblPadFWD">FWD</span>
                                <span class="pad-key" id="keyPadFWD">W</span>
                            </button>
                            <button class="btn-pad" id="padNE">
                                <span class="pad-label" id="lblPadNE">NE</span>
                                <span class="pad-key" id="keyPadNE">E</span>
                            </button>

                            <!-- ROW 2 -->
                            <button class="btn-pad" id="padLEFT">
                                <span class="pad-label" id="lblPadLEFT">LEFT</span>
                                <span class="pad-key" id="keyPadLEFT">A</span>
                            </button>
                            <button class="btn-pad pad-stop" id="padSTOP">
                                <span class="pad-label" id="lblPadSTOP">STOP</span>
                                <span class="pad-key" id="keyPadSTOP">SPACE</span>
                            </button>
                            <button class="btn-pad" id="padRIGHT">
                                <span class="pad-label" id="lblPadRIGHT">RIGHT</span>
                                <span class="pad-key" id="keyPadRIGHT">D</span>
                            </button>

                            <!-- ROW 3 -->
                            <button class="btn-pad" id="padSW">
                                <span class="pad-label" id="lblPadSW">SW</span>
                                <span class="pad-key" id="keyPadSW">Z</span>
                            </button>
                            <button class="btn-pad" id="padREV">
                                <span class="pad-label" id="lblPadREV">REV</span>
                                <span class="pad-key" id="keyPadREV">S</span>
                            </button>
                            <button class="btn-pad" id="padSE">
                                <span class="pad-label" id="lblPadSE">SE</span>
                                <span class="pad-key" id="keyPadSE">C</span>
                            </button>
                        </div>

                        <!-- 100MS HEARTBEAT MONITOR STRIP -->
                        <div class="heartbeat-strip">
                            <span>CADENCE: 100MS HEARTBEAT</span>
                            <div style="display:flex; align-items:center; gap:6px;">
                                <span id="lblHeartbeatStatus">IDLE</span>
                                <span class="heartbeat-dot" id="dotHeartbeat"></span>
                            </div>
                        </div>
                    </div>

                    <!-- RIGHT: TELEMETRY & STATUS -->
                    <div style="display:flex; flex-direction:column; justify-content:space-between; gap:12px;">
                        <table class="stat-table">
                            <tr>
                                <td class="stat-label">Active Mode</td>
                                <td class="stat-value">
                                    <span id="telemetryMode" class="badge badge-active">WHEEL DRIVE (0x001)</span>
                                </td>
                            </tr>
                            <tr>
                                <td class="stat-label">Active Opcode</td>
                                <td class="stat-value">
                                    <span id="telemetryOpcode" class="badge badge-target">0x040</span>
                                </td>
                            </tr>
                            <tr>
                                <td class="stat-label">Motion State</td>
                                <td class="stat-value">
                                    <span id="telemetryAction" style="font-family:var(--font-mono); font-weight:700;">MOTOR_STOP</span>
                                </td>
                            </tr>
                            <tr>
                                <td class="stat-label">ECU Diagnostic LED</td>
                                <td class="stat-value">
                                    <span id="telemetryLed" class="badge">OFF (HIGH, PC13)</span>
                                </td>
                            </tr>
                            <tr>
                                <td class="stat-label">CAN Bus Arbitration</td>
                                <td class="stat-value">0x000 - 0x0C1 (Priority 0)</td>
                            </tr>
                            <tr>
                                <td class="stat-label">Safety Watchdog</td>
                                <td class="stat-value">ACTIVE (AUTO-STOP ON DISCONNECT)</td>
                            </tr>
                        </table>

                        <!-- KEYBOARD GUIDE -->
                        <div style="border:1px solid var(--panel-border); background:var(--code-bg); padding:10px 12px; font-family:var(--font-mono); font-size:11px;">
                            <div style="color:var(--text-heading); font-weight:800; margin-bottom:4px; text-transform:uppercase;">
                                KEYBOARD SHORTCUTS MATRIX:
                            </div>
                            <div style="color:var(--text-muted); line-height:1.6;">
                                [W/S/A/D]: FWD / REV / LEFT / RIGHT (or ARM UP/DN/EXT/RET)<br>
                                [Q/E/Z/C]: DIAGONAL VECTORS (WHEEL MODE)<br>
                                [SPACE]: EMERGENCY STOP | [M]: TOGGLE MODE | [H]: HORN | [L]: LED
                            </div>
                        </div>

                    </div>

                </div>

            </div>
        </section>

    </main>

    <script>
        let currentTheme = localStorage.getItem('theme') || 'dark';
        applyTheme(currentTheme);

        function applyTheme(theme) {
            currentTheme = theme;
            document.documentElement.setAttribute('data-theme', theme);
            localStorage.setItem('theme', theme);
            const btn = document.getElementById('btnThemeToggle');
            if (btn) btn.innerText = 'THEME: ' + theme.toUpperCase();
        }

        function toggleTheme() {
            applyTheme(currentTheme === 'dark' ? 'light' : 'dark');
        }

        let activeApp = 1;
        let selectedFile = null;
        let selectedFileRaw = null;
        let otaSource = 'ram';
        let currentCarMode = 'MOTOR';
        let ledState = false;
        let sdFilesList = [];
        let ws = null;

        let activeHoldCmd = null;
        let activeHoldPadId = null;
        let heartbeatInterval = null;
        let isHolding = false;

        /* WEBSOCKET INITIALIZATION (PORT 81) */
        function initWebSocket() {
            const host = (window.location.hostname && window.location.hostname.length > 0) ? window.location.hostname : '192.168.4.1';
            const wsUrl = 'ws://' + host + ':81/';
            try {
                ws = new WebSocket(wsUrl);
                ws.onopen = function() {
                    const dot = document.getElementById('dotWs');
                    const txt = document.getElementById('txtWsStatus');
                    const pill = document.getElementById('pillWsStatus');
                    if (dot) dot.className = 'indicator online';
                    if (txt) txt.innerText = 'WS: ONLINE';
                    if (pill) pill.classList.add('active');
                    addLog("WebSocket telemetry stream connected (" + wsUrl + ").", "success");
                };
                ws.onmessage = function(event) {
                    try {
                        const data = JSON.parse(event.data);
                        if (data.type === 'ota_progress') {
                            updateProgress(data.percent, data.msg);
                        } else if (data.type === 'ota_query') {
                            if (data.active_app) {
                                updateBadges(data.active_app);
                            }
                        } else if (data.mode) {
                            syncCarTelemetry(data);
                        }
                    } catch (err) {
                        console.error("WS Parse error:", err);
                    }
                };
                ws.onclose = function() {
                    const dot = document.getElementById('dotWs');
                    const txt = document.getElementById('txtWsStatus');
                    const pill = document.getElementById('pillWsStatus');
                    if (dot) dot.className = 'indicator offline';
                    if (txt) txt.innerText = 'WS: OFFLINE';
                    if (pill) pill.classList.remove('active');
                    setTimeout(initWebSocket, 3000);
                };
                ws.onerror = function(err) {
                    const dot = document.getElementById('dotWs');
                    const txt = document.getElementById('txtWsStatus');
                    if (dot) dot.className = 'indicator offline';
                    if (txt) txt.innerText = 'WS: ERROR';
                    console.error("WebSocket error:", err);
                };
            } catch (e) {
                console.error("WebSocket init exception:", e);
            }
        }

        /* OTA SOURCE SWITCHER (RAM BUFFER VS MICROSD) */
        function setOtaSource(source) {
            otaSource = source;
            const bRam = document.getElementById('btnSourceRam');
            const bSd = document.getElementById('btnSourceSd');
            const pRam = document.getElementById('paneSourceRam');
            const pSd = document.getElementById('paneSourceSd');

            if (source === 'ram') {
                bRam.classList.add('active');
                bSd.classList.remove('active');
                pRam.style.display = 'block';
                pSd.style.display = 'none';

                if (selectedFile && !selectedFile.isSd) {
                    document.getElementById('lblFileSource').innerText = 'RAM Buffer (Direct)';
                    document.getElementById('lblFileName').innerText = selectedFile.name;
                    document.getElementById('lblFileSize').innerText = selectedFile.size + " Bytes (" + (selectedFile.size/1024).toFixed(1) + " KB)";
                    document.getElementById('fileInfoTable').style.display = 'table';
                    document.getElementById('btnFlash').disabled = false;
                } else {
                    document.getElementById('fileInfoTable').style.display = 'none';
                    document.getElementById('btnFlash').disabled = true;
                }
                addLog("Switched OTA source mode to DIRECT RAM BUFFER.", "info");
            } else {
                bSd.classList.add('active');
                bRam.classList.remove('active');
                pSd.style.display = 'flex';
                pRam.style.display = 'none';
                onSdFileChanged();
                addLog("Switched OTA source mode to MICROSD STORAGE.", "info");
            }
        }

        function onSdFileChanged() {
            const sel = document.getElementById('selSdFile');
            if (!sel || !sel.value) {
                selectedFile = null;
                document.getElementById('fileInfoTable').style.display = 'none';
                document.getElementById('btnFlash').disabled = true;
                return;
            }

            const name = sel.value;
            const item = sdFilesList.find(f => f.name === name);
            const size = item ? item.size : 0;

            selectedFile = { name: name, size: size, isSd: true };
            document.getElementById('lblFileSource').innerText = 'MicroSD Storage';
            document.getElementById('lblFileName').innerText = name;
            document.getElementById('lblFileSize').innerText = size + " Bytes (" + (size/1024).toFixed(1) + " KB)";
            document.getElementById('fileInfoTable').style.display = 'table';
            document.getElementById('btnFlash').disabled = false;
            addLog("Selected firmware from MicroSD: " + name + " (" + (size/1024).toFixed(1) + " KB).", "info");
        }

        function uploadFileToSd() {
            const input = document.getElementById('fileSdUploadInput');
            if (!input.files || input.files.length === 0) {
                alert("Please select a binary file (.bin) to upload to MicroSD!");
                return;
            }

            const f = input.files[0];
            if (!f.name.toLowerCase().endsWith('.bin')) {
                alert("Only binary files (.bin) can be saved to MicroSD!");
                return;
            }

            const fname = f.name.startsWith('/') ? f.name : ('/' + f.name);
            const fsize = f.size;

            const btn = document.getElementById('btnUploadToSd');
            btn.disabled = true;
            btn.innerText = "SAVING...";

            addLog("Uploading " + fname + " (" + (fsize/1024).toFixed(1) + " KB) to MicroSD...", "info");

            const formData = new FormData();
            formData.append("file", f, f.name);

            fetch('/api/ota/upload_sd', {
                method: 'POST',
                body: formData
            })
            .then(res => res.json())
            .then(data => {
                btn.disabled = false;
                btn.innerText = "SAVE TO SD";
                if (data.success) {
                    addLog("SUCCESS: Stored " + fname + " onto MicroSD card.", "success");
                    input.value = '';
                    rescanSdCard();
                } else {
                    addLog("Upload to SD failed: " + (data.message || "Unknown error"), "err");
                }
            })
            .catch(err => {
                btn.disabled = false;
                btn.innerText = "SAVE TO SD";
                addLog("HTTP upload error: " + err.message, "err");
            });
        }

        function rescanSdCard() {
            addLog("Scanning MicroSD card file system...", "info");
            const sel = document.getElementById('selSdFile');

            fetch('/api/ota/sd_status')
                .then(res => res.json())
                .then(data => {
                    sel.innerHTML = '';
                    if (data.mounted) {
                        sdFilesList = data.files || [];
                        if (sdFilesList.length === 0) {
                            const opt = document.createElement('option');
                            opt.value = "";
                            opt.innerText = "-- No .bin files found on SD --";
                            sel.appendChild(opt);
                            addLog("MicroSD mounted (" + (data.card_type || "SD") + "), but no .bin files found.", "warn");
                        } else {
                            sdFilesList.forEach(f => {
                                const opt = document.createElement('option');
                                opt.value = f.name;
                                opt.innerText = f.name + " (" + (f.size / 1024).toFixed(1) + " KB)";
                                sel.appendChild(opt);
                            });
                            addLog("MicroSD scan complete: " + sdFilesList.length + " binary file(s) detected.", "success");
                        }
                    } else {
                        sdFilesList = [];
                        const opt = document.createElement('option');
                        opt.value = "";
                        opt.innerText = "-- MicroSD Not Detected --";
                        sel.appendChild(opt);
                        addLog("MicroSD card not detected or unmounted.", "warn");
                    }
                    if (otaSource === 'sd') {
                        onSdFileChanged();
                    }
                })
                .catch(err => {
                    addLog("Failed to fetch SD status: " + err.message, "err");
                    if (otaSource === 'sd') {
                        onSdFileChanged();
                    }
                });
        }

        function deleteSelectedSdFile() {
            const sel = document.getElementById('selSdFile');
            if (!sel || !sel.value) {
                alert("Please select a file from MicroSD to delete!");
                return;
            }
            const fname = sel.value;
            if (!confirm("Are you sure you want to delete file " + fname + " on MicroSD?")) return;

            addLog("Deleting " + fname + " from MicroSD...", "info");
            const formData = new FormData();
            formData.append("filename", fname);

            fetch('/api/ota/delete_sd', {
                method: 'POST',
                body: formData
            })
            .then(res => res.json())
            .then(data => {
                if (data.success) {
                    addLog("Successfully deleted " + fname + " from MicroSD.", "success");
                    rescanSdCard();
                } else {
                    addLog("Failed to delete " + fname + ": " + (data.message || "Unknown error"), "err");
                }
            })
            .catch(err => {
                addLog("HTTP error deleting file: " + err.message, "err");
            });
        }

        function switchTab(tab) {
            const tOta = document.getElementById('tabOta');
            const tCtrl = document.getElementById('tabControl');
            const bOta = document.getElementById('tabBtnOta');
            const bCtrl = document.getElementById('tabBtnControl');

            if (tab === 'ota') {
                tOta.style.display = 'grid';
                tCtrl.style.display = 'none';
                bOta.classList.add('active');
                bCtrl.classList.remove('active');
            } else {
                tOta.style.display = 'none';
                tCtrl.style.display = 'block';
                bCtrl.classList.add('active');
                bOta.classList.remove('active');
                queryCarStatus();
            }
        }

        function addLog(msg, type = 'info') {
            const box = document.getElementById('logConsole');
            if (!box) return;
            const line = document.createElement('div');
            line.className = 'log-line ' + type;
            const time = new Date().toTimeString().split(' ')[0];
            line.innerText = '[' + time + '] ' + msg;
            box.appendChild(line);
            box.scrollTop = box.scrollHeight;
        }

        function clearLogs() {
            document.getElementById('logConsole').innerHTML = '';
        }

        function updateProgress(percent, stepText) {
            const bar = document.getElementById('progressBarFill');
            if (bar) bar.style.width = percent + '%';
            const pctEl = document.getElementById('lblProgressPct');
            if (pctEl) pctEl.innerText = percent + '%';
            if (stepText) {
                const stepEl = document.getElementById('lblProgressStep');
                if (stepEl) stepEl.innerText = stepText;
                addLog(stepText, percent === 100 ? 'success' : 'info');
            }
        }

        function updateBadges(app) {
            activeApp = app;
            const bAct = document.getElementById('lblActiveApp');
            if (app === 1) {
                if (bAct) { bAct.className = 'badge badge-active'; bAct.innerText = 'APP 1 (0x08004000)'; }
            } else {
                if (bAct) { bAct.className = 'badge badge-active'; bAct.innerText = 'APP 2 (0x08012000)'; }
            }

            const suggested = (app === 1) ? 2 : 1;
            const r1 = document.getElementById('radApp1');
            const r2 = document.getElementById('radApp2');
            if (suggested === 1 && r1) r1.checked = true;
            if (suggested === 2 && r2) r2.checked = true;
            updateTargetBadge(suggested);
            const hint = document.getElementById('lblPartitionHint');
            if (hint) hint.innerText = '(DUAL-BANK SUGGESTION: APP ' + suggested + ')';
        }

        function onPartitionRadioChanged(appId) {
            updateTargetBadge(appId);
            const hint = document.getElementById('lblPartitionHint');
            if (hint) hint.innerText = '(MANUAL SELECTION: APP ' + appId + ')';
            addLog("Target partition set to App " + appId + (appId === 1 ? " (0x08004000)" : " (0x08012000)"), "info");
        }

        function updateTargetBadge(appId) {
            const bTgt = document.getElementById('lblTargetApp');
            if (bTgt) {
                bTgt.className = 'badge badge-target';
                bTgt.innerText = 'APP ' + appId + (appId === 1 ? ' (0x08004000)' : ' (0x08012000)');
            }
        }

        function queryActiveApp() {
            addLog("Sending CAN ID 0x620 query to STM32 ECU...", "info");
            fetch('/api/ota/query', { method: 'POST' })
                .then(res => res.json())
                .then(data => {
                    if (data.success && data.active_app) {
                        updateBadges(data.active_app);
                        addLog("STM32 response: Active App " + data.active_app + " (0x080" + (data.active_app === 1 ? '04' : '12') + "000)", "success");
                    } else if (data.active_app) {
                        updateBadges(data.active_app);
                        addLog("ECU Telemetry (" + (data.source || "NVS") + "): App " + data.active_app, "warn");
                    } else {
                        addLog("Query failed: " + (data.message || "Timeout / No response"), "err");
                    }
                })
                .catch(err => {
                    addLog("HTTP query error: " + err.message, "err");
                });
        }

        function onFileSelected() {
            const input = document.getElementById('fileInput');
            if (!input.files || input.files.length === 0) return;

            const f = input.files[0];
            if (!f.name.toLowerCase().endsWith('.bin')) {
                alert("Please select a valid binary file (.bin)!");
                input.value = '';
                return;
            }

            if (f.size > 56 * 1024) {
                alert("File size exceeds 56 KB max limit for STM32 partition!");
                input.value = '';
                return;
            }

            selectedFileRaw = f;
            selectedFile = { name: f.name, size: f.size, isSd: false };
            document.getElementById('lblFileSource').innerText = 'RAM Buffer (Direct)';
            document.getElementById('lblFileName').innerText = selectedFile.name;
            document.getElementById('lblFileSize').innerText = selectedFile.size + " Bytes (" + (selectedFile.size/1024).toFixed(1) + " KB)";
            document.getElementById('fileInfoTable').style.display = 'table';
            document.getElementById('btnFlash').disabled = false;
            addLog("Loaded firmware into RAM buffer: " + selectedFile.name + " (" + selectedFile.size + " Bytes).", "info");
        }

        function startOtaFlashing() {
            if (!selectedFile) {
                addLog("No firmware file selected!", "err");
                return;
            }

            const rad = document.querySelector('input[name="targetPartition"]:checked');
            const target = rad ? parseInt(rad.value) : ((activeApp === 1) ? 2 : 1);
            const injectCrc = document.getElementById('chkInjectCrc').checked ? "1" : "0";
            const btn = document.getElementById('btnFlash');
            btn.disabled = true;

            updateProgress(0, "Initiating OTA update sequence to App " + target + "...");

            if (otaSource === 'ram') {
                if (!selectedFileRaw) {
                    addLog("No binary file in memory to upload.", "err");
                    btn.disabled = false;
                    return;
                }

                addLog("Streaming firmware to ESP32 RAM buffer for App " + target + "...", "info");
                const formData = new FormData();
                formData.append("file", selectedFileRaw, selectedFileRaw.name);

                const url = '/api/ota/upload?target_app=' + target + '&test_crc=' + injectCrc;
                fetch(url, { method: 'POST', body: formData })
                .then(res => res.json())
                .then(data => {
                    btn.disabled = false;
                    if (data.success) {
                        updateProgress(100, data.message || "OTA Flashing complete!");
                        addLog("SUCCESS: " + (data.message || "Firmware updated successfully"), "success");
                        updateBadges(target);
                    } else {
                        updateProgress(0, "OTA Flashing Failed: " + (data.message || "Unknown error"));
                        addLog("ERROR: " + (data.message || "Flash failed"), "err");
                    }
                })
                .catch(err => {
                    btn.disabled = false;
                    updateProgress(0, "Network Error");
                    addLog("Flash request error: " + err.message, "err");
                });

            } else if (otaSource === 'sd') {
                const filename = selectedFile.name;
                if (!filename) {
                    addLog("No MicroSD file selected!", "err");
                    btn.disabled = false;
                    return;
                }

                addLog("Flashing from MicroSD into App " + target + ": " + filename + "...", "info");
                const url = '/api/ota/flash_sd?filename=' + encodeURIComponent(filename) + '&target_app=' + target + '&test_crc=' + injectCrc;
                fetch(url, { method: 'POST' })
                .then(res => res.json())
                .then(data => {
                    btn.disabled = false;
                    if (data.success) {
                        updateProgress(100, data.message || "OTA Flashing complete!");
                        addLog("SUCCESS: " + (data.message || "Firmware updated successfully"), "success");
                        updateBadges(target);
                    } else {
                        updateProgress(0, "OTA Flashing Failed: " + (data.message || "Unknown error"));
                        addLog("ERROR: " + (data.message || "Flash failed"), "err");
                    }
                })
                .catch(err => {
                    btn.disabled = false;
                    updateProgress(0, "Network Error");
                    addLog("Flash request error: " + err.message, "err");
                });
            }
        }

        // ==================== VEHICLE & ACTUATOR CONTROL ====================

        function sendCarCommand(cmd) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ cmd: cmd }));
            } else {
                fetch('/api/cmd?cmd=' + encodeURIComponent(cmd), { method: 'POST' }).catch(() => {});
            }
        }

        function queryCarStatus() {
            fetch('/api/status')
                .then(res => res.json())
                .then(data => syncCarTelemetry(data))
                .catch(() => {});
        }

        function syncCarTelemetry(data) {
            if (!data) return;
            if (data.mode) {
                setCarModeUI(data.mode);
            }
            if (data.action) {
                const actEl = document.getElementById('telemetryAction');
                if (actEl) actEl.innerText = data.action;
                const headEl = document.getElementById('lblOpcode');
                if (headEl) headEl.innerText = data.action + (data.last_id ? (' [' + data.last_id + ']') : '');
            }
            if (data.last_id) {
                const idEl = document.getElementById('telemetryOpcode');
                if (idEl) idEl.innerText = data.last_id;
            }
            if (typeof data.led === 'boolean') {
                ledState = data.led;
                updateLedUI(ledState);
            }
            if (typeof data.horn === 'boolean') {
                const btnH = document.getElementById('btnHorn');
                if (btnH) {
                    if (data.horn) btnH.classList.add('active');
                    else btnH.classList.remove('active');
                }
            }
        }

        function setCarModeUI(mode) {
            const m = (mode || '').toUpperCase();
            currentCarMode = (m.includes('SERVO') || m.includes('ARM')) ? 'SERVO' : 'MOTOR';
            const btnMode = document.getElementById('btnToggleMode');
            const badgeMode = document.getElementById('telemetryMode');

            const pNW = document.getElementById('padNW');
            const pNE = document.getElementById('padNE');
            const pSW = document.getElementById('padSW');
            const pSE = document.getElementById('padSE');

            const lblFWD = document.getElementById('lblPadFWD');
            const lblREV = document.getElementById('lblPadREV');
            const lblLEFT = document.getElementById('lblPadLEFT');
            const lblRIGHT = document.getElementById('lblPadRIGHT');
            const lblSTOP = document.getElementById('lblPadSTOP');

            if (currentCarMode === 'MOTOR') {
                if (btnMode) btnMode.innerText = 'MODE: WHEEL DRIVE [M]';
                if (badgeMode) {
                    badgeMode.className = 'badge badge-active';
                    badgeMode.innerText = 'WHEEL DRIVE (0x001)';
                }
                if (pNW) pNW.classList.remove('pad-disabled');
                if (pNE) pNE.classList.remove('pad-disabled');
                if (pSW) pSW.classList.remove('pad-disabled');
                if (pSE) pSE.classList.remove('pad-disabled');

                if (lblFWD) lblFWD.innerText = 'FWD';
                if (lblREV) lblREV.innerText = 'REV';
                if (lblLEFT) lblLEFT.innerText = 'LEFT';
                if (lblRIGHT) lblRIGHT.innerText = 'RIGHT';
                if (lblSTOP) lblSTOP.innerText = 'STOP';
            } else {
                if (btnMode) btnMode.innerText = 'MODE: ROBOTIC ARM [M]';
                if (badgeMode) {
                    badgeMode.className = 'badge badge-amber';
                    badgeMode.innerText = 'ROBOTIC ARM (0x002)';
                }
                if (pNW) pNW.classList.add('pad-disabled');
                if (pNE) pNE.classList.add('pad-disabled');
                if (pSW) pSW.classList.add('pad-disabled');
                if (pSE) pSE.classList.add('pad-disabled');

                if (lblFWD) lblFWD.innerText = 'ARM UP';
                if (lblREV) lblREV.innerText = 'ARM DOWN';
                if (lblLEFT) lblLEFT.innerText = 'ARM EXT';
                if (lblRIGHT) lblRIGHT.innerText = 'ARM RET';
                if (lblSTOP) lblSTOP.innerText = 'HOLD';
            }
        }

        function toggleVehicleMode() {
            stopHoldAction();
            const nextModeCode = (currentCarMode === 'MOTOR') ? '0x002' : '0x001';
            sendCarCommand(nextModeCode);
            setCarModeUI(currentCarMode === 'MOTOR' ? 'SERVO' : 'MOTOR');
            addLog("Switched control mode to " + (currentCarMode === 'SERVO' ? "ROBOTIC ARM (0x002)" : "WHEEL DRIVE (0x001)"), "info");
        }

        function updateLedUI(on) {
            const btn = document.getElementById('btnToggleLed');
            const badge = document.getElementById('telemetryLed');
            if (on) {
                if (btn) btn.innerText = 'LED PC13: ON [L]';
                if (badge) { badge.className = 'badge badge-active'; badge.innerText = 'ON (LOW, PC13)'; }
            } else {
                if (btn) btn.innerText = 'LED PC13: OFF [L]';
                if (badge) { badge.className = 'badge'; badge.innerText = 'OFF (HIGH, PC13)'; }
            }
        }

        function toggleLedState() {
            ledState = !ledState;
            const cmd = ledState ? '0x003' : '0x004';
            sendCarCommand(cmd);
            updateLedUI(ledState);
            addLog("Diagnostic LED: " + (ledState ? "ON [0x003]" : "OFF [0x004]"), "info");
        }

        function triggerEStop() {
            stopHoldAction();
            sendCarCommand('0x000');
            clearActivePads();
            const btnStop = document.getElementById('padSTOP');
            if (btnStop) btnStop.classList.add('active');
            setTimeout(() => { if (btnStop) btnStop.classList.remove('active'); }, 300);

            const headEl = document.getElementById('lblOpcode');
            if (headEl) headEl.innerText = 'E-STOP [0x000]';
            const actEl = document.getElementById('telemetryAction');
            if (actEl) actEl.innerText = 'EMERGENCY_STOP';
            const idEl = document.getElementById('telemetryOpcode');
            if (idEl) idEl.innerText = '0x000';
            addLog("EMERGENCY STOP (0x000) broadcast to CAN Bus! All actuators halted.", "err");
        }

        function startHoldAction(cmd, padId) {
            if (!cmd) return;
            if (isHolding && activeHoldCmd === cmd) return;

            activeHoldCmd = cmd;
            activeHoldPadId = padId;
            isHolding = true;

            clearActivePads();
            const padEl = document.getElementById(padId);
            if (padEl) padEl.classList.add('active');

            sendCarCommand(cmd);

            const idEl = document.getElementById('telemetryOpcode');
            if (idEl) idEl.innerText = cmd;

            const dot = document.getElementById('dotHeartbeat');
            const status = document.getElementById('lblHeartbeatStatus');
            if (dot) dot.classList.add('pulsing');
            if (status) status.innerText = 'HOLDING [' + cmd + ']';

            if (heartbeatInterval) clearInterval(heartbeatInterval);
            heartbeatInterval = setInterval(() => {
                if (isHolding && activeHoldCmd) {
                    sendCarCommand(activeHoldCmd);
                    if (dot) {
                        dot.classList.toggle('pulsing');
                    }
                }
            }, 100);
        }

        function stopHoldAction() {
            if (!isHolding && !activeHoldCmd) return;

            if (heartbeatInterval) {
                clearInterval(heartbeatInterval);
                heartbeatInterval = null;
            }

            clearActivePads();

            const stopCmd = (currentCarMode === 'SERVO') ? '0x080' : '0x040';
            sendCarCommand(stopCmd);

            activeHoldCmd = null;
            activeHoldPadId = null;
            isHolding = false;

            const dot = document.getElementById('dotHeartbeat');
            const status = document.getElementById('lblHeartbeatStatus');
            if (dot) dot.classList.remove('pulsing');
            if (status) status.innerText = 'IDLE';

            const idEl = document.getElementById('telemetryOpcode');
            if (idEl) idEl.innerText = stopCmd;
            const actEl = document.getElementById('telemetryAction');
            if (actEl) actEl.innerText = (currentCarMode === 'SERVO') ? 'ARM_HOLD' : 'MOTOR_STOP';
            const headEl = document.getElementById('lblOpcode');
            if (headEl) headEl.innerText = (currentCarMode === 'SERVO' ? 'HOLD' : 'STOP') + ' [' + stopCmd + ']';
        }

        function clearActivePads() {
            document.querySelectorAll('.btn-pad').forEach(b => b.classList.remove('active'));
        }

        function getPadCommand(direction) {
            if (currentCarMode === 'MOTOR') {
                switch (direction) {
                    case 'FWD': return '0x041';
                    case 'REV': return '0x042';
                    case 'LEFT': return '0x043';
                    case 'RIGHT': return '0x044';
                    case 'NW': return '0x047';
                    case 'SW': return '0x046';
                    case 'NE': return '0x045';
                    case 'SE': return '0x048';
                    default: return '0x040';
                }
            } else {
                switch (direction) {
                    case 'FWD': return '0x081';
                    case 'REV': return '0x082';
                    case 'LEFT': return '0x083';
                    case 'RIGHT': return '0x084';
                    default: return '0x080';
                }
            }
        }

        function onHornPress(isDown) {
            const btn = document.getElementById('btnHorn');
            if (isDown) {
                if (btn) btn.classList.add('active');
                sendCarCommand('0x0C1');
                addLog("[ACTUATOR] HORN ACTIVE [0x0C1]", "info");
            } else {
                if (btn) btn.classList.remove('active');
                sendCarCommand('0x0C0');
            }
        }

        function bindPadPointerEvents(padId, direction) {
            const el = document.getElementById(padId);
            if (!el) return;

            el.addEventListener('pointerdown', (e) => {
                if (el.classList.contains('pad-disabled')) return;
                e.preventDefault();
                try { el.setPointerCapture(e.pointerId); } catch(ex) {}
                const cmd = getPadCommand(direction);
                startHoldAction(cmd, padId);
            });

            const handleRelease = (e) => {
                if (el.classList.contains('pad-disabled')) return;
                try { el.releasePointerCapture(e.pointerId); } catch(ex) {}
                stopHoldAction();
            };

            el.addEventListener('pointerup', handleRelease);
            el.addEventListener('pointercancel', handleRelease);
            el.addEventListener('pointerleave', (e) => {
                if (isHolding && activeHoldPadId === padId) {
                    stopHoldAction();
                }
            });
        }

        bindPadPointerEvents('padFWD', 'FWD');
        bindPadPointerEvents('padREV', 'REV');
        bindPadPointerEvents('padLEFT', 'LEFT');
        bindPadPointerEvents('padRIGHT', 'RIGHT');
        bindPadPointerEvents('padNW', 'NW');
        bindPadPointerEvents('padNE', 'NE');
        bindPadPointerEvents('padSW', 'SW');
        bindPadPointerEvents('padSE', 'SE');

        const padStopEl = document.getElementById('padSTOP');
        if (padStopEl) {
            padStopEl.addEventListener('pointerdown', (e) => {
                e.preventDefault();
                stopHoldAction();
            });
        }

        const btnHornEl = document.getElementById('btnHorn');
        if (btnHornEl) {
            btnHornEl.addEventListener('pointerdown', (e) => {
                e.preventDefault();
                try { btnHornEl.setPointerCapture(e.pointerId); } catch(ex) {}
                onHornPress(true);
            });
            const releaseHorn = (e) => {
                try { btnHornEl.releasePointerCapture(e.pointerId); } catch(ex) {}
                onHornPress(false);
            };
            btnHornEl.addEventListener('pointerup', releaseHorn);
            btnHornEl.addEventListener('pointercancel', releaseHorn);
            btnHornEl.addEventListener('pointerleave', releaseHorn);
        }

        const keyToDirection = {
            'W': 'FWD', 'ARROWUP': 'FWD',
            'S': 'REV', 'ARROWDOWN': 'REV',
            'A': 'LEFT', 'ARROWLEFT': 'LEFT',
            'D': 'RIGHT', 'ARROWRIGHT': 'RIGHT',
            'Q': 'NW', 'E': 'NE', 'Z': 'SW', 'C': 'SE'
        };

        const dirToPadId = {
            'FWD': 'padFWD', 'REV': 'padREV', 'LEFT': 'padLEFT', 'RIGHT': 'padRIGHT',
            'NW': 'padNW', 'NE': 'padNE', 'SW': 'padSW', 'SE': 'padSE'
        };

        window.addEventListener('keydown', (e) => {
            if (e.target && (e.target.tagName === 'INPUT' || e.target.tagName === 'SELECT')) return;

            const k = e.key.toUpperCase();

            if (k === 'M' && !e.repeat) {
                e.preventDefault();
                toggleVehicleMode();
                return;
            }
            if (k === 'H') {
                if (!e.repeat) {
                    e.preventDefault();
                    onHornPress(true);
                }
                return;
            }
            if (k === 'L' && !e.repeat) {
                e.preventDefault();
                toggleLedState();
                return;
            }
            if (k === ' ' || k === 'SPACE') {
                e.preventDefault();
                triggerEStop();
                return;
            }

            const dir = keyToDirection[k];
            if (dir) {
                e.preventDefault();
                if (e.repeat) return;
                if (currentCarMode === 'SERVO' && ['NW','NE','SW','SE'].includes(dir)) return;
                const cmd = getPadCommand(dir);
                startHoldAction(cmd, dirToPadId[dir]);
            }
        });

        window.addEventListener('keyup', (e) => {
            if (e.target && (e.target.tagName === 'INPUT' || e.target.tagName === 'SELECT')) return;
            const k = e.key.toUpperCase();
            if (k === 'H') {
                e.preventDefault();
                onHornPress(false);
                return;
            }
            if (keyToDirection[k]) {
                e.preventDefault();
                stopHoldAction();
            }
        });

        window.addEventListener('DOMContentLoaded', () => {
            initWebSocket();
            queryActiveApp();
            rescanSdCard();
            queryCarStatus();
        });
    </script>
</body>
</html>
)rawliteral";

#endif
