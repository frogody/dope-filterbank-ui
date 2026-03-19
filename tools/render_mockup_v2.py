#!/usr/bin/env python3
"""
DOPE FilterBank v2 mockup — no panels, one continuous surface.
Sections separated by subtle dividers and spacing.
"""

import numpy as np
from PIL import Image, ImageDraw, ImageFont, ImageFilter, ImageChops
import math

W, H = 740, 540
HEADER_H = 40
METER_H = 44

# Colors
BG = (8, 10, 16)
BG_LIGHTER = (14, 18, 28)
CYAN = (0, 187, 255)
CYAN_GLOW = (0, 221, 255)
CYAN_DIM = (10, 74, 122)
AMBER = (255, 149, 0)
METAL_MID = (42, 45, 53)
METAL_DARK = (26, 29, 37)


def alpha_blend(bg, fg, a):
    return tuple(int(b * (1 - a) + f * a) for b, f in zip(bg[:3], fg[:3]))


def multiply_alpha(img, mask):
    r, g, b, a = img.split()
    a = ImageChops.multiply(a, mask)
    return Image.merge('RGBA', (r, g, b, a))


def render_knob(img, cx, cy, size, glow_progress=0.7, has_glow=True):
    """Render a 3D metallic knob with LED glow ring."""
    draw = ImageDraw.Draw(img)
    r = size // 2
    glow_r = r + 7

    # LED glow ring
    if has_glow and glow_progress > 0.01:
        # Active arc glow
        arc = Image.new('RGBA', img.size, (0, 0, 0, 0))
        arc_d = ImageDraw.Draw(arc)
        start = 135
        sweep = glow_progress * 270
        # Wide glow
        arc_d.arc([cx - glow_r, cy - glow_r, cx + glow_r, cy + glow_r],
                  start=start, end=start + sweep, fill=(*CYAN, 160), width=10)
        arc = arc.filter(ImageFilter.GaussianBlur(radius=5))
        img.paste(Image.alpha_composite(img.convert('RGBA'), arc))
        # Bright core
        arc2 = Image.new('RGBA', img.size, (0, 0, 0, 0))
        arc_d2 = ImageDraw.Draw(arc2)
        arc_d2.arc([cx - glow_r, cy - glow_r, cx + glow_r, cy + glow_r],
                   start=start, end=start + sweep, fill=(*CYAN_GLOW, 240), width=3)
        img.paste(Image.alpha_composite(img.convert('RGBA'), arc2))
        # White hot
        arc3 = Image.new('RGBA', img.size, (0, 0, 0, 0))
        arc_d3 = ImageDraw.Draw(arc3)
        arc_d3.arc([cx - glow_r, cy - glow_r, cx + glow_r, cy + glow_r],
                   start=start, end=start + sweep, fill=(255, 255, 255, 100), width=1)
        img.paste(Image.alpha_composite(img.convert('RGBA'), arc3))

        # Dim track (full ring)
        track = Image.new('RGBA', img.size, (0, 0, 0, 0))
        track_d = ImageDraw.Draw(track)
        track_d.arc([cx - glow_r, cy - glow_r, cx + glow_r, cy + glow_r],
                    start=0, end=360, fill=(255, 255, 255, 20), width=2)
        img.paste(Image.alpha_composite(img.convert('RGBA'), track))

    # Drop shadow
    shadow = Image.new('RGBA', img.size, (0, 0, 0, 0))
    ImageDraw.Draw(shadow).ellipse(
        [cx - r - 2, cy - r + 3, cx + r + 2, cy + r + 7], fill=(0, 0, 0, 120))
    shadow = shadow.filter(ImageFilter.GaussianBlur(radius=8))
    img.paste(Image.alpha_composite(img.convert('RGBA'), shadow))

    draw = ImageDraw.Draw(img)
    # Outer ring
    draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=METAL_DARK)
    # Body
    body_r = r - 3
    draw.ellipse([cx - body_r, cy - body_r, cx + body_r, cy + body_r], fill=METAL_MID)

    # Specular highlight (top-left)
    spec = Image.new('RGBA', img.size, (0, 0, 0, 0))
    hx, hy, hr = cx - r * 0.2, cy - r * 0.25, r * 0.4
    ImageDraw.Draw(spec).ellipse([hx - hr, hy - hr, hx + hr, hy + hr], fill=(255, 255, 255, 50))
    spec = spec.filter(ImageFilter.GaussianBlur(radius=max(2, int(r * 0.25))))
    img.paste(Image.alpha_composite(img.convert('RGBA'), spec))

    # Edge shadow (bottom-right)
    edge = Image.new('RGBA', img.size, (0, 0, 0, 0))
    ex, ey, er = cx + r * 0.2, cy + r * 0.25, r * 0.5
    ImageDraw.Draw(edge).ellipse([ex - er, ey - er, ex + er, ey + er], fill=(0, 0, 0, 55))
    edge = edge.filter(ImageFilter.GaussianBlur(radius=max(2, int(r * 0.3))))
    mask = Image.new('L', img.size, 0)
    ImageDraw.Draw(mask).ellipse([cx - body_r, cy - body_r, cx + body_r, cy + body_r], fill=255)
    clipped = multiply_alpha(edge, mask)
    img.paste(Image.alpha_composite(img.convert('RGBA'), clipped))

    draw = ImageDraw.Draw(img)
    # Machined groove
    mid_r = int(body_r * 0.55)
    draw.ellipse([cx - mid_r, cy - mid_r, cx + mid_r, cy + mid_r],
                 outline=(255, 255, 255, 12), width=1)

    # Pointer
    angle_rad = math.radians(-40 + glow_progress * 60)
    p_in, p_out = body_r * 0.3, body_r * 0.82
    px1 = cx + math.sin(angle_rad) * p_in
    py1 = cy - math.cos(angle_rad) * p_in
    px2 = cx + math.sin(angle_rad) * p_out
    py2 = cy - math.cos(angle_rad) * p_out
    draw.line([(px1, py1), (px2, py2)], fill=(235, 235, 235, 220), width=2)
    draw.ellipse([px2 - 2, py2 - 2, px2 + 2, py2 + 2], fill=CYAN)

    # Center cap
    cap_r = max(3, r // 8)
    draw.ellipse([cx - cap_r, cy - cap_r, cx + cap_r, cy + cap_r], fill=(6, 8, 12))


def draw_divider(draw, x, y1, y2):
    """Subtle vertical divider with LED dot."""
    draw.line([(x, y1), (x, y2)], fill=(255, 255, 255, 15), width=1)
    # LED dot at top
    draw.ellipse([x - 2, y1 - 2, x + 2, y1 + 2], fill=(*CYAN, 120))


def render_combo(draw, x, y, w, h, text, font):
    draw.rounded_rectangle([x, y, x + w, y + h], radius=4, fill=(10, 14, 24))
    draw.rounded_rectangle([x, y, x + w, y + h], radius=4,
                          outline=alpha_blend(BG, CYAN, 0.15), width=1)
    draw.text((x + 6, y + 3), text, fill=(190, 190, 190), font=font)
    ax = x + w - 10
    ay = y + h // 2
    draw.polygon([(ax - 3, ay - 2), (ax + 3, ay - 2), (ax, ay + 2)],
                fill=alpha_blend(BG, CYAN, 0.5))


def main():
    img = Image.new('RGBA', (W, H), (*BG, 255))
    draw = ImageDraw.Draw(img)

    try:
        font_brand = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 17)
        font_title = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 11)
        font_label = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 9)
        font_sm = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 8)
    except:
        font_brand = font_title = font_label = font_sm = ImageFont.load_default()

    # ═══════════════════════════════════════════════════════════
    # HEADER (clean, minimal)
    # ═══════════════════════════════════════════════════════════
    draw.rectangle([0, 0, W, HEADER_H], fill=(6, 8, 12))
    draw.line([(0, HEADER_H - 1), (W, HEADER_H - 1)], fill=CYAN, width=1)
    # Glow below line
    glow_line = Image.new('RGBA', (W, 6), (0, 0, 0, 0))
    glow_d = ImageDraw.Draw(glow_line)
    glow_d.rectangle([0, 0, W, 6], fill=(*CYAN, 12))
    img.paste(Image.alpha_composite(
        img.crop((0, HEADER_H, W, HEADER_H + 6)).convert('RGBA'), glow_line),
        (0, HEADER_H))
    draw = ImageDraw.Draw(img)

    draw.text((16, 11), "DØPE", fill=CYAN_GLOW, font=font_brand)
    draw.text((78, 15), "FILTERBANK", fill=(255, 255, 255, 100), font=font_title)

    # Combos in header
    render_combo(draw, W // 2 - 106, 10, 96, 20, "Serial", font_label)
    render_combo(draw, W // 2 + 10, 10, 96, 20, "Soft Clip", font_label)

    draw.text((W - 48, 16), "v1.0.0", fill=CYAN_DIM, font=font_sm)
    for i in range(3):
        dx = W - 14 - i * 12
        draw.ellipse([dx - 2, 17, dx + 2, 21], fill=AMBER)

    # ═══════════════════════════════════════════════════════════
    # MAIN CONTENT — one continuous surface, NO panels
    # ═══════════════════════════════════════════════════════════
    contentY = HEADER_H + 2
    contentH = H - HEADER_H - METER_H - 4
    meterY = H - METER_H

    # Subtle control zone gradient (bottom area slightly lighter)
    topZoneH = int(contentH * 0.55)
    botZoneY = contentY + topZoneH
    botZoneH = contentH - topZoneH

    # Very subtle horizontal gradient for control zone (raised feel)
    for sy in range(botZoneH):
        a = 0.03 * (1 - abs(sy / botZoneH - 0.5) * 2)  # peaks in middle
        c = alpha_blend(BG, (20, 26, 40), a)
        draw.line([(0, botZoneY + sy), (W, botZoneY + sy)], fill=c)

    # Subtle top border of control zone
    draw.line([(20, botZoneY), (W - 20, botZoneY)], fill=(255, 255, 255, 10))

    # ── LAYOUT ──
    margin = 16
    filtersW = int(W * 0.42)
    displayX = filtersW + margin
    displayW = W - displayX - margin
    filterRowH = topZoneH // 2 - 4

    # ── SECTION LABELS (small, uppercase, cyan dim) ──
    def section_label(x, y, text):
        draw.ellipse([x, y + 1, x + 4, y + 5], fill=(*CYAN, 100))
        draw.text((x + 8, y), text, fill=alpha_blend(BG, CYAN, 0.45), font=font_label)

    # ── FILTER DISPLAY (right side) ──
    dx, dy = displayX, contentY + 4
    dw, dh = displayW, topZoneH - 8

    # Display background (slightly different shade, rounded)
    draw.rounded_rectangle([dx, dy, dx + dw, dy + dh], radius=10, fill=(10, 14, 22))
    # Inner bezel
    draw.rounded_rectangle([dx + 1, dy + 1, dx + dw - 1, dy + dh - 1],
                          radius=9, outline=(*CYAN, 50), width=1)

    # Grid
    for i in range(6):
        gy = dy + 4 + int(i / 5 * (dh - 8))
        for sx in range(dx + 4, dx + dw - 4, 8):
            draw.line([(sx, gy), (sx + 4, gy)], fill=(255, 255, 255, 8))
    for i in range(8):
        gx = dx + 4 + int(i / 7 * (dw - 8))
        for sy in range(dy + 4, dy + dh - 4, 6):
            draw.point((gx, sy), fill=(*CYAN, 10))

    # Fake LP curve
    pts = []
    for i in range(dw - 8):
        x = i / (dw - 8)
        fr = 20 * (1000 ** x) / 1000
        mag = 0 if fr < 1 else -18 * math.log10(max(fr, 0.01))
        mag = max(-36, min(12, mag + 6))
        py = dy + 4 + int((12 - mag) / 50 * (dh - 8))
        pts.append((dx + 4 + i, py))

    # Glow fill
    for px, py in pts:
        for offset in range(0, 30, 2):
            yy = py + offset
            if yy < dy + dh - 4:
                a = max(0, 10 - offset // 3)
                draw.point((px, yy), fill=(*CYAN, a))

    if len(pts) > 1:
        draw.line(pts, fill=(*CYAN, 35), width=8)
        draw.line(pts, fill=(*CYAN, 90), width=4)
        draw.line(pts, fill=(*CYAN_GLOW, 220), width=2)
        draw.line(pts, fill=(255, 255, 255, 60), width=1)

    # Freq labels
    for freq, label in [(100, "100"), (500, "500"), (1000, "1k"), (5000, "5k"), (10000, "10k")]:
        fx = dx + 4 + int(math.log10(freq / 20) / math.log10(1000) * (dw - 8))
        draw.text((fx - 6, dy + dh - 14), label, fill=(*CYAN, 80), font=font_sm)

    # ── FILTER 1 (top-left) ──
    f1y = contentY + 6
    f1cy = f1y + filterRowH // 2 + 6
    section_label(margin, f1y, "FILTER 1")
    render_combo(draw, margin, f1y + 14, 90, 18, "LP 12dB", font_sm)

    kh = min(filterRowH - 20, 70)  # hero knob size
    ks = int(kh * 0.68)  # small knob size

    render_knob(img, margin + 130, f1cy + 4, kh, glow_progress=0.65)
    draw = ImageDraw.Draw(img)
    draw.text((margin + 130 - 18, f1cy + kh // 2 + 10), "CUTOFF",
              fill=(180, 180, 180), font=font_label)

    render_knob(img, margin + 215, f1cy + 4, ks, glow_progress=0.3)
    draw = ImageDraw.Draw(img)
    draw.text((margin + 215 - 12, f1cy + ks // 2 + 10), "RESO",
              fill=(180, 180, 180), font=font_label)

    # ── FILTER 2 (bottom-left) ──
    f2y = contentY + filterRowH + 10
    f2cy = f2y + filterRowH // 2 + 6
    section_label(margin, f2y, "FILTER 2")
    render_combo(draw, margin, f2y + 14, 90, 18, "HP 12dB", font_sm)

    render_knob(img, margin + 130, f2cy + 4, kh, glow_progress=0.8)
    draw = ImageDraw.Draw(img)
    draw.text((margin + 130 - 18, f2cy + kh // 2 + 10), "CUTOFF",
              fill=(180, 180, 180), font=font_label)

    render_knob(img, margin + 215, f2cy + 4, ks, glow_progress=0.15)
    draw = ImageDraw.Draw(img)
    draw.text((margin + 215 - 12, f2cy + ks // 2 + 10), "RESO",
              fill=(180, 180, 180), font=font_label)

    # Horizontal divider between filter zone and display
    # (none needed — the display has its own background)

    # ── BOTTOM CONTROL ZONE (one strip, no panels) ──
    # Sections: DISTORTION | LIMITER | OUTPUT
    # Separated by vertical dividers

    bkh = min(botZoneH - 40, 58)  # knob size for bottom row
    bcy = botZoneY + botZoneH // 2 + 6

    # Section widths
    distSecW = int(W * 0.35)
    limSecW = int(W * 0.32)
    outSecW = W - distSecW - limSecW

    # ── DISTORTION section ──
    section_label(margin, botZoneY + 6, "DISTORTION")
    dSpacing = distSecW // 4
    for i, (name, prog) in enumerate([("DRIVE", 0.5), ("MIX", 0.9), ("TONE", 0.5)]):
        kx = margin + dSpacing * (i + 1) - dSpacing // 2
        render_knob(img, kx, bcy, bkh, glow_progress=prog)
        draw = ImageDraw.Draw(img)
        tw = len(name) * 5
        draw.text((kx - tw // 2, bcy + bkh // 2 + 8), name,
                  fill=(170, 170, 170), font=font_label)

    # Divider after distortion
    draw_divider(draw, distSecW, botZoneY + 8, botZoneY + botZoneH - 8)

    # ── LIMITER section ──
    limX = distSecW
    section_label(limX + 12, botZoneY + 6, "LIMITER")
    render_combo(draw, limX + 12, botZoneY + 20, 72, 16, "Off", font_sm)
    limSpacing = limSecW // 3
    for i, (name, prog) in enumerate([("THRESH", 0.4), ("RELEASE", 0.3)]):
        kx = limX + limSpacing * (i + 1)
        render_knob(img, kx, bcy, int(bkh * 0.85), glow_progress=prog)
        draw = ImageDraw.Draw(img)
        tw = len(name) * 5
        draw.text((kx - tw // 2, bcy + int(bkh * 0.85) // 2 + 8), name,
                  fill=(170, 170, 170), font=font_label)

    # Divider after limiter
    draw_divider(draw, distSecW + limSecW, botZoneY + 8, botZoneY + botZoneH - 8)

    # ── OUTPUT section ──
    outX = distSecW + limSecW
    section_label(outX + 12, botZoneY + 6, "OUTPUT")
    render_knob(img, outX + outSecW // 2, bcy, bkh, glow_progress=0.95)
    draw = ImageDraw.Draw(img)
    draw.text((outX + outSecW // 2 - 22, bcy + bkh // 2 + 8), "DRY/WET",
              fill=(170, 170, 170), font=font_label)

    # ═══════════════════════════════════════════════════════════
    # METER BAR (bottom strip)
    # ═══════════════════════════════════════════════════════════
    draw.rectangle([0, meterY, W, H], fill=(6, 8, 12))
    draw.line([(0, meterY), (W, meterY)], fill=(*CYAN, 20))

    # Input
    render_knob(img, 22, meterY + METER_H // 2, 24, has_glow=False, glow_progress=0)
    draw = ImageDraw.Draw(img)
    draw.text((40, meterY + 6), "IN", fill=CYAN_DIM, font=font_sm)

    # Input meter (segmented)
    mx, my, mw, mhh = 56, meterY + 18, W // 2 - 70, 8
    draw.rounded_rectangle([mx, my, mx + mw, my + mhh], radius=2, fill=(8, 14, 24))
    seg_w = 3
    seg_gap = 1
    num_segs = mw // (seg_w + seg_gap)
    lit = int(num_segs * 0.35)
    for i in range(num_segs):
        sx = mx + 1 + i * (seg_w + seg_gap)
        if i < lit:
            col = CYAN if i < num_segs * 0.8 else AMBER
            draw.rectangle([sx, my + 1, sx + seg_w, my + mhh - 1], fill=col)
        else:
            draw.rectangle([sx, my + 1, sx + seg_w, my + mhh - 1], fill=(12, 18, 28))
    draw.text((mx + mw + 4, my - 2), "-12.3 dB", fill=CYAN, font=font_sm)

    # Output
    ox = W // 2 + 10
    draw.text((ox, meterY + 6), "OUT", fill=CYAN_DIM, font=font_sm)
    draw.rounded_rectangle([ox + 20, my, ox + 20 + mw, my + mhh], radius=2, fill=(8, 14, 24))
    lit2 = int(num_segs * 0.5)
    for i in range(num_segs):
        sx = ox + 21 + i * (seg_w + seg_gap)
        if i < lit2:
            col = CYAN if i < num_segs * 0.8 else (AMBER if i < num_segs * 0.95 else (255, 50, 50))
            draw.rectangle([sx, my + 1, sx + seg_w, my + mhh - 1], fill=col)
        else:
            draw.rectangle([sx, my + 1, sx + seg_w, my + mhh - 1], fill=(12, 18, 28))
    draw.text((ox + 20 + mw + 4, my - 2), "-6.1 dB", fill=CYAN, font=font_sm)

    render_knob(img, W - 22, meterY + METER_H // 2, 24, has_glow=False, glow_progress=0)
    draw = ImageDraw.Draw(img)

    # dB markers
    draw.text((mx, my + mhh + 2), "-60", fill=(*CYAN_DIM, 100), font=font_sm)
    draw.text((mx + mw - 8, my + mhh + 2), "0", fill=(*CYAN_DIM, 100), font=font_sm)

    # ═══════════════════════════════════════════════════════════
    # BRUSHED METAL BOTTOM EDGE
    # ═══════════════════════════════════════════════════════════
    edge_h = 6
    draw.rectangle([0, H - edge_h, W, H], fill=METAL_DARK)
    for sy in range(H - edge_h + 1, H - 1, 2):
        draw.line([(0, sy), (W, sy)], fill=(255, 255, 255, 8))
    draw.line([(0, H - edge_h), (W, H - edge_h)], fill=METAL_MID)

    # Save
    img.convert('RGB').save('../mockup_v2.png', quality=95)
    print(f"Saved: mockup_v2.png ({W}x{H})")


if __name__ == '__main__':
    main()
