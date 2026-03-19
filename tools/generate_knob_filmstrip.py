#!/usr/bin/env python3
"""
Generate photorealistic 3D metallic knob filmstrip for DOPE FilterBank.
Uses per-pixel PBR-style dome lighting to create convincing brushed aluminum knobs.
Output: filmstrip PNG with N frames stacked vertically.
"""

import numpy as np
from PIL import Image, ImageDraw, ImageFilter
import math
import sys

# ── Settings ────────────────────────────────────────────
FRAME_SIZE = 150        # final frame size (pixels)
RENDER_SIZE = 300       # render at 2x for anti-aliasing
NUM_FRAMES = 128        # rotation frames
ROTATION_RANGE = 270.0  # degrees of rotation
START_ANGLE = -135.0    # degrees from 12 o'clock (7 o'clock position)

# Colors (linear, 0-1 range)
METAL_BASE = np.array([0.28, 0.31, 0.36])      # brushed aluminum base
METAL_DARK = np.array([0.08, 0.09, 0.11])      # edge/shadow
RING_COLOR = np.array([0.07, 0.08, 0.10])      # outer ring (very dark gunmetal)
POINTER_COLOR = np.array([0.95, 0.95, 0.95])   # pointer (near-white)
CAP_COLOR = np.array([0.03, 0.04, 0.06])       # center cap (very dark)


def render_knob_frame(angle_deg: float, size: int) -> Image.Image:
    """Render a single knob frame with PBR-style dome lighting."""

    img = np.zeros((size, size, 4), dtype=np.float64)
    cx, cy = size / 2.0, size / 2.0

    # Knob geometry (relative to frame)
    outer_r = size * 0.44       # outer ring radius
    body_r = size * 0.39        # main dome radius
    cap_r = size * 0.055        # center cap radius
    ptr_inner = body_r * 0.30   # pointer start
    ptr_outer = body_r * 0.88   # pointer end

    # Light direction (from top-left-front, normalized)
    light = np.array([-0.25, -0.45, 0.85])
    light /= np.linalg.norm(light)

    # View direction (straight down)
    view = np.array([0.0, 0.0, 1.0])

    # Half-vector for Blinn-Phong specular
    half_v = light + view
    half_v /= np.linalg.norm(half_v)

    # Pixel coordinates
    yy, xx = np.mgrid[0:size, 0:size]
    px = (xx - cx)  # pixel position relative to center
    py = (yy - cy)

    # ═══════════════════════════════════════════════════
    # 1. OUTER RING (dark gunmetal with knurled edge)
    # ═══════════════════════════════════════════════════
    dist_from_center = np.sqrt(px**2 + py**2)
    ring_mask = (dist_from_center >= body_r) & (dist_from_center <= outer_r)

    if np.any(ring_mask):
        # Normalized distance within ring (0=inner, 1=outer)
        ring_t = (dist_from_center[ring_mask] - body_r) / (outer_r - body_r)

        # Subtle vertical gradient on ring (top brighter)
        ring_light = 0.55 + 0.45 * (-py[ring_mask] / outer_r * 0.5 + 0.5)
        ring_base = 0.10 + 0.06 * ring_light

        # Knurled edge texture (radial lines)
        ring_angle = np.arctan2(py[ring_mask], px[ring_mask])
        knurl = 0.85 + 0.15 * np.sin(ring_angle * 50)

        # Edge anti-aliasing
        edge_aa = np.clip((outer_r - dist_from_center[ring_mask]) / 1.5, 0, 1)
        inner_aa = np.clip((dist_from_center[ring_mask] - body_r) / 1.5, 0, 1)
        ring_aa = edge_aa * inner_aa

        brightness = ring_base * knurl
        for c in range(3):
            img[ring_mask, c] = brightness * RING_COLOR[c] / 0.14 * ring_aa
        img[ring_mask, 3] = ring_aa

    # ═══════════════════════════════════════════════════
    # 2. MAIN BODY (metallic dome with PBR lighting)
    # ═══════════════════════════════════════════════════
    body_dist = dist_from_center / body_r  # 0 at center, 1 at edge
    body_mask = body_dist < 1.0

    if np.any(body_mask):
        bd = body_dist[body_mask]
        bx = px[body_mask] / body_r
        by = py[body_mask] / body_r

        # Surface normal of a dome (hemisphere)
        nz = np.sqrt(np.maximum(0.001, 1.0 - bd**2))
        nx = bx
        ny = by

        # ── Diffuse lighting ──
        n_dot_l = np.maximum(0, nx * light[0] + ny * light[1] + nz * light[2])
        diffuse = n_dot_l

        # ── Specular (Blinn-Phong) ──
        n_dot_h = np.maximum(0, nx * half_v[0] + ny * half_v[1] + nz * half_v[2])

        # Two specular lobes: tight (sharp highlight) + wide (ambient reflection)
        spec_tight = n_dot_h ** 150   # very sharp highlight (like polished metal)
        spec_wide = n_dot_h ** 8      # broader metallic sheen

        # ── Fresnel-like edge darkening (stronger for more 3D pop) ──
        fresnel = (1.0 - nz) ** 2.0
        edge_darken = 1.0 - fresnel * 0.85

        # ── Brushed metal texture (concentric rings, subtle) ──
        brushed = 0.92 + 0.08 * np.sin(bd * size * 0.5)
        # Add subtle angular variation for realism
        body_angle = np.arctan2(by, bx)
        brushed *= 0.96 + 0.04 * np.sin(body_angle * 40)

        # ── Combine lighting (higher contrast) ──
        ambient = 0.08
        color = np.zeros((np.sum(body_mask), 3))
        for c in range(3):
            metal = METAL_BASE[c]
            dark = METAL_DARK[c]
            base = dark + (metal - dark) * (ambient + diffuse * 0.65)
            color[:, c] = base * brushed * edge_darken + \
                          spec_tight * 0.95 + spec_wide * 0.18

        # Edge anti-aliasing
        body_aa = np.clip((1.0 - bd) * body_r / 1.5, 0, 1)

        for c in range(3):
            img[body_mask, c] = np.clip(color[:, c], 0, 1) * body_aa
        img[body_mask, 3] = body_aa

    # ═══════════════════════════════════════════════════
    # 3. SECONDARY SPECULAR (soft, wider - simulates environment)
    # ═══════════════════════════════════════════════════
    if np.any(body_mask):
        # Secondary light from top-right (environment fill)
        light2 = np.array([0.3, -0.3, 0.9])
        light2 /= np.linalg.norm(light2)
        half2 = light2 + view
        half2 /= np.linalg.norm(half2)

        n_dot_h2 = np.maximum(0, nx * half2[0] + ny * half2[1] + nz * half2[2])
        env_spec = n_dot_h2 ** 30 * 0.25

        # Third light: rim light from bottom (subtle, for edge definition)
        light3 = np.array([0.0, 0.5, 0.4])
        light3 /= np.linalg.norm(light3)
        half3 = light3 + view
        half3 /= np.linalg.norm(half3)
        n_dot_h3 = np.maximum(0, nx * half3[0] + ny * half3[1] + nz * half3[2])
        rim_spec = n_dot_h3 ** 50 * 0.08

        body_aa = np.clip((1.0 - bd) * body_r / 1.5, 0, 1)
        for c in range(3):
            img[body_mask, c] = np.clip(
                img[body_mask, c] + (env_spec + rim_spec) * body_aa, 0, 1)

    # ═══════════════════════════════════════════════════
    # 4. POINTER (milled groove with shadow/highlight)
    # ═══════════════════════════════════════════════════
    angle_rad = math.radians(angle_deg)
    dir_x = math.sin(angle_rad)
    dir_y = -math.cos(angle_rad)

    # Project each pixel onto pointer axis
    along = px * dir_x + py * dir_y      # distance along pointer direction
    perp = px * (-dir_y) + py * dir_x    # perpendicular distance (signed)
    abs_perp = np.abs(perp)

    # Pointer line
    ptr_width = 2.2
    ptr_glow_width = 7.0
    ptr_mask = (along >= ptr_inner) & (along <= ptr_outer) & (abs_perp <= ptr_width)
    glow_mask = (along >= ptr_inner - 3) & (along <= ptr_outer + 3) & \
                (abs_perp <= ptr_glow_width) & body_mask

    # Draw glow (soft white around pointer)
    if np.any(glow_mask):
        glow_alpha = np.maximum(0, 1.0 - abs_perp[glow_mask] / ptr_glow_width) ** 2 * 0.2
        for c in range(3):
            img[glow_mask, c] = np.clip(
                img[glow_mask, c] * (1 - glow_alpha) + glow_alpha * 0.9, 0, 1)

    # Draw pointer shadow (offset to simulate milled groove)
    shadow_offset = 0.8
    shadow_perp = perp + shadow_offset  # shadow shifted to one side
    abs_shadow_perp = np.abs(shadow_perp)
    shadow_mask = (along >= ptr_inner) & (along <= ptr_outer) & \
                  (abs_shadow_perp <= ptr_width * 1.3) & body_mask
    if np.any(shadow_mask):
        shadow_alpha = np.maximum(0, 1.0 - abs_shadow_perp[shadow_mask] / (ptr_width * 1.3)) * 0.35
        for c in range(3):
            img[shadow_mask, c] = np.clip(img[shadow_mask, c] * (1 - shadow_alpha), 0, 1)

    # Draw main pointer line
    if np.any(ptr_mask):
        ptr_alpha = np.maximum(0, 1.0 - abs_perp[ptr_mask] / ptr_width)
        for c in range(3):
            img[ptr_mask, c] = np.clip(
                img[ptr_mask, c] * (1 - ptr_alpha) + ptr_alpha * POINTER_COLOR[c], 0, 1)
        img[ptr_mask, 3] = np.maximum(img[ptr_mask, 3], ptr_alpha)

    # Draw pointer highlight (thin bright line offset from shadow)
    hl_perp = perp - shadow_offset
    abs_hl_perp = np.abs(hl_perp)
    hl_mask = (along >= ptr_inner) & (along <= ptr_outer) & \
              (abs_hl_perp <= ptr_width * 0.5) & body_mask
    if np.any(hl_mask):
        hl_alpha = np.maximum(0, 1.0 - abs_hl_perp[hl_mask] / (ptr_width * 0.5)) * 0.3
        for c in range(3):
            img[hl_mask, c] = np.clip(img[hl_mask, c] + hl_alpha, 0, 1)

    # ═══════════════════════════════════════════════════
    # 5. POINTER TIP DOT (cyan accent)
    # ═══════════════════════════════════════════════════
    tip_x = cx + dir_x * ptr_outer
    tip_y = cy + dir_y * ptr_outer
    tip_dist = np.sqrt((xx - tip_x)**2 + (yy - tip_y)**2)
    tip_r = 3.0
    tip_glow_r = 8.0

    # Glow
    tip_glow_mask = (tip_dist <= tip_glow_r) & body_mask
    if np.any(tip_glow_mask):
        glow_a = np.maximum(0, 1.0 - tip_dist[tip_glow_mask] / tip_glow_r) ** 2 * 0.4
        img[tip_glow_mask, 0] = np.clip(img[tip_glow_mask, 0] + glow_a * 0.0, 0, 1)
        img[tip_glow_mask, 1] = np.clip(img[tip_glow_mask, 1] + glow_a * 0.8, 0, 1)
        img[tip_glow_mask, 2] = np.clip(img[tip_glow_mask, 2] + glow_a * 1.0, 0, 1)

    # Core dot
    tip_core_mask = tip_dist <= tip_r
    if np.any(tip_core_mask):
        tip_a = np.maximum(0, 1.0 - tip_dist[tip_core_mask] / tip_r)
        img[tip_core_mask, 0] = np.clip(0.0 + tip_a * 0.0, 0, 1)
        img[tip_core_mask, 1] = np.clip(img[tip_core_mask, 1] + tip_a * 0.8, 0, 1)
        img[tip_core_mask, 2] = np.clip(img[tip_core_mask, 2] + tip_a * 1.0, 0, 1)
        img[tip_core_mask, 3] = np.maximum(img[tip_core_mask, 3], tip_a)

    # ═══════════════════════════════════════════════════
    # 6. CENTER CAP (recessed dark circle with ring)
    # ═══════════════════════════════════════════════════
    cap_dist = dist_from_center / cap_r
    cap_mask = cap_dist <= 1.0

    if np.any(cap_mask):
        cd = cap_dist[cap_mask]
        # Dark recess
        for c in range(3):
            img[cap_mask, c] = CAP_COLOR[c]
        img[cap_mask, 3] = 1.0

        # Subtle ring at edge of cap
        cap_edge = (cd > 0.65) & (cd <= 1.0)
        cap_edge_full = cap_mask.copy()
        cap_edge_full[cap_mask] = cap_edge
        if np.any(cap_edge_full):
            for c in range(3):
                img[cap_edge_full, c] = 0.18

    # ═══════════════════════════════════════════════════
    # 7. CONVERT & RETURN
    # ═══════════════════════════════════════════════════
    # Gamma correction (linear to sRGB)
    rgb = np.clip(img[:, :, :3], 0, 1)
    rgb = np.power(rgb, 1.0 / 2.2)
    img[:, :, :3] = rgb

    img_uint8 = np.clip(img * 255, 0, 255).astype(np.uint8)
    return Image.fromarray(img_uint8, 'RGBA')


def main():
    print(f"Generating {NUM_FRAMES}-frame metallic knob filmstrip...")
    print(f"  Render size: {RENDER_SIZE}px → downscale to {FRAME_SIZE}px")

    strip = Image.new('RGBA', (FRAME_SIZE, FRAME_SIZE * NUM_FRAMES), (0, 0, 0, 0))

    for i in range(NUM_FRAMES):
        t = i / (NUM_FRAMES - 1) if NUM_FRAMES > 1 else 0
        angle = START_ANGLE + t * ROTATION_RANGE

        # Render at high resolution
        frame = render_knob_frame(angle, RENDER_SIZE)

        # Apply slight gaussian blur before downscale (anti-aliasing)
        frame = frame.filter(ImageFilter.GaussianBlur(radius=0.5))

        # Downscale with high-quality Lanczos resampling
        frame = frame.resize((FRAME_SIZE, FRAME_SIZE), Image.LANCZOS)

        # Paste into vertical filmstrip
        strip.paste(frame, (0, i * FRAME_SIZE))

        if (i + 1) % 32 == 0 or i == 0:
            print(f"  Frame {i + 1}/{NUM_FRAMES} (angle: {angle:.1f}°)")

    # Save filmstrip
    output_path = '../assets/knob_filmstrip.png'
    strip.save(output_path, optimize=True)
    file_size = strip.tobytes().__len__()
    print(f"\nSaved: {output_path}")
    print(f"  Dimensions: {strip.size[0]}x{strip.size[1]}")
    print(f"  Frames: {NUM_FRAMES} @ {FRAME_SIZE}x{FRAME_SIZE}px")

    # Also save a single frame preview
    preview = strip.crop((0, 64 * FRAME_SIZE, FRAME_SIZE, 65 * FRAME_SIZE))
    preview.save('../assets/knob_preview.png')
    print(f"  Preview saved: ../assets/knob_preview.png (frame 64, mid-rotation)")


if __name__ == '__main__':
    main()
