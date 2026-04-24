# Icon Assets

This folder contains the Cwitcher icon source set and the generated PNG assets used by the app.

## Source of Truth

- `source/badge`
  - manual transparent PNG masters for `language`, `autostart`, `keyboard`, `sound`, and `log`
  - if `keyboard2.png` exists, it overrides `keyboard.png`
- `source/button`
  - manual transparent PNG masters for `save_white`, `delete`, `reset`, and `store`
- `source/brand`
  - brand PNG masters for `logo`, `logo_top`, and `logo_wordmark`
- `svg/badge`
  - badge icons still maintained as vector source: `info`
- `svg/brand`
  - vector backups for the logo system

## Runtime Assets

The settings window intentionally uses exact-size PNG exports instead of runtime scaling.

Current runtime files in `preview`:

- `language_24.png`
- `keyboard_28.png`
- `autostart_25.png`
- `sound_29.png`
- `info_26.png`
- `log_29.png`
- `save_white_21.png`
- `delete_21.png`
- `reset_21.png`
- `store_24.png`
- `logo_32.png`
- `logo_40.png`
- `logo_56.png`
- `logo_top_40.png`
- `logo_top_68.png`
- `logo_wordmark_header.png`
- `logo_wordmark_brand.png`
- `..\cwitcher.ico`

This avoids the jagged edges that appeared when larger PNG files were stretched down inside GDI.

## Build Pipeline

Regenerate runtime assets with:

```powershell
powershell -ExecutionPolicy Bypass -File .\resources\icons\build-assets.ps1
```

What the script does:

- tight-crops each source icon by alpha bounds
- recenters it into a square canvas
- renders through a 4x oversampled intermediate bitmap
- downsamples with high-quality bicubic interpolation
- exports exact-size PNGs for the UI
- regenerates the Windows `.ico` from `source/brand/logo_top.png` on a white background

This produces softer alpha edges and more consistent icon quality than runtime downscaling.

## Colors

- Accent icons: `#10B981`
- Contrast icons: `#0F172A`
