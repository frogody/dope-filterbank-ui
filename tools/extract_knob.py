#!/usr/bin/env python3
"""Extract a knob from the Midjourney render and prepare it for JUCE."""
from PIL import Image, ImageDraw, ImageFilter
import math

# Load the Midjourney render
src = Image.open("/Users/olmorooijackers/Downloads/olmo1995_httpss.mj.runKTeVXgGyDEI_3D_product_render_of_this_e_d9768d6a-3c36-421d-980d-5680afacf1e8_3.png")
print(f"Source: {src.size}")

# The top-right large knob looks cleanest — let's crop it
# Image is 1264x720 based on typical Midjourney output
w, h = src.size

# Identify the large knob positions (approximate from visual inspection)
# Top-right large knob appears to be around 75-85% x, 20-45% y
# Let's try a few crops and save them for inspection

crops = {
    "top_right": (int(w * 0.78), int(h * 0.12), int(w * 0.98), int(h * 0.48)),
    "top_left": (int(w * 0.02), int(h * 0.10), int(w * 0.22), int(h * 0.48)),
    "bot_right": (int(w * 0.78), int(h * 0.52), int(w * 0.98), int(h * 0.90)),
    "bot_left": (int(w * 0.02), int(h * 0.52), int(w * 0.22), int(h * 0.90)),
}

for name, box in crops.items():
    crop = src.crop(box)
    crop.save(f"../assets/knob_crop_{name}.png")
    print(f"  {name}: {box} -> {crop.size}")

print("\nCrops saved to assets/. Check which one has the cleanest knob.")
