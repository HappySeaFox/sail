"""
sailpy command-line interface.

Usage:
    python -m sailpy --help
    python -m sailpy --version
    python -m sailpy --readme

Examples:
    python -m sailpy.examples.01_quickstart
    python -m sailpy.examples.12_image_viewer
"""

import sys
from importlib.metadata import metadata, PackageNotFoundError


def show_help():
    """Show help message."""
    print(__doc__)


def show_version():
    """Show version."""
    try:
        from sailpy import __version__
        print(f"sailpy {__version__}")
    except ImportError:
        print("sailpy version unknown")


def show_readme():
    """Show README with examples."""
    try:
        m = metadata('sailpy')
        description = m.get('Description')
        if description:
            print(description)
        else:
            print("No README found in package metadata.")
    except PackageNotFoundError:
        print("sailpy package not found. Please install it first.")


def main():
    """Main entry point."""
    if len(sys.argv) < 2:
        show_help()
        return

    arg = sys.argv[1]

    if arg in ('--help', '-h'):
        show_help()
    elif arg in ('--version', '-v'):
        show_version()
    elif arg in ('--readme', '-r', '--doc'):
        show_readme()
    else:
        print(f"Unknown option: {arg}")
        show_help()
        sys.exit(1)


if __name__ == '__main__':
    main()

