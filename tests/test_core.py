"""Tests for core music theory classes"""

import sys
sys.path.insert(0, '/Users/madzine/Documents/JazzArchitect')

from src.core.pitch import PitchClass, C, D, E, F, G, A, B, Bb, Eb, Ab
from src.core.chord import ChordSymbol, ChordQuality, maj7, min7, dom7, hdim7


def test_pitch_class_basics():
    """Test PitchClass creation and properties"""
    c = PitchClass.from_name('C')
    assert c.value == 0

    fsharp = PitchClass.from_name('F#')
    assert fsharp.value == 6

    bb = PitchClass.from_name('Bb')
    assert bb.value == 10

    print("PitchClass basics: OK")


def test_pitch_class_intervals():
    """Test interval calculations"""
    c = PitchClass.from_name('C')
    g = PitchClass.from_name('G')

    # C to G = perfect 5th = 7 semitones
    assert c.interval_to(g) == 7

    # G to C = perfect 4th = 5 semitones
    assert g.interval_to(c) == 5

    # Tritone
    f = PitchClass.from_name('F')
    b = PitchClass.from_name('B')
    assert f.interval_to(b) == 6

    print("PitchClass intervals: OK")


def test_pitch_class_transpose():
    """Test transposition"""
    c = PitchClass.from_name('C')

    # Up a major 3rd
    e = c.transpose(4)
    assert e.value == 4

    # Up a tritone
    fsharp = c.transpose(6)
    assert fsharp.value == 6

    # Wrapping
    b = PitchClass.from_name('B')
    c_again = b.transpose(1)
    assert c_again.value == 0

    print("PitchClass transpose: OK")


def test_chord_symbol_creation():
    """Test ChordSymbol creation"""
    cmaj7 = maj7('C')
    assert cmaj7.root.value == 0
    assert cmaj7.quality == ChordQuality.MAJ7

    dm7 = min7('D')
    assert dm7.root.value == 2
    assert dm7.quality == ChordQuality.MIN7

    g7 = dom7('G')
    assert g7.root.value == 7
    assert g7.quality == ChordQuality.DOM7

    print("ChordSymbol creation: OK")


def test_chord_symbol_parsing():
    """Test parsing chord symbols from strings"""
    cmaj7 = ChordSymbol.from_string('Cmaj7')
    assert cmaj7.root.value == 0
    assert cmaj7.quality == ChordQuality.MAJ7

    dm7 = ChordSymbol.from_string('Dm7')
    assert dm7.root.value == 2
    assert dm7.quality == ChordQuality.MIN7

    g7 = ChordSymbol.from_string('G7')
    assert g7.root.value == 7
    assert g7.quality == ChordQuality.DOM7

    bm7b5 = ChordSymbol.from_string('Bm7b5')
    assert bm7b5.root.value == 11
    assert bm7b5.quality == ChordQuality.HDIM7

    bbmaj7 = ChordSymbol.from_string('Bbmaj7')
    assert bbmaj7.root.value == 10

    print("ChordSymbol parsing: OK")


def test_guide_tones():
    """Test guide tone extraction (3rd and 7th)"""
    # Cmaj7: third = E (4), seventh = B (11)
    cmaj7 = maj7('C')
    assert cmaj7.third.value == 4
    assert cmaj7.seventh.value == 11

    # Dm7: third = F (5), seventh = C (0)
    dm7 = min7('D')
    assert dm7.third.value == 5
    assert dm7.seventh.value == 0

    # G7: third = B (11), seventh = F (5)
    g7 = dom7('G')
    assert g7.third.value == 11
    assert g7.seventh.value == 5

    print("Guide tones: OK")


def test_tritone_substitution():
    """Test tritone substitution"""
    g7 = dom7('G')
    db7 = g7.tritone_substitute()

    # G7 -> Db7
    assert db7.root.value == 1
    assert db7.quality == ChordQuality.DOM7

    print("Tritone substitution: OK")


def test_transpose_chord():
    """Test chord transposition"""
    cmaj7 = maj7('C')
    dmaj7 = cmaj7.transpose(2)

    assert dmaj7.root.value == 2
    assert dmaj7.quality == ChordQuality.MAJ7

    print("Chord transpose: OK")


def test_roman_numeral():
    """Test Roman numeral representation"""
    c = PitchClass.from_name('C')

    cmaj7 = maj7('C')
    assert 'I' in cmaj7.as_roman_numeral(c)

    dm7 = min7('D')
    assert 'ii' in dm7.as_roman_numeral(c)

    g7 = dom7('G')
    assert 'V' in g7.as_roman_numeral(c)

    print("Roman numerals: OK")


def test_ii_v_i_progression():
    """Test classic ii-V-I progression in C"""
    dm7 = min7('D')
    g7 = dom7('G')
    cmaj7 = maj7('C')

    # Verify guide tone voice leading
    # Dm7 -> G7: 3rd (F) stays, 7th (C) -> B (down m2)
    assert dm7.third.value == 5  # F
    assert g7.seventh.value == 5  # F (3rd of Dm7 -> 7th of G7)

    # G7 -> Cmaj7: 3rd (B) -> B (7th of Cmaj7), 7th (F) -> E (down m2)
    assert g7.third.value == 11  # B
    assert cmaj7.seventh.value == 11  # B

    print("ii-V-I voice leading: OK")


if __name__ == '__main__':
    test_pitch_class_basics()
    test_pitch_class_intervals()
    test_pitch_class_transpose()
    test_chord_symbol_creation()
    test_chord_symbol_parsing()
    test_guide_tones()
    test_tritone_substitution()
    test_transpose_chord()
    test_roman_numeral()
    test_ii_v_i_progression()

    print("\n=== All tests passed ===")
