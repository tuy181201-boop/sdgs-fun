(function () {
    'use strict';

    // ---------- Сцена ----------
    const container = document.getElementById('viewport');
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x0a0e1a);

    const camera = new THREE.PerspectiveCamera(45, container.clientWidth / container.clientHeight, 0.1, 1000);
    camera.position.set(15, 12, 15);

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

    const grid = new THREE.GridHelper(20, 20, 0x2a3a66, 0x1a2a4a);
    scene.add(grid);

    // ---------- Данные ----------
    // voxelMap: "x,y,z" -> { layerId, color }
    const voxelMap = new Map();
    const layers = [{ id: 0, name: 'Слой 1', visible: true }];
    let activeLayer = 0;

    const cubeGeometry = new THREE.BoxGeometry(1, 1, 1);

    // Группы для отрисовки
    const meshGroup = new THREE.Group();
    scene.add(meshGroup);
    const meshMap = new Map(); // "x,y,z" -> Mesh

    // ---------- Утилиты ----------
    function key(x, y, z) { return `${Math.round(x)},${Math.round(y)},${Math.round(z)}`; }

    function addVoxel(x, y, z, color) {
        x = Math.round(x); y = Math.round(y); z = Math.round(z);
        const k = key(x, y, z);
        if (voxelMap.has(k)) return false;

        // Симметрия
        const positions = [[x, y, z]];
        const symX = document.getElementById('sym-x').checked;
        const symY = document.getElementById('sym-y').checked;
        const symZ = document.getElementById('sym-z').checked;

        for (const [px, py, pz] of positions.slice()) {
            if (symX) positions.push([-px, py, pz]);
            if (symY) positions.push([px, -py, pz]);
            if (symZ) positions.push([px, py, -pz]);
        }

        let added = false;
        for (const [px, py, pz] of positions) {
            const kk = key(px, py, pz);
            if (voxelMap.has(kk)) continue;

            const material = new THREE.MeshStandardMaterial({
                color, roughness: 0.4, metalness: 0.1,
                emissive: new THREE.Color(color).multiplyScalar(0.1)
            });
            const mesh = new THREE.Mesh(cubeGeometry, material);
            mesh.position.set(px, py, pz);
            meshGroup.add(mesh);

            voxelMap.set(kk, { layerId: activeLayer, color: color.clone() });
            meshMap.set(kk, mesh);
            added = true;
        }

        if (added) updateStats();
        return added;
    }

    function removeVoxel(x, y, z) {
        const k = key(x, y, z);
        const mesh = meshMap.get(k);
        if (!mesh) return false;
        mesh.material.dispose();
        meshGroup.remove(mesh);
        meshMap.delete(k);
        voxelMap.delete(k);
        updateStats();
        return true;
    }

    function clearAll() {
        for (const [k, mesh] of meshMap) {
            mesh.material.dispose();
            meshGroup.remove(mesh);
        }
        voxelMap.clear();
        meshMap.clear();
        updateStats();
    }

    function updateStats() {
        document.getElementById('count').textContent = voxelMap.size;
        document.getElementById('layer-count').textContent = layers.length;
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

        const meshes = [...meshMap.values(), plane];
        return raycaster.intersectObjects(meshes);
    }

    function getTargetPos(intersect) {
        if (intersect.object === plane) {
            return { x: Math.round(intersect.point.x), y: 0, z: Math.round(intersect.point.z) };
        }
        const n = intersect.face.normal.clone();
        n.transformDirection(intersect.object.matrixWorld);
        const dir = new THREE.Vector3(Math.round(n.x), Math.round(n.y), Math.round(n.z));
        const p = intersect.object.position.clone().add(dir);
        return { x: Math.round(p.x), y: Math.round(p.y), z: Math.round(p.z) };
    }

    // ---------- Инструменты ----------
    let currentTool = 'brush';
    let brushRadius = 3;
    let currentColor = new THREE.Color('#44ff88');
    let isDrawing = false;

    document.getElementById('tool-select').addEventListener('change', (e) => currentTool = e.target.value);
    document.getElementById('size-slider').addEventListener('input', (e) => {
        brushRadius = parseInt(e.target.value);
        document.getElementById('size-label').textContent = brushRadius;
    });
    document.getElementById('color').addEventListener('input', (e) => currentColor.set(e.target.value));

    function applyTool(center) {
        if (currentTool === 'erase') {
            removeVoxel(center.x, center.y, center.z);
            return;
        }

        if (currentTool === 'brush') {
            addVoxel(center.x, center.y, center.z, currentColor);
        } else if (currentTool === 'sphere') {
            const r = brushRadius;
            for (let dx = -r; dx <= r; dx++)
                for (let dy = -r; dy <= r; dy++)
                    for (let dz = -r; dz <= r; dz++)
                        if (dx*dx + dy*dy + dz*dz <= r*r)
                            addVoxel(center.x + dx, center.y + dy, center.z + dz, currentColor);
        } else if (currentTool === 'box') {
            const r = brushRadius;
            for (let dx = -r; dx <= r; dx++)
                for (let dy = -r; dy <= r; dy++)
                    for (let dz = -r; dz <= r; dz++)
                        addVoxel(center.x + dx, center.y + dy, center.z + dz, currentColor);
        }
    }

    // ---------- События мыши ----------
    renderer.domElement.addEventListener('mousedown', (e) => {
        const intersects = getIntersects(e);
        if (intersects.length === 0) return;

        if (e.button === 0) { isDrawing = true; applyTool(getTargetPos(intersects[0])); }
        else if (e.button === 2) {
            const obj = intersects[0].object;
            if (obj !== plane) removeVoxel(obj.position.x, obj.position.y, obj.position.z);
        }
    });

    renderer.domElement.addEventListener('mousemove', (e) => {
        if (!isDrawing) return;
        const intersects = getIntersects(e);
        if (intersects.length === 0) return;
        applyTool(getTargetPos(intersects[0]));
    });

    renderer.domElement.addEventListener('mouseup', () => isDrawing = false);
    renderer.domElement.addEventListener('contextmenu', (e) => e.preventDefault());

    // ---------- Слои ----------
    function renderLayers() {
        const el = document.getElementById('layers');
        el.innerHTML = '';
        layers.forEach((layer, i) => {
            const div = document.createElement('div');
            div.className = 'layer-item' + (layer.id === activeLayer ? ' active' : '');
            div.innerHTML = `<span>${layer.name}</span><span class="visibility">${layer.visible ? '👁' : '👁‍🗨'}</span>`;
            div.addEventListener('click', () => { activeLayer = layer.id; renderLayers(); });
            div.querySelector('.visibility').addEventListener('click', (ev) => {
                ev.stopPropagation();
                layer.visible = !layer.visible;
                // Скрыть/показать воксели слоя
                for (const [k, data] of voxelMap) {
                    const mesh = meshMap.get(k);
                    if (data.layerId === layer.id) mesh.visible = layer.visible;
                }
                renderLayers();
            });
            el.appendChild(div);
        });
    }

    document.getElementById('btn-add-layer').addEventListener('click', () => {
        const id = layers.length;
        layers.push({ id, name: `Слой ${id + 1}`, visible: true });
        activeLayer = id;
        renderLayers();
    });

    document.getElementById('btn-clear').addEventListener('click', () => {
        if (confirm('Очистить всё?')) clearAll();
    });

    // ---------- Экспорт JSON ----------
    document.getElementById('btn-export-json').addEventListener('click', () => {
        const data = {
            layers,
            voxels: [...voxelMap.entries()].map(([k, v]) => ({
                pos: k.split(',').map(Number),
                layerId: v.layerId,
                color: v.color.getHexString()
            }))
        };
        download(JSON.stringify(data, null, 2), 'voxel_editor.json', 'application/json');
    });

    // ---------- Экспорт VXPS ----------
    document.getElementById('btn-export-vxps').addEventListener('click', () => {
        if (voxelMap.size === 0) return alert('Пустая модель');

        let minX = Infinity, maxX = -Infinity, minY = Infinity, maxY = -Infinity, minZ = Infinity, maxZ = -Infinity;
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

        for (const [k, v] of voxelMap) {
            const [x, y, z] = k.split(',').map(Number);
            const r = Math.round(v.color.r * 255);
            const g = Math.round(v.color.g * 255);
            const b = Math.round(v.color.b * 255);
            code += `    Voxels[${x - minX}, ${y - minY}, ${z - minZ}] := (R=${r}, G=${g}, B=${b});\n`;
        }

        code += `end;\n\nbegin\n    FillVoxels;\nend.\n`;
        download(code, 'model.vxps', 'text/plain');
    });

    function download(data, filename, mime) {
        const blob = new Blob([data], { type: mime });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url; a.download = filename;
        document.body.appendChild(a); a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    }

    // ---------- Инициализация ----------
    renderLayers();
    updateStats();

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