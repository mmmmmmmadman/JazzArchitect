"""MusDr-inspired Evaluation Metrics for Jazz Harmony

Based on Wu & Yang (2020) MusDr framework, adapted for chord progressions.
Original MusDr focuses on melody; this adaptation applies similar concepts to harmony.
"""

from __future__ import annotations
from dataclasses import dataclass
from typing import List, Dict, Tuple, Optional
from collections import Counter
import math

from ..core.chord import ChordSymbol, ChordQuality
from ..core.pitch import PitchClass
from ..voiceleading.guidetone import average_voice_leading_cost


@dataclass
class EvaluationResult:
    """Complete evaluation results for a chord progression"""
    # Pitch-based metrics
    pitch_class_entropy: float          # H: diversity of pitch classes
    root_entropy: float                 # Diversity of chord roots

    # Progression metrics
    chord_progression_irregularity: float  # CPI: unpredictability
    bigram_entropy: float               # Diversity of chord pairs

    # Structure metrics
    structureness_indicator: float      # SI: repetition/pattern detection
    functional_coherence: float         # How well it follows functional harmony

    # Voice leading metrics
    voice_leading_cost: float           # Average VL cost per transition
    smooth_ratio: float                 # Ratio of smooth transitions

    # Quality estimates
    variety_score: float                # Overall variety (0-1)
    coherence_score: float              # Overall coherence (0-1)

    def overall_score(self) -> float:
        """Compute weighted overall quality score"""
        return (
            0.2 * self.variety_score +
            0.3 * self.coherence_score +
            0.2 * min(1.0, self.pitch_class_entropy / 3.0) +
            0.15 * (1.0 - min(1.0, self.voice_leading_cost / 4.0)) +
            0.15 * self.smooth_ratio
        )


def pitch_class_entropy(chords: List[ChordSymbol]) -> float:
    """Calculate pitch class entropy (H) for chord roots.

    Higher entropy = more variety in root notes.
    Max entropy for 12 pitch classes = log2(12) ≈ 3.58
    """
    if not chords:
        return 0.0

    root_counts = Counter(chord.root.value for chord in chords)
    total = len(chords)

    entropy = 0.0
    for count in root_counts.values():
        if count > 0:
            p = count / total
            entropy -= p * math.log2(p)

    return entropy


def root_movement_entropy(chords: List[ChordSymbol]) -> float:
    """Calculate entropy of root movements (intervals between consecutive roots)"""
    if len(chords) < 2:
        return 0.0

    movements = []
    for i in range(len(chords) - 1):
        interval = chords[i].root.interval_to(chords[i + 1].root)
        movements.append(interval)

    movement_counts = Counter(movements)
    total = len(movements)

    entropy = 0.0
    for count in movement_counts.values():
        if count > 0:
            p = count / total
            entropy -= p * math.log2(p)

    return entropy


def chord_bigram_entropy(chords: List[ChordSymbol]) -> float:
    """Calculate entropy of chord bigrams (consecutive pairs)"""
    if len(chords) < 2:
        return 0.0

    # Create bigram representation
    bigrams = []
    for i in range(len(chords) - 1):
        # Use interval and quality pair as bigram
        interval = chords[i].root.interval_to(chords[i + 1].root)
        q1 = chords[i].quality.value
        q2 = chords[i + 1].quality.value
        bigrams.append((interval, q1, q2))

    bigram_counts = Counter(bigrams)
    total = len(bigrams)

    entropy = 0.0
    for count in bigram_counts.values():
        if count > 0:
            p = count / total
            entropy -= p * math.log2(p)

    return entropy


def chord_progression_irregularity(chords: List[ChordSymbol]) -> float:
    """Calculate Chord Progression Irregularity (CPI).

    Measures how unpredictable/irregular the progression is.
    Based on deviation from common jazz patterns.
    """
    if len(chords) < 2:
        return 0.0

    # Common interval patterns in jazz (and their "regularity" weights)
    common_intervals = {
        5: 1.0,   # Perfect 4th up (ii-V, V-I)
        7: 0.9,   # Perfect 5th up (I-V, IV-I)
        2: 0.8,   # Whole step (chromatic approach)
        10: 0.7,  # Minor 3rd down (relative minor)
        3: 0.6,   # Minor 3rd up
        4: 0.5,   # Major 3rd (Coltrane)
    }

    irregularity_sum = 0.0
    for i in range(len(chords) - 1):
        interval = chords[i].root.interval_to(chords[i + 1].root)
        regularity = common_intervals.get(interval, 0.3)
        irregularity_sum += (1.0 - regularity)

    return irregularity_sum / (len(chords) - 1)


def structureness_indicator(chords: List[ChordSymbol]) -> float:
    """Calculate Structureness Indicator (SI).

    Measures the presence of repeated patterns and structure.
    Higher = more structured/repetitive.
    """
    if len(chords) < 4:
        return 0.0

    # Convert to pattern representation
    patterns = []
    for chord in chords:
        patterns.append((chord.root.value, chord.quality.value))

    # Look for repeated subsequences
    n = len(patterns)
    repetition_score = 0.0

    # Check for 2-chord patterns
    for length in [2, 4, 8]:
        if n < length * 2:
            continue

        pattern_counts = Counter()
        for i in range(n - length + 1):
            subseq = tuple(patterns[i:i + length])
            pattern_counts[subseq] += 1

        # Score based on repetitions
        for count in pattern_counts.values():
            if count > 1:
                repetition_score += (count - 1) * length / n

    return min(1.0, repetition_score)


def functional_coherence(chords: List[ChordSymbol], key: PitchClass) -> float:
    """Measure how well the progression follows functional harmony.

    Checks for proper dominant resolution, ii-V patterns, etc.
    """
    if len(chords) < 2:
        return 1.0

    coherent_moves = 0
    total_moves = len(chords) - 1

    for i in range(total_moves):
        c1, c2 = chords[i], chords[i + 1]
        interval = c1.root.interval_to(c2.root)

        # Check for functional progressions
        is_coherent = False

        # V -> I (dominant resolution)
        if c1.quality == ChordQuality.DOM7 and interval == 5:
            is_coherent = True

        # ii -> V
        if c1.quality == ChordQuality.MIN7 and c2.quality == ChordQuality.DOM7 and interval == 5:
            is_coherent = True

        # IV -> I (plagal)
        if interval == 7 and c2.root == key:
            is_coherent = True

        # I -> vi (common)
        if c1.root == key and interval == 9:
            is_coherent = True

        # vi -> ii
        if c1.quality == ChordQuality.MIN7 and c2.quality == ChordQuality.MIN7 and interval == 5:
            is_coherent = True

        # Tritone sub resolution
        if c1.quality == ChordQuality.DOM7 and interval == 1:
            is_coherent = True

        if is_coherent:
            coherent_moves += 1

    return coherent_moves / total_moves if total_moves > 0 else 1.0


def quality_variety(chords: List[ChordSymbol]) -> float:
    """Measure variety in chord qualities"""
    if not chords:
        return 0.0

    qualities = set(chord.quality for chord in chords)
    # Normalize: 1 quality = 0, 5+ qualities = 1
    return min(1.0, (len(qualities) - 1) / 4)


def evaluate_progression(
    chords: List[ChordSymbol],
    key: Optional[PitchClass] = None
) -> EvaluationResult:
    """Perform complete evaluation of a chord progression.

    Args:
        chords: List of ChordSymbol to evaluate
        key: Optional key context (defaults to first chord root)

    Returns:
        EvaluationResult with all metrics
    """
    if not chords:
        return EvaluationResult(
            pitch_class_entropy=0.0,
            root_entropy=0.0,
            chord_progression_irregularity=0.0,
            bigram_entropy=0.0,
            structureness_indicator=0.0,
            functional_coherence=0.0,
            voice_leading_cost=0.0,
            smooth_ratio=0.0,
            variety_score=0.0,
            coherence_score=0.0,
        )

    if key is None:
        key = chords[0].root

    # Calculate metrics
    h = pitch_class_entropy(chords)
    root_h = root_movement_entropy(chords)
    cpi = chord_progression_irregularity(chords)
    bigram_h = chord_bigram_entropy(chords)
    si = structureness_indicator(chords)
    fc = functional_coherence(chords, key)
    vl_cost = average_voice_leading_cost(chords)

    # Calculate smooth ratio
    smooth_count = 0
    if len(chords) >= 2:
        from ..voiceleading.guidetone import voice_leading_cost
        for i in range(len(chords) - 1):
            if voice_leading_cost(chords[i], chords[i + 1]) <= 2:
                smooth_count += 1
        smooth_ratio = smooth_count / (len(chords) - 1)
    else:
        smooth_ratio = 1.0

    # Compute aggregate scores
    variety = (
        0.4 * min(1.0, h / 2.5) +
        0.3 * quality_variety(chords) +
        0.3 * min(1.0, bigram_h / 3.0)
    )

    coherence = (
        0.5 * fc +
        0.3 * (1.0 - cpi) +
        0.2 * si
    )

    return EvaluationResult(
        pitch_class_entropy=h,
        root_entropy=root_h,
        chord_progression_irregularity=cpi,
        bigram_entropy=bigram_h,
        structureness_indicator=si,
        functional_coherence=fc,
        voice_leading_cost=vl_cost,
        smooth_ratio=smooth_ratio,
        variety_score=variety,
        coherence_score=coherence,
    )


def compare_progressions(
    prog1: List[ChordSymbol],
    prog2: List[ChordSymbol],
    key: Optional[PitchClass] = None
) -> Dict[str, Tuple[float, float]]:
    """Compare two progressions on all metrics.

    Returns dict mapping metric name to (prog1_value, prog2_value).
    """
    eval1 = evaluate_progression(prog1, key)
    eval2 = evaluate_progression(prog2, key)

    return {
        'pitch_class_entropy': (eval1.pitch_class_entropy, eval2.pitch_class_entropy),
        'root_entropy': (eval1.root_entropy, eval2.root_entropy),
        'cpi': (eval1.chord_progression_irregularity, eval2.chord_progression_irregularity),
        'bigram_entropy': (eval1.bigram_entropy, eval2.bigram_entropy),
        'structureness': (eval1.structureness_indicator, eval2.structureness_indicator),
        'functional_coherence': (eval1.functional_coherence, eval2.functional_coherence),
        'voice_leading_cost': (eval1.voice_leading_cost, eval2.voice_leading_cost),
        'smooth_ratio': (eval1.smooth_ratio, eval2.smooth_ratio),
        'variety': (eval1.variety_score, eval2.variety_score),
        'coherence': (eval1.coherence_score, eval2.coherence_score),
        'overall': (eval1.overall_score(), eval2.overall_score()),
    }


def print_evaluation(result: EvaluationResult, name: str = "Progression"):
    """Print evaluation results in readable format"""
    print(f"\n=== {name} Evaluation ===")
    print(f"Pitch Class Entropy: {result.pitch_class_entropy:.3f}")
    print(f"Root Movement Entropy: {result.root_entropy:.3f}")
    print(f"Chord Progression Irregularity: {result.chord_progression_irregularity:.3f}")
    print(f"Bigram Entropy: {result.bigram_entropy:.3f}")
    print(f"Structureness Indicator: {result.structureness_indicator:.3f}")
    print(f"Functional Coherence: {result.functional_coherence:.3f}")
    print(f"Voice Leading Cost: {result.voice_leading_cost:.3f}")
    print(f"Smooth Transition Ratio: {result.smooth_ratio:.3f}")
    print(f"---")
    print(f"Variety Score: {result.variety_score:.3f}")
    print(f"Coherence Score: {result.coherence_score:.3f}")
    print(f"Overall Score: {result.overall_score():.3f}")
