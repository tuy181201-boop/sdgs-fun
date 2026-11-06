import time
import os
from PIL import Image, ImageDraw

UI_PATH = "C:/Users/Tuy181201/Desktop/ui.png"
CLICK_PATH = "C:/Users/Tuy181201/Desktop/click.txt"
TEMP_PATH = "C:/Users/Tuy181201/Desktop/ui_temp.png" # Используем нормальное имя

def draw_ui(state):
    img = Image.new('RGBA', (800, 600), color=(40, 44, 52, 255))
    draw = ImageDraw.Draw(img)
    
    if state == 0:
        btn_color = (0, 120, 215, 255)
        text = "Real HTML Button!"
    else:
        btn_color = (40, 167, 69, 255)
        text = "Clicked! Wait 1s..."

    draw.rectangle([300, 250, 500, 300], fill=btn_color)
    draw.text((340, 265), text, fill=(255, 255, 255, 255))
    
    try:
        # Явно указываем формат PNG
        img.save(TEMP_PATH, format="PNG")
        if os.path.exists(UI_PATH):
            os.remove(UI_PATH)
        os.rename(TEMP_PATH, UI_PATH)
        print(f"[Python] UI updated to state {state}")
    except Exception as e:
        print(f"[Python] Save error: {e}")

state = 0
draw_ui(state)
print("[Python] Generator started. Waiting for clicks...")

while True:
    try:
        if os.path.exists(CLICK_PATH):
            print("[Python] Signal received! Changing UI...")
            os.remove(CLICK_PATH)
            state = 1
            draw_ui(state)
            time.sleep(1)
            state = 0
            draw_ui(state)
            print("[Python] Back to normal state.")
    except Exception as e:
        print(f"[Python] Loop error: {e}")
    
    time.sleep(0.2)