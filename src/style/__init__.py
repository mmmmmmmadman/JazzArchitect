"""Style Parameter Control"""

from .vectors import StyleVector
from .presets import (
    BEBOP, COOL, MODAL, POSTBOP, SWING, HARDBOP, FUSION, CONTEMPORARY, BLUES,
    get_style, list_styles
)
from .engine import StyleEngine, generate_styled_progression
