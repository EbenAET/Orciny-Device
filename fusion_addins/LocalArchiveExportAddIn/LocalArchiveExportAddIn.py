# V 0.1.0
import adsk.core
import adsk.fusion
import traceback
import json
import os
import re
import subprocess
import sys
from datetime import datetime

APP = adsk.core.Application.get()
UI = APP.userInterface if APP else None
HANDLERS = []

CMD_ID = 'LocalArchiveExportAddInCmd'
CMD_NAME = 'Local Archive Export'
CMD_DESCRIPTION = 'Export active design to local archive (F3D/STEP/STL)'

SYNC_CMD_ID = 'LocalArchiveExportAndSyncAddInCmd'
SYNC_CMD_NAME = 'Local Archive Export + Box Sync'
SYNC_CMD_DESCRIPTION = 'Export active design, then run local Box sync script'

WORKSPACE_IDS = [
    'FusionSolidEnvironment',
    'FusionRenderEnvironment',
]

PANEL_IDS = [
    'SolidScriptsAddinsPanel',
    'RenderScriptsAddinsPanel',
    'SolidCreatePanel',
]


def _script_dir() -> str:
    return os.path.dirname(os.path.realpath(__file__))


def _config_path() -> str:
    return os.path.join(_script_dir(), 'archive_config.json')


def _load_config() -> dict:
    path = _config_path()
    if not os.path.exists(path):
        return {
            'version': '0.1.0',
            'exportRoot': '',
            'formats': ['f3d', 'step', 'stl'],
            'stlOneFilePerBody': False,
            'createTimestampRunFolder': True,
            'boxSyncScriptPath': '..\\..\\tools\\sync-box-local.ps1',
            'boxSyncArgs': ['-Mode', 'RepoToBox', '-PruneLegacyDuplicates'],
        }

    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)


def _save_config(config: dict) -> None:
    with open(_config_path(), 'w', encoding='utf-8') as f:
        json.dump(config, f, indent=2)


def _safe_name(value: str) -> str:
    value = value.strip()
    if not value:
        return 'unnamed'
    value = re.sub(r'[\\/:*?"<>|]+', '_', value)
    value = re.sub(r'\s+', ' ', value).strip()
    return value or 'unnamed'


def _ensure_export_root(config: dict) -> str:
    export_root = str(config.get('exportRoot') or '').strip()
    if export_root and os.path.isdir(export_root):
        return export_root

    folder_dialog = UI.createFolderDialog()
    folder_dialog.title = 'Choose archive export root folder'
    result = folder_dialog.showDialog()
    if result != adsk.core.DialogResults.DialogOK:
        raise RuntimeError('No export folder selected.')

    export_root = folder_dialog.folder
    config['exportRoot'] = export_root
    _save_config(config)
    return export_root


def _as_bool(config: dict, key: str, default_value: bool) -> bool:
    value = config.get(key)
    if isinstance(value, bool):
        return value
    return default_value


def _normalized_formats(config: dict) -> list:
    allowed = {'f3d', 'step', 'stl'}
    formats = config.get('formats')

    if not isinstance(formats, list):
        return ['f3d', 'step']

    resolved = []
    for item in formats:
        fmt = str(item).strip().lower()
        if fmt in allowed and fmt not in resolved:
            resolved.append(fmt)

    if not resolved:
        return ['f3d', 'step']

    return resolved


def _resolved_box_sync_script_path(config: dict) -> str:
    configured = str(config.get('boxSyncScriptPath') or '').strip()
    if not configured:
        raise RuntimeError('boxSyncScriptPath is not set in archive_config.json')

    if os.path.isabs(configured):
        script_path = configured
    else:
        script_path = os.path.normpath(os.path.join(_script_dir(), configured))

    if not os.path.isfile(script_path):
        raise RuntimeError('Box sync script not found: {}'.format(script_path))

    return script_path


def _box_sync_args(config: dict) -> list:
    args = config.get('boxSyncArgs')
    if not isinstance(args, list):
        return ['-Mode', 'RepoToBox', '-PruneLegacyDuplicates']

    resolved = [str(item) for item in args if str(item).strip()]
    if not resolved:
        return ['-Mode', 'RepoToBox', '-PruneLegacyDuplicates']

    return resolved


def _run_box_sync(config: dict) -> str:
    if not sys.platform.startswith('win'):
        raise RuntimeError('Box sync command is currently configured for Windows PowerShell only.')

    script_path = _resolved_box_sync_script_path(config)
    args = _box_sync_args(config)

    command = [
        'powershell',
        '-NoProfile',
        '-ExecutionPolicy', 'Bypass',
        '-File', script_path,
    ] + args

    completed = subprocess.run(command, capture_output=True, text=True, check=False)
    output = (completed.stdout or '').strip()
    error = (completed.stderr or '').strip()

    if completed.returncode != 0:
        detail = error or output or 'Unknown error from Box sync script.'
        raise RuntimeError('Box sync failed (exit code {}): {}'.format(completed.returncode, detail))

    if error:
        return output + '\n' + error if output else error
    return output


def _run_export(show_message: bool = True) -> dict:
    design = adsk.fusion.Design.cast(APP.activeProduct)
    if not design:
        raise RuntimeError('No active Fusion design is open.')

    config = _load_config()
    export_root = _ensure_export_root(config)
    formats = _normalized_formats(config)
    stl_one_file_per_body = _as_bool(config, 'stlOneFilePerBody', False)
    create_run_folder = _as_bool(config, 'createTimestampRunFolder', True)

    timestamp = datetime.now().strftime('%Y%m%d-%H%M%S')
    base_root = export_root
    if create_run_folder:
        base_root = os.path.join(export_root, 'run-' + timestamp)

    doc_name = _safe_name(APP.activeDocument.name)
    doc_root = os.path.join(base_root, doc_name)
    os.makedirs(doc_root, exist_ok=True)

    export_manager = design.exportManager
    root_component = design.rootComponent
    export_base = os.path.join(doc_root, doc_name)

    exported_paths = []

    if 'f3d' in formats:
        f3d_path = export_base + '.f3d'
        f3d_options = export_manager.createFusionArchiveExportOptions(f3d_path)
        export_manager.execute(f3d_options)
        exported_paths.append(f3d_path)

    if 'step' in formats:
        step_path = export_base + '.step'
        step_options = export_manager.createSTEPExportOptions(step_path)
        export_manager.execute(step_options)
        exported_paths.append(step_path)

    if 'stl' in formats:
        stl_path = export_base + '.stl'
        stl_options = export_manager.createSTLExportOptions(root_component, stl_path)
        stl_options.sendToPrintUtility = False
        stl_options.isBinaryFormat = True
        stl_options.meshRefinement = adsk.fusion.MeshRefinementSettings.MeshRefinementMedium
        stl_options.isOneFilePerBody = stl_one_file_per_body
        export_manager.execute(stl_options)
        exported_paths.append(stl_path)

    result = {
        'documentName': APP.activeDocument.name,
        'outputFolder': doc_root,
        'files': exported_paths,
    }

    if show_message:
        message = 'Archive export complete.\n\n'
        message += 'Document: ' + result['documentName'] + '\n'
        message += 'Output folder: ' + result['outputFolder'] + '\n\n'
        message += 'Files:\n' + '\n'.join(result['files'])
        UI.messageBox(message)

    return result


class CommandExecuteHandler(adsk.core.CommandEventHandler):
    def notify(self, args: adsk.core.CommandEventArgs):
        try:
            _run_export()
        except Exception:
            UI.messageBox('Local Archive Export failed:\n{}'.format(traceback.format_exc()))


class SyncCommandExecuteHandler(adsk.core.CommandEventHandler):
    def notify(self, args: adsk.core.CommandEventArgs):
        try:
            export_result = _run_export(show_message=False)
            config = _load_config()
            sync_output = _run_box_sync(config)

            message = 'Archive export + Box sync complete.\n\n'
            message += 'Document: ' + export_result['documentName'] + '\n'
            message += 'Output folder: ' + export_result['outputFolder'] + '\n\n'
            message += 'Files:\n' + '\n'.join(export_result['files'])
            if sync_output:
                message += '\n\nBox sync output:\n' + sync_output
            UI.messageBox(message)
        except Exception:
            UI.messageBox('Local Archive Export + Box Sync failed:\n{}'.format(traceback.format_exc()))


class CommandCreatedHandler(adsk.core.CommandCreatedEventHandler):
    def notify(self, args: adsk.core.CommandCreatedEventArgs):
        try:
            execute_handler = CommandExecuteHandler()
            args.command.execute.add(execute_handler)
            HANDLERS.append(execute_handler)
        except Exception:
            UI.messageBox('Command creation failed:\n{}'.format(traceback.format_exc()))


class SyncCommandCreatedHandler(adsk.core.CommandCreatedEventHandler):
    def notify(self, args: adsk.core.CommandCreatedEventArgs):
        try:
            execute_handler = SyncCommandExecuteHandler()
            args.command.execute.add(execute_handler)
            HANDLERS.append(execute_handler)
        except Exception:
            UI.messageBox('Sync command creation failed:\n{}'.format(traceback.format_exc()))


def _get_or_create_definition() -> adsk.core.CommandDefinition:
    cmd_def = UI.commandDefinitions.itemById(CMD_ID)
    if cmd_def:
        return cmd_def

    cmd_def = UI.commandDefinitions.addButtonDefinition(
        CMD_ID,
        CMD_NAME,
        CMD_DESCRIPTION,
        ''
    )
    return cmd_def


def _get_or_create_sync_definition() -> adsk.core.CommandDefinition:
    cmd_def = UI.commandDefinitions.itemById(SYNC_CMD_ID)
    if cmd_def:
        return cmd_def

    cmd_def = UI.commandDefinitions.addButtonDefinition(
        SYNC_CMD_ID,
        SYNC_CMD_NAME,
        SYNC_CMD_DESCRIPTION,
        ''
    )
    return cmd_def


def _add_command_to_panels(cmd_def: adsk.core.CommandDefinition) -> int:
    added = 0
    created_handler = CommandCreatedHandler()
    cmd_def.commandCreated.add(created_handler)
    HANDLERS.append(created_handler)

    for workspace_id in WORKSPACE_IDS:
        workspace = UI.workspaces.itemById(workspace_id)
        if not workspace:
            continue

        for panel_id in PANEL_IDS:
            panel = workspace.toolbarPanels.itemById(panel_id)
            if not panel:
                continue

            control = panel.controls.itemById(CMD_ID)
            if not control:
                panel.controls.addCommand(cmd_def)
                added += 1
            break

    return added


def _add_sync_command_to_panels(cmd_def: adsk.core.CommandDefinition) -> int:
    added = 0
    created_handler = SyncCommandCreatedHandler()
    cmd_def.commandCreated.add(created_handler)
    HANDLERS.append(created_handler)

    for workspace_id in WORKSPACE_IDS:
        workspace = UI.workspaces.itemById(workspace_id)
        if not workspace:
            continue

        for panel_id in PANEL_IDS:
            panel = workspace.toolbarPanels.itemById(panel_id)
            if not panel:
                continue

            control = panel.controls.itemById(SYNC_CMD_ID)
            if not control:
                panel.controls.addCommand(cmd_def)
                added += 1
            break

    return added


def _remove_command_from_panels() -> None:
    for workspace_id in WORKSPACE_IDS:
        workspace = UI.workspaces.itemById(workspace_id)
        if not workspace:
            continue

        for panel_id in PANEL_IDS:
            panel = workspace.toolbarPanels.itemById(panel_id)
            if not panel:
                continue

            control = panel.controls.itemById(CMD_ID)
            if control:
                control.deleteMe()


def _remove_sync_command_from_panels() -> None:
    for workspace_id in WORKSPACE_IDS:
        workspace = UI.workspaces.itemById(workspace_id)
        if not workspace:
            continue

        for panel_id in PANEL_IDS:
            panel = workspace.toolbarPanels.itemById(panel_id)
            if not panel:
                continue

            control = panel.controls.itemById(SYNC_CMD_ID)
            if control:
                control.deleteMe()


def run(_context: str):
    try:
        cmd_def = _get_or_create_definition()
        sync_cmd_def = _get_or_create_sync_definition()
        added_count = _add_command_to_panels(cmd_def)
        sync_added_count = _add_sync_command_to_panels(sync_cmd_def)
        if added_count == 0 and sync_added_count == 0:
            UI.messageBox('Local Archive Export add-in started, but no supported toolbar panel was found.')
    except Exception:
        if UI:
            UI.messageBox('Add-in start failed:\n{}'.format(traceback.format_exc()))


def stop(_context: str):
    try:
        _remove_command_from_panels()
        _remove_sync_command_from_panels()

        cmd_def = UI.commandDefinitions.itemById(CMD_ID)
        if cmd_def:
            cmd_def.deleteMe()

        sync_cmd_def = UI.commandDefinitions.itemById(SYNC_CMD_ID)
        if sync_cmd_def:
            sync_cmd_def.deleteMe()
    except Exception:
        if UI:
            UI.messageBox('Add-in stop failed:\n{}'.format(traceback.format_exc()))
