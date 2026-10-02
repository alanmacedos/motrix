#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// ============================================================
// MOTRIX - CONTROLE RC COM ESP32
// Versao: 1.0
//
// O ESP32 cria sua propria rede Wi-Fi:
// SSID: Carrinho12345
// Senha: 12345678
//
// A interface sera acessada em:
// http://192.168.4.1
//
// IMPORTANTE:
// - Esta versao usa saidas digitais para os motores.
// - Throttle > 0 significa motor ligado.
// - O valor de throttle ainda NAO controla PWM/velocidade.
// ============================================================


// ============================================================
// CONFIGURACAO DO WI-FI
// ============================================================

const char* WIFI_SSID = "Carrinho12345";
const char* WIFI_PASSWORD = "12345678";

IPAddress AP_IP(192, 168, 4, 1);
IPAddress AP_GATEWAY(192, 168, 4, 1);
IPAddress AP_SUBNET(255, 255, 255, 0);


// ============================================================
// SERVIDOR HTTP
// ============================================================

WebServer server(80);


// ============================================================
// PINOS DOS MOTORES
// ============================================================
//
// ATENCAO:
// Mantidos de acordo com o seu codigo original.
//
// motorEsquerdoF       = lado esquerdo / sentido frente
// motorDireitoF        = lado direito / sentido frente
// motorEsquerdoTras    = lado esquerdo / sentido tras
// motorDireitoTras     = lado direito / sentido tras
//
// Pelo comportamento do seu codigo original, estes parecem ser
// pinos de direcao dos motores/H-bridge, e nao necessariamente
// "eixo dianteiro/traseiro".
// ============================================================

const uint8_t motorEsquerdoF   = 26;
const uint8_t motorDireitoF    = 27;
const uint8_t motorEsquerdoTras = 14;
const uint8_t motorDireitoTras  = 12;


// ============================================================
// CONFIGURACAO DO CONTROLE
// ============================================================

const int STEERING_MIN = -100;
const int STEERING_MAX = 100;

const int THROTTLE_MIN = 0;
const int THROTTLE_MAX = 100;

const int STEERING_DEADBAND = 20;

// Se nenhum comando chegar durante este tempo,
// os motores sao desligados.
const unsigned long COMMAND_TIMEOUT_MS = 700;


// ============================================================
// ESTADO ATUAL
// ============================================================

int steeringAtual = 0;
int throttleAtual = 0;

String driveModeAtual = "4x4";

unsigned long ultimoComandoMs = 0;

bool motoresAtivos = false;


// ============================================================
// HTML DA INTERFACE
// ============================================================

const char MOTRIX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>

<html lang="pt-BR">

<head>

  <meta charset="utf-8">

  <meta
    name="viewport"
    content="width=device-width, initial-scale=1, viewport-fit=cover"
  >

  <meta
    name="theme-color"
    content="#0b1118"
  >

  <title>MOTRIX — Controle</title>

  <style>

    /* ========================================================
       RESET
       ======================================================== */

    * {
      box-sizing: border-box;
      -webkit-tap-highlight-color: transparent;
    }

    html,
    body {
      margin: 0;
      padding: 0;
      width: 100%;
      min-height: 100%;
      background: #080d13;
      color: #e9f0f6;
      font-family:
        Inter,
        system-ui,
        -apple-system,
        BlinkMacSystemFont,
        "Segoe UI",
        sans-serif;
    }

    body {
      min-height: 100vh;
      overflow: hidden;
      user-select: none;
      -webkit-user-select: none;
    }


    /* ========================================================
       VARIAVEIS
       ======================================================== */

    :root {
      --bg: #080d13;
      --panel: #0e151e;
      --panel-2: #111b25;
      --line: #263442;
      --line-strong: #334656;
      --text: #e9f0f6;
      --muted: #82909d;
      --accent: #35e58b;
      --accent-dark: #1e9b5b;
      --danger: #ff4757;
    }


    /* ========================================================
       CONTAINER PRINCIPAL
       ======================================================== */

    .controller {
      width: 100%;
      min-height: 100vh;
      padding:
        max(14px, env(safe-area-inset-top))
        max(16px, env(safe-area-inset-right))
        max(14px, env(safe-area-inset-bottom))
        max(16px, env(safe-area-inset-left));

      display: grid;

      grid-template-rows:
        auto
        1fr
        auto;

      gap: 14px;
    }


    /* ========================================================
       TOPBAR
       ======================================================== */

    .topbar {
      display: flex;
      align-items: center;
      justify-content: space-between;

      min-height: 48px;

      border-bottom: 1px solid var(--line);
      padding-bottom: 10px;
    }

    .brand {
      display: flex;
      align-items: center;
      gap: 10px;

      font-size: 18px;
      font-weight: 800;
      letter-spacing: 0.16em;
    }

    .brand-mark {
      width: 28px;
      height: 28px;

      display: grid;
      place-items: center;

      border: 1px solid var(--accent);
      border-radius: 7px;

      color: var(--accent);

      font-size: 13px;
      font-weight: 900;
    }


    /* ========================================================
       STATUS DA CONEXAO
       ======================================================== */

    .connection {
      display: flex;
      align-items: center;
      gap: 8px;

      font-size: 11px;
      font-weight: 800;
      letter-spacing: 0.12em;
    }

    .status-dot {
      width: 9px;
      height: 9px;

      border-radius: 50%;

      background: #687580;

      box-shadow: 0 0 0 3px rgba(104, 117, 128, 0.12);
    }

    .connection[data-state="online"] .status-dot {
      background: var(--accent);

      box-shadow:
        0 0 0 3px rgba(53, 229, 139, 0.12),
        0 0 14px rgba(53, 229, 139, 0.45);
    }

    .connection[data-state="offline"] {
      color: #8b98a3;
    }

    .connection[data-state="online"] {
      color: var(--accent);
    }


    /* ========================================================
       AREA DOS CONTROLES
       ======================================================== */

    .controls {
      min-height: 0;

      display: grid;
      grid-template-columns: 1fr 1fr;

      gap: 14px;
    }


    /* ========================================================
       PAINEIS
       ======================================================== */

    .control-panel {
      min-width: 0;
      min-height: 0;

      display: flex;
      flex-direction: column;

      background:
        linear-gradient(
          180deg,
          rgba(17, 27, 37, 0.98),
          rgba(10, 16, 23, 0.98)
        );

      border: 1px solid var(--line);
      border-radius: 16px;

      padding: 18px;

      box-shadow:
        0 12px 35px rgba(0, 0, 0, 0.2),
        inset 0 1px 0 rgba(255,255,255,0.02);
    }


    /* ========================================================
       TITULOS
       ======================================================== */

    .panel-heading {
      display: flex;
      align-items: flex-start;
      justify-content: space-between;

      gap: 12px;

      margin-bottom: 15px;
    }

    .eyebrow {
      display: block;

      margin-bottom: 4px;

      color: var(--muted);

      font-size: 10px;
      font-weight: 800;

      letter-spacing: 0.18em;
    }

    h1,
    h2 {
      margin: 0;

      font-size: clamp(20px, 3vw, 30px);
      line-height: 1;

      letter-spacing: -0.03em;
    }

    .axis-icon {
      color: var(--accent);

      font-size: 24px;
      line-height: 1;

      opacity: 0.85;
    }


    /* ========================================================
       READOUT STEERING
       ======================================================== */

    .steering-readout {
      display: flex;
      align-items: baseline;

      gap: 6px;

      margin-bottom: 14px;
    }

    .steering-readout > span:first-child {
      color: var(--muted);

      font-size: 11px;
      font-weight: 800;

      letter-spacing: 0.12em;
    }

    .steering-readout output {
      margin-left: auto;

      font-size: clamp(28px, 6vw, 46px);
      line-height: 1;

      font-variant-numeric: tabular-nums;

      color: var(--accent);

      font-weight: 800;
    }

    .unit {
      color: var(--muted);

      font-size: 12px;
      font-weight: 700;
    }


    /* ========================================================
       STEERING
       ======================================================== */

    .range-wrap {
      display: grid;

      align-items: center;

      gap: 12px;
    }

    .steering-wrap {
      grid-template-columns: auto 1fr auto;

      margin-top: auto;
    }

    .range-end {
      color: var(--muted);

      font-size: 10px;
      font-weight: 800;

      letter-spacing: 0.08em;

      white-space: nowrap;
    }


    /* ========================================================
       RANGE
       ======================================================== */

    input[type="range"] {
      appearance: none;
      -webkit-appearance: none;

      width: 100%;

      margin: 0;

      background: transparent;

      cursor: pointer;
      touch-action: none;
    }

    input[type="range"]::-webkit-slider-runnable-track {
      height: 10px;

      border-radius: 999px;

      background: #202c37;

      border: 1px solid #2b3a47;
    }

    input[type="range"]::-webkit-slider-thumb {
      appearance: none;
      -webkit-appearance: none;

      width: 28px;
      height: 28px;

      margin-top: -10px;

      border-radius: 50%;

      background: var(--accent);

      border: 3px solid #08100c;

      box-shadow:
        0 0 0 1px rgba(53,229,139,0.9),
        0 4px 14px rgba(0,0,0,0.4);
    }

    input[type="range"]::-moz-range-track {
      height: 10px;

      border-radius: 999px;

      background: #202c37;

      border: 1px solid #2b3a47;
    }

    input[type="range"]::-moz-range-thumb {
      width: 28px;
      height: 28px;

      border-radius: 50%;

      background: var(--accent);

      border: 3px solid #08100c;
    }


    /* ========================================================
       ESCALA DO STEERING
       ======================================================== */

    .range-scale {
      display: flex;
      align-items: center;
      justify-content: space-between;

      margin-top: 10px;

      color: #64727e;

      font-size: 10px;
      font-weight: 700;
    }

    .scale-center {
      position: absolute;
      left: 50%;
      transform: translateX(-50%);
    }


    /* ========================================================
       THROTTLE
       ======================================================== */

    .throttle-panel {
      position: relative;
    }

    .throttle-area {
      flex: 1;
      min-height: 0;

      display: grid;

      grid-template-columns: 38px 1fr;

      gap: 14px;

      align-items: center;
    }

    .throttle-scale {
      height: min(52vh, 280px);

      display: flex;
      flex-direction: column;
      justify-content: space-between;

      color: #64727e;

      font-size: 10px;
      font-weight: 700;

      text-align: right;
    }

    #throttle {
      width: min(42vh, 300px);
      height: 42px;

      justify-self: center;

      transform: rotate(-90deg);

      touch-action: none;
    }

    #throttle::-webkit-slider-runnable-track {
      height: 12px;
    }

    #throttle::-webkit-slider-thumb {
      width: 32px;
      height: 32px;

      margin-top: -11px;
    }

    .throttle-value {
      position: absolute;

      right: 16px;
      bottom: 14px;

      display: flex;
      align-items: baseline;
      gap: 6px;
    }

    .throttle-value output {
      color: var(--accent);

      font-size: 34px;
      font-weight: 800;

      font-variant-numeric: tabular-nums;
    }

    .throttle-value span {
      color: var(--muted);

      font-size: 10px;
      font-weight: 800;

      letter-spacing: 0.08em;
    }


    /* ========================================================
       BARRA INFERIOR
       ======================================================== */

    .bottom-bar {
      display: grid;

      grid-template-columns: 1fr auto;

      align-items: end;

      gap: 14px;
    }


    /* ========================================================
       SELETOR DE TRACAO
       ======================================================== */

    .drive-selector {
      min-width: 0;

      margin: 0;
      padding: 0;

      border: 0;
    }

    .drive-selector legend {
      padding: 0;
      margin-bottom: 8px;

      color: var(--muted);

      font-size: 10px;
      font-weight: 800;

      letter-spacing: 0.14em;
    }

    .drive-options {
      display: flex;
      flex-wrap: wrap;
      gap: 8px;
    }

    .drive-options input {
      position: absolute;

      opacity: 0;

      pointer-events: none;
    }

    .drive-options label {
      min-width: 82px;

      padding: 12px 16px;

      display: grid;
      place-items: center;

      border: 1px solid var(--line-strong);
      border-radius: 10px;

      background: #0c131b;

      color: #98a5b0;

      font-size: 12px;
      font-weight: 900;

      letter-spacing: 0.1em;

      cursor: pointer;

      transition:
        background 0.15s ease,
        border-color 0.15s ease,
        color 0.15s ease,
        transform 0.15s ease;
    }

    .drive-options label:active {
      transform: scale(0.97);
    }

    .drive-options input:checked + label {
      color: #06110a;

      background: var(--accent);

      border-color: var(--accent);

      box-shadow:
        0 0 18px rgba(53,229,139,0.16);
    }


    /* ========================================================
       STOP
       ======================================================== */

    .stop-button {
      min-width: 150px;
      min-height: 58px;

      border: 1px solid #ff5664;

      border-radius: 12px;

      display: flex;
      align-items: center;
      justify-content: center;

      gap: 10px;

      background:
        linear-gradient(
          180deg,
          #ff5362,
          #dc2e3d
        );

      color: white;

      font-size: 15px;
      font-weight: 900;

      letter-spacing: 0.14em;

      cursor: pointer;

      box-shadow:
        0 8px 22px rgba(220,46,61,0.2);
    }

    .stop-button:active {
      transform: scale(0.97);
    }

    .stop-icon {
      font-size: 13px;
    }


    /* ========================================================
       RESPONSIVIDADE - LANDSCAPE
       ======================================================== */

    @media (orientation: landscape) {

      .controls {
        grid-template-columns: 1fr 1fr;
      }

      .controller {
        max-width: 1200px;
        margin: 0 auto;
      }

    }


    /* ========================================================
       RESPONSIVIDADE - PORTRAIT
       ======================================================== */

    @media (orientation: portrait) {

      body {
        overflow: auto;
      }

      .controller {
        min-height: 100vh;

        grid-template-rows:
          auto
          auto
          auto;
      }

      .controls {
        grid-template-columns: 1fr;
      }

      .control-panel {
        min-height: 320px;
      }

      .bottom-bar {
        grid-template-columns: 1fr;
      }

      .stop-button {
        width: 100%;
      }

    }


    /* ========================================================
       TELAS LANDSCAPE MUITO BAIXAS
       ======================================================== */

    @media (orientation: landscape) and (max-height: 620px) {

      .controller {
        gap: 8px;
      }

      .topbar {
        min-height: 38px;
        padding-bottom: 6px;
      }

      .control-panel {
        padding: 13px;
        border-radius: 12px;
      }

      .panel-heading {
        margin-bottom: 8px;
      }

      h1,
      h2 {
        font-size: 20px;
      }

      .throttle-scale {
        height: min(42vh, 190px);
      }

      .stop-button {
        min-height: 48px;
      }

      .drive-options label {
        min-width: 70px;
        padding: 9px 12px;
      }

    }


    /* ========================================================
       ACESSIBILIDADE
       ======================================================== */

    input[type="range"]:focus-visible,
    .drive-options label:focus-visible,
    .stop-button:focus-visible {
      outline: 2px solid var(--accent);

      outline-offset: 3px;
    }

  </style>

</head>


<body>

  <main
    class="controller"
    aria-label="Controle virtual MOTRIX"
  >


    <!-- =====================================================
         TOPO
         ===================================================== -->

    <header class="topbar">

      <div class="brand">

        <span
          class="brand-mark"
          aria-hidden="true"
        >
          M
        </span>

        <span>
          MOTRIX
        </span>

      </div>


      <div
        class="connection"
        id="connection-status"
        data-state="offline"
        aria-live="polite"
      >

        <span
          class="status-dot"
          aria-hidden="true"
        ></span>

        <span id="connection-label">
          OFFLINE
        </span>

      </div>

    </header>



    <!-- =====================================================
         CONTROLES
         ===================================================== -->

    <section
      class="controls"
      aria-label="Controles de direção e aceleração"
    >


      <!-- ===================================================
           STEERING
           =================================================== -->

      <section
        class="control-panel steering-panel"
        aria-labelledby="steering-title"
      >

        <div class="panel-heading">

          <div>

            <span class="eyebrow">
              CONTROLE 01
            </span>

            <h1 id="steering-title">
              Direção
            </h1>

          </div>

          <span
            class="axis-icon"
            aria-hidden="true"
          >
            ↔
          </span>

        </div>


        <div class="steering-readout">

          <span>
            STEERING
          </span>

          <output
            id="steering-value"
            for="steering"
          >
            0
          </output>

          <span class="unit">
            %
          </span>

        </div>


        <div class="range-wrap steering-wrap">

          <span class="range-end">
            ESQUERDA
          </span>


          <input
            id="steering"
            name="steering"
            type="range"
            min="-100"
            max="100"
            value="0"
            step="1"
            aria-label="Direção: esquerda ou direita"
          >


          <span class="range-end">
            DIREITA
          </span>

        </div>


        <div
          class="range-scale"
          aria-hidden="true"
        >

          <span>
            −100
          </span>

          <span class="scale-center">
            0
          </span>

          <span>
            +100
          </span>

        </div>

      </section>



      <!-- ===================================================
           THROTTLE
           =================================================== -->

      <section
        class="control-panel throttle-panel"
        aria-labelledby="throttle-title"
      >

        <div class="panel-heading">

          <div>

            <span class="eyebrow">
              CONTROLE 02
            </span>

            <h2 id="throttle-title">
              Aceleração
            </h2>

          </div>

          <span
            class="axis-icon"
            aria-hidden="true"
          >
            ↕
          </span>

        </div>


        <div class="throttle-area">

          <div
            class="throttle-scale"
            aria-hidden="true"
          >

            <span>
              100
            </span>

            <span>
              50
            </span>

            <span>
              0
            </span>

          </div>


          <input
            id="throttle"
            name="throttle"
            type="range"
            min="0"
            max="100"
            value="0"
            step="1"
            aria-label="Aceleração de zero a cem por cento"
          >


          <div class="throttle-value">

            <output
              id="throttle-value"
              for="throttle"
            >
              0
            </output>

            <span>
              THROTTLE %
            </span>

          </div>

        </div>

      </section>

    </section>



    <!-- =====================================================
         RODAPE
         ===================================================== -->

    <footer class="bottom-bar">


      <fieldset class="drive-selector">

        <legend>
          TRAÇÃO
        </legend>


        <div class="drive-options">

          <input
            type="radio"
            name="drive"
            id="driveFWD"
            value="FWD"
          >

          <label for="driveFWD">
            FWD
          </label>


          <input
            type="radio"
            name="drive"
            id="drive4x4"
            value="4x4"
            checked
          >

          <label for="drive4x4">
            4×4
          </label>


          <input
            type="radio"
            name="drive"
            id="driveRWD"
            value="RWD"
          >

          <label for="driveRWD">
            RWD
          </label>

        </div>

      </fieldset>



      <button
        class="stop-button"
        id="stop-button"
        type="button"
        aria-label="Parar os motores do carrinho"
      >

        <span
          class="stop-icon"
          aria-hidden="true"
        >
          ■
        </span>

        <span>
          STOP
        </span>

      </button>

    </footer>

  </main>



  <!-- =======================================================
       JAVASCRIPT
       ======================================================= -->

  <script>

    // ========================================================
    // ELEMENTOS
    // ========================================================

    const steering =
      document.querySelector('#steering');

    const throttle =
      document.querySelector('#throttle');

    const steeringValue =
      document.querySelector('#steering-value');

    const throttleValue =
      document.querySelector('#throttle-value');

    const status =
      document.querySelector('#connection-status');

    const statusLabel =
      document.querySelector('#connection-label');

    const stopButton =
      document.querySelector('#stop-button');

    const driveInputs =
      document.querySelectorAll(
        'input[name="drive"]'
      );


    // ========================================================
    // ESTADO
    // ========================================================

    let sending = false;
    let commandQueued = false;

    let connectionOnline = false;


    // ========================================================
    // FUNCAO AUXILIAR DE CONEXAO
    // ========================================================

    function setConnection(online) {

      connectionOnline = online;

      status.dataset.state =
        online
          ? 'online'
          : 'offline';

      statusLabel.textContent =
        online
          ? 'CONNECTED'
          : 'OFFLINE';

    }


    // ========================================================
    // LEITURA DO COMANDO ATUAL
    // ========================================================

    function currentCommand() {

      const selectedDrive =
        document.querySelector(
          'input[name="drive"]:checked'
        );

      return {

        steering:
          Number(steering.value),

        throttle:
          Number(throttle.value),

        driveMode:
          selectedDrive
            ? selectedDrive.value
            : '4x4'

      };

    }


    // ========================================================
    // ATUALIZA VALORES DA INTERFACE
    // ========================================================

    function updateReadouts() {

      steeringValue.textContent =
        steering.value;

      throttleValue.textContent =
        throttle.value;

    }


    // ========================================================
    // REQUISICAO COM TIMEOUT
    // ========================================================

    async function request(url) {

      const controller =
        new AbortController();

      const timeout =
        setTimeout(
          () => controller.abort(),
          550
        );

      try {

        const response =
          await fetch(
            url,
            {
              method: 'GET',
              cache: 'no-store',
              signal: controller.signal
            }
          );

        return response;

      } finally {

        clearTimeout(timeout);

      }

    }


    // ========================================================
    // ENVIO DO COMANDO
    // ========================================================

    async function sendCommand() {

      // Se ja existe uma requisicao em andamento,
      // registramos que existe outro comando aguardando.
      if (sending) {

        commandQueued = true;

        return;

      }


      sending = true;


      const command =
        currentCommand();


      const params =
        new URLSearchParams({

          steering:
            command.steering,

          throttle:
            command.throttle,

          driveMode:
            command.driveMode

        });


      try {

        const response =
          await request(
            `/api/command?${params.toString()}`
          );


        if (!response.ok) {

          throw new Error(
            `HTTP ${response.status}`
          );

        }


        setConnection(true);

      }

      catch (error) {

        setConnection(false);

      }

      finally {

        sending = false;


        // Se outro comando chegou durante o envio,
        // mandamos o estado mais recente.
        if (commandQueued) {

          commandQueued = false;

          sendCommand();

        }

      }

    }


    // ========================================================
    // TESTE DE CONEXAO
    // ========================================================

    async function checkConnection() {

      try {

        const response =
          await request(
            '/api/status'
          );


        if (!response.ok) {

          throw new Error(
            `HTTP ${response.status}`
          );

        }


        setConnection(true);

      }

      catch (error) {

        setConnection(false);

      }

    }


    // ========================================================
    // STOP
    // ========================================================

    async function stopMotors() {

      // Primeiro atualiza a interface.
      throttle.value = '0';
      steering.value = '0';

      updateReadouts();


      try {

        const response =
          await request(
            '/api/stop'
          );


        if (!response.ok) {

          throw new Error(
            `HTTP ${response.status}`
          );

        }


        setConnection(true);

      }

      catch (error) {

        setConnection(false);

      }

    }


    // ========================================================
    // EVENTOS - STEERING
    // ========================================================

    steering.addEventListener(
      'input',
      () => {

        updateReadouts();

        sendCommand();

      }
    );


    // ========================================================
    // EVENTOS - THROTTLE
    // ========================================================

    throttle.addEventListener(
      'input',
      () => {

        updateReadouts();

        sendCommand();

      }
    );


    // ========================================================
    // EVENTOS - FWD / 4x4 / RWD
    // ========================================================

    driveInputs.forEach(
      (input) => {

        input.addEventListener(
          'change',
          () => {

            sendCommand();

          }
        );

      }
    );


    // ========================================================
    // EVENTO - STOP
    // ========================================================

    stopButton.addEventListener(
      'click',
      stopMotors
    );


    // ========================================================
    // WATCHDOG DO NAVEGADOR
    //
    // Enquanto houver steering ou throttle ativo,
    // enviamos o estado periodicamente.
    //
    // 180 ms < 700 ms do watchdog do ESP32.
    // ========================================================

    window.setInterval(
      () => {

        const active =
          Number(throttle.value) > 0 ||
          Number(steering.value) !== 0;


        if (active) {

          sendCommand();

        }

      },
      180
    );


    // ========================================================
    // VERIFICACAO DE CONEXAO
    // ========================================================

    window.setInterval(
      checkConnection,
      1000
    );


    // ========================================================
    // AO FECHAR / SAIR DA PAGINA
    //
    // Tentamos mandar STOP para evitar que o ESP32
    // mantenha o ultimo comando ativo.
    //
    // Mesmo que isso falhe, o watchdog do ESP32
    // tambem fara a parada apos 700 ms.
    // ========================================================

    window.addEventListener(
      'pagehide',
      () => {

        fetch(
          '/api/stop',
          {
            method: 'GET',
            cache: 'no-store',
            keepalive: true
          }
        ).catch(
          () => {}
        );

      }
    );


    // ========================================================
    // INICIALIZACAO
    // ========================================================

    updateReadouts();

    checkConnection();

    sendCommand();

  </script>

</body>

</html>
)rawliteral";


// ============================================================
// FUNCOES DOS MOTORES
// ============================================================

void parar() {

  digitalWrite(
    motorEsquerdoF,
    LOW
  );

  digitalWrite(
    motorDireitoF,
    LOW
  );

  digitalWrite(
    motorEsquerdoTras,
    LOW
  );

  digitalWrite(
    motorDireitoTras,
    LOW
  );

  motoresAtivos = false;
}


void frente() {

  digitalWrite(
    motorEsquerdoF,
    HIGH
  );

  digitalWrite(
    motorDireitoF,
    HIGH
  );

  digitalWrite(
    motorEsquerdoTras,
    LOW
  );

  digitalWrite(
    motorDireitoTras,
    LOW
  );

  motoresAtivos = true;
}


void tras() {

  digitalWrite(
    motorEsquerdoF,
    LOW
  );

  digitalWrite(
    motorDireitoF,
    LOW
  );

  digitalWrite(
    motorEsquerdoTras,
    HIGH
  );

  digitalWrite(
    motorDireitoTras,
    HIGH
  );

  motoresAtivos = true;
}


void esquerda() {

  digitalWrite(
    motorEsquerdoF,
    LOW
  );

  digitalWrite(
    motorDireitoF,
    HIGH
  );

  digitalWrite(
    motorEsquerdoTras,
    HIGH
  );

  digitalWrite(
    motorDireitoTras,
    LOW
  );

  motoresAtivos = true;
}


void direita() {

  digitalWrite(
    motorEsquerdoF,
    HIGH
  );

  digitalWrite(
    motorDireitoF,
    LOW
  );

  digitalWrite(
    motorEsquerdoTras,
    LOW
  );

  digitalWrite(
    motorDireitoTras,
    HIGH
  );

  motoresAtivos = true;
}


// ============================================================
// RESPOSTA DA PAGINA HTML
// ============================================================

void responderHtml() {

  server.sendHeader(
    "Cache-Control",
    "no-store, no-cache, must-revalidate, max-age=0"
  );

  server.send_P(
    200,
    "text/html; charset=utf-8",
    MOTRIX_HTML
  );

}


// ============================================================
// API DE STATUS
// ============================================================

void responderStatus() {

  String resposta = "{";

  resposta += "\"online\":true,";
  resposta += "\"steering\":" + String(steeringAtual) + ",";
  resposta += "\"throttle\":" + String(throttleAtual) + ",";
  resposta += "\"driveMode\":\"" + driveModeAtual + "\",";
  resposta += "\"motorsActive\":";
  resposta += motoresAtivos ? "true" : "false";

  resposta += "}";


  server.sendHeader(
    "Cache-Control",
    "no-store"
  );

  server.send(
    200,
    "application/json",
    resposta
  );

}


// ============================================================
// API DEBUG
// ============================================================

void responderDebug() {

  String resposta = "{";

  resposta += "\"online\":true,";

  resposta += "\"ip\":\"";
  resposta += WiFi.softAPIP().toString();
  resposta += "\",";

  resposta += "\"clients\":";
  resposta += String(
    WiFi.softAPgetStationNum()
  );
  resposta += ",";

  resposta += "\"steering\":";
  resposta += String(steeringAtual);
  resposta += ",";

  resposta += "\"throttle\":";
  resposta += String(throttleAtual);
  resposta += ",";

  resposta += "\"driveMode\":\"";
  resposta += driveModeAtual;
  resposta += "\",";

  resposta += "\"motorsActive\":";
  resposta += motoresAtivos ? "true" : "false";

  resposta += "}";


  server.sendHeader(
    "Cache-Control",
    "no-store"
  );

  server.send(
    200,
    "application/json",
    resposta
  );

}


// ============================================================
// RECEBE COMANDO DO NAVEGADOR
// ============================================================

void receberComando() {

  // ----------------------------------------------------------
  // Verifica se os parametros principais existem.
  // ----------------------------------------------------------

  if (
    !server.hasArg("steering") ||
    !server.hasArg("throttle")
  ) {

    server.send(
      400,
      "application/json",
      "{\"ok\":false,\"error\":\"steering e throttle sao obrigatorios\"}"
    );

    return;

  }


  // ----------------------------------------------------------
  // Le e limita os valores.
  // ----------------------------------------------------------

  steeringAtual =
    constrain(
      server.arg("steering").toInt(),
      STEERING_MIN,
      STEERING_MAX
    );


  throttleAtual =
    constrain(
      server.arg("throttle").toInt(),
      THROTTLE_MIN,
      THROTTLE_MAX
    );


  // ----------------------------------------------------------
  // Le modo de tracao.
  // ----------------------------------------------------------

  if (
    server.hasArg("driveMode")
  ) {

    String recebido =
      server.arg("driveMode");


    if (
      recebido == "FWD" ||
      recebido == "4x4" ||
      recebido == "RWD"
    ) {

      driveModeAtual =
        recebido;

    }

  }


  // ----------------------------------------------------------
  // Atualiza o watchdog.
  // ----------------------------------------------------------

  ultimoComandoMs =
    millis();


  // ----------------------------------------------------------
  // CONTROLE DOS MOTORES
  //
  // IMPORTANTE:
  // Neste firmware throttle e usado como "ativo/inativo".
  //
  // Qualquer valor > 0 liga os motores.
  //
  // O valor numerico do throttle ainda nao e convertido
  // para velocidade PWM.
  // ----------------------------------------------------------

  if (
    throttleAtual == 0
  ) {

    parar();

  }

  else if (
    steeringAtual < -STEERING_DEADBAND
  ) {

    esquerda();

  }

  else if (
    steeringAtual > STEERING_DEADBAND
  ) {

    direita();

  }

  else {

    frente();

  }


  // ----------------------------------------------------------
  // Resposta JSON.
  // ----------------------------------------------------------

  String resposta = "{";

  resposta += "\"ok\":true,";
  resposta += "\"steering\":" + String(steeringAtual) + ",";
  resposta += "\"throttle\":" + String(throttleAtual) + ",";
  resposta += "\"driveMode\":\"" + driveModeAtual + "\",";
  resposta += "\"motorsActive\":";
  resposta += motoresAtivos ? "true" : "false";

  resposta += "}";


  server.sendHeader(
    "Cache-Control",
    "no-store"
  );

  server.send(
    200,
    "application/json",
    resposta
  );

}


// ============================================================
// API STOP
// ============================================================

void pararPeloComando() {

  steeringAtual = 0;
  throttleAtual = 0;

  parar();

  ultimoComandoMs =
    millis();


  server.sendHeader(
    "Cache-Control",
    "no-store"
  );

  server.send(
    200,
    "application/json",
    "{\"ok\":true,\"stopped\":true}"
  );

}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  delay(500);


  // ----------------------------------------------------------
  // Configura os pinos dos motores
  // ----------------------------------------------------------

  pinMode(
    motorEsquerdoF,
    OUTPUT
  );

  pinMode(
    motorDireitoF,
    OUTPUT
  );

  pinMode(
    motorEsquerdoTras,
    OUTPUT
  );

  pinMode(
    motorDireitoTras,
    OUTPUT
  );


  // ----------------------------------------------------------
  // Garante motores desligados ao iniciar.
  // ----------------------------------------------------------

  parar();


  // ----------------------------------------------------------
  // Inicia Wi-Fi em modo Access Point.
  // ----------------------------------------------------------

  WiFi.mode(
    WIFI_AP
  );


  // Desativa power saving para melhorar estabilidade
  // da comunicacao local.
  WiFi.setSleep(false);


  // ----------------------------------------------------------
  // Configura IP fixo.
  // ----------------------------------------------------------

  bool configOK =
    WiFi.softAPConfig(
      AP_IP,
      AP_GATEWAY,
      AP_SUBNET
    );


  if (!configOK) {

    Serial.println(
      "ERRO: falha no softAPConfig()."
    );

  }


  // ----------------------------------------------------------
  // Inicia a rede Wi-Fi.
  // ----------------------------------------------------------

  bool apOK =
    WiFi.softAP(
      WIFI_SSID,
      WIFI_PASSWORD
    );


  if (!apOK) {

    Serial.println();
    Serial.println(
      "========================================"
    );
    Serial.println(
      "ERRO: NAO FOI POSSIVEL INICIAR O WIFI"
    );
    Serial.println(
      "========================================"
    );

    return;

  }


  // ----------------------------------------------------------
  // Informacoes no Monitor Serial.
  // ----------------------------------------------------------

  Serial.println();
  Serial.println(
    "========================================"
  );

  Serial.println(
    "          MOTRIX - ESP32"
  );

  Serial.println(
    "========================================"
  );

  Serial.println();

  Serial.print(
    "SSID: "
  );

  Serial.println(
    WIFI_SSID
  );

  Serial.print(
    "IP do ESP32: "
  );

  Serial.println(
    WiFi.softAPIP()
  );

  Serial.println();

  Serial.println(
    "Conecte o computador/celular na rede:"
  );

  Serial.println(
    WIFI_SSID
  );

  Serial.println();

  Serial.println(
    "Depois abra no navegador:"
  );

  Serial.println(
    "http://192.168.4.1"
  );

  Serial.println();

  Serial.println(
    "Teste de status:"
  );

  Serial.println(
    "http://192.168.4.1/api/status"
  );

  Serial.println();

  Serial.println(
    "Teste de diagnostico:"
  );

  Serial.println(
    "http://192.168.4.1/api/debug"
  );

  Serial.println();

  Serial.println(
    "========================================"
  );


  // ========================================================
  // ROTAS HTTP
  // ========================================================

  // --------------------------------------------------------
  // Pagina principal
  // --------------------------------------------------------

  server.on(
    "/",
    HTTP_GET,
    responderHtml
  );


  // --------------------------------------------------------
  // Alias para index.html
  // --------------------------------------------------------

  server.on(
    "/index.html",
    HTTP_GET,
    responderHtml
  );


  // --------------------------------------------------------
  // API STATUS
  // --------------------------------------------------------

  server.on(
    "/api/status",
    HTTP_GET,
    responderStatus
  );


  // --------------------------------------------------------
  // API DEBUG
  // --------------------------------------------------------

  server.on(
    "/api/debug",
    HTTP_GET,
    responderDebug
  );


  // --------------------------------------------------------
  // API COMMAND
  // --------------------------------------------------------

  server.on(
    "/api/command",
    HTTP_GET,
    receberComando
  );


  // --------------------------------------------------------
  // API STOP
  // --------------------------------------------------------

  server.on(
    "/api/stop",
    HTTP_GET,
    pararPeloComando
  );


  // --------------------------------------------------------
  // Favicon:
  // evita alguns 404 desnecessarios no navegador.
  // --------------------------------------------------------

  server.on(
    "/favicon.ico",
    HTTP_GET,
    []() {

      server.send(
        204,
        "text/plain",
        ""
      );

    }
  );


  // --------------------------------------------------------
  // ROTA NAO ENCONTRADA
  // --------------------------------------------------------

  server.onNotFound(
    []() {

      server.send(
        404,
        "text/plain; charset=utf-8",
        "Rota nao encontrada"
      );

    }
  );


  // --------------------------------------------------------
  // Inicia servidor.
  // --------------------------------------------------------

  server.begin();


  Serial.println(
    "Servidor HTTP iniciado."
  );

  Serial.println();

}


// ============================================================
// LOOP
// ============================================================

void loop() {

  // ----------------------------------------------------------
  // Processa requisicoes HTTP.
  // ----------------------------------------------------------

  server.handleClient();


  // ----------------------------------------------------------
  // WATCHDOG
  //
  // Se os motores estiverem ativos e o navegador deixar
  // de enviar comandos por mais de 700 ms, para tudo.
  // ----------------------------------------------------------

  if (
    motoresAtivos &&
    (
      millis() - ultimoComandoMs
      >
      COMMAND_TIMEOUT_MS
    )
  ) {

    parar();

    steeringAtual = 0;
    throttleAtual = 0;

    Serial.println(
      "WATCHDOG: motores parados por falta de comandos."
    );

  }

}