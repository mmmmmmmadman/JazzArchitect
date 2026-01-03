"""Tritone Substitution Rules"""

from __future__ import annotations
from typing import List, Optional
from ..core.chord import ChordSymbol, ChordQuality
from ..core.pitch import PitchClass


def tritone_substitute(chord: ChordSymbol) -> ChordSymbol:
    """Apply tritone substitution to a dominant chord.

    V7 -> bII7 (root moves down by tritone/6 semitones)

    The tritone sub works because:
    - The guide tones (3rd and 7th) are swapped
    - G7 (B, F) -> Db7 (F, Cb/B) - same tritone interval
    """
    new_root = chord.root.transpose(6)  # Tritone = 6 semitones
    return ChordSymbol(
        root=new_root,
        quality=chord.quality,
        extensions=chord.extensions.copy(),
        alterations=chord.alterations.copy()
    )


def reverse_tritone_substitute(chord: ChordSymbol) -> ChordSymbol:
    """Reverse a tritone substitution (same operation, since tritone is symmetric)"""
    return tritone_substitute(chord)


def can_apply_tritone_sub(chord: ChordSymbol) -> bool:
    """Check if tritone substitution can be applied"""
    return chord.quality in (ChordQuality.DOM7, ChordQuality.SUS4)


def apply_tritone_to_ii_v(ii: ChordSymbol, v: ChordSymbol) -> tuple[ChordSymbol, ChordSymbol]:
    """Apply tritone substitution to a ii-V progression.

    Dm7 - G7 can become:
    - Dm7 - Db7 (sub V only)
    - Abm7 - Db7 (sub both - less common)
    """
    if not can_apply_tritone_sub(v):
        return ii, v

    new_v = tritone_substitute(v)
    return ii, new_v


def tritone_sub_chain(dominants: List[ChordSymbol], sub_indices: Optional[List[int]] = None) -> List[ChordSymbol]:
    """Apply tritone substitution to selected dominants in a chain.

    Example: G7 - C7 - F7 with sub_indices=[1] becomes G7 - Gb7 - F7
    """
    result = []
    sub_set = set(sub_indices) if sub_indices else set()

    for i, chord in enumerate(dominants):
        if i in sub_set and can_apply_tritone_sub(chord):
            result.append(tritone_substitute(chord))
        else:
            result.append(chord)

    return result


class TritoneSubstitution:
    """Tritone substitution rule handler"""

    def __init__(self, probability: float = 0.3):
        """
        Args:
            probability: Probability of applying tritone sub when possible (0-1)
        """
        self.probability = probability

    def maybe_substitute(self, chord: ChordSymbol, random_val: float) -> ChordSymbol:
        """Conditionally apply tritone substitution based on probability"""
        if random_val < self.probability and can_apply_tritone_sub(chord):
            return tritone_substitute(chord)
        return chord

    def substitute_progression(
        self,
        chords: List[ChordSymbol],
        random_vals: List[float]
    ) -> List[ChordSymbol]:
        """Apply tritone substitution to a progression"""
        result = []
        for chord, rv in zip(chords, random_vals):
            result.append(self.maybe_substitute(chord, rv))
        return result
