"""Backdoor Dominant Resolution"""

from __future__ import annotations
from typing import List, Tuple
from ..core.chord import ChordSymbol, ChordQuality
from ..core.pitch import PitchClass


def backdoor_dominant(tonic: ChordSymbol) -> ChordSymbol:
    """Create the backdoor dominant for a given tonic.

    The backdoor dominant is bVII7, which resolves to I.
    Example: F7 -> Cmaj7 (instead of G7 -> Cmaj7)

    This works because:
    - bVII7 contains the 4th scale degree (Bb in C)
    - Resolves to 3rd (E) by half step
    - Creates a plagal-like motion
    """
    # bVII = root - 2 semitones (whole step down)
    backdoor_root = tonic.root.transpose(-2)
    return ChordSymbol(
        root=backdoor_root,
        quality=ChordQuality.DOM7
    )


def backdoor_ii_v(tonic: ChordSymbol) -> Tuple[ChordSymbol, ChordSymbol]:
    """Create the backdoor ii-V for a given tonic.

    iv7 -> bVII7 -> Imaj7
    Example: Fm7 -> Bb7 -> Cmaj7

    This is the minor iv borrowed from parallel minor,
    followed by its dominant (which is bVII of the major key).
    """
    # iv = root + 5 semitones (perfect 4th), minor quality
    iv_root = tonic.root.transpose(5)
    iv = ChordSymbol(root=iv_root, quality=ChordQuality.MIN7)

    # bVII7 = root - 2 semitones
    bvii = backdoor_dominant(tonic)

    return iv, bvii


def is_backdoor_resolution(v: ChordSymbol, i: ChordSymbol) -> bool:
    """Check if this is a backdoor resolution (bVII7 -> I)"""
    if v.quality != ChordQuality.DOM7:
        return False

    # bVII to I is 2 semitones up
    interval = v.root.interval_to(i.root)
    return interval == 2


def create_backdoor_approach(tonic: ChordSymbol) -> List[ChordSymbol]:
    """Create full backdoor approach: iv7 -> bVII7 -> Imaj7"""
    iv, bvii = backdoor_ii_v(tonic)
    return [iv, bvii, tonic]


class BackdoorSubstitution:
    """Backdoor dominant substitution handler"""

    def __init__(self, probability: float = 0.15):
        """
        Args:
            probability: Probability of using backdoor instead of V7 (0-1)
        """
        self.probability = probability

    def maybe_substitute_dominant(
        self,
        dominant: ChordSymbol,
        tonic: ChordSymbol,
        random_val: float
    ) -> ChordSymbol:
        """Maybe replace V7 with bVII7"""
        if random_val >= self.probability:
            return dominant

        if dominant.quality != ChordQuality.DOM7:
            return dominant

        # Check if this is actually a V-I relationship
        # V to I is 5 semitones up (or 7 down)
        interval = dominant.root.interval_to(tonic.root)
        if interval == 5:  # This is V -> I
            return backdoor_dominant(tonic)

        return dominant
