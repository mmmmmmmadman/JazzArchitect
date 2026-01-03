"""Integration tests for Jazz Harmony Generator"""

import sys
sys.path.insert(0, '/Users/madzine/Documents/JazzArchitect')

from src.core.pitch import PitchClass
from src.core.chord import ChordSymbol, ChordQuality, min7, dom7, maj7
from src.style import StyleEngine, get_style, list_styles, BEBOP, MODAL
from src.grammar.generator import generate_progression, format_progression
from src.substitution import tritone_substitute, coltrane_substitution
from src.voiceleading import voice_leading_cost, analyze_progression
from src.evaluation import evaluate_progression


def test_style_generation():
    """Test generation with different styles"""
    print("Testing style generation...")

    for style_name in ['bebop', 'modal', 'cool', 'postbop']:
        engine = StyleEngine(get_style(style_name))
        result = engine.generate(length=8, key='C', seed=42)

        assert len(result.chords) == 8
        assert result.style is not None
        print(f"  {style_name}: {' | '.join(str(c) for c in result.chords[:4])}...")

    print("  OK")


def test_substitutions():
    """Test chord substitutions"""
    print("Testing substitutions...")

    # Tritone sub
    g7 = dom7('G')
    db7 = tritone_substitute(g7)
    assert db7.root.value == 1  # Db
    assert db7.quality == ChordQuality.DOM7
    print(f"  Tritone: {g7} -> {db7}")

    # Coltrane changes
    cmaj7 = maj7('C')
    coltrane = coltrane_substitution(cmaj7)
    assert len(coltrane) == 7
    print(f"  Coltrane: {' -> '.join(str(c) for c in coltrane)}")

    print("  OK")


def test_voice_leading():
    """Test voice leading analysis"""
    print("Testing voice leading...")

    # ii-V-I should have smooth voice leading
    dm7 = min7('D')
    g7 = dom7('G')
    cmaj7 = maj7('C')

    cost_ii_v = voice_leading_cost(dm7, g7)
    cost_v_i = voice_leading_cost(g7, cmaj7)

    assert cost_ii_v <= 2, f"ii-V cost too high: {cost_ii_v}"
    assert cost_v_i <= 2, f"V-I cost too high: {cost_v_i}"
    print(f"  ii-V cost: {cost_ii_v}, V-I cost: {cost_v_i}")

    # Analyze full progression
    analysis = analyze_progression([dm7, g7, cmaj7])
    assert analysis.smooth_transitions >= 2
    print(f"  Smooth transitions: {analysis.smooth_transitions}/{len([dm7, g7, cmaj7])-1}")

    print("  OK")


def test_evaluation():
    """Test evaluation metrics"""
    print("Testing evaluation...")

    # Generate and evaluate
    chords = generate_progression(length=16, key='C', seed=123)
    result = evaluate_progression(chords)

    assert 0 <= result.variety_score <= 1
    assert 0 <= result.coherence_score <= 1
    assert result.pitch_class_entropy >= 0

    print(f"  Variety: {result.variety_score:.3f}")
    print(f"  Coherence: {result.coherence_score:.3f}")
    print(f"  Overall: {result.overall_score():.3f}")

    print("  OK")


def test_blues_generation():
    """Test blues changes generation"""
    print("Testing blues generation...")

    engine = StyleEngine(BEBOP)
    blues = engine.generate_blues_changes(key='F', bars=12)

    assert len(blues) == 12
    print(f"  Blues in F: {format_progression(blues, bars_per_line=4)}")

    print("  OK")


def test_turnaround():
    """Test turnaround generation"""
    print("Testing turnaround...")

    engine = StyleEngine(BEBOP)
    turnaround = engine.generate_turnaround(key='C')

    assert len(turnaround) >= 4
    print(f"  Turnaround: {' | '.join(str(c) for c in turnaround)}")

    print("  OK")


def test_style_blending():
    """Test style vector blending"""
    print("Testing style blending...")

    bebop = get_style('bebop')
    modal = get_style('modal')

    blended = bebop.blend(modal, 0.5)

    # Blended values should be between the two
    assert bebop.tritone_sub_prob >= blended.tritone_sub_prob >= modal.tritone_sub_prob or \
           modal.tritone_sub_prob >= blended.tritone_sub_prob >= bebop.tritone_sub_prob

    print(f"  Bebop tritone prob: {bebop.tritone_sub_prob}")
    print(f"  Modal tritone prob: {modal.tritone_sub_prob}")
    print(f"  Blended tritone prob: {blended.tritone_sub_prob}")

    print("  OK")


def test_available_styles():
    """Test listing available styles"""
    print("Testing style listing...")

    styles = list_styles()
    assert 'bebop' in styles
    assert 'modal' in styles
    assert 'cool' in styles
    assert len(styles) >= 5

    print(f"  Available: {', '.join(styles)}")
    print("  OK")


if __name__ == '__main__':
    print("=" * 50)
    print("Jazz Harmony Generator - Integration Tests")
    print("=" * 50)
    print()

    test_style_generation()
    test_substitutions()
    test_voice_leading()
    test_evaluation()
    test_blues_generation()
    test_turnaround()
    test_style_blending()
    test_available_styles()

    print()
    print("=" * 50)
    print("All integration tests passed!")
    print("=" * 50)
