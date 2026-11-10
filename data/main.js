(() => {
    const tractionOptions = [...document.querySelectorAll('.traction-option')];
    const lightsToggle = document.querySelector('#lightsToggle');
    const activeDrives = new Set();

    // Atualiza o valor e a barra de progresso de um input range
    function updateRange(input, output) {
        const value = Number(input.value);

        output.textContent = `${value}%`;

        const progress =
            (value - Number(input.min)) /
            (Number(input.max) - Number(input.min));

        const shell = input.closest('.range-shell');

        if (shell) {
            shell.style.setProperty('--progress', progress);
        }
    }

    // Configura o comportamento de um input range
    function bindRange(id, outputId) {
        const input = document.querySelector(`#${id}`);
        const output = document.querySelector(`#${outputId}`);

        let pressed = false;

        // Retorna o controle ao valor zero após a interação
        const reset = () => {
            if (!pressed) return;

            pressed = false;
            input.value = '0';

            updateRange(input, output);

            input.dispatchEvent(
                new Event('change', { bubbles: true })
            );
        };

        input.addEventListener('input', () => {
            updateRange(input, output);
        });

        input.addEventListener('pointerdown', () => {
            pressed = true;
        });

        input.addEventListener('keydown', (event) => {
            if (
                event.key.startsWith('Arrow') ||
                event.key === ' '
            ) {
                pressed = true;
            }
        });

        input.addEventListener('keyup', reset);
        input.addEventListener('blur', reset);

        window.addEventListener('pointerup', reset);
        window.addEventListener('pointercancel', reset);

        updateRange(input, output);
    }

    // Inicializa os controles de direção e aceleração
    bindRange('steering', 'steeringValue');
    bindRange('reverse', 'reverseValue');
    bindRange('throttle', 'throttleValue');

    // Atualiza o estado visual dos modos de tração
    function renderDrives() {
        tractionOptions.forEach((option) => {
            option.setAttribute(
                'aria-pressed',
                String(activeDrives.has(option))
            );
        });
    }

    // Permite selecionar um modo de tração por vez
    tractionOptions.forEach((option) => {
        option.addEventListener('click', () => {
            activeDrives.clear();
            activeDrives.add(option);

            renderDrives();
        });
    });

    // Alterna o estado dos faróis
    lightsToggle.addEventListener('click', () => {
        lightsToggle.setAttribute(
            'aria-pressed',
            String(
                lightsToggle.getAttribute('aria-pressed') !== 'true'
            )
        );
    });

    // Zera todos os controles de movimento
    function releaseAll() {
        document
            .querySelectorAll('.vertical-range, .horizontal-range')
            .forEach((input) => {
                if (input.value !== '0') {
                    input.value = '0';

                    input.dispatchEvent(
                        new Event('input', { bubbles: true })
                    );
                }
            });
    }

    // Interrompe os controles quando a janela perde o foco
    window.addEventListener('blur', releaseAll);

    // Interrompe os controles quando a página fica oculta
    document.addEventListener('visibilitychange', () => {
        if (document.hidden) {
            releaseAll();
        }
    });

    // Gerenciamento da instalação do PWA
    let installPrompt = null;

    const installButton = document.querySelector('#installButton');

    window.addEventListener('beforeinstallprompt', (event) => {
        event.preventDefault();

        installPrompt = event;
        installButton.hidden = false;
    });

    // Solicita a instalação quando o usuário clica no botão
    installButton.addEventListener('click', async () => {
        if (!installPrompt) return;

        installPrompt.prompt();

        await installPrompt.userChoice;

        installPrompt = null;
        installButton.hidden = true;
    });

    // Oculta o botão após a instalação do aplicativo
    window.addEventListener('appinstalled', () => {
        installButton.hidden = true;
        installPrompt = null;
    });

    // Registra o Service Worker do PWA
    if ('serviceWorker' in navigator) {
        window.addEventListener('load', () => {
            navigator.serviceWorker
                .register('./service-worker.js')
                .catch(() => {});
        });
    }
})();