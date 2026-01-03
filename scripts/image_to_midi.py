#!/usr/bin/env python3
"""
Image to MIDI Converter for Jazz Architect
Uses oemer for OMR and music21 for MIDI conversion

Usage:
    python image_to_midi.py <input_image> [output_midi]

License: MIT (same as Jazz Architect project)
"""

import sys
import os
import tempfile
import subprocess
import json
from pathlib import Path

def check_dependencies():
    """Check if required packages are installed"""
    missing = []
    try:
        import oemer
    except ImportError:
        missing.append("oemer")

    try:
        import music21
    except ImportError:
        missing.append("music21")

    if missing:
        print(f"Error: Missing packages: {', '.join(missing)}", file=sys.stderr)
        print("Install with: pip install oemer music21", file=sys.stderr)
        return False
    return True

def run_oemer(image_path: str, output_dir: str) -> str:
    """
    Run oemer CLI to convert image to MusicXML
    Returns path to generated MusicXML file
    """
    print(f"Processing image: {image_path}", file=sys.stderr)

    try:
        # Find oemer in the same directory as this Python interpreter
        python_dir = Path(sys.executable).parent
        oemer_path = python_dir / "oemer"

        if not oemer_path.exists():
            # Try as module
            oemer_cmd = [sys.executable, "-m", "oemer"]
        else:
            oemer_cmd = [str(oemer_path)]

        # Run oemer
        result = subprocess.run(
            oemer_cmd + ["-o", output_dir, image_path],
            capture_output=True,
            text=True,
            timeout=600  # 10 minutes max
        )

        if result.returncode != 0:
            print(f"oemer stderr: {result.stderr}", file=sys.stderr)
            raise RuntimeError(f"oemer failed: {result.stderr}")

        # Find generated MusicXML file
        base_name = Path(image_path).stem
        xml_path = os.path.join(output_dir, f"{base_name}.musicxml")

        if not os.path.exists(xml_path):
            # Try alternative naming
            for f in os.listdir(output_dir):
                if f.endswith('.musicxml'):
                    xml_path = os.path.join(output_dir, f)
                    break

        if not os.path.exists(xml_path):
            raise RuntimeError("MusicXML file not generated")

        print(f"MusicXML saved: {xml_path}", file=sys.stderr)
        return xml_path

    except subprocess.TimeoutExpired:
        raise RuntimeError("oemer timed out (>10 minutes)")
    except Exception as e:
        print(f"oemer error: {e}", file=sys.stderr)
        raise

def musicxml_to_midi(xml_path: str, midi_path: str) -> bool:
    """
    Convert MusicXML to MIDI using music21
    """
    try:
        from music21 import converter, midi

        print(f"Converting to MIDI: {midi_path}", file=sys.stderr)

        # Parse MusicXML
        score = converter.parse(xml_path)

        # Write MIDI
        mf = midi.translate.music21ObjectToMidiFile(score)
        mf.open(midi_path, 'wb')
        mf.write()
        mf.close()

        print(f"MIDI saved: {midi_path}", file=sys.stderr)
        return True

    except Exception as e:
        print(f"music21 error: {e}", file=sys.stderr)
        return False

def extract_chords_from_midi(midi_path: str) -> list:
    """
    Extract chord information from MIDI for Jazz Architect
    Returns list of chord events with timing
    """
    try:
        from music21 import converter, chord, note

        score = converter.parse(midi_path)
        chords = []

        for element in score.flat.notes:
            if isinstance(element, chord.Chord):
                chord_info = {
                    "offset": float(element.offset),
                    "duration": float(element.duration.quarterLength),
                    "pitches": [p.midi for p in element.pitches],
                    "root": element.root().midi if element.root() else None
                }
                chords.append(chord_info)
            elif isinstance(element, note.Note):
                note_info = {
                    "offset": float(element.offset),
                    "duration": float(element.duration.quarterLength),
                    "pitches": [element.pitch.midi],
                    "root": element.pitch.midi
                }
                chords.append(note_info)

        return chords

    except Exception as e:
        print(f"Chord extraction error: {e}", file=sys.stderr)
        return []

def main():
    if len(sys.argv) < 2 or sys.argv[1] in ['-h', '--help']:
        print("Usage: python image_to_midi.py <input_image> [output_midi]")
        print("")
        print("Converts sheet music image to MIDI file")
        print("Supported formats: PNG, JPG, JPEG")
        print("")
        print("Requirements: oemer, music21")
        sys.exit(0 if '--help' in sys.argv or '-h' in sys.argv else 1)

    input_image = sys.argv[1]

    # Validate input
    if not os.path.exists(input_image):
        print(f"Error: File not found: {input_image}", file=sys.stderr)
        sys.exit(1)

    # Check dependencies
    if not check_dependencies():
        sys.exit(1)

    # Determine output path
    if len(sys.argv) >= 3:
        output_midi = sys.argv[2]
    else:
        base_name = Path(input_image).stem
        output_midi = str(Path(input_image).parent / f"{base_name}.mid")

    # Create temp directory for intermediate files
    with tempfile.TemporaryDirectory() as temp_dir:
        try:
            # Step 1: Image -> MusicXML (oemer)
            xml_path = run_oemer(input_image, temp_dir)

            # Step 2: MusicXML -> MIDI (music21)
            if musicxml_to_midi(xml_path, output_midi):
                # Step 3: Extract chord info for Jazz Architect
                chords = extract_chords_from_midi(output_midi)

                # Output result as JSON
                result = {
                    "success": True,
                    "midi_path": output_midi,
                    "chord_count": len(chords),
                    "chords": chords[:20]  # First 20 chords for preview
                }
                print(json.dumps(result, indent=2))
                sys.exit(0)
            else:
                print(json.dumps({"success": False, "error": "MIDI conversion failed"}))
                sys.exit(1)

        except Exception as e:
            print(json.dumps({"success": False, "error": str(e)}))
            sys.exit(1)

if __name__ == "__main__":
    main()
