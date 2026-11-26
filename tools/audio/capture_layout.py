"""Explicit PlatformIO evidence target; does not invoke upload or access a device."""
import json
from pathlib import Path
import sys

Import('env')  # provided by SCons
sys.path.insert(0, str(Path(env.subst('$PROJECT_DIR')) / 'tools' / 'audio'))
from recovery_layout import check_files, require, SLOTS


def capture(target, source, env):
    build = Path(env.subst('$BUILD_DIR'))
    items = [(env.subst(str(offset)), Path(env.subst(str(path))))
             for offset, path in env.get('FLASH_EXTRA_IMAGES', [])]
    items.append((env.subst('$ESP32_APP_OFFSET'), build / 'firmware.bin'))
    offsets, files = {}, {}
    for offset, path in items:
        name = path.name
        require(name in SLOTS and name not in files, 'unknown or duplicate uploader image')
        require(0 < path.stat().st_size <= SLOTS[name][1], 'uploader image size')
        offsets[name] = int(offset, 0)
        files[name] = path.read_bytes()
    report = check_files(files, offsets)
    out = build / 'layout-evidence'
    out.mkdir(exist_ok=True)
    for name, data in files.items():
        (out / name).write_bytes(data)
    (out / 'upload-layout.json').write_text(json.dumps({'offsets': offsets, **report}, indent=2) + '\n', encoding='utf-8')
    print('Offline uploader-layout evidence captured; no flash approval.')


env.AddCustomTarget('mhac_layout', '$BUILD_DIR/${PROGNAME}.bin', capture,
                    title='Offline CYD layout evidence', description=__doc__)
