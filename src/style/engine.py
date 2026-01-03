"""Style Engine - Integrates style parameters with PCFG generation"""

from __future__ import annotations
from dataclasses import dataclass
from typing import List, Optional, Tuple
import random

from .vectors import StyleVector
from .presets import get_style, BEBOP
from ..grammar.rules import (
    PCFG, GrammarRule, NonTerminal, NTSymbol, TerminalSymbol,
    RuleType, create_base_grammar
)
from ..grammar.generator import HarmonyGenerator, GeneratorConfig
from ..substitution.tritone import TritoneSubstitution
from ..substitution.backdoor import BackdoorSubstitution
from ..substitution.coltrane import ColtraneSubstitution
from ..core.chord import ChordSymbol, ChordQuality
from ..core.pitch import PitchClass


def style_to_pcfg(style: StyleVector, base_grammar: Optional[PCFG] = None) -> PCFG:
    """Convert a style vector to PCFG rule probabilities.

    This modifies the base grammar's rule probabilities based on
    the style parameters.
    """
    grammar = base_grammar or create_base_grammar()

    # Adjust preparation rules based on ii_v_preference
    for rule in grammar.get_rules(NonTerminal.D):
        if rule.name == 'ii_v':
            rule.prob = 0.4 * style.ii_v_preference
        elif rule.name == 'd_terminal':
            rule.prob = 0.4 * (1 - style.ii_v_preference * 0.3)
        elif rule.name == 'tritone_sub':
            rule.prob = 0.15 * style.tritone_sub_prob

    # Adjust prolongation based on style
    for rule in grammar.get_rules(NonTerminal.T):
        if 'prolong' in (rule.name or ''):
            rule.prob = 0.1 * (1 + style.prolongation_depth * 0.2)
        elif rule.name == 'authentic_cadence':
            rule.prob = 0.35 * style.ii_v_preference

    # Adjust prep chain depth
    for rule in grammar.get_rules(NonTerminal.PREP):
        if rule.name == 'prep_chain':
            rule.prob = 0.15 * min(1.0, style.dominant_chain_depth / 4)
        elif rule.name == 'prep_secondary_dom':
            rule.prob = 0.15 * style.secondary_dom_prob

    # Normalize
    grammar.normalize()
    return grammar


@dataclass
class StyledProgression:
    """A chord progression with style metadata"""
    chords: List[ChordSymbol]
    style: StyleVector
    key: PitchClass
    substitutions_applied: List[str]


class StyleEngine:
    """Engine that generates progressions according to style parameters"""

    def __init__(self, style: Optional[StyleVector] = None):
        self.style = style or BEBOP
        self.grammar = style_to_pcfg(self.style)

        # Initialize substitution handlers
        self.tritone_sub = TritoneSubstitution(self.style.tritone_sub_prob)
        self.backdoor_sub = BackdoorSubstitution(self.style.backdoor_prob)
        self.coltrane_sub = ColtraneSubstitution(self.style.coltrane_prob)

    def set_style(self, style: StyleVector):
        """Update the style and regenerate grammar"""
        self.style = style
        self.grammar = style_to_pcfg(style)
        self.tritone_sub = TritoneSubstitution(style.tritone_sub_prob)
        self.backdoor_sub = BackdoorSubstitution(style.backdoor_prob)
        self.coltrane_sub = ColtraneSubstitution(style.coltrane_prob)

    def set_style_by_name(self, name: str):
        """Set style by preset name"""
        self.set_style(get_style(name))

    def generate(
        self,
        length: int = 8,
        key: str = 'C',
        seed: Optional[int] = None
    ) -> StyledProgression:
        """Generate a styled chord progression.

        Args:
            length: Number of chords
            key: Key name (e.g., 'C', 'F#', 'Bb')
            seed: Random seed for reproducibility

        Returns:
            StyledProgression with chords and metadata
        """
        if seed is not None:
            random.seed(seed)

        key_pc = PitchClass.from_name(key)

        config = GeneratorConfig(
            max_depth=min(6, length // 2 + 2),
            key=key_pc.value,
            seed=seed
        )

        generator = HarmonyGenerator(grammar=self.grammar, config=config)
        raw_chords = generator.generate()

        # Apply style-based post-processing
        chords, subs = self._apply_substitutions(raw_chords, key_pc)

        # Trim/pad to length
        chords = self._adjust_length(chords, length, key_pc)

        return StyledProgression(
            chords=chords,
            style=self.style,
            key=key_pc,
            substitutions_applied=subs
        )

    def _apply_substitutions(
        self,
        chords: List[ChordSymbol],
        key: PitchClass
    ) -> Tuple[List[ChordSymbol], List[str]]:
        """Apply style-based substitutions to chord sequence"""
        result = []
        substitutions = []

        i = 0
        while i < len(chords):
            chord = chords[i]

            # Check for ii-V-I pattern
            if i + 2 < len(chords):
                c1, c2, c3 = chords[i], chords[i + 1], chords[i + 2]
                if self._is_ii_v_i(c1, c2, c3, key):
                    # Maybe apply Coltrane changes
                    expanded = self.coltrane_sub.maybe_expand_ii_v_i(
                        c1, c2, c3, random.random()
                    )
                    if len(expanded) > 3:
                        result.extend(expanded)
                        substitutions.append('coltrane_ii_v_i')
                        i += 3
                        continue

            # Check for V-I pattern
            if i + 1 < len(chords):
                v, target = chords[i], chords[i + 1]
                if v.quality == ChordQuality.DOM7:
                    # Maybe apply tritone sub
                    if random.random() < self.style.tritone_sub_prob:
                        from ..substitution.tritone import tritone_substitute
                        result.append(tritone_substitute(v))
                        substitutions.append('tritone_sub')
                        i += 1
                        continue

                    # Maybe apply backdoor
                    new_v = self.backdoor_sub.maybe_substitute_dominant(
                        v, target, random.random()
                    )
                    if new_v != v:
                        result.append(new_v)
                        substitutions.append('backdoor')
                        i += 1
                        continue

            result.append(chord)
            i += 1

        return result, substitutions

    def _is_ii_v_i(
        self,
        c1: ChordSymbol,
        c2: ChordSymbol,
        c3: ChordSymbol,
        key: PitchClass
    ) -> bool:
        """Check if three chords form a ii-V-I pattern"""
        # ii should be minor, root = key + 2
        if c1.quality != ChordQuality.MIN7:
            return False
        if c1.root.interval_to(key) != 10:  # key is 2 semitones above ii
            return False

        # V should be dom7, root = key + 7
        if c2.quality != ChordQuality.DOM7:
            return False
        if c2.root.interval_to(key) != 5:  # key is 5 semitones above V
            return False

        # I should be maj7, root = key
        if c3.quality not in (ChordQuality.MAJ7, ChordQuality.MAJ6):
            return False
        if c3.root != key:
            return False

        return True

    def _adjust_length(
        self,
        chords: List[ChordSymbol],
        target: int,
        key: PitchClass
    ) -> List[ChordSymbol]:
        """Adjust progression to target length"""
        if len(chords) >= target:
            return chords[:target]

        # Pad with appropriate chords
        tonic = ChordSymbol(key, ChordQuality.MAJ7)
        while len(chords) < target:
            # Add turnaround or repeat tonic
            if random.random() < self.style.turnaround_prob and len(chords) < target - 1:
                # Add simple ii-V turnaround
                ii_root = key.transpose(2)
                v_root = key.transpose(7)
                chords.append(ChordSymbol(ii_root, ChordQuality.MIN7))
                if len(chords) < target:
                    chords.append(ChordSymbol(v_root, ChordQuality.DOM7))
            else:
                chords.append(tonic)

        return chords[:target]

    def generate_turnaround(self, key: str = 'C') -> List[ChordSymbol]:
        """Generate a style-appropriate turnaround"""
        key_pc = PitchClass.from_name(key)

        # Basic I-vi-ii-V
        i = ChordSymbol(key_pc, ChordQuality.MAJ7)
        vi = ChordSymbol(key_pc.transpose(9), ChordQuality.MIN7)
        ii = ChordSymbol(key_pc.transpose(2), ChordQuality.MIN7)
        v = ChordSymbol(key_pc.transpose(7), ChordQuality.DOM7)

        turnaround = [i, vi, ii, v]

        # Apply substitutions based on style
        if random.random() < self.style.tritone_sub_prob:
            from ..substitution.tritone import tritone_substitute
            turnaround[3] = tritone_substitute(v)

        if random.random() < self.style.chromatic_approach:
            # Add chromatic approach to V
            approach = ChordSymbol(key_pc.transpose(8), ChordQuality.DOM7)
            turnaround.insert(3, approach)

        return turnaround

    def generate_blues_changes(self, key: str = 'C', bars: int = 12) -> List[ChordSymbol]:
        """Generate blues changes with style-appropriate substitutions"""
        key_pc = PitchClass.from_name(key)

        i7 = ChordSymbol(key_pc, ChordQuality.DOM7)
        iv7 = ChordSymbol(key_pc.transpose(5), ChordQuality.DOM7)
        v7 = ChordSymbol(key_pc.transpose(7), ChordQuality.DOM7)

        # Basic 12-bar blues
        if bars == 12:
            changes = [
                i7, iv7, i7, i7,
                iv7, iv7, i7, i7,
                v7, iv7, i7, v7
            ]
        else:
            changes = [i7] * bars

        # Apply style substitutions
        result = []
        for i, chord in enumerate(changes):
            if random.random() < self.style.tritone_sub_prob and chord.quality == ChordQuality.DOM7:
                from ..substitution.tritone import tritone_substitute
                result.append(tritone_substitute(chord))
            elif random.random() < self.style.minor_iv_prob and chord == iv7:
                # Minor iv
                result.append(ChordSymbol(key_pc.transpose(5), ChordQuality.MIN7))
            else:
                result.append(chord)

        return result


def generate_styled_progression(
    style: str = 'bebop',
    length: int = 8,
    key: str = 'C',
    seed: Optional[int] = None
) -> List[ChordSymbol]:
    """Convenience function to generate a styled progression"""
    engine = StyleEngine(get_style(style))
    result = engine.generate(length=length, key=key, seed=seed)
    return result.chords
