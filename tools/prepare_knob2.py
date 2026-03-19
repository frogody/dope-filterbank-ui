#!/usr/bin/env python3
"""Better knob extraction — try multiple positions, find the cleanest one."""
from PIL import Image, ImageDraw, ImageFilter

src = Image.open("/Users/olmorooijackers/Downloads/olmo1995_httpss.mj.runKTeVXgGyDEI_3D_product_render_of_this_e_d9768d6a-3c36-421d-980d-5680afacf1e8_3.png")
w, h = src.size
print(f"Image: {w}x{h}")

# Let me try multiple knob positions and radii
# Looking at the image (1280x928):
# Top-left large knob: ~(152, 245), r~90
# Top-right large knob: ~(1085, 245), r~90
# Bottom-right large knob: ~(1130, 710), r~95

knobs = {
    "tl": (152, 240, 92),
    "tr": (1090, 250, 92),
    "br": (1130, 710, 98),
}

for name, (cx, cy, kr) in knobs.items():
    # Tight crop — just the knob, no text
    pad = 12
    box = (cx - kr - pad, cy - kr - pad, cx + kr + pad, cy + kr + pad)

    # Clamp to image bounds
    box = (max(0, box[0]), max(0, box[1]), min(w, box[2]), min(h, box[3]))
    crop = src.crop(box)

    # Make square
    s = min(crop.size)
    crop = crop.crop((0, 0, s, s))

    # Circular mask
    mask = Image.new('L', (s, s), 0)
    ImageDraw.Draw(mask).ellipse([2, 2, s-2, s-2], fill=255)
    mask = mask.filter(ImageFilter.GaussianBlur(radius=2))

    rgba = crop.convert('RGBA')
    r, g, b, a = rgba.split()
    rgba = Image.merge('RGBA', (r, g, b, mask))

    # Resize to 200px
    rgba = rgba.resize((200, 200), Image.LANCZOS)

    # Preview on dark bg
    preview = Image.new('RGBA', (240, 240), (5, 6, 8, 255))
    preview.paste(rgba, (20, 20), rgba)
    preview.save(f"../assets/knob_{name}_preview.png")
    rgba.save(f"../assets/knob_{name}.png")
    print(f"  {name}: center=({cx},{cy}) r={kr} -> saved")

print("\nDone! Check the previews.")
