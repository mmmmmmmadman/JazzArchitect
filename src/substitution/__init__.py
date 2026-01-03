"""Chord Substitution Rules Module"""

from .tritone import TritoneSubstitution, tritone_substitute, can_apply_tritone_sub
from .backdoor import BackdoorSubstitution, backdoor_dominant, backdoor_ii_v
from .coltrane import ColtraneSubstitution, coltrane_substitution, coltrane_over_ii_v_i
