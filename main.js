(() => {
  const tractionOptions = [...document.querySelectorAll('.traction-option')];
  const lightsToggle = document.querySelector('#lightsToggle');
  const activeDrives = new Set();

  function updateRange(input, output) {
    const value = Number(input.value);
    output.textContent = `${value}%`;
    const progress = (value - Number(input.min)) / (Number(input.max) - Number(input.min));
    const shell = input.closest('.range-shell');
    if (shell) shell.style.setProperty('--progress', progress);
  }

  function bindRange(id, outputId) {
    const input = document.querySelector(`#${id}`);
    const output = document.querySelector(`#${outputId}`);
    let pressed = false;
    const reset = () => {
      if (!pressed) return;
      pressed = false;
      input.value = '0';
      updateRange(input, output);
      input.dispatchEvent(new Event('change', { bubbles: true }));
    };
    input.addEventListener('input', () => updateRange(input, output));
    input.addEventListener('pointerdown', () => { pressed = true; });
    input.addEventListener('keydown', (event) => {
      if (event.key.startsWith('Arrow') || event.key === ' ') pressed = true;
    });
    input.addEventListener('keyup', reset);
    input.addEventListener('blur', reset);
    window.addEventListener('pointerup', reset);
    window.addEventListener('pointercancel', reset);
    updateRange(input, output);
  }

  bindRange('steering', 'steeringValue');
  bindRange('reverse', 'reverseValue');
  bindRange('throttle', 'throttleValue');

  function renderDrives() {
    tractionOptions.forEach((option) => option.setAttribute('aria-pressed', String(activeDrives.has(option))));
  }
  tractionOptions.forEach((option) => option.addEventListener('click', () => {
    activeDrives.clear();
    activeDrives.add(option);
    renderDrives();
  }));
  lightsToggle.addEventListener('click', () => {
    lightsToggle.setAttribute('aria-pressed', String(lightsToggle.getAttribute('aria-pressed') !== 'true'));
  });

  function releaseAll() {
    document.querySelectorAll('.vertical-range, .horizontal-range').forEach((input) => {
      if (input.value !== '0') {
        input.value = '0';
        input.dispatchEvent(new Event('input', { bubbles: true }));
      }
    });
  }
  window.addEventListener('blur', releaseAll);
  document.addEventListener('visibilitychange', () => { if (document.hidden) releaseAll(); });

  let installPrompt = null;
  const installButton = document.querySelector('#installButton');
  window.addEventListener('beforeinstallprompt', (event) => {
    event.preventDefault();
    installPrompt = event;
    installButton.hidden = false;
  });
  installButton.addEventListener('click', async () => {
    if (!installPrompt) return;
    installPrompt.prompt();
    await installPrompt.userChoice;
    installPrompt = null;
    installButton.hidden = true;
  });
  window.addEventListener('appinstalled', () => {
    installButton.hidden = true;
    installPrompt = null;
  });
  if ('serviceWorker' in navigator) {
    window.addEventListener('load', () => navigator.serviceWorker.register('./service-worker.js').catch(() => {}));
  }
})();
