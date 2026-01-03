"""Style Vector Definition for Jazz Harmony Generation"""

from __future__ import annotations
from dataclasses import dataclass, field
from typing import Dict, Optional
import copy


@dataclass
class StyleVector:
    """Style parameters that control harmony generation.

    Each parameter affects the probability of certain harmonic choices.
    """

    # Substitution probabilities
    tritone_sub_prob: float = 0.3       # Probability of tritone substitution
    backdoor_prob: float = 0.15         # Probability of backdoor dominant
    coltrane_prob: float = 0.1          # Probability of Coltrane changes

    # Preparation preferences
    ii_v_preference: float = 0.8        # Preference for ii-V over direct V (0-1)
    secondary_dom_prob: float = 0.3     # Probability of secondary dominants

    # Modal/borrowed chord usage
    modal_interchange: float = 0.2      # Frequency of borrowed chords
    minor_iv_prob: float = 0.15         # Probability of minor iv in major

    # Approach patterns
    chromatic_approach: float = 0.2     # Chromatic neighbor chord frequency
    diminished_approach: float = 0.1    # Diminished passing chord frequency

    # Structure parameters
    dominant_chain_depth: int = 3       # Max depth of dominant chains
    prolongation_depth: int = 2         # Max prolongation recursion

    # Rhythmic density (harmonic rhythm)
    rhythm_density: float = 0.5         # Chord change frequency (0=sparse, 1=dense)
    turnaround_prob: float = 0.4        # Probability of turnaround at phrase end

    # Tension parameters
    extension_level: float = 0.5        # Average extension level (9ths, 11ths, 13ths)
    alteration_prob: float = 0.2        # Probability of altered dominants

    def __post_init__(self):
        self._validate()

    def _validate(self):
        """Validate all parameters are in valid ranges"""
        for name in ['tritone_sub_prob', 'backdoor_prob', 'coltrane_prob',
                     'ii_v_preference', 'secondary_dom_prob', 'modal_interchange',
                     'minor_iv_prob', 'chromatic_approach', 'diminished_approach',
                     'rhythm_density', 'turnaround_prob', 'extension_level',
                     'alteration_prob']:
            val = getattr(self, name)
            if not 0.0 <= val <= 1.0:
                raise ValueError(f"{name} must be in [0, 1], got {val}")

        if self.dominant_chain_depth < 1:
            raise ValueError("dominant_chain_depth must be >= 1")
        if self.prolongation_depth < 0:
            raise ValueError("prolongation_depth must be >= 0")

    def copy(self) -> StyleVector:
        """Create a copy of this style vector"""
        return copy.deepcopy(self)

    def blend(self, other: StyleVector, weight: float = 0.5) -> StyleVector:
        """Blend with another style vector.

        Args:
            other: Another StyleVector to blend with
            weight: Weight for the other vector (0 = self, 1 = other)

        Returns:
            New StyleVector with blended values
        """
        w1, w2 = 1 - weight, weight

        return StyleVector(
            tritone_sub_prob=w1 * self.tritone_sub_prob + w2 * other.tritone_sub_prob,
            backdoor_prob=w1 * self.backdoor_prob + w2 * other.backdoor_prob,
            coltrane_prob=w1 * self.coltrane_prob + w2 * other.coltrane_prob,
            ii_v_preference=w1 * self.ii_v_preference + w2 * other.ii_v_preference,
            secondary_dom_prob=w1 * self.secondary_dom_prob + w2 * other.secondary_dom_prob,
            modal_interchange=w1 * self.modal_interchange + w2 * other.modal_interchange,
            minor_iv_prob=w1 * self.minor_iv_prob + w2 * other.minor_iv_prob,
            chromatic_approach=w1 * self.chromatic_approach + w2 * other.chromatic_approach,
            diminished_approach=w1 * self.diminished_approach + w2 * other.diminished_approach,
            dominant_chain_depth=int(w1 * self.dominant_chain_depth + w2 * other.dominant_chain_depth),
            prolongation_depth=int(w1 * self.prolongation_depth + w2 * other.prolongation_depth),
            rhythm_density=w1 * self.rhythm_density + w2 * other.rhythm_density,
            turnaround_prob=w1 * self.turnaround_prob + w2 * other.turnaround_prob,
            extension_level=w1 * self.extension_level + w2 * other.extension_level,
            alteration_prob=w1 * self.alteration_prob + w2 * other.alteration_prob,
        )

    def to_dict(self) -> Dict[str, float]:
        """Convert to dictionary"""
        return {
            'tritone_sub_prob': self.tritone_sub_prob,
            'backdoor_prob': self.backdoor_prob,
            'coltrane_prob': self.coltrane_prob,
            'ii_v_preference': self.ii_v_preference,
            'secondary_dom_prob': self.secondary_dom_prob,
            'modal_interchange': self.modal_interchange,
            'minor_iv_prob': self.minor_iv_prob,
            'chromatic_approach': self.chromatic_approach,
            'diminished_approach': self.diminished_approach,
            'dominant_chain_depth': self.dominant_chain_depth,
            'prolongation_depth': self.prolongation_depth,
            'rhythm_density': self.rhythm_density,
            'turnaround_prob': self.turnaround_prob,
            'extension_level': self.extension_level,
            'alteration_prob': self.alteration_prob,
        }

    @classmethod
    def from_dict(cls, d: Dict[str, float]) -> StyleVector:
        """Create from dictionary"""
        return cls(**d)

    def describe(self) -> str:
        """Generate human-readable description of the style"""
        parts = []

        if self.tritone_sub_prob > 0.4:
            parts.append("heavy tritone substitution")
        elif self.tritone_sub_prob > 0.2:
            parts.append("moderate tritone subs")

        if self.ii_v_preference > 0.7:
            parts.append("strong ii-V preference")

        if self.coltrane_prob > 0.15:
            parts.append("Coltrane-influenced")

        if self.modal_interchange > 0.4:
            parts.append("modal borrowing")

        if self.rhythm_density > 0.7:
            parts.append("dense harmonic rhythm")
        elif self.rhythm_density < 0.3:
            parts.append("sparse changes")

        if self.extension_level > 0.6:
            parts.append("extended harmonies")

        if self.alteration_prob > 0.3:
            parts.append("altered dominants")

        return ", ".join(parts) if parts else "standard jazz harmony"
