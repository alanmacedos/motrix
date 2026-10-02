#pragma once
// Generated from src/index.html and src/style.css by embed_web.py.
const char MOTRIX_HTML[] PROGMEM = R"MOTRIXHTML(
<!doctype html>
<html lang="pt-BR">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
  <meta name="theme-color" content="#0b1118">
  <title>MOTRIX — Controle</title>
  <link rel="stylesheet" href="style.css">
</head>
<body>
  <main class="controller" aria-label="Controle virtual MOTRIX">
    <header class="topbar">
      <div class="brand"><span class="brand-mark" aria-hidden="true">M</span><span>MOTRIX</span></div>
      <div class="connection" id="connection-status" data-state="offline"><span class="status-dot" aria-hidden="true"></span><span id="connection-label">OFFLINE</span></div>
    </header>

    <section class="controls" aria-label="Controles de direção e aceleração">
      <section class="control-panel steering-panel" aria-labelledby="steering-title">
        <div class="panel-heading">
          <div><span class="eyebrow">CONTROLE 01</span><h1 id="steering-title">Direção</h1></div>
          <span class="axis-icon" aria-hidden="true">↔</span>
        </div>
        <div class="steering-readout"><span>STEERING</span><output id="steering-value" for="steering">0</output><span class="unit">%</span></div>
        <div class="range-wrap steering-wrap">
          <span class="range-end">ESQUERDA</span>
          <input id="steering" name="steering" type="range" min="-100" max="100" value="0" aria-label="Direção: esquerda ou direita">
          <span class="range-end">DIREITA</span>
        </div>
        <div class="range-scale" aria-hidden="true"><span>−100</span><span class="scale-center">0</span><span>+100</span></div>
      </section>

      <section class="control-panel throttle-panel" aria-labelledby="throttle-title">
        <div class="panel-heading">
          <div><span class="eyebrow">CONTROLE 02</span><h2 id="throttle-title">Aceleração</h2></div>
          <span class="axis-icon" aria-hidden="true">↕</span>
        </div>
        <div class="throttle-area">
          <div class="throttle-scale" aria-hidden="true"><span>100</span><span>50</span><span>0</span></div>
          <input id="throttle" name="throttle" type="range" min="0" max="100" value="0" aria-label="Aceleração de zero a cem por cento">
          <div class="throttle-value"><output id="throttle-value" for="throttle">0</output><span>THROTTLE&nbsp; %</span></div>
        </div>
      </section>
    </section>

    <footer class="bottom-bar">
      <fieldset class="drive-selector">
        <legend>TRAÇÃO</legend>
        <div class="drive-options">
          <input type="radio" name="drive" id="driveFWD" value="FWD">
          <label for="driveFWD">FWD</label>
          <input type="radio" name="drive" id="drive4x4" value="4x4" checked>
          <label for="drive4x4">4×4</label>
          <input type="radio" name="drive" id="driveRWD" value="RWD">
          <label for="driveRWD">RWD</label>
        </div>
      </fieldset>
      <button class="stop-button" id="stop-button" type="button" aria-label="Parar os motores do carrinho">
        <span class="stop-icon" aria-hidden="true">■</span><span>STOP</span>
      </button>
    </footer>
  </main>
 <script>
    const steering = document.querySelector('#steering');
    const throttle = document.querySelector('#throttle');

    const steeringValue = document.querySelector('#steering-value');
    const throttleValue = document.querySelector('#throttle-value');

    const status = document.querySelector('#connection-status');
    const statusLabel = document.querySelector('#connection-label');

    const stopButton = document.querySelector('#stop-button');

    function setConnection(online) {
        status.dataset.state = online ? 'online' : 'offline';
        statusLabel.textContent = online ? 'CONNECTED' : 'OFFLINE';
    }

    function currentCommand() {
        return {
            steering: Number(steering.value),
            throttle: Number(throttle.value),
            driveMode: document.querySelector(
                'input[name="drive"]:checked'
            ).value
        };
    }

    function sendCommand() {
        const command = currentCommand();

        console.log('Comando MOTRIX:', command);
    }

    function stopMotors() {
        steering.value = '0';
        throttle.value = '0';

        steeringValue.value = '0';
        throttleValue.value = '0';

        const command = {
            steering: 0,
            throttle: 0,
            driveMode: document.querySelector(
                'input[name="drive"]:checked'
            ).value
        };

        console.log('STOP:', command);
    }

    steering.addEventListener('input', () => {
        steeringValue.value = steering.value;
        sendCommand();
    });

    throttle.addEventListener('input', () => {
        throttleValue.value = throttle.value;
        sendCommand();
    });

    document
        .querySelectorAll('input[name="drive"]')
        .forEach((input) => {
            input.addEventListener('change', sendCommand);
        });

    stopButton.addEventListener('click', stopMotors);

    // Nesta fase estamos testando apenas a interface.
    setConnection(false);

    // Mostra o estado inicial no console.
    console.log('MOTRIX iniciado.');
    console.log('Comando inicial:', currentCommand());
</script>
</body>
</html>

)MOTRIXHTML";

const char MOTRIX_CSS[] PROGMEM = R"MOTRIXCSS(
:root {
  color-scheme: dark;
  font-family: Inter, "Segoe UI", Roboto, Arial, sans-serif;
  background: #0b1118;
  color: #edf3f8;
  --muted: #8796a4;
  --line: #283541;
  --panel: #111a23;
  --accent: #b9f34a;
}

* { box-sizing: border-box; }

html, body { width: 100%; min-width: 320px; min-height: 100%; margin: 0; }
body { min-height: 100vh; min-height: 100dvh; background: radial-gradient(ellipse at 50% -20%, #1c2a35 0, #0b1118 60%); }
button, input { font: inherit; }

.controller {
  width: min(100%, 1100px);
  min-height: 100vh;
  min-height: 100dvh;
  margin: 0 auto;
  padding: max(14px, env(safe-area-inset-top)) max(18px, env(safe-area-inset-right)) max(14px, env(safe-area-inset-bottom)) max(18px, env(safe-area-inset-left));
  display: grid;
  grid-template-rows: auto minmax(0, 1fr) auto;
  gap: clamp(12px, 2.2vh, 22px);
}

.topbar, .brand, .connection, .panel-heading, .steering-readout, .bottom-bar, .drive-options, .stop-button { display: flex; align-items: center; }
.topbar { justify-content: space-between; min-height: 38px; }
.brand { gap: 10px; font-size: 1rem; font-weight: 850; letter-spacing: .22em; }
.brand-mark { display: grid; width: 29px; aspect-ratio: 1; place-items: center; border: 1px solid var(--accent); border-radius: 8px; color: var(--accent); font-size: .8rem; letter-spacing: 0; }
.connection { gap: 9px; color: #a7b4bf; font-size: .68rem; font-weight: 750; letter-spacing: .14em; }
.status-dot { width: 8px; aspect-ratio: 1; border-radius: 50%; background: #8796a4; box-shadow: 0 0 10px #8796a455; }
.connection[data-state="online"] { color: var(--accent); }
.connection[data-state="online"] .status-dot { background: var(--accent); box-shadow: 0 0 12px #b9f34a88; }

.controls { min-height: 0; display: grid; grid-template-columns: 1fr 1fr; gap: clamp(12px, 2vw, 22px); }
.control-panel { position: relative; min-width: 0; overflow: hidden; padding: clamp(16px, 3vh, 28px) clamp(16px, 3vw, 34px); border: 1px solid var(--line); border-radius: 20px; background: linear-gradient(145deg, #141f29, var(--panel)); box-shadow: 0 16px 45px #0003, inset 0 1px #ffffff08; }
.panel-heading { justify-content: space-between; }
.eyebrow, .drive-selector legend { color: var(--muted); font-size: .62rem; font-weight: 750; letter-spacing: .16em; }
h1, h2 { margin: 5px 0 0; font-size: clamp(1.1rem, 2.5vw, 1.55rem); font-weight: 650; letter-spacing: -.03em; }
.axis-icon { color: #71808d; font-size: 1.4rem; }
.steering-readout { margin-top: clamp(20px, 5vh, 48px); gap: 8px; color: var(--muted); font-size: .68rem; font-weight: 750; letter-spacing: .12em; }
.steering-readout output, .throttle-value output { color: var(--accent); font-variant-numeric: tabular-nums; font-size: clamp(2rem, 5vw, 3.5rem); font-weight: 650; letter-spacing: -.06em; }
.steering-readout .unit { align-self: flex-end; padding-bottom: 7px; }
.range-wrap { display: grid; align-items: center; gap: 12px; }
.steering-wrap { margin-top: clamp(22px, 6vh, 56px); grid-template-columns: auto minmax(0, 1fr) auto; }
.range-end { color: #a1adb7; font-size: .58rem; font-weight: 750; letter-spacing: .08em; }
input[type="range"] { width: 100%; height: 34px; margin: 0; appearance: none; background: transparent; touch-action: pan-x; cursor: pointer; }
input[type="range"]::-webkit-slider-runnable-track { height: 6px; border-radius: 9px; background: #35434e; }
input[type="range"]::-moz-range-track { height: 6px; border-radius: 9px; background: #35434e; }
input[type="range"]::-moz-range-progress { height: 6px; border-radius: 9px; background: var(--accent); }
input[type="range"]::-webkit-slider-thumb { width: 24px; height: 24px; margin-top: -9px; appearance: none; border: 5px solid #d8f8a0; border-radius: 50%; background: var(--accent); box-shadow: 0 0 0 5px #b9f34a22, 0 2px 12px #0008; }
input[type="range"]::-moz-range-thumb { width: 15px; height: 15px; border: 5px solid #d8f8a0; border-radius: 50%; background: var(--accent); box-shadow: 0 0 0 5px #b9f34a22; }
input[type="range"]:focus-visible { outline: 2px solid var(--accent); outline-offset: 8px; border-radius: 9px; }
.range-scale { display: flex; justify-content: space-between; margin: 0 1px; color: #74828e; font-size: .62rem; font-variant-numeric: tabular-nums; }
.scale-center { transform: translateX(1px); }

.throttle-panel { display: flex; flex-direction: column; }
.throttle-area { flex: 1; min-height: 0; display: flex; align-items: center; justify-content: center; gap: clamp(14px, 3vw, 34px); padding-top: 8px; }
.throttle-scale { align-self: stretch; display: flex; flex-direction: column; justify-content: space-between; padding: 15px 0; color: #74828e; font-size: .62rem; font-variant-numeric: tabular-nums; }
#throttle { width: min(30vh, 260px); height: 36px; transform: rotate(-90deg); touch-action: pan-y; }
#throttle::-webkit-slider-runnable-track { background: #35434e; }
#throttle::-moz-range-progress { background: var(--accent); }
.throttle-value { min-width: 74px; display: flex; flex-direction: column; gap: 3px; }
.throttle-value output { font-size: clamp(2rem, 5vw, 3.2rem); }
.throttle-value span { color: var(--muted); font-size: .56rem; font-weight: 750; letter-spacing: .1em; }

.bottom-bar { justify-content: space-between; gap: 16px; }
.drive-selector { min-width: 0; margin: 0; padding: 0; border: 0; }
.drive-selector legend { margin-bottom: 8px; padding: 0; }
.drive-options { gap: 7px; }
.drive-options input { position: absolute; width: 1px; height: 1px; opacity: 0; }
.drive-options label { min-width: clamp(54px, 12vw, 82px); min-height: 42px; display: grid; place-items: center; border: 1px solid var(--line); border-radius: 11px; background: #121b24; color: #abb7c0; font-size: .72rem; font-weight: 800; letter-spacing: .08em; cursor: pointer; transition: color .16s, background .16s, border-color .16s, transform .16s; }
.drive-options label:active, .stop-button:active { transform: scale(.97); }
.drive-options input:checked + label { border-color: #b9f34a99; background: #b9f34a17; color: var(--accent); box-shadow: inset 0 0 18px #b9f34a0b; }
.drive-options input:focus-visible + label { outline: 2px solid var(--accent); outline-offset: 3px; }
.stop-button { min-width: clamp(100px, 22vw, 160px); min-height: 52px; justify-content: center; gap: 10px; border: 1px solid #ff686866; border-radius: 13px; background: linear-gradient(135deg, #e84a4a, #b72e37); color: white; box-shadow: 0 7px 22px #df343426, inset 0 1px #ffffff24; font-size: .78rem; font-weight: 850; letter-spacing: .14em; cursor: pointer; transition: transform .16s, filter .16s; }
.stop-button:hover { filter: brightness(1.1); }
.stop-icon { font-size: .8rem; }

@media (orientation: landscape) and (min-width: 600px) {
  .controller { padding-top: max(10px, env(safe-area-inset-top)); padding-bottom: max(10px, env(safe-area-inset-bottom)); gap: clamp(8px, 1.5vh, 15px); }
  .topbar { min-height: 30px; }
  .control-panel { padding-top: clamp(12px, 2vh, 20px); padding-bottom: clamp(12px, 2vh, 20px); }
  .steering-readout { margin-top: clamp(12px, 3vh, 28px); }
  .steering-wrap { margin-top: clamp(12px, 3vh, 28px); }
}

@media (orientation: portrait) {
  .controller { min-height: 100dvh; }
  .controls { grid-template-columns: 1fr; grid-template-rows: minmax(220px, 1fr) minmax(230px, 1fr); }
  .steering-wrap { margin-top: 22px; }
  .steering-readout { margin-top: 18px; }
  #throttle { width: min(27vh, 220px); }
}

@media (max-height: 430px) and (orientation: landscape) {
  .controller { gap: 7px; padding-top: 7px; padding-bottom: 7px; }
  .topbar { min-height: 25px; }
  .brand-mark { width: 23px; border-radius: 6px; }
  .control-panel { padding: 10px 18px; border-radius: 15px; }
  .steering-readout { margin-top: 8px; }
  .steering-wrap { margin-top: 7px; }
  .bottom-bar { gap: 8px; }
  .drive-selector legend { margin-bottom: 4px; }
  .drive-options label { min-height: 36px; }
  .stop-button { min-height: 43px; }
}

@media (prefers-reduced-motion: reduce) { *, *::before, *::after { scroll-behavior: auto !important; transition-duration: .01ms !important; } }

)MOTRIXCSS";
