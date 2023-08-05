from unihiker import GUI
import time

gui = GUI()

gui.draw_text(x=30, y=88, color="red", text="你", font_size=20) # Display the text "你" in red color with font size 20 at the coordinates (30, 88)
gui.draw_text(x=60, y=88, color="orange", text="好", font_size=20,) # Display the text "好" in orange color with font size 20 at the coordinates (60, 88)
gui.draw_text(x=90, y=88, color="yellow", text=",", font_size=20) # Display a comma in yellow color with font size 20 at the coordinates (90, 88)
gui.draw_text(x=120, y=88, color="green", text="行", font_size=20) # Display the text "行" in green color with font size 20 at the coordinates (120, 88)
gui.draw_text(x=150, y=88, color="cyan", text="空", font_size=20) # Display the text "空" in cyan color with font size 20 at the coordinates (150, 88)
gui.draw_text(x=180, y=88, color="blue", text="板", font_size=20) # Display the text "板" in blue color with font size 20 at the coordinates (180, 88)
gui.draw_text(x=210, y=88, color="purple", text="!", font_size=20) # Display an exclamation mark in purple color with font size 20 at the coordinates (210, 88)

gui.draw_text(x=15, y=150, color=(255,105,180), text="Hello,", font_size=20) # Display the text "Hello," in a custom color with font size 20 at the coordinates (15, 150)
gui.draw_text(x=95, y=150, color=(0,191,255), text="UNIHIKER!", font_size=20) # Display the text "UNIHIKER!" in a custom color with font size 20 at the coordinates (95, 150)

gui.draw_emoji(x=120, y=230, w=100, h=100, emoji="Wink", duration=0.1,origin="center") # Display the built-in emoji "Wink" at the coordinates (120, 230), with a size of 100x100 pixels, a duration of 0.1 seconds for each image switch, and center alignment.

while True:  # Loop indefinitely
    time.sleep(1)  # Delay for 1 second