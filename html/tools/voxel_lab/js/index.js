(function () {
    'use strict';

    // ---------- Сцена ----------
    const container = document.getElementById('viewport');
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x0a0e1a);

    const camera = new THREE.PerspectiveCamera(45, container.clientWidth / container.clientHeight, 0.1, 1000);
    camera.position.set(12, 10, 12);

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

    // ---------- Сетка ----------
    const grid = new THREE.GridHelper(20, 20, 0x2a3a66, 0x1a2a4a);
    grid.position.y = -0.5;
    scene.add(grid);

    // ---------- Состояние ----------
    const voxelMap = new Map(); // "x,y,z" -> { mesh, color }
    let currentColor = new THREE.Color('#00d4ff');
    let brushSize = 1;
    let isDrawing = false;
    let isErasing = false;

    // ---------- Палитра ----------
    const presets = ['#00d4ff', '#ff4da6', '#ffaa00', '#44ff88', '#aa88ff', '#ff6b6b',
                     '#ffffff', '#ff8844', '#00ffcc', '#ff00ff', '#4488ff', '#000000'];
    const paletteEl = document.getElementById('palette');

    presets.forEach(hex => {
        const swatch = document.createElement('div');
        swatch.className = 'swatch';
        swatch.style.background = hex;
        swatch.dataset.color = hex;
        swatch.addEventListener('click', () => setColor(hex));
        paletteEl.appendChild(swatch);
    });

    function setColor(hex) {
        currentColor.set(hex);
        document.querySelectorAll('.swatch').forEach(el => {
            el.classList.toggle('active', el.dataset.color === hex);
        });
        document.getElementById('custom-color').value = hex;
    }
    setColor('#00d4ff');

    document.getElementById('custom-color').addEventListener('input', (e) => setColor(e.target.value));

    // ---------- Воксели ----------
    const cubeGeometry = new THREE.BoxGeometry(1, 1, 1);

    function key(x, y, z) { return `${Math.round(x)},${Math.round(y)},${Math.round(z)}`; }

    function addVoxel(x, y, z, color) {
        const k = key(x, y, z);
        if (voxelMap.has(k)) return false;

        const material = new THREE.MeshStandardMaterial({
            color, roughness: 0.4, metalness: 0.1,
            emissive: new THREE.Color(color).multiplyScalar(0.1)
        });
        const mesh = new THREE.Mesh(cubeGeometry, material);
        mesh.position.set(Math.round(x), Math.round(y), Math.round(z));
        scene.add(mesh);

        voxelMap.set(k, { mesh, color: color.clone() });
        updateStats();
        return true;
    }

    function removeVoxel(x, y, z) {
        const k = key(x, y, z);
        const v = voxelMap.get(k);
        if (!v) return false;
        v.mesh.material.dispose();
        scene.remove(v.mesh);
        voxelMap.delete(k);
        updateStats();
        return true;
    }

    function clearAll() {
        for (const { mesh } of voxelMap.values()) {
            mesh.material.dispose();
            scene.remove(mesh);
        }
        voxelMap.clear();
        updateStats();
    }

    function updateStats() {
        document.getElementById('count').textContent = voxelMap.size;
        const voxelSize = 0.316227766; // √10/10 мм
        const volume = voxelMap.size * Math.pow(voxelSize, 3);
        document.getElementById('volume').textContent = (volume * 1000).toFixed(2);
    }

    // ---------- Raycaster ----------
    const raycaster = new THREE.Raycaster();
    const mouse = new THREE.Vector2();

    const plane = new THREE.Mesh(
        new THREE.PlaneGeometry(50, 50),
        new THREE.MeshBasicMaterial({ visible: false, side: THREE.DoubleSide })
    );
    plane.rotation.x = -Math.PI / 2;
    scene.add(plane);

    function getIntersects(event) {
        const rect = renderer.domElement.getBoundingClientRect();
        mouse.x = ((event.clientX - rect.left) / rect.width) * 2 - 1;
        mouse.y = -((event.clientY - rect.top) / rect.height) * 2 + 1;
        raycaster.setFromCamera(mouse, camera);

        const meshes = [];
        for (const { mesh } of voxelMap.values()) meshes.push(mesh);
        meshes.push(plane);
        return raycaster.intersectObjects(meshes);
    }

    function getAddPos(intersect) {
        if (intersect.object === plane) {
            return {
                x: Math.round(intersect.point.x),
                y: 0,
                z: Math.round(intersect.point.z)
            };
        }
        const normal = intersect.face.normal.clone();
        normal.transformDirection(intersect.object.matrixWorld);
        const dir = new THREE.Vector3(Math.round(normal.x), Math.round(normal.y), Math.round(normal.z));
        const pos = intersect.object.position.clone().add(dir);
        return { x: Math.round(pos.x), y: Math.round(pos.y), z: Math.round(pos.z) };
    }

    function paintBrush(center, color) {
        const half = Math.floor(brushSize / 2);
        for (let dx = -half; dx <= half; dx++)
            for (let dy = -half; dy <= half; dy++)
                for (let dz = -half; dz <= half; dz++)
                    addVoxel(center.x + dx, center.y + dy, center.z + dz, color);
    }

    // ---------- События мыши ----------
    renderer.domElement.addEventListener('mousedown', (e) => {
        const intersects = getIntersects(e);
        if (intersects.length === 0) return;

        if (e.button === 0) { isDrawing = true; paintBrush(getAddPos(intersects[0]), currentColor); }
        else if (e.button === 2) {
            isErasing = true;
            const obj = intersects[0].object;
            if (obj !== plane) {
                const pos = obj.position;
                removeVoxel(pos.x, pos.y, pos.z);
            }
        }
    });

    renderer.domElement.addEventListener('mousemove', (e) => {
        if (!isDrawing && !isErasing) return;
        const intersects = getIntersects(e);
        if (intersects.length === 0) return;

        if (isDrawing) paintBrush(getAddPos(intersects[0]), currentColor);
        else if (isErasing) {
            const obj = intersects[0].object;
            if (obj !== plane) {
                const pos = obj.position;
                removeVoxel(pos.x, pos.y, pos.z);
            }
        }
    });

    renderer.domElement.addEventListener('mouseup', () => { isDrawing = false; isErasing = false; });
    renderer.domElement.addEventListener('mouseleave', () => { isDrawing = false; isErasing = false; });
    renderer.domElement.addEventListener('contextmenu', (e) => e.preventDefault());

    document.getElementById('brush-size').addEventListener('change', (e) => brushSize = parseInt(e.target.value));

    // ---------- Экспорт VXPS ----------
    function exportVXPS() {
        if (voxelMap.size === 0) return alert('Пустая модель');

        let minX = Infinity, maxX = -Infinity;
        let minY = Infinity, maxY = -Infinity;
        let minZ = Infinity, maxZ = -Infinity;

        for (const k of voxelMap.keys()) {
            const [x, y, z] = k.split(',').map(Number);
            if (x < minX) minX = x; if (x > maxX) maxX = x;
            if (y < minY) minY = y; if (y > maxY) maxY = y;
            if (z < minZ) minZ = z; if (z > maxZ) maxZ = z;
        }

        const W = maxX - minX + 1, H = maxY - minY + 1, D = maxZ - minZ + 1;
        let code = `program ${Date.now().toString(16)};\n\n`;
        code += `type\n    TPixel = packed record\n        R, G, B: byte;\n    end;\n\n`;
        code += `const\n    WIDTH = ${W};\n    HEIGHT = ${H};\n    DEPTH = ${D};\n\n`;
        code += `var\n    Voxels: array[0..WIDTH-1, 0..HEIGHT-1, 0..DEPTH-1] of TPixel;\n\n`;
        code += `procedure FillVoxels;\nbegin\n`;

        for (const [k, { color }] of voxelMap) {
            const [x, y, z] = k.split(',').map(Number);
            const r = Math.round(color.r * 255);
            const g = Math.round(color.g * 255);
            const b = Math.round(color.b * 255);
            code += `    Voxels[${x - minX}, ${y - minY}, ${z - minZ}] := (R=${r}, G=${g}, B=${b});\n`;
        }

        code += `end;\n\nbegin\n    FillVoxels;\nend.\n`;
        download(code, 'model.vxps', 'text/plain');
    }

    // ---------- Экспорт VXL ----------
    function exportVXL() {
        if (voxelMap.size === 0) return alert('Пустая модель');

        let minX = Infinity, maxX = -Infinity;
        let minY = Infinity, maxY = -Infinity;
        let minZ = Infinity, maxZ = -Infinity;

        for (const k of voxelMap.keys()) {
            const [x, y, z] = k.split(',').map(Number);
            if (x < minX) minX = x; if (x > maxX) maxX = x;
            if (y < minY) minY = y; if (y > maxY) maxY = y;
            if (z < minZ) minZ = z; if (z > maxZ) maxZ = z;
        }

        const W = maxX - minX + 1, H = maxY - minY + 1, D = maxZ - minZ + 1;
        const BPP = 12; // RGBA + Mass (uint64)
        const total = W * H * D;
        const data = new Uint8Array(total * BPP);

        for (const [k, { color }] of voxelMap) {
            const [x, y, z] = k.split(',').map(Number);
            const idx = ((x - minX) + (y - minY) * W + (z - minZ) * W * H) * BPP;
            data[idx] = Math.round(color.r * 255);
            data[idx + 1] = Math.round(color.g * 255);
            data[idx + 2] = Math.round(color.b * 255);
            data[idx + 3] = 255;
            // Mass — 8 байт, оставляем нулями
        }

        // Заголовок [UID 16][D 1][Lx 1][Ly 1][Lz 1][Lc 1][X 1][Y 1][Z 1][C 1]
        const header = new Uint8Array(16 + 1 + 4 + 4);
        // UID — генерируем случайный
        crypto.getRandomValues(header.subarray(0, 16));
        header[16] = 4;         // D
        header[17] = 1;         // Lx
        header[18] = 1;         // Ly
        header[19] = 1;         // Lz
        header[20] = 1;         // Lc
        header[21] = W;
        header[22] = H;
        header[23] = D;
        header[24] = 5;         // C = RGBA + Mass

        const final = new Uint8Array(header.length + data.length);
        final.set(header, 0);
        final.set(data, header.length);

        download(final, 'model.vxl', 'application/octet-stream');
    }

    // ---------- Экспорт STL ----------
    function exportSTL() {
        if (voxelMap.size === 0) return alert('Пустая модель');

        const tempScene = new THREE.Scene();
        for (const { mesh } of voxelMap.values()) {
            const clone = new THREE.Mesh(cubeGeometry, new THREE.MeshBasicMaterial());
            clone.position.copy(mesh.position);
            tempScene.add(clone);
        }

        const exporter = new THREE.STLExporter();
        const result = exporter.parse(tempScene, { binary: true });
        download(result, 'model.stl', 'application/octet-stream');
    }

    function download(data, filename, mime) {
        const blob = new Blob([data], { type: mime });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url; a.download = filename;
        document.body.appendChild(a); a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    }

    document.getElementById('btn-vxps').addEventListener('click', exportVXPS);
    document.getElementById('btn-vxl').addEventListener('click', exportVXL);
    document.getElementById('btn-stl').addEventListener('click', exportSTL);
    document.getElementById('btn-clear').addEventListener('click', () => {
        if (confirm('Очистить модель?')) clearAll();
    });

    // ---------- Undo/Redo (упрощённо) ----------
    const history = [];
    let historyIndex = -1;

    function snapshot() {
        const snap = {};
        for (const [k, v] of voxelMap) snap[k] = v.color.getHex();
        return snap;
    }

    function restore(snap) {
        clearAll();
        for (const [k, hex] of Object.entries(snap)) {
            const [x, y, z] = k.split(',').map(Number);
            addVoxel(x, y, z, new THREE.Color(hex));
        }
    }

    document.getElementById('btn-undo').addEventListener('click', () => {
        if (historyIndex > 0) restore(history[--historyIndex]);
    });
    document.getElementById('btn-redo').addEventListener('click', () => {
        if (historyIndex < history.length - 1) restore(history[++historyIndex]);
    });

    // ---------- Resize & Animation ----------
    window.addEventListener('resize', () => {
        camera.aspect = container.clientWidth / container.clientHeight;
        camera.updateProjectionMatrix();
        renderer.setSize(container.clientWidth, container.clientHeight);
    });

    function animate() {
        requestAnimationFrame(animate);
        controls.update();
        renderer.render(scene, camera);
    }
    animate();
})();