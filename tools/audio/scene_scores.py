"""Six original scene audition scores for AUDIO-028 score_render (offline only).

Usage: python tools/audio/scene_scores.py ./score_render output_directory
Coordinates are post-mapping audio values, not raw touch positions. The visual
scene names/patch IDs are pinned to VISUAL-027 36e4aa83625f5654475d8e4db380260840c0ceba.
No visual code is imported; this is a creative audition, not a touch replay.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import wave
import struct


def pitch(n):
    return (n * 4095 + 18) // 36


# Each tuple: blocks, x, y, state (0 release, 1 gate, 2 mute), curve.
# Stepped notes on lattice/chromatic surfaces; stepped Y on rhythm bands.
SCENES = (
    ("Lantern-Tide", 1, (
        (32,pitch(12),2200,2,1), (96,pitch(12),1000,1,1),
        (64,pitch(15),2400,1,1), (64,pitch(19),3600,1,1),
        (96,pitch(24),4000,1,1), (96,pitch(19),1600,1,1),
        (48,pitch(12),500,0,1), (64,pitch(12),500,2,1))),
    ("Cinder-Clock", 4, (
        (32,pitch(12),1365,2,1), (188,pitch(12),0,1,1),
        (188,pitch(12),1365,1,1), (188,pitch(15),2730,1,1),
        (188,pitch(10),4095,1,1), (188,pitch(12),0,1,1),
        (48,pitch(12),0,0,1), (64,pitch(12),0,2,1))),
    ("Silver-Bells", 6, (
        (32,0,900,2,1), (28,pitch(12),900,1,1),
        (40,pitch(12),900,0,1), (28,pitch(19),2800,1,1),
        (40,pitch(19),2800,0,1), (28,pitch(24),3900,1,1),
        (40,pitch(24),3900,0,1), (28,pitch(15),1400,1,1),
        (144,pitch(15),1400,0,1), (64,pitch(15),1400,2,1))),
    ("Memory-Crossing", 8, (
        (32,pitch(12),3000,2,1), (100,pitch(12),500,1,1),
        (100,pitch(19),2100,1,1), (100,pitch(24),4095,1,1),
        (100,pitch(19),4095,1,1), (100,pitch(12),3900,1,1),
        (48,pitch(12),3900,0,1), (64,pitch(12),3900,2,1))),
    ("Afterwater", 5, (
        (32,683,1500,2,1), (128,683,1500,1,1),
        (160,1200,3200,1,2), (144,1700,4000,1,2),
        (160,900,2200,1,2), (48,900,2200,0,1),
        (64,900,2200,2,1))),
    ("Dust-Prism", 2, (
        (32,2048,2000,2,1), (64,2048,2000,1,1),
        (144,950,4095,1,2), (144,3000,1000,1,2),
        (144,1700,3500,1,2), (48,1700,3500,0,1),
        (64,1700,3500,2,1))),
)


def inspect(path, frames, bound):
    with wave.open(str(path), "rb") as w:
        if (w.getnchannels(), w.getsampwidth(), w.getframerate(), w.getnframes()) != (1, 2, 24000, frames):
            raise ValueError(f"wrong WAV format: {path}")
        samples = struct.unpack(f"<{frames}h", w.readframes(frames))
    peak = max(map(abs, samples))
    if not 0 < peak <= bound or any(samples[:4096]) or any(samples[-128:]):
        raise ValueError(f"invalid peak or mute: {path}")
    return {"frames": frames, "peak": peak, "active": sum(v != 0 for v in samples),
            "sha256": hashlib.sha256(path.read_bytes()).hexdigest()}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("renderer", type=Path)
    p.add_argument("output", type=Path)
    args = p.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    report = {}
    for name, mode, rows in SCENES:
        score = args.output / (name + ".csv")
        score.write_bytes(("# blocks,mode,x,y,state,curve\n" + "\n".join(
            f"{b},{mode},{x},{y},{s},{c}" for b,x,y,s,c in rows) + "\n").encode("ascii"))
        frames = 128 * sum(r[0] for r in rows)
        prefix = args.output / name
        result = subprocess.run([str(args.renderer.resolve()), str(score), str(prefix)],
                                check=True, capture_output=True, text=True)
        report[name] = {"renderer": result.stdout.strip(),
                        "pcm": inspect(Path(str(prefix)+"-PCM.wav"), frames, 8191),
                        "dac": inspect(Path(str(prefix)+"-DAC.wav"), frames, 8448)}
    (args.output / "receipts.json").write_text(json.dumps(report, indent=2, sort_keys=True)+"\n", encoding="utf-8")
    print(json.dumps(report, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
