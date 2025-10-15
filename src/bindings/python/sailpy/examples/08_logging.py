#  This file is part of SAIL (https://github.com/HappySeaFox/sail)
#
#  Copyright (c) 2025 Dmitry Baryshev
#
#  The MIT License

"""
SAIL Python Bindings - Logging Examples

Custom logging, log level control, and debugging.
"""

import os
import sailpy


def main():
    """Logging examples"""
    print("\nSAIL Logging Examples")
    print("=" * 70 + "\n")

    # Example 1: Log level control
    print("=" * 70)
    print("EXAMPLE 1: Log Level Control")
    print("=" * 70 + "\n")

    print("Available log levels:")
    for level in [sailpy.LogLevel.SILENCE, sailpy.LogLevel.ERROR,
                  sailpy.LogLevel.WARNING, sailpy.LogLevel.INFO,
                  sailpy.LogLevel.DEBUG]:
        print(f"  • {level.name}")

    print("\nSetting to SILENCE (no logs)...")
    sailpy.set_log_barrier(sailpy.LogLevel.SILENCE)

    # No logs will be shown
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 16, 16)

    print("✓ Silent mode active (no logs printed)")

    # Restore
    sailpy.set_log_barrier(sailpy.LogLevel.ERROR)

    print()

    # Example 2: Custom logger
    print("=" * 70)
    print("EXAMPLE 2: Custom Logger")
    print("=" * 70 + "\n")

    log_messages = []

    def custom_logger(level, file, line, message):
        level_names = {
            1: "ERROR", 2: "WARN", 3: "INFO",
            4: "MSG", 5: "DEBUG", 6: "TRACE"
        }
        level_str = level_names.get(level, "?")
        filename = os.path.basename(file)
        log_messages.append(f"[{level_str}] {filename}:{line} - {message}")
        return True

    print("Installing custom logger...")
    sailpy.set_logger(custom_logger)
    sailpy.set_log_barrier(sailpy.LogLevel.DEBUG)

    # Trigger some operations
    codec = sailpy.CodecInfo.from_name("JPEG")
    img = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 32, 32)

    print(f"✓ Captured {len(log_messages)} log messages")
    if log_messages:
        print(f"  First message: {log_messages[0][:60]}...")
        if len(log_messages) > 1:
            print(f"  Last message:  {log_messages[-1][:60]}...")

    # Restore default logger
    sailpy.set_logger(None)
    sailpy.set_log_barrier(sailpy.LogLevel.ERROR)

    print()

    # Example 3: Filtering by level
    print("=" * 70)
    print("EXAMPLE 3: Filtering by Log Level")
    print("=" * 70 + "\n")

    levels_to_test = [
        (sailpy.LogLevel.ERROR, "ERROR only"),
        (sailpy.LogLevel.WARNING, "WARNING and above"),
        (sailpy.LogLevel.DEBUG, "DEBUG and above (verbose)")
    ]

    for level, description in levels_to_test:
        messages = []

        def logger(lvl, file, line, msg):
            messages.append(msg)
            return True  # Consume the message

        sailpy.set_logger(logger)
        sailpy.set_log_barrier(level)

        # Trigger operations
        _ = sailpy.Image(sailpy.PixelFormat.BPP24_RGB, 8, 8)

        print(f"{description}: {len(messages)} messages")

        sailpy.set_logger(None)

    print("\n✓ Use set_log_barrier() to control verbosity")

    print()

    print("=" * 70)
    print("✓ All examples completed!")
    print()


if __name__ == "__main__":
    main()
