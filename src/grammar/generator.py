"""Top-down derivation generator for jazz chord progressions"""

from __future__ import annotations
from dataclasses import dataclass, field
from typing import List, Optional, Tuple, Dict, Any
import random

from .rules import (
    PCFG, GrammarRule, NonTerminal, Symbol, NTSymbol, TerminalSymbol,
    RuleType, create_base_grammar, DEGREE_TO_SEMITONES
)
from ..core.pitch import PitchClass
from ..core.chord import ChordSymbol, ChordQuality


# Map degree strings to ChordQuality
QUALITY_MAP = {
    'maj7': ChordQuality.MAJ7,
    'min7': ChordQuality.MIN7,
    '7': ChordQuality.DOM7,
    'm7b5': ChordQuality.HDIM7,
    'dim7': ChordQuality.DIM7,
    'maj6': ChordQuality.MAJ6,
    'min6': ChordQuality.MIN6,
}


@dataclass
class DerivationNode:
    """Node in derivation tree"""
    symbol: Symbol
    children: List[DerivationNode] = field(default_factory=list)
    rule_used: Optional[GrammarRule] = None
    key: int = 0  # Current key context (0-11)

    def is_terminal(self) -> bool:
        return isinstance(self.symbol, TerminalSymbol)

    def get_terminals(self) -> List[Tuple[TerminalSymbol, int]]:
        """Get all terminal symbols with their key contexts"""
        if self.is_terminal():
            return [(self.symbol, self.key)]
        result = []
        for child in self.children:
            result.extend(child.get_terminals())
        return result

    def depth(self) -> int:
        if not self.children:
            return 0
        return 1 + max(c.depth() for c in self.children)

    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for JSON serialization"""
        if self.is_terminal():
            return {
                'type': 'terminal',
                'symbol': str(self.symbol),
                'key': self.key
            }
        return {
            'type': 'nonterminal',
            'symbol': str(self.symbol),
            'rule': str(self.rule_used) if self.rule_used else None,
            'key': self.key,
            'children': [c.to_dict() for c in self.children]
        }


@dataclass
class GeneratorConfig:
    """Configuration for the generator"""
    max_depth: int = 6
    min_chords: int = 4
    max_chords: int = 32
    key: int = 0  # Key in pitch class (C=0)
    seed: Optional[int] = None


class HarmonyGenerator:
    """Top-down derivation generator for jazz progressions"""

    def __init__(self, grammar: Optional[PCFG] = None, config: Optional[GeneratorConfig] = None):
        self.grammar = grammar or create_base_grammar()
        self.config = config or GeneratorConfig()

        if self.config.seed is not None:
            random.seed(self.config.seed)

    def generate(self) -> List[ChordSymbol]:
        """Generate a chord progression"""
        tree = self._derive(
            NTSymbol(self.grammar.start_symbol, self.config.key),
            depth=0
        )
        terminals = tree.get_terminals()
        return self._terminals_to_chords(terminals)

    def generate_with_tree(self) -> Tuple[List[ChordSymbol], DerivationNode]:
        """Generate progression and return derivation tree"""
        tree = self._derive(
            NTSymbol(self.grammar.start_symbol, self.config.key),
            depth=0
        )
        terminals = tree.get_terminals()
        chords = self._terminals_to_chords(terminals)
        return chords, tree

    def _derive(self, symbol: Symbol, depth: int) -> DerivationNode:
        """Recursively derive from a symbol"""
        if isinstance(symbol, TerminalSymbol):
            key = self.config.key
            return DerivationNode(symbol=symbol, key=key)

        if isinstance(symbol, NTSymbol):
            nt = symbol.nt
            key = symbol.key if symbol.key is not None else self.config.key

            # Check depth limit
            if depth >= self.config.max_depth:
                # Force terminal
                terminal_rules = [r for r in self.grammar.get_rules(nt)
                                  if r.rule_type == RuleType.TERMINAL]
                if terminal_rules:
                    rule = random.choice(terminal_rules)
                else:
                    # Fallback: create default terminal
                    default_terminal = self._get_default_terminal(nt)
                    return DerivationNode(symbol=default_terminal, key=key)
            else:
                rule = self.grammar.sample_rule(nt)

            if rule is None:
                # No rules available, use default terminal
                default_terminal = self._get_default_terminal(nt)
                return DerivationNode(symbol=default_terminal, key=key)

            # Apply rule
            children = []
            for rhs_sym in rule.rhs:
                child_key = key
                if isinstance(rhs_sym, NTSymbol) and rhs_sym.key is not None:
                    child_key = rhs_sym.key

                # Handle key modulation for secondary dominants
                if isinstance(rhs_sym, TerminalSymbol) and 'V/' in rhs_sym.degree:
                    # Secondary dominant: adjust key
                    target = rhs_sym.degree.split('/')[1]
                    if target in DEGREE_TO_SEMITONES:
                        child_key = (key + DEGREE_TO_SEMITONES[target]) % 12

                child_sym = rhs_sym
                if isinstance(child_sym, NTSymbol):
                    child_sym = NTSymbol(child_sym.nt, child_key)

                child_node = self._derive(child_sym, depth + 1)
                child_node.key = child_key
                children.append(child_node)

            return DerivationNode(
                symbol=symbol,
                children=children,
                rule_used=rule,
                key=key
            )

        raise ValueError(f"Unknown symbol type: {type(symbol)}")

    def _get_default_terminal(self, nt: NonTerminal) -> TerminalSymbol:
        """Get default terminal for a non-terminal"""
        defaults = {
            NonTerminal.T: TerminalSymbol("I", "maj7"),
            NonTerminal.D: TerminalSymbol("V", "7"),
            NonTerminal.SD: TerminalSymbol("IV", "maj7"),
            NonTerminal.PREP: TerminalSymbol("ii", "min7"),
            NonTerminal.PROL: TerminalSymbol("vi", "min7"),
            NonTerminal.S: TerminalSymbol("I", "maj7"),
            NonTerminal.PHRASE: TerminalSymbol("I", "maj7"),
        }
        return defaults.get(nt, TerminalSymbol("I", "maj7"))

    def _terminals_to_chords(self, terminals: List[Tuple[TerminalSymbol, int]]) -> List[ChordSymbol]:
        """Convert terminal symbols to ChordSymbol objects"""
        chords = []
        for term, key in terminals:
            chord = self._terminal_to_chord(term, key)
            chords.append(chord)
        return chords

    def _terminal_to_chord(self, terminal: TerminalSymbol, key: int) -> ChordSymbol:
        """Convert a terminal symbol to a ChordSymbol"""
        degree = terminal.degree

        # Handle secondary dominants
        if '/' in degree:
            degree = 'V'  # Simplify to V for now

        # Get interval from degree
        interval = DEGREE_TO_SEMITONES.get(degree, 0)

        # Calculate root
        root_pc = (key + interval) % 12
        root = PitchClass.from_value(root_pc)

        # Get quality
        quality = QUALITY_MAP.get(terminal.quality, ChordQuality.MAJ7)

        return ChordSymbol(root=root, quality=quality)

    def set_key(self, key: int):
        """Set the key for generation"""
        self.config.key = key % 12

    def set_seed(self, seed: int):
        """Set random seed for reproducibility"""
        self.config.seed = seed
        random.seed(seed)


def generate_progression(
    length: int = 8,
    key: str = 'C',
    style: str = 'bebop',
    seed: Optional[int] = None
) -> List[ChordSymbol]:
    """Convenience function to generate a progression"""
    key_pc = PitchClass.from_name(key).value

    config = GeneratorConfig(
        max_depth=min(6, length // 2 + 2),
        min_chords=length,
        max_chords=length * 2,
        key=key_pc,
        seed=seed
    )

    generator = HarmonyGenerator(config=config)
    chords = generator.generate()

    # Trim or pad to desired length
    if len(chords) > length:
        chords = chords[:length]
    elif len(chords) < length:
        # Repeat last chord or add tonic
        tonic = ChordSymbol(PitchClass.from_value(key_pc), ChordQuality.MAJ7)
        while len(chords) < length:
            chords.append(tonic)

    return chords


def format_progression(chords: List[ChordSymbol], bars_per_line: int = 4) -> str:
    """Format chord progression for display"""
    lines = []
    current_line = []

    for i, chord in enumerate(chords):
        current_line.append(str(chord))
        if (i + 1) % bars_per_line == 0:
            lines.append(" | ".join(current_line))
            current_line = []

    if current_line:
        lines.append(" | ".join(current_line))

    return "\n".join(lines)


if __name__ == '__main__':
    import sys

    # Parse arguments
    key = sys.argv[1] if len(sys.argv) > 1 else 'C'
    length = int(sys.argv[2]) if len(sys.argv) > 2 else 8
    seed = int(sys.argv[3]) if len(sys.argv) > 3 else None

    print(f"Generating {length}-chord progression in {key}")
    print("=" * 40)

    chords = generate_progression(length=length, key=key, seed=seed)
    print(format_progression(chords))

    print("\n" + "=" * 40)
    print("Chord details:")
    for i, chord in enumerate(chords, 1):
        print(f"  {i}. {chord} (root={chord.root}, quality={chord.quality.value})")
