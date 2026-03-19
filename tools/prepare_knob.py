#!/usr/bin/env python3
"""Crop and clean the knob from the Midjourney render."""
from PIL import Image, ImageDraw, ImageFilter

src = Image.open("/Users/olmorooijackers/Downloads/olmo1995_httpss.mj.runKTeVXgGyDEI_3D_product_render_of_this_e_d9768d6a-3c36-421d-980d-5680afacf1e8_3.png")
w, h = src.size

# The top-left large knob — refine the crop to just the knob circle
# From inspection: knob center is roughly at x=155, y=265 in the full image
# Knob radius is about 95px
kcx, kcy, kr = 155, 265, 100
pad = 20  # extra space for glow ring

box = (kcx - kr - pad, kcy - kr - pad, kcx + kr + pad, kcy + kr + pad)
knob_crop = src.crop(box)

# Make it square (it should already be)
size = min(knob_crop.size)
knob_crop = knob_crop.resize((size, size), Image.LANCZOS)

# Create circular mask (soft edge for clean cutout)
mask = Image.new('L', (size, size), 0)
mask_draw = ImageDraw.Draw(mask)
margin = 4
mask_draw.ellipse([margin, margin, size - margin, size - margin], fill=255)
mask = mask.filter(ImageFilter.GaussianBlur(radius=3))

# Apply mask
knob_rgba = knob_crop.convert('RGBA')
r, g, b, a = knob_rgba.split()
knob_rgba = Image.merge('RGBA', (r, g, b, mask))

# Save at 200px for quality
knob_final = knob_rgba.resize((200, 200), Image.LANCZOS)
knob_final.save("../assets/knob_body.png")
print(f"Saved knob_body.png ({knob_final.size})")

# Also save a version without the pointer area (for static body)
# We'll paint the pointer in JUCE code, so the baked-in pointer is OK for now
# — it adds to the realism

# Preview: show on black background
preview = Image.new('RGBA', (250, 250), (5, 6, 8, 255))
preview.paste(knob_final, (25, 25), knob_final)
preview.save("../assets/knob_body_preview.png")
print("Saved knob_body_preview.png")
