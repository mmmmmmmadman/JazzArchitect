"""Guide-tone Voice Leading Optimization (based on Smither 2019)"""

from __future__ import annotations
from dataclasses import dataclass
from typing import List, Optional, Tuple, Dict
from enum import Enum

from ..core.chord import ChordSymbol, ChordQuality
from ..core.pitch import PitchClass


class VoiceLeadingType(Enum):
    """Types of voice leading motion"""
    COMMON_TONE = "common"      # Note stays the same
    STEP = "step"               # Half or whole step
    SKIP = "skip"               # Third
    LEAP = "leap"               # Fourth or larger


@dataclass
class GuideTone:
    """A guide tone (3rd or 7th of a chord)"""
    pitch: PitchClass
    function: str  # 'third' or 'seventh'
    chord: ChordSymbol


@dataclass
class VoiceLeadingConnection:
    """Connection between two guide tones"""
    from_tone: GuideTone
    to_tone: GuideTone
    interval: int           # Semitones (0-11)
    motion_type: VoiceLeadingType
    cost: float             # Lower is better


def min_interval(pc1: PitchClass, pc2: PitchClass) -> int:
    """Calculate minimum interval between two pitch classes (0-6 semitones)"""
    forward = pc1.interval_to(pc2)
    backward = pc2.interval_to(pc1)
    return min(forward, backward)


def classify_motion(interval: int) -> VoiceLeadingType:
    """Classify the type of motion based on interval"""
    if interval == 0:
        return VoiceLeadingType.COMMON_TONE
    elif interval <= 2:
        return VoiceLeadingType.STEP
    elif interval <= 4:
        return VoiceLeadingType.SKIP
    else:
        return VoiceLeadingType.LEAP


def voice_leading_cost(chord1: ChordSymbol, chord2: ChordSymbol) -> float:
    """Calculate voice leading cost between two chords based on guide tones.

    Lower cost = smoother voice leading.
    Optimal progressions have cost <= 2.

    Based on Smither (2019) guide-tone voice leading theory.
    """
    third1 = chord1.third
    seventh1 = chord1.seventh
    third2 = chord2.third
    seventh2 = chord2.seventh

    # If no seventh, use fifth as substitute
    if seventh1 is None:
        seventh1 = chord1.fifth
    if seventh2 is None:
        seventh2 = chord2.fifth

    # Calculate all possible voice leading paths
    # Path 1: 3rd -> 3rd, 7th -> 7th
    cost1 = min_interval(third1, third2) + min_interval(seventh1, seventh2)

    # Path 2: 3rd -> 7th, 7th -> 3rd (voice exchange)
    cost2 = min_interval(third1, seventh2) + min_interval(seventh1, third2)

    return min(cost1, cost2)


def analyze_voice_leading(
    chord1: ChordSymbol,
    chord2: ChordSymbol
) -> List[VoiceLeadingConnection]:
    """Analyze voice leading connections between two chords"""
    connections = []

    gt1_3 = GuideTone(chord1.third, 'third', chord1)
    gt1_7 = GuideTone(chord1.seventh or chord1.fifth, 'seventh', chord1)
    gt2_3 = GuideTone(chord2.third, 'third', chord2)
    gt2_7 = GuideTone(chord2.seventh or chord2.fifth, 'seventh', chord2)

    for from_gt in [gt1_3, gt1_7]:
        for to_gt in [gt2_3, gt2_7]:
            interval = min_interval(from_gt.pitch, to_gt.pitch)
            motion = classify_motion(interval)
            cost = interval * (1.0 if motion in (VoiceLeadingType.COMMON_TONE, VoiceLeadingType.STEP) else 1.5)

            connections.append(VoiceLeadingConnection(
                from_tone=from_gt,
                to_tone=to_gt,
                interval=interval,
                motion_type=motion,
                cost=cost
            ))

    return connections


def progression_voice_leading_cost(chords: List[ChordSymbol]) -> float:
    """Calculate total voice leading cost for a progression"""
    if len(chords) < 2:
        return 0.0

    total = 0.0
    for i in range(len(chords) - 1):
        total += voice_leading_cost(chords[i], chords[i + 1])

    return total


def average_voice_leading_cost(chords: List[ChordSymbol]) -> float:
    """Calculate average voice leading cost per transition"""
    if len(chords) < 2:
        return 0.0

    return progression_voice_leading_cost(chords) / (len(chords) - 1)


def find_smoothest_voicing(
    chord: ChordSymbol,
    prev_guide_tones: Tuple[PitchClass, PitchClass]
) -> Tuple[PitchClass, PitchClass]:
    """Find the smoothest guide tone voicing given previous guide tones.

    Returns the (3rd, 7th) voicing that minimizes voice leading distance.
    """
    third = chord.third
    seventh = chord.seventh or chord.fifth

    prev_3, prev_7 = prev_guide_tones

    # Option 1: 3rd -> 3rd, 7th -> 7th
    cost1 = min_interval(prev_3, third) + min_interval(prev_7, seventh)

    # Option 2: Exchange (3rd -> 7th, 7th -> 3rd)
    cost2 = min_interval(prev_3, seventh) + min_interval(prev_7, third)

    if cost1 <= cost2:
        return (third, seventh)
    else:
        return (seventh, third)


@dataclass
class VoiceLeadingAnalysis:
    """Complete voice leading analysis for a progression"""
    chords: List[ChordSymbol]
    connections: List[List[VoiceLeadingConnection]]
    total_cost: float
    average_cost: float
    smooth_transitions: int  # Transitions with cost <= 2
    rough_transitions: int   # Transitions with cost > 4


def analyze_progression(chords: List[ChordSymbol]) -> VoiceLeadingAnalysis:
    """Perform complete voice leading analysis on a progression"""
    all_connections = []
    smooth = 0
    rough = 0

    for i in range(len(chords) - 1):
        connections = analyze_voice_leading(chords[i], chords[i + 1])
        all_connections.append(connections)

        cost = voice_leading_cost(chords[i], chords[i + 1])
        if cost <= 2:
            smooth += 1
        elif cost > 4:
            rough += 1

    total = progression_voice_leading_cost(chords)
    avg = average_voice_leading_cost(chords)

    return VoiceLeadingAnalysis(
        chords=chords,
        connections=all_connections,
        total_cost=total,
        average_cost=avg,
        smooth_transitions=smooth,
        rough_transitions=rough
    )


def optimize_progression(
    chords: List[ChordSymbol],
    max_iterations: int = 100
) -> Tuple[List[ChordSymbol], float]:
    """Optimize a progression for better voice leading.

    Uses local search to find substitutions that improve voice leading.
    Returns the optimized progression and its cost.
    """
    from ..substitution.tritone import tritone_substitute, can_apply_tritone_sub

    current = chords.copy()
    current_cost = progression_voice_leading_cost(current)

    for _ in range(max_iterations):
        improved = False

        for i in range(len(current)):
            chord = current[i]

            # Try tritone substitution
            if can_apply_tritone_sub(chord):
                test = current.copy()
                test[i] = tritone_substitute(chord)
                new_cost = progression_voice_leading_cost(test)

                if new_cost < current_cost:
                    current = test
                    current_cost = new_cost
                    improved = True
                    break

        if not improved:
            break

    return current, current_cost


# Common voice leading patterns in jazz
SMOOTH_PROGRESSIONS = {
    ('ii7', 'V7'): 1,    # 3rd stays, 7th -> 3rd
    ('V7', 'Imaj7'): 1,  # 3rd -> 7th, 7th -> 3rd
    ('Imaj7', 'vi7'): 2, # Common tone 3rd, 7th moves
    ('vi7', 'ii7'): 2,   # Step motion
    ('V7', 'bII7'): 1,   # Tritone sub: 3rd/7th exchange
}


def is_smooth_progression(chord1: ChordSymbol, chord2: ChordSymbol) -> bool:
    """Check if a two-chord progression has smooth voice leading"""
    return voice_leading_cost(chord1, chord2) <= 2


def suggest_connecting_chord(
    chord1: ChordSymbol,
    chord2: ChordSymbol,
    key: PitchClass
) -> Optional[ChordSymbol]:
    """Suggest a chord to connect two chords with rough voice leading.

    Returns None if no improvement is possible or voice leading is already smooth.
    """
    direct_cost = voice_leading_cost(chord1, chord2)

    if direct_cost <= 2:
        return None  # Already smooth

    # Try various connecting chords
    candidates = []

    # Passing diminished
    dim_root = chord1.root.transpose(1)
    dim = ChordSymbol(dim_root, ChordQuality.DIM7)
    cost_via_dim = voice_leading_cost(chord1, dim) + voice_leading_cost(dim, chord2)
    if cost_via_dim < direct_cost:
        candidates.append((dim, cost_via_dim))

    # Secondary dominant
    v_of_2 = ChordSymbol(chord2.root.transpose(7), ChordQuality.DOM7)
    cost_via_v = voice_leading_cost(chord1, v_of_2) + voice_leading_cost(v_of_2, chord2)
    if cost_via_v < direct_cost:
        candidates.append((v_of_2, cost_via_v))

    if candidates:
        candidates.sort(key=lambda x: x[1])
        return candidates[0][0]

    return None
