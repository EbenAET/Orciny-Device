# LocalArchiveExport (Fusion Script)

This package can be imported in Fusion 360 using Scripts and Add-Ins.

## What It Does

- Exports the active Fusion design to a local archive folder.
- Supports `f3d`, `step`, and `stl` output formats.
- Creates a timestamped run folder by default.

## Import In Fusion

1. Open Fusion 360.
2. Go to Utilities > Scripts and Add-Ins.
3. In Scripts, click the + icon (Add script from folder).
4. Select this folder:
   - `Orciny-Device/fusion_scripts/LocalArchiveExport`
5. Select LocalArchiveExport and click Run.

## First Run

- On first run, the script asks you to choose an export root folder.
- That folder is saved to `archive_config.json`.

## Config

Edit `archive_config.json`:

- `exportRoot`: absolute folder path for exports
- `formats`: list of `f3d`, `step`, `stl`
- `stlOneFilePerBody`: `true` or `false`
- `createTimestampRunFolder`: `true` or `false`

## Output Layout

When timestamp folders are enabled:

- `<exportRoot>/run-YYYYMMDD-HHMMSS/<DocumentName>/<DocumentName>.f3d`
- `<exportRoot>/run-YYYYMMDD-HHMMSS/<DocumentName>/<DocumentName>.step`
- `<exportRoot>/run-YYYYMMDD-HHMMSS/<DocumentName>/<DocumentName>.stl`
