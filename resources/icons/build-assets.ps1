[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Drawing

$iconsRoot = $PSScriptRoot
$sourceRoot = Join-Path $iconsRoot 'source'
$svgRoot = Join-Path $iconsRoot 'svg'
$previewRoot = Join-Path $iconsRoot 'preview'

function Get-InkscapePath {
    $candidates = @(
        'C:\Program Files\Inkscape\bin\inkscape.exe',
        'C:\Program Files\Inkscape\inkscape.exe',
        'C:\Program Files (x86)\Inkscape\inkscape.exe'
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    $command = Get-Command inkscape -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    throw 'Inkscape.exe not found. Install Inkscape or update build-assets.ps1.'
}

function Get-ContentBounds {
    param(
        [Parameter(Mandatory = $true)]
        [System.Drawing.Bitmap]$Bitmap
    )

    $minX = $Bitmap.Width
    $minY = $Bitmap.Height
    $maxX = -1
    $maxY = -1

    for ($y = 0; $y -lt $Bitmap.Height; $y++) {
        for ($x = 0; $x -lt $Bitmap.Width; $x++) {
            $pixel = $Bitmap.GetPixel($x, $y)
            if ($pixel.A -gt 0) {
                if ($x -lt $minX) { $minX = $x }
                if ($y -lt $minY) { $minY = $y }
                if ($x -gt $maxX) { $maxX = $x }
                if ($y -gt $maxY) { $maxY = $y }
            }
        }
    }

    if ($maxX -lt 0) {
        throw "No visible pixels found in source bitmap."
    }

    return [PSCustomObject]@{
        X      = $minX
        Y      = $minY
        Width  = $maxX - $minX + 1
        Height = $maxY - $minY + 1
    }
}

function New-SmoothPreview {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SourcePng,

        [Parameter(Mandatory = $true)]
        [string]$OutputPng,

        [Parameter(Mandatory = $true)]
        [int]$TargetSize,

        [int]$Padding = 0
    )

    $sourceBitmap = [System.Drawing.Bitmap]::FromFile($SourcePng)
    try {
        $bounds = Get-ContentBounds -Bitmap $sourceBitmap
        $oversample = $TargetSize * 4
        $scale = [Math]::Min(
            ($oversample - (2 * $Padding)) / $bounds.Width,
            ($oversample - (2 * $Padding)) / $bounds.Height)

        $drawWidth = [int][Math]::Round($bounds.Width * $scale)
        $drawHeight = [int][Math]::Round($bounds.Height * $scale)
        $offsetX = [int][Math]::Floor(($oversample - $drawWidth) / 2)
        $offsetY = [int][Math]::Floor(($oversample - $drawHeight) / 2)

        $hiBitmap = New-Object System.Drawing.Bitmap($oversample, $oversample, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        try {
            $graphics = [System.Drawing.Graphics]::FromImage($hiBitmap)
            try {
                $graphics.Clear([System.Drawing.Color]::Transparent)
                $graphics.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
                $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
                $graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
                $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality

                $srcRect = New-Object System.Drawing.Rectangle($bounds.X, $bounds.Y, $bounds.Width, $bounds.Height)
                $dstRect = New-Object System.Drawing.Rectangle($offsetX, $offsetY, $drawWidth, $drawHeight)
                $graphics.DrawImage($sourceBitmap, $dstRect, $srcRect, [System.Drawing.GraphicsUnit]::Pixel)
            } finally {
                $graphics.Dispose()
            }

            $outBitmap = New-Object System.Drawing.Bitmap($TargetSize, $TargetSize, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
            try {
                $outGraphics = [System.Drawing.Graphics]::FromImage($outBitmap)
                try {
                    $outGraphics.Clear([System.Drawing.Color]::Transparent)
                    $outGraphics.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
                    $outGraphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
                    $outGraphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
                    $outGraphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality

                    $imageAttributes = New-Object System.Drawing.Imaging.ImageAttributes
                    try {
                        $imageAttributes.SetWrapMode([System.Drawing.Drawing2D.WrapMode]::TileFlipXY)
                        $outGraphics.DrawImage(
                            $hiBitmap,
                            [System.Drawing.Rectangle]::new(0, 0, $TargetSize, $TargetSize),
                            0,
                            0,
                            $oversample,
                            $oversample,
                            [System.Drawing.GraphicsUnit]::Pixel,
                            $imageAttributes)
                    } finally {
                        $imageAttributes.Dispose()
                    }
                } finally {
                    $outGraphics.Dispose()
                }

                $outBitmap.Save($OutputPng, [System.Drawing.Imaging.ImageFormat]::Png)
            } finally {
                $outBitmap.Dispose()
            }
        } finally {
            $hiBitmap.Dispose()
        }
    } finally {
        $sourceBitmap.Dispose()
    }
}

function New-SmoothPreviewRect {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SourcePng,

        [Parameter(Mandatory = $true)]
        [string]$OutputPng,

        [Parameter(Mandatory = $true)]
        [int]$TargetWidth,

        [Parameter(Mandatory = $true)]
        [int]$TargetHeight,

        [int]$Padding = 0
    )

    $sourceBitmap = [System.Drawing.Bitmap]::FromFile($SourcePng)
    try {
        $bounds = Get-ContentBounds -Bitmap $sourceBitmap
        $oversampleWidth = $TargetWidth * 4
        $oversampleHeight = $TargetHeight * 4
        $oversamplePadding = $Padding * 4
        $scale = [Math]::Min(
            ($oversampleWidth - (2 * $oversamplePadding)) / $bounds.Width,
            ($oversampleHeight - (2 * $oversamplePadding)) / $bounds.Height)

        $drawWidth = [int][Math]::Round($bounds.Width * $scale)
        $drawHeight = [int][Math]::Round($bounds.Height * $scale)
        $offsetX = [int][Math]::Floor(($oversampleWidth - $drawWidth) / 2)
        $offsetY = [int][Math]::Floor(($oversampleHeight - $drawHeight) / 2)

        $hiBitmap = New-Object System.Drawing.Bitmap($oversampleWidth, $oversampleHeight, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        try {
            $graphics = [System.Drawing.Graphics]::FromImage($hiBitmap)
            try {
                $graphics.Clear([System.Drawing.Color]::Transparent)
                $graphics.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
                $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
                $graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
                $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality

                $srcRect = New-Object System.Drawing.Rectangle($bounds.X, $bounds.Y, $bounds.Width, $bounds.Height)
                $dstRect = New-Object System.Drawing.Rectangle($offsetX, $offsetY, $drawWidth, $drawHeight)
                $graphics.DrawImage($sourceBitmap, $dstRect, $srcRect, [System.Drawing.GraphicsUnit]::Pixel)
            } finally {
                $graphics.Dispose()
            }

            $outBitmap = New-Object System.Drawing.Bitmap($TargetWidth, $TargetHeight, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
            try {
                $outGraphics = [System.Drawing.Graphics]::FromImage($outBitmap)
                try {
                    $outGraphics.Clear([System.Drawing.Color]::Transparent)
                    $outGraphics.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
                    $outGraphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
                    $outGraphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
                    $outGraphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality

                    $imageAttributes = New-Object System.Drawing.Imaging.ImageAttributes
                    try {
                        $imageAttributes.SetWrapMode([System.Drawing.Drawing2D.WrapMode]::TileFlipXY)
                        $outGraphics.DrawImage(
                            $hiBitmap,
                            [System.Drawing.Rectangle]::new(0, 0, $TargetWidth, $TargetHeight),
                            0,
                            0,
                            $oversampleWidth,
                            $oversampleHeight,
                            [System.Drawing.GraphicsUnit]::Pixel,
                            $imageAttributes)
                    } finally {
                        $imageAttributes.Dispose()
                    }
                } finally {
                    $outGraphics.Dispose()
                }

                $outBitmap.Save($OutputPng, [System.Drawing.Imaging.ImageFormat]::Png)
            } finally {
                $outBitmap.Dispose()
            }
        } finally {
            $hiBitmap.Dispose()
        }
    } finally {
        $sourceBitmap.Dispose()
    }
}

function Export-SvgMaster {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SvgPath,

        [Parameter(Mandatory = $true)]
        [string]$OutputPng
    )

    $inkscape = Get-InkscapePath
    & $inkscape $SvgPath --export-filename=$OutputPng -w 1024 -h 1024 | Out-Null
}

function Build-PngAsset {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SourcePng,

        [Parameter(Mandatory = $true)]
        [Object[]]$Targets
    )

    foreach ($target in $Targets) {
        if ($target.Width -and $target.Height) {
            New-SmoothPreviewRect -SourcePng $SourcePng -OutputPng (Join-Path $previewRoot $target.Name) -TargetWidth $target.Width -TargetHeight $target.Height -Padding $target.Padding
        } else {
            New-SmoothPreview -SourcePng $SourcePng -OutputPng (Join-Path $previewRoot $target.Name) -TargetSize $target.Size -Padding $target.Padding
        }
        Write-Host "PNG  -> $($target.Name)"
    }
}

function Build-SvgAsset {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SourceSvg,

        [Parameter(Mandatory = $true)]
        [Object[]]$Targets
    )

    $tempFile = Join-Path ([System.IO.Path]::GetTempPath()) ([System.IO.Path]::GetRandomFileName() + '.png')
    try {
        Export-SvgMaster -SvgPath $SourceSvg -OutputPng $tempFile
        Build-PngAsset -SourcePng $tempFile -Targets $Targets
    } finally {
        if (Test-Path $tempFile) {
            Remove-Item $tempFile -Force
        }
    }
}

function New-AppIcon {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SourcePng,

        [Parameter(Mandatory = $true)]
        [string]$OutputIco
    )

    $sizes = @(16, 24, 32, 48, 64, 128, 256)
    $sourceBitmap = [System.Drawing.Bitmap]::FromFile($SourcePng)
    $frames = New-Object System.Collections.Generic.List[byte[]]

    try {
        $bounds = Get-ContentBounds -Bitmap $sourceBitmap

        foreach ($size in $sizes) {
            $bitmap = New-Object System.Drawing.Bitmap($size, $size, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
            try {
                $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
                try {
                    $graphics.Clear([System.Drawing.Color]::White)
                    $graphics.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
                    $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
                    $graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
                    $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality

                    $padding = [Math]::Max(1, [int][Math]::Round($size * 0.08))
                    $scale = [Math]::Min(
                        ($size - (2 * $padding)) / $bounds.Width,
                        ($size - (2 * $padding)) / $bounds.Height)
                    $drawWidth = [int][Math]::Round($bounds.Width * $scale)
                    $drawHeight = [int][Math]::Round($bounds.Height * $scale)
                    $offsetX = [int][Math]::Floor(($size - $drawWidth) / 2)
                    $offsetY = [int][Math]::Floor(($size - $drawHeight) / 2)

                    $srcRect = New-Object System.Drawing.Rectangle($bounds.X, $bounds.Y, $bounds.Width, $bounds.Height)
                    $dstRect = New-Object System.Drawing.Rectangle($offsetX, $offsetY, $drawWidth, $drawHeight)
                    $graphics.DrawImage($sourceBitmap, $dstRect, $srcRect, [System.Drawing.GraphicsUnit]::Pixel)
                } finally {
                    $graphics.Dispose()
                }

                $stream = New-Object System.IO.MemoryStream
                try {
                    $bitmap.Save($stream, [System.Drawing.Imaging.ImageFormat]::Png)
                    $frames.Add($stream.ToArray())
                } finally {
                    $stream.Dispose()
                }
            } finally {
                $bitmap.Dispose()
            }
        }
    } finally {
        $sourceBitmap.Dispose()
    }

    $outputStream = [System.IO.File]::Create($OutputIco)
    try {
        $writer = New-Object System.IO.BinaryWriter($outputStream)
        try {
            $writer.Write([UInt16]0)
            $writer.Write([UInt16]1)
            $writer.Write([UInt16]$sizes.Count)

            $offset = 6 + (16 * $sizes.Count)
            for ($i = 0; $i -lt $sizes.Count; $i++) {
                $size = $sizes[$i]
                $frame = $frames[$i]
                $writer.Write([byte]$(if ($size -eq 256) { 0 } else { $size }))
                $writer.Write([byte]$(if ($size -eq 256) { 0 } else { $size }))
                $writer.Write([byte]0)
                $writer.Write([byte]0)
                $writer.Write([UInt16]1)
                $writer.Write([UInt16]32)
                $writer.Write([UInt32]$frame.Length)
                $writer.Write([UInt32]$offset)
                $offset += $frame.Length
            }

            foreach ($frame in $frames) {
                $writer.Write($frame)
            }
        } finally {
            $writer.Dispose()
        }
    } finally {
        $outputStream.Dispose()
    }

    Write-Host "ICO  -> cwitcher.ico"
}

$keyboardSource = Join-Path $sourceRoot 'badge\keyboard2.png'
if (-not (Test-Path $keyboardSource)) {
    $keyboardSource = Join-Path $sourceRoot 'badge\keyboard.png'
}

$jobs = @(
    @{
        Kind = 'png'
        Source = Join-Path $sourceRoot 'badge\language.png'
        Targets = @(
            @{ Name = 'language_24.png'; Size = 24; Padding = 5 }
        )
    },
    @{
        Kind = 'png'
        Source = $keyboardSource
        Targets = @(
            @{ Name = 'keyboard_28.png'; Size = 28; Padding = 4 }
        )
    },
    @{
        Kind = 'png'
        Source = Join-Path $sourceRoot 'badge\autostart.png'
        Targets = @(
            @{ Name = 'autostart_25.png'; Size = 25; Padding = 6 }
        )
    },
    @{
        Kind = 'png'
        Source = Join-Path $sourceRoot 'badge\sound.png'
        Targets = @(
            @{ Name = 'sound_29.png'; Size = 29; Padding = 2 }
        )
    },
    @{
        Kind = 'svg'
        Source = Join-Path $svgRoot 'badge\info.svg'
        Targets = @(
            @{ Name = 'info_26.png'; Size = 26; Padding = 3 }
        )
    },
    @{
        Kind = 'png'
        Source = Join-Path $sourceRoot 'badge\log.png'
        Targets = @(
            @{ Name = 'log_29.png'; Size = 29; Padding = 3 }
        )
    },
    @{
        Kind = 'png'
        Source = Join-Path $sourceRoot 'button\save_white.png'
        Targets = @(
            @{ Name = 'save_white_21.png'; Size = 21; Padding = 1 }
        )
    },
    @{
        Kind = 'png'
        Source = Join-Path $sourceRoot 'button\delete.png'
        Targets = @(
            @{ Name = 'delete_21.png'; Size = 21; Padding = 1 }
        )
    },
    @{
        Kind = 'png'
        Source = Join-Path $sourceRoot 'button\reset.png'
        Targets = @(
            @{ Name = 'reset_21.png'; Size = 21; Padding = 1 }
        )
    },
    @{
        Kind = 'png'
        Source = Join-Path $sourceRoot 'button\store.png'
        Targets = @(
            @{ Name = 'store_24.png'; Size = 24; Padding = 2 }
        )
    },
    @{
        Kind = 'png'
        Source = Join-Path $sourceRoot 'brand\logo.png'
        Targets = @(
            @{ Name = 'logo_32.png'; Size = 32; Padding = 0 }
            @{ Name = 'logo_40.png'; Size = 40; Padding = 0 }
            @{ Name = 'logo_56.png'; Size = 56; Padding = 0 }
        )
    },
    @{
        Kind = 'png'
        Source = Join-Path $sourceRoot 'brand\logo_top.png'
        Targets = @(
            @{ Name = 'logo_top_40.png'; Size = 40; Padding = 0 }
            @{ Name = 'logo_top_68.png'; Size = 68; Padding = 0 }
        )
    },
    @{
        Kind = 'png'
        Source = Join-Path $sourceRoot 'brand\logo_wordmark.png'
        Targets = @(
            @{ Name = 'logo_wordmark_header.png'; Width = 156; Height = 48; Padding = 0 }
            @{ Name = 'logo_wordmark_brand.png'; Width = 164; Height = 50; Padding = 0 }
        )
    }
)

foreach ($job in $jobs) {
    if ($job.Kind -eq 'svg') {
        Build-SvgAsset -SourceSvg $job.Source -Targets $job.Targets
    } else {
        Build-PngAsset -SourcePng $job.Source -Targets $job.Targets
    }
}

New-AppIcon -SourcePng (Join-Path $sourceRoot 'brand\logo_top.png') -OutputIco (Join-Path (Split-Path $iconsRoot -Parent) 'cwitcher.ico')

Write-Host 'Done.'
