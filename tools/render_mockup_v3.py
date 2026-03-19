#!/usr/bin/env python3
"""
DOPE FilterBank v3 mockup — dark chrome knobs, hardware theme.
Reference: black chrome knobs with dramatic specular highlights.
"""

import numpy as np
from PIL import Image, ImageDraw, ImageFont, ImageFilter, ImageChops
import math

W, H = 740, 540
HEADER_H = 38
METER_H = 42

# Dark hardware palette
BG = (4, 5, 8)
BG_SURFACE = (8, 10, 16)
CYAN = (0, 187, 255)
CYAN_GLOW = (0, 221, 255)
CYAN_DIM = (8, 60, 100)
AMBER = (255, 149, 0)


def alpha_blend(bg, fg, a):
    return tuple(int(b * (1 - a) + f * a) for b, f in zip(bg[:3], fg[:3]))


def multiply_alpha(img, mask):
    r, g, b, a = img.split()
    a = ImageChops.multiply(a, mask)
    return Image.merge('RGBA', (r, g, b, a))


def render_knob_dark_chrome(img, cx, cy, size, glow_progress=0.7, has_glow=True):
    """Dark chrome knob — nearly black body with dramatic white specular."""
    r = size // 2
    glow_r = r + 6

    # ── LED GLOW RING ──
    if has_glow and glow_progress > 0.01:
        start = 135
        sweep = glow_progress * 270

        # Wide glow (blurred)
        arc = Image.new('RGBA', img.size, (0, 0, 0, 0))
        ImageDraw.Draw(arc).arc(
            [cx - glow_r, cy - glow_r, cx + glow_r, cy + glow_r],
            start=start, end=start + sweep, fill=(*CYAN, 140), width=12)
        arc = arc.filter(ImageFilter.GaussianBlur(radius=6))
        img.paste(Image.alpha_composite(img.convert('RGBA'), arc))

        # Bright core
        arc2 = Image.new('RGBA', img.size, (0, 0, 0, 0))
        ImageDraw.Draw(arc2).arc(
            [cx - glow_r, cy - glow_r, cx + glow_r, cy + glow_r],
            start=start, end=start + sweep, fill=(*CYAN_GLOW, 250), width=3)
        img.paste(Image.alpha_composite(img.convert('RGBA'), arc2))

        # White hot center
        arc3 = Image.new('RGBA', img.size, (0, 0, 0, 0))
        ImageDraw.Draw(arc3).arc(
            [cx - glow_r, cy - glow_r, cx + glow_r, cy + glow_r],
            start=start, end=start + sweep, fill=(255, 255, 255, 120), width=1)
        img.paste(Image.alpha_composite(img.convert('RGBA'), arc3))

        # Dim track
        track = Image.new('RGBA', img.size, (0, 0, 0, 0))
        ImageDraw.Draw(track).arc(
            [cx - glow_r, cy - glow_r, cx + glow_r, cy + glow_r],
            start=0, end=360, fill=(255, 255, 255, 15), width=2)
        img.paste(Image.alpha_composite(img.convert('RGBA'), track))

    # ── DROP SHADOW (deeper for dark theme) ──
    shadow = Image.new('RGBA', img.size, (0, 0, 0, 0))
    ImageDraw.Draw(shadow).ellipse(
        [cx - r - 4, cy - r + 4, cx + r + 4, cy + r + 10],
        fill=(0, 0, 0, 180))
    shadow = shadow.filter(ImageFilter.GaussianBlur(radius=10))
    img.paste(Image.alpha_composite(img.convert('RGBA'), shadow))

    draw = ImageDraw.Draw(img)

    # ── OUTER RING (very dark) ──
    draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(12, 13, 18))

    # ── KNURLED EDGE TEXTURE ──
    num_knurls = max(30, int(r * 1.2))
    for i in range(num_knurls):
        a = i / num_knurls * math.pi * 2
        inner = r - 4
        outer = r - 1
        x1 = cx + math.cos(a) * inner
        y1 = cy + math.sin(a) * inner
        x2 = cx + math.cos(a) * outer
        y2 = cy + math.sin(a) * outer
        draw.line([(x1, y1), (x2, y2)], fill=(30, 32, 40), width=1)

    # ── MAIN BODY (very dark chrome — nearly black) ──
    body_r = r - 5
    draw.ellipse([cx - body_r, cy - body_r, cx + body_r, cy + body_r],
                 fill=(14, 16, 22))

    # Subtle radial gradient (slightly lighter center)
    for ring_i in range(body_r, 0, -2):
        t = ring_i / body_r  # 1 at edge, 0 at center
        bright = int(14 + (1 - t) * 12)  # 14 at edge → 26 at center
        draw.ellipse([cx - ring_i, cy - ring_i, cx + ring_i, cy + ring_i],
                     fill=(bright, bright + 1, bright + 4))

    # ── SPECULAR HIGHLIGHT (dramatic, white — the "money shot") ──
    # Main specular (top-left, tight)
    spec = Image.new('RGBA', img.size, (0, 0, 0, 0))
    hx = cx - r * 0.22
    hy = cy - r * 0.28
    hr = r * 0.28
    ImageDraw.Draw(spec).ellipse(
        [hx - hr, hy - hr, hx + hr, hy + hr],
        fill=(255, 255, 255, 90))
    spec = spec.filter(ImageFilter.GaussianBlur(radius=max(3, int(r * 0.15))))
    # Clip to body
    mask = Image.new('L', img.size, 0)
    ImageDraw.Draw(mask).ellipse(
        [cx - body_r, cy - body_r, cx + body_r, cy + body_r], fill=255)
    spec = multiply_alpha(spec, mask)
    img.paste(Image.alpha_composite(img.convert('RGBA'), spec))

    # Secondary specular (smaller, sharper — the bright "hot spot")
    spec2 = Image.new('RGBA', img.size, (0, 0, 0, 0))
    hx2 = cx - r * 0.18
    hy2 = cy - r * 0.24
    hr2 = r * 0.12
    ImageDraw.Draw(spec2).ellipse(
        [hx2 - hr2, hy2 - hr2, hx2 + hr2, hy2 + hr2],
        fill=(255, 255, 255, 140))
    spec2 = spec2.filter(ImageFilter.GaussianBlur(radius=max(2, int(r * 0.08))))
    spec2 = multiply_alpha(spec2, mask)
    img.paste(Image.alpha_composite(img.convert('RGBA'), spec2))

    # Rim light (subtle edge highlight from top)
    rim = Image.new('RGBA', img.size, (0, 0, 0, 0))
    rim_draw = ImageDraw.Draw(rim)
    rim_r = body_r + 1
    rim_draw.arc([cx - rim_r, cy - rim_r, cx + rim_r, cy + rim_r],
                 start=200, end=340, fill=(255, 255, 255, 30), width=2)
    rim = rim.filter(ImageFilter.GaussianBlur(radius=2))
    img.paste(Image.alpha_composite(img.convert('RGBA'), rim))

    # Edge shadow (bottom-right, darker)
    edge = Image.new('RGBA', img.size, (0, 0, 0, 0))
    ex, ey, er = cx + r * 0.2, cy + r * 0.28, r * 0.45
    ImageDraw.Draw(edge).ellipse([ex - er, ey - er, ex + er, ey + er],
                                  fill=(0, 0, 0, 80))
    edge = edge.filter(ImageFilter.GaussianBlur(radius=max(3, int(r * 0.25))))
    edge = multiply_alpha(edge, mask)
    img.paste(Image.alpha_composite(img.convert('RGBA'), edge))

    draw = ImageDraw.Draw(img)

    # ── MACHINED GROOVE (concentric ring) ──
    mid_r = int(body_r * 0.52)
    draw.ellipse([cx - mid_r, cy - mid_r, cx + mid_r, cy + mid_r],
                 outline=(255, 255, 255, 8), width=1)

    # ── POINTER (bright white line, sharp) ──
    angle_rad = math.radians(-50 + glow_progress * 80)
    p_in = body_r * 0.28
    p_out = body_r * 0.84
    px1 = cx + math.sin(angle_rad) * p_in
    py1 = cy - math.cos(angle_rad) * p_in
    px2 = cx + math.sin(angle_rad) * p_out
    py2 = cy - math.cos(angle_rad) * p_out

    # Pointer glow
    glow_ptr = Image.new('RGBA', img.size, (0, 0, 0, 0))
    ImageDraw.Draw(glow_ptr).line([(px1, py1), (px2, py2)],
                                   fill=(200, 220, 255, 60), width=6)
    glow_ptr = glow_ptr.filter(ImageFilter.GaussianBlur(radius=3))
    img.paste(Image.alpha_composite(img.convert('RGBA'), glow_ptr))

    draw = ImageDraw.Draw(img)
    # Shadow side
    sa = angle_rad + 0.04
    draw.line([(cx + math.sin(sa) * p_in, cy - math.cos(sa) * p_in),
               (cx + math.sin(sa) * p_out, cy - math.cos(sa) * p_out)],
              fill=(0, 0, 0, 80), width=2)
    # Main pointer
    draw.line([(px1, py1), (px2, py2)], fill=(220, 225, 235), width=2)
    # Highlight side
    sa2 = angle_rad - 0.04
    draw.line([(cx + math.sin(sa2) * (p_in + 2), cy - math.cos(sa2) * (p_in + 2)),
               (cx + math.sin(sa2) * (p_out - 2), cy - math.cos(sa2) * (p_out - 2))],
              fill=(255, 255, 255, 40), width=1)

    # Cyan tip
    draw.ellipse([px2 - 2, py2 - 2, px2 + 2, py2 + 2], fill=CYAN)
    # Tip glow
    tip_glow = Image.new('RGBA', img.size, (0, 0, 0, 0))
    ImageDraw.Draw(tip_glow).ellipse([px2 - 5, py2 - 5, px2 + 5, py2 + 5],
                                      fill=(*CYAN, 60))
    tip_glow = tip_glow.filter(ImageFilter.GaussianBlur(radius=3))
    img.paste(Image.alpha_composite(img.convert('RGBA'), tip_glow))

    draw = ImageDraw.Draw(img)

    # ── CENTER CAP (dark recess) ──
    cap_r = max(3, r // 7)
    draw.ellipse([cx - cap_r, cy - cap_r, cx + cap_r, cy + cap_r], fill=(4, 5, 8))
    draw.ellipse([cx - cap_r, cy - cap_r, cx + cap_r, cy + cap_r],
                 outline=(30, 33, 42), width=1)
    # Tiny screw dot
    draw.ellipse([cx - 1, cy - 1, cx + 1, cy + 1], fill=(2, 3, 4))


def draw_divider(draw, x, y1, y2):
    draw.line([(x, y1), (x, y2)], fill=(255, 255, 255, 12), width=1)
    draw.ellipse([x - 2, y1 - 2, x + 2, y1 + 2], fill=(*CYAN, 90))


def render_combo(draw, x, y, w, h, text, font):
    draw.rounded_rectangle([x, y, x + w, y + h], radius=3, fill=(6, 8, 14))
    draw.rounded_rectangle([x, y, x + w, y + h], radius=3,
                          outline=alpha_blend(BG, CYAN, 0.12), width=1)
    draw.text((x + 5, y + 2), text, fill=(170, 170, 170), font=font)
    ax = x + w - 9
    ay = y + h // 2
    draw.polygon([(ax - 3, ay - 2), (ax + 3, ay - 2), (ax, ay + 2)],
                fill=alpha_blend(BG, CYAN, 0.4))


def main():
    img = Image.new('RGBA', (W, H), (*BG, 255))
    draw = ImageDraw.Draw(img)

    try:
        font_brand = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 16)
        font_title = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 10)
        font_label = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 9)
        font_sm = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 8)
    except:
        font_brand = font_title = font_label = font_sm = ImageFont.load_default()

    # ── Brushed metal texture on entire background ──
    for sy in range(H):
        noise = np.random.randint(-1, 2)
        c = max(0, min(255, BG[0] + noise))
        draw.line([(0, sy), (W, sy)], fill=(c, c + 1, c + 2))
    # Subtle vertical grain
    for _ in range(W * H // 40):
        rx, ry = np.random.randint(0, W), np.random.randint(0, H)
        draw.point((rx, ry), fill=(255, 255, 255, 3))

    # ═══════════════════════════════════════════════════════════
    # HEADER
    # ═══════════════════════════════════════════════════════════
    draw.rectangle([0, 0, W, HEADER_H], fill=(3, 4, 7))
    draw.line([(0, HEADER_H - 1), (W, HEADER_H - 1)], fill=(*CYAN, 200))
    # Glow below accent line
    for i in range(4):
        draw.line([(0, HEADER_H + i), (W, HEADER_H + i)],
                  fill=(*CYAN, max(0, 10 - i * 3)))

    draw.text((14, 10), "DØPE", fill=CYAN_GLOW, font=font_brand)
    draw.text((72, 14), "FILTERBANK", fill=(255, 255, 255, 90), font=font_title)
    render_combo(draw, W // 2 - 100, 9, 90, 18, "Serial", font_sm)
    render_combo(draw, W // 2 + 10, 9, 90, 18, "Soft Clip", font_sm)
    draw.text((W - 44, 15), "v1.0.0", fill=CYAN_DIM, font=font_sm)
    for i in range(3):
        draw.ellipse([W - 14 - i * 11 - 2, 16, W - 14 - i * 11 + 2, 20], fill=AMBER)

    # ═══════════════════════════════════════════════════════════
    # LAYOUT — no panels, continuous surface
    # ═══════════════════════════════════════════════════════════
    contentY = HEADER_H + 4
    contentH = H - HEADER_H - METER_H - 8
    meterY = H - METER_H

    topZoneH = int(contentH * 0.54)
    botZoneY = contentY + topZoneH + 4
    botZoneH = contentH - topZoneH - 4

    margin = 14
    filtersW = int(W * 0.40)
    displayX = filtersW + 8
    displayW = W - displayX - margin
    filterRowH = topZoneH // 2 - 2

    # Subtle control zone surface
    draw.line([(margin, botZoneY - 2), (W - margin, botZoneY - 2)],
              fill=(255, 255, 255, 8))

    def section_label(x, y, text):
        draw.ellipse([x, y + 1, x + 3, y + 4], fill=(*CYAN, 80))
        draw.text((x + 7, y - 1), text, fill=(*CYAN, 90), font=font_label)

    # ═══════════════════════════════════════════════════════════
    # DISPLAY (CRT style)
    # ═══════════════════════════════════════════════════════════
    dx, dy, dw, dh = displayX, contentY, displayW, topZoneH
    draw.rounded_rectangle([dx, dy, dx + dw, dy + dh], radius=8, fill=(6, 9, 16))
    draw.rounded_rectangle([dx + 1, dy + 1, dx + dw - 1, dy + dh - 1],
                          radius=7, outline=(*CYAN, 35), width=1)

    # Scan lines
    for sy in range(dy + 2, dy + dh - 2, 2):
        draw.line([(dx + 2, sy), (dx + dw - 2, sy)], fill=(0, 0, 0, 8))

    # Grid
    for i in range(6):
        gy = dy + 6 + int(i / 5 * (dh - 12))
        for sx in range(dx + 6, dx + dw - 6, 8):
            draw.line([(sx, gy), (sx + 4, gy)], fill=(255, 255, 255, 6))

    # Frequency curve
    pts = []
    for i in range(dw - 12):
        x = i / (dw - 12)
        fr = 20 * (1000 ** x) / 1000
        mag = 0 if fr < 1 else -18 * math.log10(max(fr, 0.01))
        mag = max(-36, min(14, mag + 8))
        py = dy + 6 + int((14 - mag) / 52 * (dh - 12))
        pts.append((dx + 6 + i, py))

    # Fill glow
    for px, py in pts[::2]:
        for off in range(0, 35, 2):
            yy = py + off
            if yy < dy + dh - 4:
                draw.point((px, yy), fill=(*CYAN, max(0, 8 - off // 4)))

    if len(pts) > 1:
        draw.line(pts, fill=(*CYAN, 30), width=10)
        draw.line(pts, fill=(*CYAN, 80), width=5)
        draw.line(pts, fill=(*CYAN_GLOW, 230), width=2)
        draw.line(pts, fill=(255, 255, 255, 70), width=1)

    # Freq labels
    for freq, lbl in [(100, "100"), (500, "500"), (1000, "1k"), (5000, "5k")]:
        fx = dx + 6 + int(math.log10(freq / 20) / math.log10(1000) * (dw - 12))
        draw.text((fx - 5, dy + dh - 12), lbl, fill=(*CYAN, 60), font=font_sm)

    # ═══════════════════════════════════════════════════════════
    # FILTER 1 — big dark chrome knobs
    # ═══════════════════════════════════════════════════════════
    f1y = contentY + 4
    f1cy = f1y + filterRowH // 2 + 8
    section_label(margin, f1y, "FILTER 1")
    render_combo(draw, margin, f1y + 12, 82, 16, "LP 12dB", font_sm)

    kh = min(filterRowH - 16, 76)  # BIGGER hero knobs
    ks = int(kh * 0.65)

    render_knob_dark_chrome(img, margin + 125, f1cy, kh, glow_progress=0.65)
    draw = ImageDraw.Draw(img)
    draw.text((margin + 125 - 18, f1cy + kh // 2 + 10), "CUTOFF",
              fill=(140, 140, 140), font=font_label)

    render_knob_dark_chrome(img, margin + 220, f1cy, ks, glow_progress=0.3)
    draw = ImageDraw.Draw(img)
    draw.text((margin + 220 - 12, f1cy + ks // 2 + 10), "RESO",
              fill=(140, 140, 140), font=font_label)

    # ═══════════════════════════════════════════════════════════
    # FILTER 2
    # ═══════════════════════════════════════════════════════════
    f2y = contentY + filterRowH + 6
    f2cy = f2y + filterRowH // 2 + 8
    section_label(margin, f2y, "FILTER 2")
    render_combo(draw, margin, f2y + 12, 82, 16, "HP 12dB", font_sm)

    render_knob_dark_chrome(img, margin + 125, f2cy, kh, glow_progress=0.8)
    draw = ImageDraw.Draw(img)
    draw.text((margin + 125 - 18, f2cy + kh // 2 + 10), "CUTOFF",
              fill=(140, 140, 140), font=font_label)

    render_knob_dark_chrome(img, margin + 220, f2cy, ks, glow_progress=0.15)
    draw = ImageDraw.Draw(img)
    draw.text((margin + 220 - 12, f2cy + ks // 2 + 10), "RESO",
              fill=(140, 140, 140), font=font_label)

    # ═══════════════════════════════════════════════════════════
    # BOTTOM ZONE — DISTORTION | LIMITER | OUTPUT
    # ═══════════════════════════════════════════════════════════
    bkh = min(botZoneH - 36, 64)
    bcy = botZoneY + botZoneH // 2 + 8

    distSecW = int(W * 0.36)
    limSecW = int(W * 0.32)
    outSecW = W - distSecW - limSecW

    # DISTORTION
    section_label(margin, botZoneY + 4, "DISTORTION")
    dSp = distSecW // 4
    for i, (name, prog) in enumerate([("DRIVE", 0.55), ("MIX", 0.9), ("TONE", 0.45)]):
        kx = margin + dSp * (i + 1) - dSp // 3
        render_knob_dark_chrome(img, kx, bcy, bkh, glow_progress=prog)
        draw = ImageDraw.Draw(img)
        tw = len(name) * 5
        draw.text((kx - tw // 2, bcy + bkh // 2 + 8), name,
                  fill=(130, 130, 130), font=font_label)

    draw_divider(draw, distSecW, botZoneY + 6, botZoneY + botZoneH - 6)

    # LIMITER
    limX = distSecW
    section_label(limX + 10, botZoneY + 4, "LIMITER")
    render_combo(draw, limX + 10, botZoneY + 16, 66, 14, "Off", font_sm)
    limSp = limSecW // 3
    for i, (name, prog) in enumerate([("THRESH", 0.4), ("REL", 0.35)]):
        kx = limX + limSp * (i + 1) + 4
        render_knob_dark_chrome(img, kx, bcy, int(bkh * 0.82), glow_progress=prog)
        draw = ImageDraw.Draw(img)
        tw = len(name) * 5
        draw.text((kx - tw // 2, bcy + int(bkh * 0.82) // 2 + 8), name,
                  fill=(130, 130, 130), font=font_label)

    draw_divider(draw, distSecW + limSecW, botZoneY + 6, botZoneY + botZoneH - 6)

    # OUTPUT
    outX = distSecW + limSecW
    section_label(outX + 10, botZoneY + 4, "OUTPUT")
    render_knob_dark_chrome(img, outX + outSecW // 2, bcy, bkh, glow_progress=0.95)
    draw = ImageDraw.Draw(img)
    draw.text((outX + outSecW // 2 - 22, bcy + bkh // 2 + 8), "DRY/WET",
              fill=(130, 130, 130), font=font_label)

    # ═══════════════════════════════════════════════════════════
    # METER BAR
    # ═══════════════════════════════════════════════════════════
    draw.rectangle([0, meterY, W, H], fill=(3, 4, 7))
    draw.line([(0, meterY), (W, meterY)], fill=(*CYAN, 18))

    render_knob_dark_chrome(img, 20, meterY + METER_H // 2, 22,
                            has_glow=False, glow_progress=0)
    draw = ImageDraw.Draw(img)

    # Segmented meters
    mx, my, mw, mhh = 50, meterY + 17, W // 2 - 64, 8
    draw.text((38, meterY + 5), "IN", fill=CYAN_DIM, font=font_sm)

    seg_w, seg_gap = 3, 1
    num_segs = mw // (seg_w + seg_gap)
    lit = int(num_segs * 0.35)
    for i in range(num_segs):
        sx = mx + i * (seg_w + seg_gap)
        if i < lit:
            col = CYAN if i < num_segs * 0.8 else AMBER
        else:
            col = (8, 12, 20)
        draw.rectangle([sx, my, sx + seg_w, my + mhh], fill=col)
    draw.text((mx + mw + 3, my - 1), "-12.3 dB", fill=CYAN, font=font_sm)

    ox = W // 2 + 8
    draw.text((ox, meterY + 5), "OUT", fill=CYAN_DIM, font=font_sm)
    lit2 = int(num_segs * 0.52)
    for i in range(num_segs):
        sx = ox + 20 + i * (seg_w + seg_gap)
        if i < lit2:
            col = CYAN if i < num_segs * 0.8 else (AMBER if i < num_segs * 0.95 else (255, 50, 50))
        else:
            col = (8, 12, 20)
        draw.rectangle([sx, my, sx + seg_w, my + mhh], fill=col)
    draw.text((ox + 20 + mw + 3, my - 1), "-6.1 dB", fill=CYAN, font=font_sm)

    render_knob_dark_chrome(img, W - 20, meterY + METER_H // 2, 22,
                            has_glow=False, glow_progress=0)
    draw = ImageDraw.Draw(img)

    # Bottom metal edge
    draw.rectangle([0, H - 5, W, H], fill=(16, 18, 24))
    for sy in range(H - 4, H - 1, 2):
        draw.line([(0, sy), (W, sy)], fill=(255, 255, 255, 6))
    draw.line([(0, H - 5), (W, H - 5)], fill=(30, 33, 42))

    # Save
    img.convert('RGB').save('../mockup_v3.png', quality=95)
    print(f"Saved: mockup_v3.png ({W}x{H})")


if __name__ == '__main__':
    main()
