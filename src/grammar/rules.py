"""PCFG Rule Structure for Jazz Harmony Grammar"""

from __future__ import annotations
from dataclasses import dataclass, field
from enum import Enum, auto
from typing import List, Optional, Union, Dict, Tuple
import random


class RuleType(Enum):
    """Types of grammar rules in jazz harmony"""
    PROLONGATION = auto()   # X -> X X (extension)
    PREPARATION = auto()    # X -> Prep X (approach)
    SUBSTITUTION = auto()   # X -> Y (chord substitution)
    TERMINAL = auto()       # X -> chord (leaf node)
    STRUCTURAL = auto()     # S -> T D T (top-level structure)


class NonTerminal(Enum):
    """Non-terminal symbols in the grammar"""
    S = "S"         # Start symbol (entire piece)
    T = "T"         # Tonic constituent
    D = "D"         # Dominant constituent
    SD = "SD"       # Subdominant constituent
    PROL = "Prol"   # Prolongation
    PREP = "Prep"   # Preparation
    PHRASE = "Ph"   # Phrase boundary


class FunctionalCategory(Enum):
    """Functional categories for chord classification"""
    TONIC = "T"
    DOMINANT = "D"
    SUBDOMINANT = "SD"
    APPLIED = "App"     # Applied/secondary dominant


@dataclass
class Symbol:
    """Base class for grammar symbols"""
    pass


@dataclass
class NTSymbol(Symbol):
    """Non-terminal symbol wrapper"""
    nt: NonTerminal
    key: Optional[int] = None  # Key context (0-11), None = inherit

    def __str__(self) -> str:
        if self.key is not None:
            return f"{self.nt.value}[{self.key}]"
        return self.nt.value


@dataclass
class TerminalSymbol(Symbol):
    """Terminal symbol representing a chord function"""
    degree: str             # Roman numeral (I, ii, V, bII, etc.)
    quality: str            # maj7, min7, 7, m7b5, etc.
    key_relative: bool = True  # Is degree relative to current key?

    def __str__(self) -> str:
        return f"{self.degree}{self.quality}"


@dataclass
class GrammarRule:
    """A single PCFG production rule"""
    lhs: NonTerminal
    rhs: List[Symbol]
    prob: float = 1.0
    rule_type: RuleType = RuleType.STRUCTURAL
    name: Optional[str] = None

    def __post_init__(self):
        if not 0.0 <= self.prob <= 1.0:
            raise ValueError(f"Probability must be in [0, 1], got {self.prob}")

    def __str__(self) -> str:
        rhs_str = " ".join(str(s) for s in self.rhs)
        return f"{self.lhs.value} -> {rhs_str} [{self.prob:.3f}]"

    def apply(self, key: int = 0) -> List[Symbol]:
        """Apply rule, propagating key context"""
        result = []
        for sym in self.rhs:
            if isinstance(sym, NTSymbol):
                # Inherit key if not specified
                new_key = sym.key if sym.key is not None else key
                result.append(NTSymbol(sym.nt, new_key))
            else:
                result.append(sym)
        return result


@dataclass
class PCFG:
    """Probabilistic Context-Free Grammar for jazz harmony"""
    rules: Dict[NonTerminal, List[GrammarRule]] = field(default_factory=dict)
    start_symbol: NonTerminal = NonTerminal.S

    def add_rule(self, rule: GrammarRule):
        """Add a rule to the grammar"""
        if rule.lhs not in self.rules:
            self.rules[rule.lhs] = []
        self.rules[rule.lhs].append(rule)

    def get_rules(self, nt: NonTerminal) -> List[GrammarRule]:
        """Get all rules for a non-terminal"""
        return self.rules.get(nt, [])

    def normalize(self):
        """Normalize probabilities for each non-terminal"""
        for nt, rule_list in self.rules.items():
            total = sum(r.prob for r in rule_list)
            if total > 0:
                for r in rule_list:
                    r.prob = r.prob / total

    def sample_rule(self, nt: NonTerminal) -> Optional[GrammarRule]:
        """Sample a rule according to probabilities"""
        rules = self.get_rules(nt)
        if not rules:
            return None

        probs = [r.prob for r in rules]
        total = sum(probs)
        if total == 0:
            return random.choice(rules)

        r = random.random() * total
        cumsum = 0
        for rule, prob in zip(rules, probs):
            cumsum += prob
            if r <= cumsum:
                return rule
        return rules[-1]

    def __str__(self) -> str:
        lines = []
        for nt in NonTerminal:
            rules = self.get_rules(nt)
            for rule in rules:
                lines.append(str(rule))
        return "\n".join(lines)


def create_base_grammar() -> PCFG:
    """Create the base jazz harmony grammar based on Rohrmeier (2020)"""
    grammar = PCFG()

    # Structural rules (S)
    grammar.add_rule(GrammarRule(
        NonTerminal.S,
        [NTSymbol(NonTerminal.T)],
        prob=0.3,
        rule_type=RuleType.STRUCTURAL,
        name="single_phrase"
    ))
    grammar.add_rule(GrammarRule(
        NonTerminal.S,
        [NTSymbol(NonTerminal.T), NTSymbol(NonTerminal.D), NTSymbol(NonTerminal.T)],
        prob=0.5,
        rule_type=RuleType.STRUCTURAL,
        name="tdt_form"
    ))
    grammar.add_rule(GrammarRule(
        NonTerminal.S,
        [NTSymbol(NonTerminal.T), NTSymbol(NonTerminal.T)],
        prob=0.2,
        rule_type=RuleType.STRUCTURAL,
        name="tt_form"
    ))

    # Tonic rules (T)
    grammar.add_rule(GrammarRule(
        NonTerminal.T,
        [TerminalSymbol("I", "maj7")],
        prob=0.3,
        rule_type=RuleType.TERMINAL,
        name="t_terminal"
    ))
    grammar.add_rule(GrammarRule(
        NonTerminal.T,
        [NTSymbol(NonTerminal.D), NTSymbol(NonTerminal.T)],
        prob=0.35,
        rule_type=RuleType.PREPARATION,
        name="authentic_cadence"
    ))
    grammar.add_rule(GrammarRule(
        NonTerminal.T,
        [NTSymbol(NonTerminal.SD), NTSymbol(NonTerminal.T)],
        prob=0.15,
        rule_type=RuleType.PREPARATION,
        name="plagal_cadence"
    ))
    grammar.add_rule(GrammarRule(
        NonTerminal.T,
        [NTSymbol(NonTerminal.T), NTSymbol(NonTerminal.PROL)],
        prob=0.1,
        rule_type=RuleType.PROLONGATION,
        name="t_right_prolong"
    ))
    grammar.add_rule(GrammarRule(
        NonTerminal.T,
        [NTSymbol(NonTerminal.PROL), NTSymbol(NonTerminal.T)],
        prob=0.1,
        rule_type=RuleType.PROLONGATION,
        name="t_left_prolong"
    ))

    # Dominant rules (D)
    grammar.add_rule(GrammarRule(
        NonTerminal.D,
        [TerminalSymbol("V", "7")],
        prob=0.4,
        rule_type=RuleType.TERMINAL,
        name="d_terminal"
    ))
    grammar.add_rule(GrammarRule(
        NonTerminal.D,
        [NTSymbol(NonTerminal.PREP), NTSymbol(NonTerminal.D)],
        prob=0.4,
        rule_type=RuleType.PREPARATION,
        name="ii_v"
    ))
    grammar.add_rule(GrammarRule(
        NonTerminal.D,
        [NTSymbol(NonTerminal.D), NTSymbol(NonTerminal.PROL)],
        prob=0.1,
        rule_type=RuleType.PROLONGATION,
        name="d_prolong"
    ))
    grammar.add_rule(GrammarRule(
        NonTerminal.D,
        [TerminalSymbol("bII", "7")],
        prob=0.1,
        rule_type=RuleType.SUBSTITUTION,
        name="tritone_sub"
    ))

    # Subdominant rules (SD)
    grammar.add_rule(GrammarRule(
        NonTerminal.SD,
        [TerminalSymbol("IV", "maj7")],
        prob=0.5,
        rule_type=RuleType.TERMINAL,
        name="sd_iv"
    ))
    grammar.add_rule(GrammarRule(
        NonTerminal.SD,
        [TerminalSymbol("ii", "min7")],
        prob=0.3,
        rule_type=RuleType.TERMINAL,
        name="sd_ii"
    ))
    grammar.add_rule(GrammarRule(
        NonTerminal.SD,
        [TerminalSymbol("iv", "min7")],
        prob=0.2,
        rule_type=RuleType.TERMINAL,
        name="sd_borrowed_iv"
    ))

    # Preparation rules (Prep)
    grammar.add_rule(GrammarRule(
        NonTerminal.PREP,
        [TerminalSymbol("ii", "min7")],
        prob=0.5,
        rule_type=RuleType.TERMINAL,
        name="prep_ii"
    ))
    grammar.add_rule(GrammarRule(
        NonTerminal.PREP,
        [TerminalSymbol("IV", "maj7")],
        prob=0.2,
        rule_type=RuleType.TERMINAL,
        name="prep_iv"
    ))
    grammar.add_rule(GrammarRule(
        NonTerminal.PREP,
        [TerminalSymbol("V/V", "7")],
        prob=0.15,
        rule_type=RuleType.TERMINAL,
        name="prep_secondary_dom"
    ))
    grammar.add_rule(GrammarRule(
        NonTerminal.PREP,
        [NTSymbol(NonTerminal.PREP), NTSymbol(NonTerminal.PREP)],
        prob=0.15,
        rule_type=RuleType.PROLONGATION,
        name="prep_chain"
    ))

    # Prolongation rules (Prol)
    grammar.add_rule(GrammarRule(
        NonTerminal.PROL,
        [TerminalSymbol("iii", "min7")],
        prob=0.3,
        rule_type=RuleType.TERMINAL,
        name="prol_iii"
    ))
    grammar.add_rule(GrammarRule(
        NonTerminal.PROL,
        [TerminalSymbol("vi", "min7")],
        prob=0.4,
        rule_type=RuleType.TERMINAL,
        name="prol_vi"
    ))
    grammar.add_rule(GrammarRule(
        NonTerminal.PROL,
        [TerminalSymbol("I", "maj7")],
        prob=0.3,
        rule_type=RuleType.TERMINAL,
        name="prol_i"
    ))

    grammar.normalize()
    return grammar


# Degree to semitone mapping
DEGREE_TO_SEMITONES = {
    "I": 0, "bII": 1, "II": 2, "bIII": 3, "III": 4,
    "IV": 5, "#IV": 6, "V": 7, "bVI": 8, "VI": 9,
    "bVII": 10, "VII": 11,
    "i": 0, "ii": 2, "iii": 4, "iv": 5, "v": 7, "vi": 9, "vii": 11,
    "V/V": 2,  # Secondary dominant of V
    "V/ii": 9,  # Secondary dominant of ii
    "V/IV": 0,  # Secondary dominant of IV
}
