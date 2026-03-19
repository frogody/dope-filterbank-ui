#!/usr/bin/env python3
"""
Prepare the Midjourney render as a full plugin background.
Resize to 740x540 (our plugin size) and extract clean knob.
"""
from PIL import Image, ImageDraw, ImageFilter

# Load the Midjourney render
src = Image.open("/Users/olmorooijackers/Downloads/olmo1995_httpss.mj.runKTeVXgGyDEI_3D_product_render_of_this_e_d9768d6a-3c36-421d-980d-5680afacf1e8_3.png")
print(f"Source: {src.size}")

# Crop to remove excess black borders (the render has the hardware unit centered)
# The unit itself takes up roughly the center 90% of the image
w, h = src.size
# Trim black borders
left = int(w * 0.02)
top = int(h * 0.02)
right = int(w * 0.98)
bottom = int(h * 0.98)
cropped = src.crop((left, top, right, bottom))

# Resize to exactly 740x540 (our plugin window)
bg = cropped.resize((740, 540), Image.LANCZOS)
bg.save("../assets/ui_background.png", quality=95)
print(f"Saved ui_background.png (740x540)")

# Now extract a CLEAN knob — the top-left large knob
# From the resized 740x540 image, find the knob position
# In 1280x928 original, top-left knob was at ~(152, 240), r~92
# After crop and resize: scale factor = 740/1254 ≈ 0.59 for x, 540/909 ≈ 0.594 for y
scale_x = 740 / (right - left)
scale_y = 540 / (bottom - top)

knob_cx = int((152 - left) * scale_x)
knob_cy = int((240 - top) * scale_y)
knob_r = int(92 * scale_x)

print(f"Knob in resized image: center=({knob_cx}, {knob_cy}), r={knob_r}")

# Extract knob with padding for glow
pad = 8
box = (knob_cx - knob_r - pad, knob_cy - knob_r - pad,
       knob_cx + knob_r + pad, knob_cy + knob_r + pad)
box = (max(0, box[0]), max(0, box[1]), min(740, box[2]), min(540, box[3]))
knob_crop = bg.crop(box)

# Make it square
s = min(knob_crop.size)
knob_crop = knob_crop.crop((0, 0, s, s))

# Circular mask
mask = Image.new('L', (s, s), 0)
ImageDraw.Draw(mask).ellipse([2, 2, s-2, s-2], fill=255)
mask = mask.filter(ImageFilter.GaussianBlur(radius=2))

rgba = knob_crop.convert('RGBA')
r, g, b, a = rgba.split()
rgba = Image.merge('RGBA', (r, g, b, mask))

# Save at 200x200
knob_final = rgba.resize((200, 200), Image.LANCZOS)
knob_final.save("../assets/knob_body.png")

# Preview
preview = Image.new('RGBA', (240, 240), (5, 6, 8, 255))
preview.paste(knob_final, (20, 20), knob_final)
preview.save("../assets/knob_body_preview.png")
print(f"Saved knob_body.png (200x200)")

# Also try extracting the top-right larger knob (might be cleaner)
tr_cx = int((1090 - left) * scale_x)
tr_cy = int((250 - top) * scale_y)
tr_r = int(95 * scale_x)
print(f"Top-right knob: center=({tr_cx}, {tr_cy}), r={tr_r}")

box2 = (tr_cx - tr_r - pad, tr_cy - tr_r - pad,
        tr_cx + tr_r + pad, tr_cy + tr_r + pad)
box2 = (max(0, box2[0]), max(0, box2[1]), min(740, box2[2]), min(540, box2[3]))
knob2 = bg.crop(box2)
s2 = min(knob2.size)
knob2 = knob2.crop((0, 0, s2, s2))
mask2 = Image.new('L', (s2, s2), 0)
ImageDraw.Draw(mask2).ellipse([2, 2, s2-2, s2-2], fill=255)
mask2 = mask2.filter(ImageFilter.GaussianBlur(radius=2))
rgba2 = knob2.convert('RGBA')
r2, g2, b2, a2 = rgba2.split()
rgba2 = Image.merge('RGBA', (r2, g2, b2, mask2))
knob2_final = rgba2.resize((200, 200), Image.LANCZOS)
knob2_final.save("../assets/knob_body_alt.png")

preview2 = Image.new('RGBA', (240, 240), (5, 6, 8, 255))
preview2.paste(knob2_final, (20, 20), knob2_final)
preview2.save("../assets/knob_body_alt_preview.png")
print(f"Saved knob_body_alt.png (200x200)")

print("\nDone! Background + 2 knob variants ready.")
