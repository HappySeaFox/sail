#  This file is part of SAIL (https://github.com/HappySeaFox/sail)
#
#  Copyright (c) 2025 Dmitry Baryshev
#
#  The MIT License

"""
SAIL Python Bindings - Simple Image Viewer with Qt

A compact image viewer demonstrating SAIL integration with Qt/PySide6.
Supports single and multi-frame images with animation playback.

Requirements:
    pip install PySide6

Keyboard Shortcuts:
    Ctrl+O          - Open image
    Ctrl+S          - Save image
    Ctrl+Q          - Quit
    +               - Zoom in
    =               - Reset zoom (100%)
    -               - Zoom out
    Left/Right      - Previous/Next frame
    PageUp/PageDown - Previous/Next frame
"""

import sys
from pathlib import Path

from PySide6.QtWidgets import (
    QApplication, QMainWindow, QLabel, QFileDialog, QMessageBox, QScrollArea
)
from PySide6.QtGui import QPixmap, QImage, QShortcut, QKeySequence
from PySide6.QtCore import Qt, QTimer

import sailpy


class ImageViewer(QMainWindow):
    """Simple Qt-based image viewer using SAIL."""

    def __init__(self):
        super().__init__()
        self.setWindowTitle("SAIL Image Viewer")
        self.resize(1024, 768)

        # Image state
        self.frames = []
        self.current_frame = 0
        self.current_path = None
        self.zoom_level = 1.0
        self.last_directory = str(Path.home())

        # Create central widget
        self.image_label = QLabel()
        self.image_label.setAlignment(Qt.AlignCenter)
        self.image_label.setStyleSheet(
            "QLabel { background-color: #2a2a2a; color: white; }")
        self.image_label.setText("Press Ctrl+O to open an image")
        self.image_label.setMinimumSize(400, 300)

        # Scroll area
        self.scroll_area = QScrollArea()
        self.scroll_area.setWidget(self.image_label)
        self.scroll_area.setWidgetResizable(True)
        self.scroll_area.setStyleSheet(
            "QScrollArea { background-color: #2a2a2a; }")
        self.scroll_area.setAlignment(Qt.AlignCenter)
        self.setCentralWidget(self.scroll_area)

        # Info overlay label (positioned over image)
        self.info_label = QLabel(self.scroll_area)
        self.info_label.setStyleSheet("""
            QLabel {
                background-color: rgba(0, 0, 0, 128);
                color: white;
                padding: 8px 12px;
                border-radius: 4px;
                font-family: monospace;
            }
        """)
        self.info_label.setText("No image loaded")
        self.info_label.adjustSize()
        self.update_info_position()

        # Animation timer
        self.animation_timer = QTimer()
        self.animation_timer.timeout.connect(self.next_frame)

        self._create_toolbar()
        self._create_shortcuts()

    def _create_toolbar(self):
        """Create toolbar."""
        toolbar = self.addToolBar("Main")

        toolbar.addAction("Open...", self.open_image)
        toolbar.addAction("Save..", self.save_image)
        toolbar.addSeparator()
        toolbar.addAction("Zoom+", lambda: self.change_zoom(1.25))
        toolbar.addAction("Zoom-", lambda: self.change_zoom(0.75))
        toolbar.addSeparator()
        toolbar.addAction("◀ Prev", self.prev_frame)
        toolbar.addAction("Next ▶", self.next_frame)

    def _create_shortcuts(self):
        """Create application-wide keyboard shortcuts."""
        # File operations
        QShortcut(QKeySequence("Ctrl+O"), self).activated.connect(self.open_image)
        QShortcut(QKeySequence("Ctrl+S"), self).activated.connect(self.save_image)
        QShortcut(QKeySequence("Ctrl+Q"), self).activated.connect(self.close)

        # Zoom
        QShortcut(QKeySequence("+"), self).activated.connect(lambda: self.change_zoom(1.25))
        QShortcut(QKeySequence("="), self).activated.connect(lambda: self.change_zoom(1.0))
        QShortcut(QKeySequence("-"), self).activated.connect(lambda: self.change_zoom(0.75))

        # Frame navigation
        QShortcut(QKeySequence("Left"), self).activated.connect(self.prev_frame)
        QShortcut(QKeySequence("Right"), self).activated.connect(self.next_frame)
        QShortcut(QKeySequence("PageUp"), self).activated.connect(self.prev_frame)
        QShortcut(QKeySequence("PageDown"), self).activated.connect(self.next_frame)


    def update_info_position(self):
        """Update info label position (bottom-left)."""
        margin = 10
        x = margin
        y = self.scroll_area.height() - self.info_label.height() - margin
        self.info_label.move(x, y)
        self.info_label.raise_()  # Bring to front

    def update_info(self):
        """Update info display."""
        if not self.frames:
            self.info_label.setText("No image loaded")
            self.info_label.adjustSize()
            self.update_info_position()
            return

        frame = self.frames[self.current_frame]
        info_parts = []

        if self.current_path:
            info_parts.append(f"File: {Path(self.current_path).name}")

        info_parts.append(f"Size: {frame.width}x{frame.height}")
        info_parts.append(f"Format: {frame.pixel_format.name}")
        info_parts.append(
            f"Frame: {self.current_frame + 1}/{len(self.frames)}")
        info_parts.append(f"Zoom: {self.zoom_level:.0%}")

        self.info_label.setText("\n".join(info_parts))
        self.info_label.adjustSize()
        self.update_info_position()

    def resizeEvent(self, event):
        """Handle window resize."""
        super().resizeEvent(event)
        self.update_info_position()

    def open_image(self):
        """Open image using Qt file dialog."""
        # Build filters: All files first, then each codec
        filters = ["All Files (*.*)"]

        for codec in sailpy.CodecInfo.list():
            ext_patterns = [f"*.{ext}" for ext in codec.extensions]
            filter_name = f"{codec.description} ({' '.join(ext_patterns)})"
            filters.append(filter_name)

        filter_str = ";;".join(filters)

        path, _ = QFileDialog.getOpenFileName(
            self, "Open Image", self.last_directory, filter_str
        )

        if not path:
            return

        try:
            codec = sailpy.CodecInfo.from_path(path)
            if not codec.can_load:
                QMessageBox.critical(self, "Error", "SAIL doesn't support loading the selected image format")
                return
        except ValueError:
            QMessageBox.critical(self, "Error", "SAIL doesn't support loading the selected image format")
            return

        self.last_directory = str(Path(path).parent)

        try:
            reader = sailpy.ImageReader(path)
            self.frames = reader.read_all()
            self.current_frame = 0
            self.current_path = path
            self.zoom_level = 1.0

            if self.frames:
                frame = self.frames[0]

                # Setup animation if multi-frame with positive delay
                if len(self.frames) > 1 and frame.delay > 0:
                    self.animation_timer.start(frame.delay)
                else:
                    self.animation_timer.stop()

                self.fit_to_window()
                self.display_current_frame()
            else:
                QMessageBox.critical(self, "Error", "Failed to load the image")

        except FileNotFoundError:
            QMessageBox.critical(self, "Error", f"File not found: {path}")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to load image: {e}")

    def save_image(self):
        """Save current frame."""
        if not self.frames:
            QMessageBox.warning(self, "No Image", "No image loaded to save")
            return

        # Build file types: All files first, then codecs with save support
        filetypes = []

        for codec in sailpy.CodecInfo.list():
            # Check if codec supports saving
            if codec.save_features and len(codec.save_features.pixel_formats) > 0:
                # Get extensions for this codec
                ext_patterns = [f"*.{ext}" for ext in codec.extensions]
                filter_name = f"{codec.description} ({' '.join(ext_patterns)})"
                filetypes.append(filter_name)

        filetypes.sort()
        filetypes.insert(0, "All Files (*.*)")

        # Default filename with last directory
        default_name = "saved_image.png"
        if self.current_path:
            default_name = Path(self.current_path).name

        default_path = str(Path(self.last_directory) / default_name)

        path, _ = QFileDialog.getSaveFileName(
            self, "Save Image", default_path, ";;".join(filetypes)
        )

        if not path:
            return

        self.last_directory = str(Path(path).parent)

        try:
            # Get codec from file extension
            codec = sailpy.CodecInfo.from_path(path)
            if not codec.can_save:
                QMessageBox.critical(self, "Error", "SAIL doesn't support saving to the selected image format")
                return

            # Create a copy of the frame to convert
            frame = self.frames[self.current_frame]
            save_frame = sailpy.Image(
                frame.pixel_format, frame.width, frame.height)
            save_frame.to_numpy()[:] = frame.to_numpy()

            # Copy metadata
            save_frame.delay = frame.delay
            save_frame.resolution = frame.resolution

            # Convert to format supported by codec
            save_frame.convert(codec.save_features)

            # Save converted frame
            save_frame.save(path)
        except ValueError as e:
            QMessageBox.warning(self, "Unsupported Format", f"Unsupported format: {e}")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to save: {e}")

    def display_current_frame(self):
        """Display current frame."""
        if not self.frames:
            return

        frame = self.frames[self.current_frame]

        # Convert to RGBA for Qt (use convert_to to create new image)
        try:
            if frame.pixel_format != sailpy.PixelFormat.BPP32_RGBA:
                frame.convert(sailpy.PixelFormat.BPP32_RGBA)
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to convert {frame.pixel_format.name} to RGBA:\n{e}")
            return

        # Create QImage from numpy array
        arr = frame.to_numpy()
        height, width, channels = arr.shape

        qimage = QImage(
            arr.data, width, height, width * 4, QImage.Format_RGBA8888
        )

        # Create pixmap and apply zoom
        pixmap = QPixmap.fromImage(qimage)
        if self.zoom_level != 1.0:
            new_size = pixmap.size() * self.zoom_level
            pixmap = pixmap.scaled(
                new_size, Qt.KeepAspectRatio, Qt.SmoothTransformation
            )

        # Disable auto-resize when showing image
        self.scroll_area.setWidgetResizable(False)
        self.image_label.setPixmap(pixmap)
        self.image_label.resize(pixmap.size())

        # Update window title
        if self.current_path:
            self.setWindowTitle(
                f"SAIL Image Viewer - {Path(self.current_path).name}")
        else:
            self.setWindowTitle("SAIL Image Viewer")

        # Update info overlay
        self.update_info()

    def change_zoom(self, factor):
        """Change zoom level."""
        self.zoom_level = max(0.1, min(10.0, self.zoom_level * factor))
        self.display_current_frame()

    def set_zoom(self, level):
        """Set zoom level."""
        self.zoom_level = level
        self.display_current_frame()

    def fit_to_window(self):
        """Adjust zoom to fit image in window."""
        if not self.frames:
            return

        frame = self.frames[self.current_frame]
        viewport_size = self.scroll_area.viewport().size()

        # Calculate zoom to fit both width and height
        zoom_w = viewport_size.width() / frame.width
        zoom_h = viewport_size.height() / frame.height
        zoom = min(zoom_w, zoom_h)

        if zoom < 1.0:
            self.zoom_level = max(0.01, zoom)

    def next_frame(self):
        """Show next frame."""
        if len(self.frames) > 1:
            self.current_frame = (self.current_frame + 1) % len(self.frames)
            self.display_current_frame()
            # Update timer interval for new frame's delay
            frame = self.frames[self.current_frame]
            if frame.delay > 0:
                self.animation_timer.setInterval(frame.delay)

    def prev_frame(self):
        """Show previous frame."""
        if len(self.frames) > 1:
            self.current_frame = (self.current_frame - 1) % len(self.frames)
            self.display_current_frame()
            # Update timer interval for new frame's delay
            frame = self.frames[self.current_frame]
            if frame.delay > 0:
                self.animation_timer.setInterval(frame.delay)


def main():
    """Entry point."""
    sailpy.set_log_barrier(sailpy.LogLevel.DEBUG)

    app = QApplication(sys.argv)
    viewer = ImageViewer()
    viewer.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
