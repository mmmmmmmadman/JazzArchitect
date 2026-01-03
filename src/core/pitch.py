"""PitchClass - fundamental pitch representation (0-11)"""

from __future__ import annotations
from typing import Optional


class PitchClass:
    """Represents a pitch class (0-11) independent of octave.

    C=0, C#/Db=1, D=2, D#/Eb=3, E=4, F=5, F#/Gb=6, G=7, G#/Ab=8, A=9, A#/Bb=10, B=11
    """

    _NAMES_SHARP = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']
    _NAMES_FLAT = ['C', 'Db', 'D', 'Eb', 'E', 'F', 'Gb', 'G', 'Ab', 'A', 'Bb', 'B']
    _NAME_TO_VALUE = {
        'C': 0, 'B#': 0,
        'C#': 1, 'Db': 1,
        'D': 2,
        'D#': 3, 'Eb': 3,
        'E': 4, 'Fb': 4,
        'F': 5, 'E#': 5,
        'F#': 6, 'Gb': 6,
        'G': 7,
        'G#': 8, 'Ab': 8,
        'A': 9,
        'A#': 10, 'Bb': 10,
        'B': 11, 'Cb': 11,
    }

    __slots__ = ('_value', '_spelling')

    def __init__(self, value: int, spelling: Optional[str] = None):
        self._value = value % 12
        self._spelling = spelling

    @property
    def value(self) -> int:
        return self._value

    @classmethod
    def from_name(cls, name: str) -> PitchClass:
        """Create PitchClass from note name (e.g., 'C', 'F#', 'Bb')"""
        name = name.strip()
        if name not in cls._NAME_TO_VALUE:
            raise ValueError(f"Unknown pitch name: {name}")
        return cls(cls._NAME_TO_VALUE[name], spelling=name)

    @classmethod
    def from_value(cls, value: int) -> PitchClass:
        """Create PitchClass from integer value (0-11)"""
        return cls(value % 12)

    def interval_to(self, other: PitchClass) -> int:
        """Calculate ascending interval in semitones to another pitch class"""
        return (other.value - self.value) % 12

    def transpose(self, semitones: int) -> PitchClass:
        """Return new PitchClass transposed by given semitones"""
        return PitchClass((self._value + semitones) % 12)

    def name(self, prefer_flat: bool = False) -> str:
        """Get note name, optionally preferring flat spelling"""
        if self._spelling:
            return self._spelling
        names = self._NAMES_FLAT if prefer_flat else self._NAMES_SHARP
        return names[self._value]

    def enharmonic_equal(self, other: PitchClass) -> bool:
        """Check if two pitch classes are enharmonically equivalent"""
        return self._value == other._value

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, PitchClass):
            return NotImplemented
        return self._value == other._value

    def __hash__(self) -> int:
        return hash(self._value)

    def __repr__(self) -> str:
        return f"PitchClass({self.name()})"

    def __str__(self) -> str:
        return self.name()


# Common pitch class constants
C = PitchClass(0)
Db = PitchClass(1, 'Db')
D = PitchClass(2)
Eb = PitchClass(3, 'Eb')
E = PitchClass(4)
F = PitchClass(5)
Gb = PitchClass(6, 'Gb')
G = PitchClass(7)
Ab = PitchClass(8, 'Ab')
A = PitchClass(9)
Bb = PitchClass(10, 'Bb')
B = PitchClass(11)
