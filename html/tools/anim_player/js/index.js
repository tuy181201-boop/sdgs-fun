(function () {
    'use strict';

    const container = document.getElementById('viewport');
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x0a0e1a);

    const camera = new THREE.PerspectiveCamera(45, container.clientWidth / container.clientHeight, 0.1, 1000);
    camera.position.set(25, 20, 25);

    const renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setSize(container.clientWidth, container.clientHeight);
    renderer.setPixelRatio(Math.min(devicePixelRatio, 2));
    container.appendChild(renderer.domElement);

    const controls = new THREE.OrbitControls(camera, renderer.domElement);
    controls.enableDamping = true;

    scene.add(new THREE.AmbientLight(0x404060));
    const dirLight = new THREE.DirectionalLight(0xffffff, 1);
    dirLight.position.set(1, 2, 1);
    scene.add(dirLight);

    const grid = new THREE.GridHelper(30, 30, 0x2a3a66, 0x1a2a4a);
    grid.position.y = -5;
    scene.add(grid);

    // Состояние
    let frames = [];      // массив кадров { voxels: [{x, y, z, color}] }
    let currentFrame = 0;
    let playing = false;
    let speed = 1;
    let fps = 30;
    let lastTime = 0;
    const frameGroup = new THREE.Group();
    scene.add(frameGroup);

    const cubeGeometry = new THREE.BoxGeometry(1, 1, 1);

    // ---------- Парсер .anim ----------
    // Формат: [UID 16][D 1][L1..LD][S1..SD][Data]
    // Для анимации D = 4 (T, X, Y, Z)
    function parseAnim(buffer) {
        const view = new DataView(buffer);
        const D = view.getUint8(16);
        const lengths = [];
        for (let i = 0; i < D; i++) lengths.push(view.getUint8(17 + i));

        let offset = 17 + D;
        const sizes = [];
        for (let i = 0; i < D; i++) {
            let size = 0;
            for (let b = 0; b < lengths[i]; b++) size = (size << 8) | view.getUint8(offset + b);
            sizes.push(size);
            offset += lengths[i];
        }

        // Предполагаем порядок: [T, X, Y, Z]
        const [T, X, Y, Z] = sizes;
        const result = [];

        for (let t = 0; t < T; t++) {
            const frame = [];
            for (let z = 0; z < Z; z++) {
                for (let y = 0; y < Y; y++) {
                    for (let x = 0; x < X; x++) {
                        const base = offset + ((t * X * Y * Z) + (z * X * Y) + (y * X) + x) * 4;
                        if (base + 4 > buffer.byteLength) continue;
                        const r = view.getUint8(base);
                        const g = view.getUint8(base + 1);
                        const b = view.getUint8(base + 2);
                        const a = view.getUint8(base + 3);
                        if (a > 0) frame.push({ x, y, z, color: new THREE.Color(r / 255, g / 255, b / 255) });
                    }
                }
            }
            result.push(frame);
        }

        return { frames: result, dims: [X, Y, Z], total: T };
    }

    // ---------- Демо-анимация (если нет файла) ----------
    function generateDemoAnimation() {
        const result = [];
        const total = 30;
        for (let t = 0; t < total; t++) {
            const frame = [];
            const y = Math.floor(Math.sin(t / total * Math.PI * 2) * 3 + 5);
            for (let x = 0; x < 3; x++)
                for (let z = 0; z < 3; z++)
                    for (let dy = 0; dy < 3; dy++)
                        frame.push({ x, y: y + dy, z, color: new THREE.Color(0.0, 0.8, 1.0) });
            result.push(frame);
        }
        return { frames: result, dims: [3, 10, 3], total };
    }

    // ---------- Отрисовка кадра ----------
    function renderFrame(index) {
        // Очистить
        while (frameGroup.children.length) {
            const c = frameGroup.children[0];
            c.material.dispose();
            frameGroup.remove(c);
        }

        if (!frames[index]) return;

        // Группировка по цвету
        const byColor = new Map();
        frames[index].forEach(v => {
            const key = v.color.getHex();
            if (!byColor.has(key)) byColor.set(key, []);
            byColor.get(key).push(v);
        });

        for (const [hex, positions] of byColor) {
            const material = new THREE.MeshStandardMaterial({
                color: hex, roughness: 0.4, metalness: 0.1
            });
            const inst = new THREE.InstancedMesh(cubeGeometry, material, positions.length);
            const matrix = new THREE.Matrix4();
            positions.forEach((p, i) => {
                matrix.makeTranslation(p.x - 1.5, p.y, p.z - 1.5);
                inst.setMatrixAt(i, matrix);
            });
            frameGroup.add(inst);
        }

        document.getElementById('frame-label').textContent = `${index + 1} / ${frames.length}`;
        document.getElementById('frame-slider').value = index;
    }

    // ---------- Управление ----------
    document.getElementById('file-input').addEventListener('change', async (e) => {
        const file = e.target.files[0];
        if (!file) return;

        try {
            const buffer = await file.arrayBuffer();
            const parsed = parseAnim(buffer);
            frames = parsed.frames;
            document.getElementById('total-frames').textContent = parsed.total;
            document.getElementById('dims').textContent = parsed.dims.join(' × ');
            document.getElementById('frame-slider').max = parsed.total - 1;
            renderFrame(0);
        } catch (err) {
            console.error(err);
            alert('Ошибка парсинга .anim: ' + err.message);
        }
    });

    document.getElementById('frame-slider').addEventListener('input', (e) => {
        currentFrame = parseInt(e.target.value);
        renderFrame(currentFrame);
    });

    document.getElementById('speed-slider').addEventListener('input', (e) => {
        speed = parseFloat(e.target.value);
        document.getElementById('speed-label').textContent = `${speed.toFixed(1)}×`;
    });

    document.getElementById('btn-play').addEventListener('click', () => {
        playing = !playing;
        document.getElementById('btn-play').textContent = playing ? '⏸ Пауза' : '▶ Играть';
        lastTime = performance.now();
    });

    document.getElementById('btn-stop').addEventListener('click', () => {
        playing = false;
        currentFrame = 0;
        renderFrame(0);
        document.getElementById('btn-play').textContent = '▶ Играть';
    });

    // ---------- Цикл воспроизведения ----------
    function animate(now) {
        requestAnimationFrame(animate);
        controls.update();

        if (playing && frames.length > 0) {
            const dt = (now - lastTime) / 1000;
            const frameDuration = 1 / (fps * speed);
            if (dt >= frameDuration) {
                currentFrame = (currentFrame + 1) % frames.length;
                renderFrame(currentFrame);
                lastTime = now;
            }
        }

        renderer.render(scene, camera);
    }

    // ---------- Инициализация демо ----------
    const demo = generateDemoAnimation();
    frames = demo.frames;
    document.getElementById('total-frames').textContent = demo.total;
    document.getElementById('dims').textContent = demo.dims.join(' × ');
    document.getElementById('frame-slider').max = demo.total - 1;
    renderFrame(0);

    window.addEventListener('resize', () => {
        camera.aspect = container.clientWidth / container.clientHeight;
        camera.updateProjectionMatrix();
        renderer.setSize(container.clientWidth, container.clientHeight);
    });

    requestAnimationFrame(animate);
})();