#!/usr/bin/env python3
"""
DOPE FilterBank v3 — Plugin GUI mockup renderer.
Generates a pixel-perfect PNG mockup for design iteration.
"""

import numpy as np
from PIL import Image, ImageDraw, ImageFont, ImageFilter
import math

W, H = 740, 540
HEADER_H = 44
METER_H = 48

# Colors
BG = (5, 6, 8)
PANEL = (12, 18, 32)
PANEL_LITE = (16, 24, 40)
CYAN = (0, 187, 255)
CYAN_GLOW = (0, 221, 255)
CYAN_DIM = (10, 74, 122)
AMBER = (255, 149, 0)
WHITE = (255, 255, 255)
METAL_LIGHT = (74, 77, 85)
METAL_MID = (42, 45, 53)
METAL_DARK = (26, 29, 37)
TEXT_DIM = (255, 255, 255, 128)


def alpha_blend(bg_color, fg_color, alpha):
    """Blend foreground onto background with alpha (0-1)."""
    return tuple(int(b * (1 - alpha) + f * alpha) for b, f in zip(bg_color[:3], fg_color[:3]))


def draw_rounded_rect(draw, xy, radius, fill=None, outline=None, width=1):
    """Draw a rounded rectangle."""
    x1, y1, x2, y2 = xy
    if fill:
        draw.rounded_rectangle(xy, radius=radius, fill=fill)
    if outline:
        draw.rounded_rectangle(xy, radius=radius, outline=outline, width=width)


def draw_glow_circle(img, cx, cy, radius, color, glow_radius=20, alpha=0.3):
    """Draw a circle with glow effect."""
    glow = Image.new('RGBA', img.size, (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow)
    glow_draw.ellipse(
        [cx - glow_radius, cy - glow_radius, cx + glow_radius, cy + glow_radius],
        fill=(*color, int(alpha * 60))
    )
    glow = glow.filter(ImageFilter.GaussianBlur(radius=glow_radius // 2))
    img.paste(Image.alpha_composite(
        img.convert('RGBA'),
        glow
    ))

    # Core circle
    draw = ImageDraw.Draw(img)
    draw.ellipse(
        [cx - radius, cy - radius, cx + radius, cy + radius],
        fill=(*color, int(alpha * 255))
    )


def render_knob(img, cx, cy, size, has_glow=True, glow_progress=0.7):
    """Render a 3D metallic knob with LED glow ring."""
    draw = ImageDraw.Draw(img)
    r = size // 2
    glow_r = r + 8

    # LED glow ring (behind knob)
    if has_glow:
        # Full ring background (dim)
        for width_mult, alpha in [(6, 0.08), (3, 0.15)]:
            ring = Image.new('RGBA', img.size, (0, 0, 0, 0))
            ring_draw = ImageDraw.Draw(ring)
            ring_draw.ellipse(
                [cx - glow_r, cy - glow_r, cx + glow_r, cy + glow_r],
                outline=(*CYAN_DIM, int(alpha * 255)), width=width_mult
            )
            img.paste(Image.alpha_composite(img.convert('RGBA'), ring))

        # Active arc glow (bright cyan)
        arc_img = Image.new('RGBA', img.size, (0, 0, 0, 0))
        arc_draw = ImageDraw.Draw(arc_img)
        start_angle = 135  # 7 o'clock
        sweep = glow_progress * 270
        # Wide glow
        arc_draw.arc(
            [cx - glow_r, cy - glow_r, cx + glow_r, cy + glow_r],
            start=start_angle, end=start_angle + sweep,
            fill=(*CYAN, 180), width=8
        )
        arc_img_blur = arc_img.filter(ImageFilter.GaussianBlur(radius=4))
        img.paste(Image.alpha_composite(img.convert('RGBA'), arc_img_blur))
        # Core arc
        arc_img2 = Image.new('RGBA', img.size, (0, 0, 0, 0))
        arc_draw2 = ImageDraw.Draw(arc_img2)
        arc_draw2.arc(
            [cx - glow_r, cy - glow_r, cx + glow_r, cy + glow_r],
            start=start_angle, end=start_angle + sweep,
            fill=(*CYAN_GLOW, 230), width=3
        )
        img.paste(Image.alpha_composite(img.convert('RGBA'), arc_img2))

    # Drop shadow
    shadow = Image.new('RGBA', img.size, (0, 0, 0, 0))
    shadow_draw = ImageDraw.Draw(shadow)
    shadow_draw.ellipse(
        [cx - r - 2, cy - r + 2, cx + r + 2, cy + r + 6],
        fill=(0, 0, 0, 100)
    )
    shadow = shadow.filter(ImageFilter.GaussianBlur(radius=8))
    img.paste(Image.alpha_composite(img.convert('RGBA'), shadow))

    draw = ImageDraw.Draw(img)

    # Outer ring (dark gunmetal)
    draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=METAL_DARK)

    # Main body (metallic dome — gradient simulation)
    body_r = r - 3
    # Base metal
    draw.ellipse([cx - body_r, cy - body_r, cx + body_r, cy + body_r], fill=METAL_MID)

    # Specular highlight (top-left bright spot)
    spec = Image.new('RGBA', img.size, (0, 0, 0, 0))
    spec_draw = ImageDraw.Draw(spec)
    highlight_cx = cx - r * 0.2
    highlight_cy = cy - r * 0.25
    highlight_r = r * 0.45
    spec_draw.ellipse(
        [highlight_cx - highlight_r, highlight_cy - highlight_r,
         highlight_cx + highlight_r, highlight_cy + highlight_r],
        fill=(255, 255, 255, 45)
    )
    spec = spec.filter(ImageFilter.GaussianBlur(radius=int(r * 0.3)))
    img.paste(Image.alpha_composite(img.convert('RGBA'), spec))

    # Edge darkening (bottom-right shadow on dome)
    edge = Image.new('RGBA', img.size, (0, 0, 0, 0))
    edge_draw = ImageDraw.Draw(edge)
    edge_cx = cx + r * 0.25
    edge_cy = cy + r * 0.3
    edge_r = r * 0.6
    edge_draw.ellipse(
        [edge_cx - edge_r, edge_cy - edge_r, edge_cx + edge_r, edge_cy + edge_r],
        fill=(0, 0, 0, 50)
    )
    edge = edge.filter(ImageFilter.GaussianBlur(radius=int(r * 0.35)))
    # Clip to body circle
    mask = Image.new('L', img.size, 0)
    mask_draw = ImageDraw.Draw(mask)
    mask_draw.ellipse([cx - body_r, cy - body_r, cx + body_r, cy + body_r], fill=255)
    clipped = ImageChops_multiply_alpha(edge, mask)
    img.paste(Image.alpha_composite(img.convert('RGBA'), clipped))

    draw = ImageDraw.Draw(img)

    # Concentric ring (machined groove)
    mid_r = int(body_r * 0.55)
    draw.ellipse([cx - mid_r, cy - mid_r, cx + mid_r, cy + mid_r],
                 outline=(255, 255, 255, 15), width=1)

    # Pointer line
    angle_rad = math.radians(-45)  # pointing roughly 1-2 o'clock
    ptr_inner = body_r * 0.3
    ptr_outer = body_r * 0.8
    px1 = cx + math.sin(angle_rad) * ptr_inner
    py1 = cy - math.cos(angle_rad) * ptr_inner
    px2 = cx + math.sin(angle_rad) * ptr_outer
    py2 = cy - math.cos(angle_rad) * ptr_outer
    draw.line([(px1, py1), (px2, py2)], fill=(240, 240, 240, 230), width=2)

    # Pointer tip (cyan dot)
    draw.ellipse([px2 - 2, py2 - 2, px2 + 2, py2 + 2], fill=CYAN)

    # Center cap
    cap_r = max(3, r // 8)
    draw.ellipse([cx - cap_r, cy - cap_r, cx + cap_r, cy + cap_r], fill=(8, 10, 16))
    draw.ellipse([cx - cap_r, cy - cap_r, cx + cap_r, cy + cap_r],
                 outline=(50, 53, 60, 128), width=1)


def ImageChops_multiply_alpha(img, mask):
    """Multiply the alpha channel of img by mask."""
    r, g, b, a = img.split()
    from PIL import ImageChops
    a = ImageChops.multiply(a, mask)
    return Image.merge('RGBA', (r, g, b, a))


def render_panel(draw, x, y, w, h, label=None):
    """Render a dark panel with subtle glow border."""
    draw.rounded_rectangle([x, y, x + w, y + h], radius=8, fill=PANEL)
    # Border
    draw.rounded_rectangle([x, y, x + w, y + h], radius=8,
                          outline=alpha_blend(BG, CYAN, 0.15), width=1)
    # Label
    if label:
        # LED dot
        draw.ellipse([x + 12, y + 12, x + 18, y + 18], fill=CYAN)
        draw.ellipse([x + 10, y + 10, x + 20, y + 20],
                    fill=alpha_blend(BG, CYAN, 0.1))
        # Text
        try:
            font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 11)
        except:
            font = ImageFont.load_default()
        draw.text((x + 24, y + 10), label, fill=alpha_blend(BG, CYAN, 0.8), font=font)


def render_combo(draw, x, y, w, h, text):
    """Render a combobox."""
    draw.rounded_rectangle([x, y, x + w, y + h], radius=4, fill=(12, 18, 32))
    draw.rounded_rectangle([x, y, x + w, y + h], radius=4,
                          outline=alpha_blend(BG, CYAN, 0.2), width=1)
    try:
        font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 10)
    except:
        font = ImageFont.load_default()
    draw.text((x + 6, y + 4), text, fill=(200, 200, 200), font=font)
    # Arrow
    ax = x + w - 12
    ay = y + h // 2
    draw.polygon([(ax - 3, ay - 2), (ax + 3, ay - 2), (ax, ay + 2)],
                fill=alpha_blend(BG, CYAN, 0.6))


def main():
    img = Image.new('RGBA', (W, H), (*BG, 255))
    draw = ImageDraw.Draw(img)

    try:
        font_bold = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 18)
        font_med = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 12)
        font_sm = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 9)
        font_label = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 10)
    except:
        font_bold = font_med = font_sm = font_label = ImageFont.load_default()

    # ═══════════════════════════════════════════════════════════
    # HEADER BAR
    # ═══════════════════════════════════════════════════════════
    draw.rectangle([0, 0, W, HEADER_H], fill=(14, 20, 32))
    # Cyan accent line
    draw.rectangle([0, HEADER_H - 2, W, HEADER_H], fill=CYAN)
    # DOPE text
    draw.text((16, 12), "DØPE", fill=CYAN_GLOW, font=font_bold)
    draw.text((80, 16), "FILTERBANK", fill=(255, 255, 255, 112), font=font_med)
    # Version
    draw.text((W - 50, 18), "v1.0.0", fill=CYAN_DIM, font=font_sm)
    # LED dots (amber)
    for i in range(3):
        dx = W - 16 - i * 14
        draw.ellipse([dx - 2, 18, dx + 2, 22], fill=AMBER)
    # Routing + Dist type combos in header
    render_combo(draw, W // 2 - 108, 11, 100, 22, "Serial")
    render_combo(draw, W // 2 + 8, 11, 100, 22, "Soft Clip")

    # ═══════════════════════════════════════════════════════════
    # LAYOUT (original: filters left, display right)
    # ═══════════════════════════════════════════════════════════
    gap = 10
    contentY = HEADER_H
    contentH = H - HEADER_H - METER_H
    topH = int(contentH * 0.56)
    bottomH = contentH - topH - gap
    bottomY = contentY + topH + gap

    filtersW = int(W * 0.45)
    displayW = W - filtersW - gap
    filterRowH = (topH - gap) // 2

    # Panel areas
    f1Area = (0, contentY, filtersW, filterRowH)
    f2Area = (0, contentY + filterRowH + gap, filtersW, filterRowH)
    displayArea = (filtersW + gap, contentY, displayW, topH)

    distW = int(W * 0.28)
    limW = int(W * 0.24)
    outW = int(W * 0.18)
    logoW = W - distW - limW - outW - gap * 3

    distArea = (0, bottomY, distW, bottomH)
    limArea = (distW + gap, bottomY, limW, bottomH)
    outArea = (distW + gap + limW + gap, bottomY, outW, bottomH)
    logoArea = (distW + gap + limW + gap + outW + gap, bottomY, logoW, bottomH)

    # Draw panels
    render_panel(draw, *f1Area, "FILTER 1")
    render_panel(draw, *f2Area, "FILTER 2")
    render_panel(draw, displayArea[0], displayArea[1], displayArea[2], displayArea[3])
    render_panel(draw, *distArea, "DISTORTION")
    render_panel(draw, *limArea, "LIMITER")
    render_panel(draw, *outArea, "OUTPUT")
    render_panel(draw, logoArea[0], logoArea[1], logoArea[2], logoArea[3])

    # ═══════════════════════════════════════════════════════════
    # FILTER DISPLAY (right panel — fake frequency response)
    # ═══════════════════════════════════════════════════════════
    dx, dy = displayArea[0] + 5, displayArea[1] + 5
    dw, dh = displayArea[2] - 10, displayArea[3] - 10

    # Grid lines
    for i in range(7):
        gy = dy + int(i / 6 * dh)
        draw.line([(dx, gy), (dx + dw, gy)], fill=alpha_blend(BG, CYAN, 0.06), width=1)
    for i in range(8):
        gx = dx + int(i / 7 * dw)
        draw.line([(gx, dy), (gx, dy + dh)], fill=alpha_blend(BG, CYAN, 0.04), width=1)

    # Fake frequency curve (LP filter shape)
    curve_points = []
    for i in range(dw):
        x = i / dw
        # Simple LP response curve
        freq_ratio = 20 * (1000 ** x) / 1000
        if freq_ratio < 1:
            mag = 0
        else:
            mag = -20 * math.log10(freq_ratio) if freq_ratio > 0.01 else 0
        mag = max(-40, min(12, mag + 6))
        py = dy + int((12 - mag) / 54 * dh)
        curve_points.append((dx + i, py))

    # Glow under curve
    for px, py in curve_points:
        for alpha_val in range(0, 40, 2):
            y_offset = py + alpha_val
            if y_offset < dy + dh:
                a = max(0, 12 - alpha_val // 3)
                draw.point((px, y_offset), fill=(*CYAN, a))

    # Curve line (multi-layer glow)
    if len(curve_points) > 1:
        draw.line(curve_points, fill=(*CYAN, 40), width=8)
        draw.line(curve_points, fill=(*CYAN, 100), width=4)
        draw.line(curve_points, fill=(*CYAN_GLOW, 220), width=2)

    # ═══════════════════════════════════════════════════════════
    # KNOBS
    # ═══════════════════════════════════════════════════════════

    # Filter 1: ComboBox + CUTOFF + RESO
    f1y = contentY
    f1cy = f1y + filterRowH // 2 + 10
    render_combo(draw, 12, f1y + 28, 100, 20, "LP 12dB")
    knob_size_hero = min(filterRowH - 24, filtersW // 5)
    knob_size_small = int(knob_size_hero * 0.7)

    render_knob(img, 160, f1cy, knob_size_hero, glow_progress=0.65)
    draw = ImageDraw.Draw(img)  # refresh draw after knob
    draw.text((160 - 22, f1cy + knob_size_hero // 2 + 6), "CUTOFF",
              fill=(200, 200, 200, 180), font=font_label)

    render_knob(img, 240, f1cy, knob_size_small, glow_progress=0.3)
    draw = ImageDraw.Draw(img)
    draw.text((240 - 14, f1cy + knob_size_small // 2 + 6), "RESO",
              fill=(200, 200, 200, 180), font=font_label)

    # Filter 2: ComboBox + CUTOFF + RESO
    f2y = contentY + filterRowH + gap
    f2cy = f2y + filterRowH // 2 + 10
    render_combo(draw, 12, f2y + 28, 100, 20, "HP 12dB")

    render_knob(img, 160, f2cy, knob_size_hero, glow_progress=0.8)
    draw = ImageDraw.Draw(img)
    draw.text((160 - 22, f2cy + knob_size_hero // 2 + 6), "CUTOFF",
              fill=(200, 200, 200, 180), font=font_label)

    render_knob(img, 240, f2cy, knob_size_small, glow_progress=0.15)
    draw = ImageDraw.Draw(img)
    draw.text((240 - 14, f2cy + knob_size_small // 2 + 6), "RESO",
              fill=(200, 200, 200, 180), font=font_label)

    # Distortion: DRIVE + MIX + TONE
    dcy = bottomY + bottomH // 2 + 6
    dSpacing = distW // 4
    dist_knob = min(bottomH - 34, distW // 4)

    render_knob(img, dSpacing, dcy, dist_knob, glow_progress=0.5)
    draw = ImageDraw.Draw(img)
    draw.text((dSpacing - 16, dcy + dist_knob // 2 + 6), "DRIVE",
              fill=(200, 200, 200, 180), font=font_label)

    render_knob(img, dSpacing * 2, dcy, dist_knob, glow_progress=0.9)
    draw = ImageDraw.Draw(img)
    draw.text((dSpacing * 2 - 10, dcy + dist_knob // 2 + 6), "MIX",
              fill=(200, 200, 200, 180), font=font_label)

    render_knob(img, dSpacing * 3, dcy, dist_knob, glow_progress=0.5)
    draw = ImageDraw.Draw(img)
    draw.text((dSpacing * 3 - 14, dcy + dist_knob // 2 + 6), "TONE",
              fill=(200, 200, 200, 180), font=font_label)

    # Limiter: ComboBox + THRESH + RELEASE
    limX = distW + gap
    lim_knob = min(bottomH - 34, limW // 3)
    render_combo(draw, limX + 10, bottomY + 28, 80, 20, "Off")
    limSpacing = limW // 3

    render_knob(img, limX + limSpacing, dcy, lim_knob, glow_progress=0.4)
    draw = ImageDraw.Draw(img)
    draw.text((limX + limSpacing - 18, dcy + lim_knob // 2 + 6), "THRESH",
              fill=(200, 200, 200, 180), font=font_label)

    render_knob(img, limX + limSpacing * 2, dcy, lim_knob, glow_progress=0.3)
    draw = ImageDraw.Draw(img)
    draw.text((limX + limSpacing * 2 - 22, dcy + lim_knob // 2 + 6), "RELEASE",
              fill=(200, 200, 200, 180), font=font_label)

    # Output: DRY/WET
    outX = distW + gap + limW + gap
    render_knob(img, outX + outW // 2, dcy, dist_knob, glow_progress=0.95)
    draw = ImageDraw.Draw(img)
    draw.text((outX + outW // 2 - 22, dcy + dist_knob // 2 + 6), "DRY/WET",
              fill=(200, 200, 200, 180), font=font_label)

    # DOPE logo panel
    lx = distW + gap + limW + gap + outW + gap
    draw.text((lx + logoW // 2 - 25, bottomY + bottomH // 2 - 8), "DØPE",
              fill=CYAN_GLOW, font=font_bold)

    # ═══════════════════════════════════════════════════════════
    # METER BAR
    # ═══════════════════════════════════════════════════════════
    meterY = H - METER_H
    draw.rectangle([0, meterY, W, H], fill=(14, 20, 32))
    draw.line([(0, meterY), (W, meterY)], fill=alpha_blend(BG, CYAN, 0.5), width=1)

    # Input meter
    draw.text((60, meterY + 6), "INPUT", fill=CYAN_DIM, font=font_sm)
    meter_x = 110
    meter_w = W // 2 - meter_x - 10
    draw.rounded_rectangle([meter_x, meterY + 20, meter_x + meter_w, meterY + 28],
                          radius=2, fill=(10, 20, 34))
    # Filled portion
    fill_w = int(meter_w * 0.35)
    draw.rounded_rectangle([meter_x, meterY + 20, meter_x + fill_w, meterY + 28],
                          radius=2, fill=CYAN)
    draw.text((meter_x + fill_w + 4, meterY + 18), "-12.3 dB",
              fill=CYAN, font=font_sm)

    # Output meter
    out_x = W // 2 + 10
    draw.text((out_x, meterY + 6), "OUTPUT", fill=CYAN_DIM, font=font_sm)
    draw.rounded_rectangle([out_x, meterY + 20, out_x + meter_w, meterY + 28],
                          radius=2, fill=(10, 20, 34))
    fill_w2 = int(meter_w * 0.5)
    draw.rounded_rectangle([out_x, meterY + 20, out_x + fill_w2, meterY + 28],
                          radius=2, fill=CYAN)
    draw.text((out_x + fill_w2 + 4, meterY + 18), "-6.1 dB",
              fill=CYAN, font=font_sm)

    # Micro knobs for input/output gain
    render_knob(img, 24, meterY + METER_H // 2 + 2, 28, has_glow=False)
    draw = ImageDraw.Draw(img)
    render_knob(img, W - 24, meterY + METER_H // 2 + 2, 28, has_glow=False)
    draw = ImageDraw.Draw(img)

    # ═══════════════════════════════════════════════════════════
    # SAVE
    # ═══════════════════════════════════════════════════════════
    output = img.convert('RGB')
    output.save('../mockup_v1.png', quality=95)
    print(f"Saved: ../mockup_v1.png ({W}x{H})")


if __name__ == '__main__':
    main()
