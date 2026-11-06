import webview, os, threading
from mca2raw_core import MCA2RawCore

class Api:
    def __init__(self):
        self._window = None
        self.core = MCA2RawCore()
        self.html = os.path.join(os.path.dirname(__file__), "index.html")

    def set_window(self, window):
        self._window = window

    def start_export(self, region_path, output_path, seed):
        threading.Thread(target=self._run, args=(region_path, output_path, seed)).start()
        return "Started"

    def _run(self, r_path, o_path, seed):
        def on_table(lines):
            safe = [l.replace('\\', '\\\\').replace('"', '\\"') for l in lines]
            self._window.evaluate_js(f"renderTable({safe})")
            
        def on_progress(pct):
            self._window.evaluate_js(f"updateProgress({pct})")

        self._window.evaluate_js("logToConsole('Запуск Java-процесса...')")
        res = self.core.export(r_path, o_path, seed, on_table, on_progress)
        
        if res == "SUCCESS":
            self._window.evaluate_js("logToConsole('✅ ЭКСПОРТ ЗАВЕРШЕН!')")
        else:
            self._window.evaluate_js(f"logToConsole('❌ Ошибка: {res}')")

if __name__ == '__main__':
    api = Api()
    window = webview.create_window('MCA2RAW Protocol', api.html, js_api=api, width=950, height=700)
    api.set_window(window)
    webview.start()