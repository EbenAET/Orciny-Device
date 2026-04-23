# V 1.0.0 (Show Release)
import adsk.core
import adsk.fusion
import traceback
import json
import os
import re
from datetime import datetime


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


def _ensure_export_root(ui: adsk.core.UserInterface, config: dict) -> str:
    export_root = str(config.get('exportRoot') or '').strip()
    if export_root and os.path.isdir(export_root):
        return export_root

    folder_dialog = ui.createFolderDialog()
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


def run(_context: str):
    app = adsk.core.Application.get()
    ui = app.userInterface

    try:
        design = adsk.fusion.Design.cast(app.activeProduct)
        if not design:
            ui.messageBox('No active Fusion design is open.')
            return

        config = _load_config()
        export_root = _ensure_export_root(ui, config)
        formats = _normalized_formats(config)
        stl_one_file_per_body = _as_bool(config, 'stlOneFilePerBody', False)
        create_run_folder = _as_bool(config, 'createTimestampRunFolder', True)

        timestamp = datetime.now().strftime('%Y%m%d-%H%M%S')
        base_root = export_root
        if create_run_folder:
            base_root = os.path.join(export_root, 'run-' + timestamp)

        doc_name = _safe_name(app.activeDocument.name)
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

        message = 'Archive export complete.\n\n'
        message += 'Document: ' + app.activeDocument.name + '\n'
        message += 'Output folder: ' + doc_root + '\n\n'
        message += 'Files:\n' + '\n'.join(exported_paths)
        ui.messageBox(message)

    except Exception:
        if ui:
            ui.messageBox('Archive export failed:\n{}'.format(traceback.format_exc()))
