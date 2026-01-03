"""ChordSymbol - jazz chord representation with quality and extensions"""

from __future__ import annotations
from dataclasses import dataclass, field
from enum import Enum
from typing import Dict, List, Optional, Tuple

from .pitch import PitchClass


class ChordQuality(Enum):
    """Common jazz chord qualities"""
    MAJ7 = "maj7"       # Major 7th (Imaj7)
    MIN7 = "min7"       # Minor 7th (ii7, iii7, vi7)
    DOM7 = "7"          # Dominant 7th (V7)
    HDIM7 = "m7b5"      # Half-diminished (vii7)
    DIM7 = "dim7"       # Fully diminished
    AUG = "aug"         # Augmented
    MIN_MAJ7 = "mMaj7"  # Minor-major 7th
    MAJ6 = "6"          # Major 6th
    MIN6 = "m6"         # Minor 6th
    SUS4 = "sus4"       # Suspended 4th
    SUS2 = "sus2"       # Suspended 2nd

    @property
    def intervals(self) -> Tuple[int, ...]:
        """Return intervals from root in semitones"""
        return _QUALITY_INTERVALS[self]


_QUALITY_INTERVALS: Dict[ChordQuality, Tuple[int, ...]] = {
    ChordQuality.MAJ7: (0, 4, 7, 11),
    ChordQuality.MIN7: (0, 3, 7, 10),
    ChordQuality.DOM7: (0, 4, 7, 10),
    ChordQuality.HDIM7: (0, 3, 6, 10),
    ChordQuality.DIM7: (0, 3, 6, 9),
    ChordQuality.AUG: (0, 4, 8),
    ChordQuality.MIN_MAJ7: (0, 3, 7, 11),
    ChordQuality.MAJ6: (0, 4, 7, 9),
    ChordQuality.MIN6: (0, 3, 7, 9),
    ChordQuality.SUS4: (0, 5, 7, 10),
    ChordQuality.SUS2: (0, 2, 7, 10),
}


@dataclass
class ChordSymbol:
    """Represents a jazz chord symbol with root, quality, extensions, and alterations.

    Examples:
        Cmaj7, Dm7, G7, Am7b5, Bb7#11, F#m7b5
    """
    root: PitchClass
    quality: ChordQuality
    extensions: List[int] = field(default_factory=list)
    alterations: Dict[str, str] = field(default_factory=dict)
    bass: Optional[PitchClass] = None  # For slash chords

    def __post_init__(self):
        # Ensure extensions are sorted
        self.extensions = sorted(set(self.extensions))

    @classmethod
    def from_string(cls, symbol: str) -> ChordSymbol:
        """Parse chord symbol string (e.g., 'Cmaj7', 'Dm7', 'G7#11')"""
        return _parse_chord_symbol(symbol)

    @property
    def third(self) -> PitchClass:
        """Get the third of the chord (guide tone)"""
        intervals = self.quality.intervals
        third_interval = intervals[1] if len(intervals) > 1 else 4
        return self.root.transpose(third_interval)

    @property
    def seventh(self) -> Optional[PitchClass]:
        """Get the seventh of the chord (guide tone), None if no 7th"""
        intervals = self.quality.intervals
        if len(intervals) < 4:
            return None
        return self.root.transpose(intervals[3])

    @property
    def fifth(self) -> PitchClass:
        """Get the fifth of the chord"""
        intervals = self.quality.intervals
        fifth_interval = intervals[2] if len(intervals) > 2 else 7
        # Apply alterations
        if '5' in self.alterations:
            if self.alterations['5'] == 'b':
                fifth_interval = 6
            elif self.alterations['5'] == '#':
                fifth_interval = 8
        return self.root.transpose(fifth_interval)

    def get_pitch_classes(self) -> List[PitchClass]:
        """Get all pitch classes in the chord including extensions"""
        pitches = [self.root.transpose(i) for i in self.quality.intervals]

        # Add extensions
        extension_intervals = {
            9: 2 if '9' not in self.alterations else (1 if self.alterations['9'] == 'b' else 3),
            11: 5 if '11' not in self.alterations else (6 if self.alterations['11'] == '#' else 4),
            13: 9 if '13' not in self.alterations else (8 if self.alterations['13'] == 'b' else 10),
        }

        for ext in self.extensions:
            if ext in extension_intervals:
                pitches.append(self.root.transpose(extension_intervals[ext]))

        return pitches

    def transpose(self, semitones: int) -> ChordSymbol:
        """Return new ChordSymbol transposed by given semitones"""
        new_bass = self.bass.transpose(semitones) if self.bass else None
        return ChordSymbol(
            root=self.root.transpose(semitones),
            quality=self.quality,
            extensions=self.extensions.copy(),
            alterations=self.alterations.copy(),
            bass=new_bass
        )

    def tritone_substitute(self) -> ChordSymbol:
        """Return tritone substitution of this chord (for dominant chords)"""
        return ChordSymbol(
            root=self.root.transpose(6),
            quality=self.quality,
            extensions=self.extensions.copy(),
            alterations=self.alterations.copy()
        )

    def as_roman_numeral(self, key: PitchClass) -> str:
        """Return Roman numeral representation relative to key"""
        interval = key.interval_to(self.root)
        numerals = ['I', 'bII', 'II', 'bIII', 'III', 'IV', '#IV', 'V', 'bVI', 'VI', 'bVII', 'VII']
        base = numerals[interval]

        # Lowercase for minor qualities
        if self.quality in (ChordQuality.MIN7, ChordQuality.HDIM7, ChordQuality.MIN6, ChordQuality.MIN_MAJ7):
            base = base.lower()

        return base + self.quality.value

    def __str__(self) -> str:
        result = str(self.root) + self.quality.value

        # Add alterations
        for degree, alt in sorted(self.alterations.items()):
            result += alt + degree

        # Add extensions
        for ext in self.extensions:
            if str(ext) not in self.alterations:
                result += f"({ext})"

        # Add bass note for slash chords
        if self.bass and self.bass != self.root:
            result += f"/{self.bass}"

        return result

    def __repr__(self) -> str:
        return f"ChordSymbol({str(self)})"

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, ChordSymbol):
            return NotImplemented
        return (self.root == other.root and
                self.quality == other.quality and
                self.extensions == other.extensions and
                self.alterations == other.alterations)

    def __hash__(self) -> int:
        return hash((self.root, self.quality, tuple(self.extensions),
                     tuple(sorted(self.alterations.items()))))


def _parse_chord_symbol(symbol: str) -> ChordSymbol:
    """Parse a chord symbol string into ChordSymbol object"""
    import re

    symbol = symbol.strip()
    if not symbol:
        raise ValueError("Empty chord symbol")

    # Extract root note
    root_match = re.match(r'^([A-G][#b]?)', symbol)
    if not root_match:
        raise ValueError(f"Invalid chord symbol: {symbol}")

    root_str = root_match.group(1)
    root = PitchClass.from_name(root_str)
    remainder = symbol[len(root_str):]

    # Check for slash chord
    bass = None
    if '/' in remainder:
        parts = remainder.split('/')
        remainder = parts[0]
        if len(parts) > 1 and parts[1]:
            bass = PitchClass.from_name(parts[1])

    # Determine quality
    quality = ChordQuality.DOM7  # Default

    quality_patterns = [
        (r'^maj7', ChordQuality.MAJ7),
        (r'^Maj7', ChordQuality.MAJ7),
        (r'^M7', ChordQuality.MAJ7),
        (r'^Δ7?', ChordQuality.MAJ7),
        (r'^mMaj7', ChordQuality.MIN_MAJ7),
        (r'^m7b5', ChordQuality.HDIM7),
        (r'^m7\-5', ChordQuality.HDIM7),
        (r'^ø7?', ChordQuality.HDIM7),
        (r'^dim7', ChordQuality.DIM7),
        (r'^o7', ChordQuality.DIM7),
        (r'^m7', ChordQuality.MIN7),
        (r'^min7', ChordQuality.MIN7),
        (r'^\-7', ChordQuality.MIN7),
        (r'^m6', ChordQuality.MIN6),
        (r'^min6', ChordQuality.MIN6),
        (r'^6', ChordQuality.MAJ6),
        (r'^aug', ChordQuality.AUG),
        (r'^\+', ChordQuality.AUG),
        (r'^sus4', ChordQuality.SUS4),
        (r'^sus2', ChordQuality.SUS2),
        (r'^7', ChordQuality.DOM7),
    ]

    for pattern, q in quality_patterns:
        match = re.match(pattern, remainder)
        if match:
            quality = q
            remainder = remainder[match.end():]
            break

    # Parse extensions and alterations
    extensions = []
    alterations = {}

    ext_pattern = r'([b#]?)(9|11|13)'
    for match in re.finditer(ext_pattern, remainder):
        alt, degree = match.groups()
        if alt:
            alterations[degree] = alt
        extensions.append(int(degree))

    # Check for altered 5th
    if 'b5' in remainder and '5' not in alterations:
        alterations['5'] = 'b'
    elif '#5' in remainder and '5' not in alterations:
        alterations['5'] = '#'

    return ChordSymbol(
        root=root,
        quality=quality,
        extensions=extensions,
        alterations=alterations,
        bass=bass
    )


# Common chord factory functions
def maj7(root: str) -> ChordSymbol:
    return ChordSymbol(PitchClass.from_name(root), ChordQuality.MAJ7)

def min7(root: str) -> ChordSymbol:
    return ChordSymbol(PitchClass.from_name(root), ChordQuality.MIN7)

def dom7(root: str) -> ChordSymbol:
    return ChordSymbol(PitchClass.from_name(root), ChordQuality.DOM7)

def hdim7(root: str) -> ChordSymbol:
    return ChordSymbol(PitchClass.from_name(root), ChordQuality.HDIM7)

def dim7(root: str) -> ChordSymbol:
    return ChordSymbol(PitchClass.from_name(root), ChordQuality.DIM7)
