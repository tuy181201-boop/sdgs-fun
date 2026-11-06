import subprocess, os

class MCA2RawCore:
    def __init__(self):
        self.base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        self.libs_dir = os.path.join(self.base_dir, "libs")
        self.jar_path = os.path.join(self.libs_dir, "McExtractor.jar")
        self.nbt_jar = os.path.join(self.libs_dir, "NBT-6.1.jar")
        
        # Ищем локальную Java в MCAGenerator
        local_java = os.path.join(self.base_dir, "MCAGenerator", "jdk-21", "bin", "java.exe")
        self.java_exec = local_java if os.path.exists(local_java) else "java"

    def export(self, region_path, output_path, seed, on_table=None, on_progress=None):
        final_out = os.path.join(output_path, seed)
        os.makedirs(final_out, exist_ok=True)
        
        # Используем найденный путь к Java
        cmd = [
            self.java_exec, 
            "-cp", f"{self.jar_path};{self.nbt_jar}", 
            "McExtractor", 
            region_path, 
            final_out
        ]
        
        try:
            process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1)
            in_table = False
            table_lines = []

            for line in process.stdout:
                line = line.strip()
                if line == "TABLE_START":
                    in_table = True
                    continue
                elif line == "TABLE_END":
                    in_table = False
                    if on_table: on_table(table_lines)
                    continue
                
                if in_table:
                    table_lines.append(line)
                elif line.startswith("PROGRESS:"):
                    if on_progress: on_progress(int(line.split(":")[1]))
                elif "ERROR" in line:
                    print("JAVA ERROR:", line) 
                    
            process.wait()
            return "SUCCESS" if process.returncode == 0 else "ERROR"
        except Exception as e:
            return f"CRITICAL ERROR: {str(e)}"