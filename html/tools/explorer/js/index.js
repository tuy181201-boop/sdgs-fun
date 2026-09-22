(function () {
    'use strict';

    // ---------- Сцена ----------
    const container = document.getElementById('viewport');
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x0a0e1a);

    const camera = new THREE.PerspectiveCamera(45, container.clientWidth / container.clientHeight, 0.1, 1000);
    camera.position.set(20, 15, 20);

    const renderer = new THREE.WebGLRenderer({ antialias: true, preserveDrawingBuffer: true });
    renderer.setSize(container.clientWidth, container.clientHeight);
    renderer.setPixelRatio(Math.min(devicePixelRatio, 2));
    container.appendChild(renderer.domElement);

    const controls = new THREE.OrbitControls(camera, renderer.domElement);
    controls.enableDamping = true;
    controls.dampingFactor = 0.05;

    // ---------- Свет ----------
    scene.add(new THREE.AmbientLight(0x404060));
    const dirLight = new THREE.DirectionalLight(0xffffff, 1);
    dirLight.position.set(1, 2, 1);
    scene.add(dirLight);
    const backLight = new THREE.DirectionalLight(0x4488ff, 0.5);
    backLight.position.set(-1, -0.5, -1);
    scene.add(backLight);

    // ---------- Сетка ----------
    const grid = new THREE.GridHelper(30, 30, 0x2a3a66, 0x1a2a4a);
    scene.add(grid);

    // ---------- Группа для контента ----------
    const contentGroup = new THREE.Group();
    scene.add(contentGroup);

    // ---------- Состояние ----------
    let currentWireframe = false;
    const statusEl = document.getElementById('status');
    const infoEl = document.getElementById('file-info');

    function setStatus(msg) { statusEl.textContent = msg; }

    // ---------- Очистка сцены ----------
    function clearContent() {
        while (contentGroup.children.length) {
            const c = contentGroup.children[0];
            if (c.geometry) c.geometry.dispose();
            if (c.material) {
                if (Array.isArray(c.material)) c.material.forEach(m => m.dispose());
                else c.material.dispose();
            }
            contentGroup.remove(c);
        }
    }

    // ---------- Парсер заголовка SDGS ----------
    function parseSDGSHeader(buffer) {
        const view = new DataView(buffer);
        const uid = new Uint8Array(buffer, 0, 16);
        const D = view.getUint8(16);
        const lengths = [];
        for (let i = 0; i < D; i++) lengths.push(view.getUint8(17 + i));

        let offset = 17 + D;
        const sizes = [];
        for (let i = 0; i < D; i++) {
            let size = 0;
            for (let b = 0; b < lengths[i]; b++) {
                size = (size << 8) | view.getUint8(offset + b);
            }
            sizes.push(size);
            offset += lengths[i];
        }

        return {
            uid: Array.from(uid).map(b => b.toString(16).padStart(2, '0')).join(''),
            D, lengths, sizes, dataOffset: offset
        };
    }

    // ---------- Рендер VXL ----------
    function renderVXL(buffer) {
        clearContent();
        const header = parseSDGSHeader(buffer);
        const [X, Y, Z] = header.sizes;

        infoEl.innerHTML = `
            <p><strong>Тип:</strong> VXL (воксельная модель)</p>
            <p><strong>UID:</strong> <code>${header.uid.substring(0, 16)}…</code></p>
            <p><strong>Размеры:</strong> ${X} × ${Y} × ${Z}</p>
            <p><strong>Измерений:</strong> ${header.D}</p>
        `;

        // Куб-геометрия для инстансинга
        const geometry = new THREE.BoxGeometry(1, 1, 1);

        let offset = header.dataOffset;
        const view = new DataView(buffer);

        // Собираем воксели по цветам (простая группировка)
        const voxelsByColor = new Map();

        for (let z = 0; z < Z; z++) {
            for (let y = 0; y < Y; y++) {
                for (let x = 0; x < X; x++) {
                    // Каждый воксель: R, G, B, A (4 байта) — старый формат
                    // Или R, G, B, A, Mass (12 байт) — новый формат
                    const bpp = 12; // предполагаем новый формат
                    const base = offset + (x + y * X + z * X * Y) * bpp;
                    if (base + 4 > buffer.byteLength) continue;

                    const r = view.getUint8(base);
                    const g = view.getUint8(base + 1);
                    const b = view.getUint8(base + 2);
                    const a = view.getUint8(base + 3);

                    if (a === 0) continue; // пусто

                    const key = (r << 16) | (g << 8) | b;
                    if (!voxelsByColor.has(key)) voxelsByColor.set(key, []);
                    voxelsByColor.get(key).push({ x, y, z });
                }
            }
        }

        // Отрисовываем по группам цветов
        for (const [key, positions] of voxelsByColor) {
            const r = (key >> 16) & 0xFF;
            const g = (key >> 8) & 0xFF;
            const b = key & 0xFF;
            const color = new THREE.Color(r / 255, g / 255, b / 255);

            const material = new THREE.MeshStandardMaterial({
                color, roughness: 0.4, metalness: 0.1, wireframe: currentWireframe
            });

            const instanced = new THREE.InstancedMesh(geometry, material, positions.length);
            const matrix = new THREE.Matrix4();
            positions.forEach((p, i) => {
                matrix.makeTranslation(
                    p.x - X / 2,
                    p.y - Y / 2,
                    p.z - Z / 2
                );
                instanced.setMatrixAt(i, matrix);
            });
            contentGroup.add(instanced);
        }

        // Авто-масштаб камеры
        const maxDim = Math.max(X, Y, Z);
        camera.position.set(maxDim * 1.5, maxDim * 1.2, maxDim * 1.5);
        controls.target.set(0, 0, 0);
        controls.update();

        setStatus(`Загружено ${voxelsByColor.size} цветов, ${[...voxelsByColor.values()].reduce((a, b) => a + b.length, 0)} вокселей`);
    }

    // ---------- Рендер RGS (реестр) ----------
    function renderRGS(buffer) {
        clearContent();
        const text = new TextDecoder('utf-8').decode(buffer);
        let data;
        try { data = JSON.parse(text); }
        catch (e) { setStatus('Ошибка парсинга RGS: ' + e.message); return; }

        const entries = Object.entries(data).filter(([k]) => k !== '_comment');
        infoEl.innerHTML = `
            <p><strong>Тип:</strong> RGS (реестр)</p>
            <p><strong>Записей:</strong> ${entries.length}</p>
        `;

        // Визуализируем записи как узлы на графе
        const geometry = new THREE.SphereGeometry(0.3, 8, 8);
        const material = new THREE.MeshStandardMaterial({ color: 0x00d4ff, emissive: 0x004466 });

        const cols = Math.ceil(Math.sqrt(entries.length));
        entries.forEach(([uid, path], i) => {
            const sphere = new THREE.Mesh(geometry, material);
            sphere.position.set(
                (i % cols) * 2 - cols,
                0,
                Math.floor(i / cols) * 2 - cols
            );
            contentGroup.add(sphere);
        });

        setStatus(`Загружено ${entries.length} записей RGS`);
    }

    // ---------- Рендер DNA ----------
    function renderDNA(buffer) {
        clearContent();
        const text = new TextDecoder('utf-8').decode(buffer);
        infoEl.innerHTML = `
            <p><strong>Тип:</strong> DNA (геном мира)</p>
            <p><strong>Размер:</strong> ${text.length} байт</p>
            <p><strong>Превью:</strong></p>
            <pre style="font-size:0.7rem;color:#8899bb;max-height:200px;overflow:auto;">${text.substring(0, 800)}…</pre>
        `;

        // Визуализация генома как "дерева параметров"
        const geometry = new THREE.IcosahedronGeometry(2, 1);
        const material = new THREE.MeshStandardMaterial({
            color: 0xaa88ff, wireframe: true, emissive: 0x442266
        });
        const genome = new THREE.Mesh(geometry, material);
        contentGroup.add(genome);

        setStatus('DNA загружен (превью в панели)');
    }

    // ---------- Определение формата по расширению ----------
    function getExtension(filename) {
        const parts = filename.split('.');
        return parts.length > 1 ? parts.pop().toLowerCase() : '';
    }

    // ---------- Обработчик файла ----------
    document.getElementById('file-input').addEventListener('change', async (e) => {
        const file = e.target.files[0];
        if (!file) return;

        setStatus(`Загрузка ${file.name}…`);
        const buffer = await file.arrayBuffer();
        const ext = getExtension(file.name);

        try {
            switch (ext) {
                case 'vxl': renderVXL(buffer); break;
                case 'rgs': renderRGS(buffer); break;
                case 'dna': renderDNA(buffer); break;
                case 'swt':
                    // SWT — это ZIP+JSON, покажем метаданные
                    infoEl.innerHTML = `<p><strong>Тип:</strong> SWT (ландшафт)</p><p><strong>Размер:</strong> ${buffer.byteLength} байт</p>`;
                    setStatus('SWT загружен (парсинг ZIP не реализован)');
                    break;
                default:
                    setStatus(`Неизвестный формат: .${ext}`);
            }
        } catch (err) {
            console.error(err);
            setStatus(`Ошибка: ${err.message}`);
        }
    });

    // ---------- Кнопки управления ----------
    document.getElementById('btn-wireframe').addEventListener('click', () => {
        currentWireframe = !currentWireframe;
        contentGroup.traverse(c => {
            if (c.isMesh || c.isInstancedMesh) {
                if (Array.isArray(c.material)) c.material.forEach(m => m.wireframe = currentWireframe);
                else c.material.wireframe = currentWireframe;
            }
        });
    });

    document.getElementById('btn-reset-cam').addEventListener('click', () => {
        camera.position.set(20, 15, 20);
        controls.target.set(0, 0, 0);
        controls.update();
    });

    document.getElementById('btn-screenshot').addEventListener('click', () => {
        renderer.render(scene, camera);
        const url = renderer.domElement.toDataURL('image/png');
        const a = document.createElement('a');
        a.href = url;
        a.download = `sdgs_screenshot_${Date.now()}.png`;
        a.click();
    });

    // ---------- Resize ----------
    window.addEventListener('resize', () => {
        camera.aspect = container.clientWidth / container.clientHeight;
        camera.updateProjectionMatrix();
        renderer.setSize(container.clientWidth, container.clientHeight);
    });

    // ---------- Анимация ----------
    function animate() {
        requestAnimationFrame(animate);
        controls.update();
        renderer.render(scene, camera);
    }
    animate();
})();