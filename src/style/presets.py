"""Predefined Style Presets for Different Jazz Eras"""

from .vectors import StyleVector


# =============================================================================
# Classic Era Styles
# =============================================================================

SWING = StyleVector(
    tritone_sub_prob=0.1,
    backdoor_prob=0.1,
    coltrane_prob=0.0,
    ii_v_preference=0.7,
    secondary_dom_prob=0.2,
    modal_interchange=0.1,
    minor_iv_prob=0.1,
    chromatic_approach=0.1,
    diminished_approach=0.15,
    dominant_chain_depth=2,
    prolongation_depth=1,
    rhythm_density=0.4,
    turnaround_prob=0.5,
    extension_level=0.3,
    alteration_prob=0.1,
)

# =============================================================================
# Bebop Era (1940s-1950s)
# =============================================================================

BEBOP = StyleVector(
    tritone_sub_prob=0.3,
    backdoor_prob=0.15,
    coltrane_prob=0.05,
    ii_v_preference=0.9,
    secondary_dom_prob=0.4,
    modal_interchange=0.2,
    minor_iv_prob=0.15,
    chromatic_approach=0.4,
    diminished_approach=0.2,
    dominant_chain_depth=4,
    prolongation_depth=2,
    rhythm_density=0.8,
    turnaround_prob=0.6,
    extension_level=0.5,
    alteration_prob=0.3,
)

# =============================================================================
# Cool Jazz (1950s)
# =============================================================================

COOL = StyleVector(
    tritone_sub_prob=0.2,
    backdoor_prob=0.1,
    coltrane_prob=0.0,
    ii_v_preference=0.7,
    secondary_dom_prob=0.25,
    modal_interchange=0.3,
    minor_iv_prob=0.2,
    chromatic_approach=0.2,
    diminished_approach=0.1,
    dominant_chain_depth=3,
    prolongation_depth=2,
    rhythm_density=0.5,
    turnaround_prob=0.4,
    extension_level=0.4,
    alteration_prob=0.15,
)

# =============================================================================
# Hard Bop (1950s-1960s)
# =============================================================================

HARDBOP = StyleVector(
    tritone_sub_prob=0.25,
    backdoor_prob=0.2,
    coltrane_prob=0.1,
    ii_v_preference=0.85,
    secondary_dom_prob=0.35,
    modal_interchange=0.25,
    minor_iv_prob=0.2,
    chromatic_approach=0.35,
    diminished_approach=0.15,
    dominant_chain_depth=4,
    prolongation_depth=2,
    rhythm_density=0.7,
    turnaround_prob=0.5,
    extension_level=0.55,
    alteration_prob=0.25,
)

# =============================================================================
# Modal Jazz (1960s)
# =============================================================================

MODAL = StyleVector(
    tritone_sub_prob=0.1,
    backdoor_prob=0.05,
    coltrane_prob=0.15,
    ii_v_preference=0.3,
    secondary_dom_prob=0.1,
    modal_interchange=0.6,
    minor_iv_prob=0.3,
    chromatic_approach=0.1,
    diminished_approach=0.05,
    dominant_chain_depth=2,
    prolongation_depth=3,
    rhythm_density=0.3,
    turnaround_prob=0.2,
    extension_level=0.6,
    alteration_prob=0.1,
)

# =============================================================================
# Post-Bop (1960s-1970s)
# =============================================================================

POSTBOP = StyleVector(
    tritone_sub_prob=0.4,
    backdoor_prob=0.25,
    coltrane_prob=0.25,
    ii_v_preference=0.6,
    secondary_dom_prob=0.45,
    modal_interchange=0.5,
    minor_iv_prob=0.3,
    chromatic_approach=0.5,
    diminished_approach=0.2,
    dominant_chain_depth=5,
    prolongation_depth=3,
    rhythm_density=0.6,
    turnaround_prob=0.4,
    extension_level=0.7,
    alteration_prob=0.4,
)

# =============================================================================
# Fusion (1970s)
# =============================================================================

FUSION = StyleVector(
    tritone_sub_prob=0.35,
    backdoor_prob=0.2,
    coltrane_prob=0.1,
    ii_v_preference=0.5,
    secondary_dom_prob=0.3,
    modal_interchange=0.6,
    minor_iv_prob=0.35,
    chromatic_approach=0.4,
    diminished_approach=0.15,
    dominant_chain_depth=3,
    prolongation_depth=2,
    rhythm_density=0.65,
    turnaround_prob=0.3,
    extension_level=0.75,
    alteration_prob=0.35,
)

# =============================================================================
# Contemporary/Modern (1980s-present)
# =============================================================================

CONTEMPORARY = StyleVector(
    tritone_sub_prob=0.35,
    backdoor_prob=0.2,
    coltrane_prob=0.15,
    ii_v_preference=0.65,
    secondary_dom_prob=0.35,
    modal_interchange=0.45,
    minor_iv_prob=0.25,
    chromatic_approach=0.35,
    diminished_approach=0.15,
    dominant_chain_depth=4,
    prolongation_depth=2,
    rhythm_density=0.55,
    turnaround_prob=0.35,
    extension_level=0.65,
    alteration_prob=0.3,
)

# =============================================================================
# Blues-influenced
# =============================================================================

BLUES = StyleVector(
    tritone_sub_prob=0.15,
    backdoor_prob=0.25,
    coltrane_prob=0.0,
    ii_v_preference=0.5,
    secondary_dom_prob=0.2,
    modal_interchange=0.4,
    minor_iv_prob=0.4,
    chromatic_approach=0.2,
    diminished_approach=0.1,
    dominant_chain_depth=2,
    prolongation_depth=1,
    rhythm_density=0.4,
    turnaround_prob=0.6,
    extension_level=0.4,
    alteration_prob=0.2,
)

# =============================================================================
# Style Registry
# =============================================================================

STYLE_PRESETS = {
    'swing': SWING,
    'bebop': BEBOP,
    'cool': COOL,
    'hardbop': HARDBOP,
    'modal': MODAL,
    'postbop': POSTBOP,
    'fusion': FUSION,
    'contemporary': CONTEMPORARY,
    'blues': BLUES,
}


def get_style(name: str) -> StyleVector:
    """Get a style preset by name.

    Args:
        name: Style name (case-insensitive)

    Returns:
        StyleVector for the requested style

    Raises:
        ValueError: If style name is not recognized
    """
    name_lower = name.lower().replace('-', '').replace('_', '').replace(' ', '')

    # Handle aliases
    aliases = {
        'bop': 'bebop',
        'hard': 'hardbop',
        'post': 'postbop',
        'modern': 'contemporary',
        'coltrane': 'postbop',
    }

    if name_lower in aliases:
        name_lower = aliases[name_lower]

    if name_lower not in STYLE_PRESETS:
        available = ', '.join(STYLE_PRESETS.keys())
        raise ValueError(f"Unknown style '{name}'. Available: {available}")

    return STYLE_PRESETS[name_lower].copy()


def list_styles() -> list:
    """List all available style presets"""
    return list(STYLE_PRESETS.keys())
