# LocalArchiveExportAddIn (Fusion Add-In)

Adds a persistent toolbar command in Fusion to export the active design to your local archive.

## Install In Fusion

1. Open Fusion 360.
2. Go to Utilities > Scripts and Add-Ins.
3. Open the Add-Ins tab.
4. Click the + icon (Add-Ins from folder).
5. Select this folder:
   - `Orciny-Device/fusion_addins/LocalArchiveExportAddIn`
6. Select LocalArchiveExportAddIn and click Run.
7. Optional: enable Run on Startup.

## Where The Button Appears

The add-in adds two commands to a supported panel in the Design/Render workspaces:

- **Local Archive Export**
- **Local Archive Export + Box Sync**

## Config

Edit `archive_config.json` in this add-in folder:

- `exportRoot`: absolute export root path (empty means prompt on first run)
- `formats`: list of `f3d`, `step`, `stl`
- `stlOneFilePerBody`: `true` or `false`
- `createTimestampRunFolder`: `true` or `false`
- `boxSyncScriptPath`: PowerShell script path for Box sync (absolute or relative to add-in folder)
- `boxSyncArgs`: argument array passed to that script

Default Box sync args are:

- `-Mode RepoToBox -PruneLegacyDuplicates`

If the add-in folder is moved outside this repo, update `boxSyncScriptPath` accordingly.

## Output Layout

When timestamp folders are enabled:

- `<exportRoot>/run-YYYYMMDD-HHMMSS/<DocumentName>/<DocumentName>.f3d`
- `<exportRoot>/run-YYYYMMDD-HHMMSS/<DocumentName>/<DocumentName>.step`
- `<exportRoot>/run-YYYYMMDD-HHMMSS/<DocumentName>/<DocumentName>.stl`
