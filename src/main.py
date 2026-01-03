"""Jazz Harmony Generator - Main Entry Point"""

from __future__ import annotations
import argparse
import json
import sys
from typing import Optional

from .core.pitch import PitchClass
from .core.chord import ChordSymbol
from .style import StyleEngine, get_style, list_styles
from .grammar.generator import format_progression
from .evaluation import evaluate_progression, EvaluationResult
from .voiceleading import analyze_progression


def generate_command(args):
    """Generate a chord progression"""
    engine = StyleEngine(get_style(args.style))
    result = engine.generate(
        length=args.length,
        key=args.key,
        seed=args.seed
    )

    print(f"Style: {args.style}")
    print(f"Key: {args.key}")
    print(f"Length: {args.length} chords")
    if result.substitutions_applied:
        print(f"Substitutions: {', '.join(result.substitutions_applied)}")
    print()
    print(format_progression(result.chords, bars_per_line=4))

    if args.evaluate:
        print()
        eval_result = evaluate_progression(result.chords, result.key)
        print_evaluation_summary(eval_result)

    if args.output:
        output_data = {
            'style': args.style,
            'key': args.key,
            'chords': [str(c) for c in result.chords],
            'substitutions': result.substitutions_applied,
        }
        with open(args.output, 'w') as f:
            json.dump(output_data, f, indent=2)
        print(f"\nSaved to {args.output}")


def evaluate_command(args):
    """Evaluate a chord progression from input"""
    if args.input:
        with open(args.input, 'r') as f:
            data = json.load(f)
        chord_strings = data.get('chords', [])
        key_str = data.get('key', 'C')
    else:
        # Read from stdin or argument
        chord_strings = args.chords.split(',')
        key_str = args.key

    chords = [ChordSymbol.from_string(c.strip()) for c in chord_strings]
    key = PitchClass.from_name(key_str)

    print(f"Evaluating: {' | '.join(str(c) for c in chords)}")
    print()

    result = evaluate_progression(chords, key)
    print_evaluation_summary(result)

    if args.verbose:
        print("\n--- Voice Leading Analysis ---")
        vl = analyze_progression(chords)
        print(f"Total VL Cost: {vl.total_cost:.2f}")
        print(f"Smooth transitions: {vl.smooth_transitions}")
        print(f"Rough transitions: {vl.rough_transitions}")


def blues_command(args):
    """Generate blues changes"""
    engine = StyleEngine(get_style(args.style))
    chords = engine.generate_blues_changes(key=args.key, bars=args.bars)

    print(f"Blues in {args.key} ({args.bars} bars, {args.style} style)")
    print()
    print(format_progression(chords, bars_per_line=4))


def turnaround_command(args):
    """Generate a turnaround"""
    engine = StyleEngine(get_style(args.style))
    chords = engine.generate_turnaround(key=args.key)

    print(f"Turnaround in {args.key} ({args.style} style)")
    print()
    print(" | ".join(str(c) for c in chords))


def styles_command(args):
    """List available styles"""
    print("Available styles:")
    for name in list_styles():
        style = get_style(name)
        print(f"  {name:15} - {style.describe()}")


def print_evaluation_summary(result: EvaluationResult):
    """Print a summary of evaluation results"""
    print("=== Evaluation Summary ===")
    print(f"Variety Score:      {result.variety_score:.3f}")
    print(f"Coherence Score:    {result.coherence_score:.3f}")
    print(f"Voice Leading Cost: {result.voice_leading_cost:.3f}")
    print(f"Smooth Ratio:       {result.smooth_ratio:.1%}")
    print(f"---")
    print(f"Overall Score:      {result.overall_score():.3f}")


def main():
    parser = argparse.ArgumentParser(
        description="Jazz Harmony Generator - Rule-based chord progression generation"
    )
    subparsers = parser.add_subparsers(dest='command', help='Commands')

    # Generate command
    gen_parser = subparsers.add_parser('generate', help='Generate a chord progression')
    gen_parser.add_argument('-s', '--style', default='bebop', help='Style preset')
    gen_parser.add_argument('-k', '--key', default='C', help='Key (e.g., C, F#, Bb)')
    gen_parser.add_argument('-l', '--length', type=int, default=8, help='Number of chords')
    gen_parser.add_argument('--seed', type=int, help='Random seed')
    gen_parser.add_argument('-e', '--evaluate', action='store_true', help='Show evaluation')
    gen_parser.add_argument('-o', '--output', help='Output JSON file')

    # Evaluate command
    eval_parser = subparsers.add_parser('evaluate', help='Evaluate a progression')
    eval_parser.add_argument('-i', '--input', help='Input JSON file')
    eval_parser.add_argument('-c', '--chords', help='Comma-separated chords')
    eval_parser.add_argument('-k', '--key', default='C', help='Key')
    eval_parser.add_argument('-v', '--verbose', action='store_true')

    # Blues command
    blues_parser = subparsers.add_parser('blues', help='Generate blues changes')
    blues_parser.add_argument('-k', '--key', default='C', help='Key')
    blues_parser.add_argument('-b', '--bars', type=int, default=12, help='Number of bars')
    blues_parser.add_argument('-s', '--style', default='bebop', help='Style')

    # Turnaround command
    turn_parser = subparsers.add_parser('turnaround', help='Generate a turnaround')
    turn_parser.add_argument('-k', '--key', default='C', help='Key')
    turn_parser.add_argument('-s', '--style', default='bebop', help='Style')

    # Styles command
    subparsers.add_parser('styles', help='List available styles')

    args = parser.parse_args()

    if args.command == 'generate':
        generate_command(args)
    elif args.command == 'evaluate':
        evaluate_command(args)
    elif args.command == 'blues':
        blues_command(args)
    elif args.command == 'turnaround':
        turnaround_command(args)
    elif args.command == 'styles':
        styles_command(args)
    else:
        parser.print_help()


if __name__ == '__main__':
    main()
