import os, re, random, shutil, subprocess, time, threading, webview
from mcrcon import MCRcon
from mca2raw_core import MCA2RawCore

# --- УМНЫЕ ПУТИ ---
APP_DIR = os.path.dirname(os.path.abspath(__file__))
BASE_DIR = os.path.dirname(APP_DIR)
SERVER_DIR = os.path.join(BASE_DIR, "MCAGenerator")
LIBS_DIR = os.path.join(BASE_DIR, "libs")

SERVER_JAR = "server.jar"
WORLD_NAME = "world"
RCON_HOST = "127.0.0.1"
RCON_PORT = 25575
RCON_PASS = "mcautogen"
JAVA_ARGS = ["-Xms2G", "-Xmx4G"]
OUTPUT_DIR = os.path.join(BASE_DIR, "output")

# Ищем локальную Java 21 внутри MCAGenerator
LOCAL_JAVA = os.path.join(SERVER_DIR, "jdk-21", "bin", "java.exe")
JAVA_EXEC = LOCAL_JAVA if os.path.exists(LOCAL_JAVA) else "java" 
# Если локальной нет, возьмет системную

import urllib.request
import zipfile
def ensure_jdk():
    """Проверяет наличие JDK 21. Если нет — скачивает и распаковывает."""
    jdk_dir = os.path.join(SERVER_DIR, "jdk-21")
    java_exe = os.path.join(jdk_dir, "bin", "java.exe")

    if os.path.exists(java_exe):
        return True # JDK уже на месте

    print("JDK 21 не найден. Начинаем автоматическое скачивание...")
    # Прямая ссылка на официальный архив Temurin 21 (безопасная)
    url = "https://github.com/adoptium/temurin21-binaries/releases/download/jdk-21.0.13%2B9/OpenJDK21U-jdk_x64_windows_hotspot_21.0.13_9.zip"
    zip_path = os.path.join(SERVER_DIR, "jdk.zip")

    try:
        # 1. Скачиваем архив
        urllib.request.urlretrieve(url, zip_path)
        
        # 2. Распаковываем
        print("Распаковка JDK...")
        with zipfile.ZipFile(zip_path, 'r') as zip_ref:
            zip_ref.extractall(SERVER_DIR)
            
        # 3. Переименовываем папку (архив распаковывается как jdk-21.0.13+9)
        extracted_dir = os.path.join(SERVER_DIR, "jdk-21.0.13+9")
        if os.path.exists(extracted_dir):
            os.rename(extracted_dir, jdk_dir)
            
        # 4. Удаляем архив
        os.remove(zip_path)
        print("✅ JDK 21 успешно установлен!")
        return True
    except Exception as e:
        print(f"❌ Ошибка при скачивании JDK: {e}")
        print("Скачайте вручную: " + url)
        return False

class Api:
    def __init__(self):
        self.window = None
        self.proc = None
        self.rcon = None
        self.running = False

    def set_window(self, window): self.window = window

    def log(self, text):
        if self.window:
            safe = text.replace("\\", "\\\\").replace("`", "\\`").replace("$", "\\$")
            self.window.evaluate_js(f"appendLog(`{safe}`)")

    def set_progress(self, percent, eta=""):
        if self.window: self.window.evaluate_js(f"setProgress({percent}, '{eta}')")

    def start_generation(self, seed_str):
        if self.running: return {"ok": False}
        self.running = True
        threading.Thread(target=self._run, args=(seed_str,), daemon=True).start()
        return {"ok": True}

    def _run(self, seed_str):
        try:
            if not ensure_jdk():
                self.log("❌ Не удалось установить JDK 21.")
                return
            seed = int(seed_str.strip()) if seed_str.strip() else random.randint(-2**63, 2**63 - 1)
            self.log(f"Сид: {seed}")

            if os.path.exists(WORLD_NAME): shutil.rmtree(WORLD_NAME)
            self._set_seed_in_properties(seed)

            with open("server_log.txt", "w") as log_file:
                # Используем JAVA_EXEC вместо "java"
                self.proc = subprocess.Popen([JAVA_EXEC, *JAVA_ARGS, "-jar", SERVER_JAR, "nogui"], stdout=log_file, stderr=subprocess.STDOUT)

            if not self._wait_for_rcon(120): return self.log("❌ RCON не поднялся.")

            for cmd in ["chunky center 0 0", "chunky radius 192c", "chunky pattern region", "chunky start"]:
                self.rcon.command(cmd); time.sleep(0.5)

            while True:
                out = self.rcon.command("chunky progress")
                m = re.search(r'(\d+(?:\.\d+)?)%', out)
                pct = float(m.group(1)) if m else 0.0
                self.set_progress(pct)
                if pct >= 100: break
                time.sleep(5)

            self.rcon.command("save-all"); time.sleep(3)
            self.rcon.command("stop"); self.proc.wait(60)

            dst = os.path.join(OUTPUT_DIR, f"seed_{seed}")
            os.makedirs(dst, exist_ok=True)
            for f in os.listdir(os.path.join(WORLD_NAME, "region")):
                if f.endswith(".mca"): shutil.copy2(os.path.join(WORLD_NAME, "region", f), dst)

            self._run_mca_extractor(dst, seed)
        except Exception as e:
            self.log(f"Критическая ошибка: {e}")
        finally:
            self.running = False
            if self.rcon: self.rcon.disconnect()

    def _set_seed_in_properties(self, seed):
        lines = []
        if os.path.exists("server.properties"):
            with open("server.properties", "r", encoding="utf-8") as f: lines = f.readlines()
        lines = [l for l in lines if not l.startswith("level-seed=")]
        lines.append(f"level-seed={seed}\n")
        with open("server.properties", "w", encoding="utf-8") as f: f.writelines(lines)

    def _run_mca_extractor(self, region_dir, seed):
        out_raw = os.path.join(OUTPUT_DIR, f"seed_{seed}", "raw")
        proc = subprocess.Popen(["java", "-jar", MCA_EXTRACTOR_JAR, region_dir, out_raw], stdout=subprocess.PIPE, text=True)
        for line in proc.stdout:
            if line.startswith("PROGRESS:"): self.set_progress(int(line.split(":")[1]), "Convert")
            elif "SUCCESS" in line: self.log("✅ RAW готовы!")

    def _wait_for_rcon(self, timeout):
        start = time.time()
        while time.time() - start < timeout:
            try:
                self.rcon = MCRcon(RCON_HOST, RCON_PASS, port=RCON_PORT); self.rcon.connect(); return True
            except: time.sleep(2)
        return False

if __name__ == "__main__":
    api = Api()
    window = webview.create_window("Zero-Touch Generator", "index.html", js_api=api, width=720, height=520)
    api.set_window(window)
    webview.start()