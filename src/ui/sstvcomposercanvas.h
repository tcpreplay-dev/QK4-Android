#ifndef SSTVCOMPOSERCANVAS_H
#define SSTVCOMPOSERCANVAS_H

#include <QColor>
#include <QFont>
#include <QImage>
#include <QJsonObject>
#include <QPointF>
#include <QVector>
#include <QWidget>

// Touch-first, non-destructive SSTV composition canvas. Coordinates are kept
// in the selected mode's native pixel space so the preview and TX frame match.
class SstvComposerCanvas : public QWidget {
    Q_OBJECT
public:
    enum class Tool { Select, Draw, Shape };
    enum class ShapeType { Line, Arrow, Rectangle, Ellipse };

    explicit SstvComposerCanvas(QWidget *parent = nullptr);

    void setBackground(const QImage &image);
    QImage renderedImage() const;
    bool hasBackground() const { return !m_background.isNull(); }

    void setTool(Tool tool);
    Tool tool() const { return m_tool; }
    void setInk(const QColor &color, int width);
    void setShapeType(ShapeType type) { m_shapeType = type; }
    void addTextBlock(const QString &text, const QFont &font, const QColor &color,
                      const QPointF &normalizedPosition = QPointF(0.5, 0.5));
    void updateSelectedText(const QString &text, const QFont &font, const QColor &color);
    void updateSelectedTextColor(const QColor &color);
    void updateSelectedTextFont(const QFont &font);
    bool hasSelectedText() const;
    QString selectedText() const;
    QFont selectedTextFont() const;
    QColor selectedTextColor() const;
    void deleteSelectedText();
    QJsonObject compositionState() const;
    bool restoreCompositionState(const QJsonObject &state);
    void undo();
    void redo();
    void resetComposition();
    // A newly selected source image starts a new editing session. Unlike the
    // user-facing reset command, old markup must not remain in undo history.
    void clearCompositionForNewImage();
    bool canUndo() const { return !m_undo.isEmpty(); }
    bool canRedo() const { return !m_redo.isEmpty(); }

signals:
    void compositionChanged();
    void selectionChanged(bool textSelected);
    void backgroundPanRequested(const QPointF &normalizedDelta);
    void backgroundZoomRequested(qreal scaleFactor, const QPointF &normalizedAnchor);

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    struct TextBlock {
        QString text;
        QFont font;
        QColor color;
        QPointF position;
    };
    struct Stroke {
        QColor color;
        int width = 4;
        QVector<QPointF> points;
    };
    struct Shape {
        ShapeType type = ShapeType::Line;
        QColor color;
        int width = 4;
        QPointF start;
        QPointF end;
    };
    struct Snapshot {
        QVector<TextBlock> texts;
        QVector<Stroke> strokes;
        QVector<Shape> shapes;
    };

    QRectF imageRect() const;
    QPointF imagePoint(const QPointF &widgetPoint) const;
    QImage renderComposition() const;
    int textAt(const QPointF &imagePoint) const;
    void saveUndo();
    void restore(const Snapshot &snapshot);
    void emitChanged();

    QImage m_background;
    QVector<TextBlock> m_texts;
    QVector<Stroke> m_strokes;
    QVector<Shape> m_shapes;
    QVector<Snapshot> m_undo;
    QVector<Snapshot> m_redo;
    Tool m_tool = Tool::Select;
    QColor m_inkColor = Qt::white;
    int m_inkWidth = 4;
    ShapeType m_shapeType = ShapeType::Line;
    int m_selectedText = -1;
    bool m_draggingText = false;
    bool m_panningBackground = false;
    bool m_dragUndoCaptured = false;
    QPointF m_lastPoint;
    static constexpr int MaxHistory = 20;
};

#endif // SSTVCOMPOSERCANVAS_H
