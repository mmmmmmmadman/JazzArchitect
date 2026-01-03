"""Coltrane Changes (Giant Steps / Countdown substitutions)"""

from __future__ import annotations
from typing import List
from ..core.chord import ChordSymbol, ChordQuality
from ..core.pitch import PitchClass


def coltrane_substitution(tonic: ChordSymbol) -> List[ChordSymbol]:
    """Generate Coltrane changes for a tonic chord.

    The Coltrane substitution divides the octave into three equal parts
    (major thirds) and creates a cycle of dominant resolutions.

    I -> III7 -> bVI -> bII7 (-> V7 -> I)

    In C: Cmaj7 -> E7 -> Abmaj7 -> B7 -> Ebmaj7 -> G7 -> Cmaj7

    This creates three key centers a major third apart.
    """
    root = tonic.root

    # Three key centers (major 3rds apart)
    center1 = root  # C
    center2 = root.transpose(4)  # E (major 3rd up)
    center3 = root.transpose(8)  # Ab (another major 3rd up)

    # For each center, we have V7 -> I
    # V of center1 (C) = G7
    # V of center2 (E) = B7
    # V of center3 (Ab) = Eb7

    v_of_1 = ChordSymbol(root.transpose(7), ChordQuality.DOM7)      # G7
    v_of_2 = ChordSymbol(center2.transpose(7), ChordQuality.DOM7)   # B7
    v_of_3 = ChordSymbol(center3.transpose(7), ChordQuality.DOM7)   # Eb7

    i_1 = ChordSymbol(center1, ChordQuality.MAJ7)   # Cmaj7
    i_2 = ChordSymbol(center2, ChordQuality.MAJ7)   # Emaj7
    i_3 = ChordSymbol(center3, ChordQuality.MAJ7)   # Abmaj7

    # Full Coltrane cycle: starting from I
    # Cmaj7 -> B7 -> Emaj7 -> Eb7 -> Abmaj7 -> G7 -> Cmaj7
    return [i_1, v_of_2, i_2, v_of_3, i_3, v_of_1, i_1]


def coltrane_over_ii_v_i(ii: ChordSymbol, v: ChordSymbol, i: ChordSymbol) -> List[ChordSymbol]:
    """Apply Coltrane substitution over a ii-V-I progression.

    This is the "Countdown" pattern:
    Dm7 - G7 - Cmaj7 becomes:
    Dm7 - Eb7 - Abmaj7 - B7 - Emaj7 - G7 - Cmaj7

    Or simplified for 2 bars:
    Dm7 Eb7 | Abmaj7 B7 | Emaj7 G7 | Cmaj7
    """
    root = i.root

    # Key centers
    center2 = root.transpose(4)  # Major 3rd up
    center3 = root.transpose(8)  # 2 major 3rds up

    return [
        ii,
        ChordSymbol(center3.transpose(7), ChordQuality.DOM7),  # Eb7
        ChordSymbol(center3, ChordQuality.MAJ7),               # Abmaj7
        ChordSymbol(center2.transpose(7), ChordQuality.DOM7),  # B7
        ChordSymbol(center2, ChordQuality.MAJ7),               # Emaj7
        v,                                                      # G7
        i                                                       # Cmaj7
    ]


def thirds_cycle(start: ChordSymbol, direction: str = 'down', steps: int = 3) -> List[ChordSymbol]:
    """Generate a cycle of major or minor thirds.

    Args:
        start: Starting chord
        direction: 'up' or 'down'
        steps: Number of steps in the cycle

    Used in post-bop and modern jazz for non-functional progressions.
    """
    interval = 4 if direction == 'up' else -4  # Major third
    chords = [start]

    current = start
    for _ in range(steps - 1):
        new_root = current.root.transpose(interval)
        current = ChordSymbol(root=new_root, quality=current.quality)
        chords.append(current)

    return chords


def chromatic_dominant_descent(start_root: PitchClass, steps: int = 4) -> List[ChordSymbol]:
    """Generate descending chromatic dominant sequence.

    Common in Coltrane's playing: G7 - Gb7 - F7 - E7 etc.
    Each dominant can resolve to a key a tritone away (tritone sub relationship).
    """
    chords = []
    current = start_root

    for _ in range(steps):
        chords.append(ChordSymbol(current, ChordQuality.DOM7))
        current = current.transpose(-1)  # Descend by half step

    return chords


class ColtraneSubstitution:
    """Coltrane changes substitution handler"""

    def __init__(self, probability: float = 0.1):
        """
        Args:
            probability: Probability of applying Coltrane changes (0-1)
        """
        self.probability = probability

    def maybe_expand_ii_v_i(
        self,
        ii: ChordSymbol,
        v: ChordSymbol,
        i: ChordSymbol,
        random_val: float
    ) -> List[ChordSymbol]:
        """Maybe expand ii-V-I to Coltrane changes"""
        if random_val < self.probability:
            return coltrane_over_ii_v_i(ii, v, i)
        return [ii, v, i]

    def maybe_expand_tonic(
        self,
        tonic: ChordSymbol,
        bars: int,
        random_val: float
    ) -> List[ChordSymbol]:
        """Maybe expand a tonic chord to Coltrane cycle"""
        if random_val < self.probability and bars >= 4:
            cycle = coltrane_substitution(tonic)
            # Trim to fit bars
            return cycle[:bars]
        return [tonic] * bars
