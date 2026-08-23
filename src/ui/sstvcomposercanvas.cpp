#include "sstvcomposercanvas.h"

#include <QMouseEvent>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QPainter>
#include <QJsonArray>
#include <QLineF>
#include <QStringList>

namespace {
QRectF textBounds(const QFont &font, const QString &text) {
    const QFontMetricsF metrics(font);
    const QStringList lines = text.split(QLatin1Char('\n'));
    qreal width = 0.0;
    for (const QString &line : lines)
        width = qMax(width, metrics.horizontalAdvance(line));
    return QRectF(0.0, 0.0, width, metrics.lineSpacing() * qMax(1, lines.size()));
}

void drawTextBlock(QPainter &painter, const QFont &font, const QPointF &center, const QString &text) {
    const QFontMetricsF metrics(font);
    const QRectF bounds = textBounds(font, text);
    const QPointF topLeft = center - QPointF(bounds.width() * 0.5, bounds.height() * 0.5);
    const QStringList lines = text.split(QLatin1Char('\n'));
    for (int i = 0; i < lines.size(); ++i)
        painter.drawText(QPointF(topLeft.x(), topLeft.y() + metrics.ascent() + i * metrics.lineSpacing()), lines.at(i));
}

void drawShape(QPainter &painter, SstvComposerCanvas::ShapeType type,
               const QPointF &start, const QPointF &end) {
    const QRectF bounds(start, end);
    switch (type) {
    case SstvComposerCanvas::ShapeType::Line:
        painter.drawLine(start, end);
        break;
    case SstvComposerCanvas::ShapeType::Arrow: {
        const QLineF shaft(start, end);
        painter.drawLine(shaft);
        if (shaft.length() < 2.0)
            break;
        const qreal headLength = qMin<qreal>(18.0, qMax<qreal>(7.0, shaft.length() * 0.18));
        QLineF left(end, end);
        left.setLength(headLength);
        left.setAngle(shaft.angle() + 150.0);
        QLineF right(end, end);
        right.setLength(headLength);
        right.setAngle(shaft.angle() - 150.0);
        painter.drawLine(left);
        painter.drawLine(right);
        break;
    }
    case SstvComposerCanvas::ShapeType::Rectangle:
        painter.drawRect(bounds.normalized());
        break;
    case SstvComposerCanvas::ShapeType::Ellipse:
        painter.drawEllipse(bounds.normalized());
        break;
    }
}
}

SstvComposerCanvas::SstvComposerCanvas(QWidget *parent) : QWidget(parent) {
    setMinimumSize(240, 180);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_AcceptTouchEvents);
    grabGesture(Qt::PinchGesture);
    setStyleSheet(QStringLiteral("background: #1b2022; border: 1px solid #f2ad20;"));
}

bool SstvComposerCanvas::event(QEvent *event) {
    if (event->type() == QEvent::Gesture && !m_background.isNull()) {
        auto *gestureEvent = static_cast<QGestureEvent *>(event);
        if (auto *pinch = static_cast<QPinchGesture *>(
                gestureEvent->gesture(Qt::PinchGesture))) {
            if (pinch->state() == Qt::GestureStarted) {
                m_draggingText = false;
                m_panningBackground = false;
                m_dragUndoCaptured = false;
            } else if (pinch->state() == Qt::GestureUpdated
                       && pinch->changeFlags().testFlag(QPinchGesture::ScaleFactorChanged)) {
                // QPinchGesture::scaleFactor() is already the relative change
                // from the previous gesture event. Dividing it by
                // lastScaleFactor() makes consecutive updates alternate and
                // visibly fight the synchronized zoom slider.
                const qreal factor = pinch->scaleFactor();
                const QRectF target = imageRect();
                if (!target.isEmpty() && qAbs(factor - 1.0) > 0.001) {
                    const QPointF center = pinch->centerPoint();
                    const QPointF anchor(
                        qBound(0.0, (center.x() - target.left()) / target.width(), 1.0),
                        qBound(0.0, (center.y() - target.top()) / target.height(), 1.0));
                    emit backgroundZoomRequested(factor, anchor);
                }
            }
            gestureEvent->accept(pinch);
            return true;
        }
    }
    return QWidget::event(event);
}

void SstvComposerCanvas::setBackground(const QImage &image) {
    if (image.isNull())
        return;
    const QSize oldSize = m_background.size();
    m_background = image.convertToFormat(QImage::Format_RGB32);
    if (!oldSize.isEmpty() && oldSize != m_background.size()) {
        const qreal xScale = static_cast<qreal>(m_background.width()) / oldSize.width();
        const qreal yScale = static_cast<qreal>(m_background.height()) / oldSize.height();
        for (TextBlock &block : m_texts)
            block.position = QPointF(block.position.x() * xScale, block.position.y() * yScale);
        for (Stroke &stroke : m_strokes) {
            stroke.width = qMax(1, qRound(stroke.width * (xScale + yScale) * 0.5));
            for (QPointF &point : stroke.points)
                point = QPointF(point.x() * xScale, point.y() * yScale);
        }
        for (Shape &shape : m_shapes) {
            shape.width = qMax(1, qRound(shape.width * (xScale + yScale) * 0.5));
            shape.start = QPointF(shape.start.x() * xScale, shape.start.y() * yScale);
            shape.end = QPointF(shape.end.x() * xScale, shape.end.y() * yScale);
        }
    }
    update();
}

QImage SstvComposerCanvas::renderedImage() const {
    return renderComposition();
}

void SstvComposerCanvas::setTool(Tool tool) {
    m_tool = tool;
    setCursor(tool == Tool::Select ? Qt::ArrowCursor : Qt::CrossCursor);
}

void SstvComposerCanvas::setInk(const QColor &color, int width) {
    m_inkColor = color;
    m_inkWidth = qBound(1, width, 40);
}

void SstvComposerCanvas::addTextBlock(const QString &text, const QFont &font, const QColor &color,
                                      const QPointF &normalizedPosition) {
    if (m_background.isNull() || text.trimmed().isEmpty())
        return;
    saveUndo();
    m_texts.append({text, font, color,
                    QPointF(qBound(0.0, normalizedPosition.x(), 1.0) * m_background.width(),
                            qBound(0.0, normalizedPosition.y(), 1.0) * m_background.height())});
    m_selectedText = m_texts.size() - 1;
    emitChanged();
}

void SstvComposerCanvas::updateSelectedText(const QString &text, const QFont &font, const QColor &color) {
    if (m_selectedText < 0 || m_selectedText >= m_texts.size() || text.trimmed().isEmpty())
        return;
    saveUndo();
    TextBlock &block = m_texts[m_selectedText];
    block.text = text;
    block.font = font;
    block.color = color;
    emitChanged();
}

void SstvComposerCanvas::updateSelectedTextColor(const QColor &color) {
    if (!hasSelectedText() || !color.isValid() || m_texts.at(m_selectedText).color == color)
        return;
    saveUndo();
    m_texts[m_selectedText].color = color;
    emitChanged();
}

void SstvComposerCanvas::updateSelectedTextFont(const QFont &font) {
    if (!hasSelectedText() || m_texts.at(m_selectedText).font == font)
        return;
    saveUndo();
    m_texts[m_selectedText].font = font;
    emitChanged();
}

bool SstvComposerCanvas::hasSelectedText() const {
    return m_selectedText >= 0 && m_selectedText < m_texts.size();
}

QString SstvComposerCanvas::selectedText() const {
    return hasSelectedText() ? m_texts.at(m_selectedText).text : QString();
}

QFont SstvComposerCanvas::selectedTextFont() const {
    return hasSelectedText() ? m_texts.at(m_selectedText).font : QFont();
}

QColor SstvComposerCanvas::selectedTextColor() const {
    return hasSelectedText() ? m_texts.at(m_selectedText).color : QColor();
}

void SstvComposerCanvas::deleteSelectedText() {
    if (!hasSelectedText())
        return;
    saveUndo();
    m_texts.removeAt(m_selectedText);
    m_selectedText = -1;
    emitChanged();
}

QJsonObject SstvComposerCanvas::compositionState() const {
    QJsonArray texts;
    for (const TextBlock &block : m_texts) {
        const qreal width = qMax(1, m_background.width());
        const qreal height = qMax(1, m_background.height());
        texts.append(QJsonObject{{QStringLiteral("text"), block.text},
                                 {QStringLiteral("font"), block.font.family()},
                                 {QStringLiteral("pixelSize"), block.font.pixelSize()},
                                 {QStringLiteral("weight"), block.font.weight()},
                                 {QStringLiteral("stretch"), block.font.stretch()},
                                 {QStringLiteral("bold"), block.font.bold()},
                                 {QStringLiteral("italic"), block.font.italic()},
                                 {QStringLiteral("color"), block.color.name(QColor::HexArgb)},
                                 {QStringLiteral("x"), block.position.x() / width},
                                 {QStringLiteral("y"), block.position.y() / height}});
    }
    QJsonArray strokes;
    for (const Stroke &stroke : m_strokes) {
        QJsonArray points;
        const qreal width = qMax(1, m_background.width());
        const qreal height = qMax(1, m_background.height());
        for (const QPointF &point : stroke.points)
            points.append(QJsonArray{point.x() / width, point.y() / height});
        strokes.append(QJsonObject{{QStringLiteral("color"), stroke.color.name(QColor::HexArgb)},
                                   {QStringLiteral("width"), stroke.width / width},
                                   {QStringLiteral("points"), points}});
    }
    QJsonArray shapes;
    const qreal imageWidth = qMax(1, m_background.width());
    const qreal imageHeight = qMax(1, m_background.height());
    for (const Shape &shape : m_shapes) {
        shapes.append(QJsonObject{{QStringLiteral("type"), static_cast<int>(shape.type)},
                                  {QStringLiteral("color"), shape.color.name(QColor::HexArgb)},
                                  {QStringLiteral("width"), shape.width / imageWidth},
                                  {QStringLiteral("x1"), shape.start.x() / imageWidth},
                                  {QStringLiteral("y1"), shape.start.y() / imageHeight},
                                  {QStringLiteral("x2"), shape.end.x() / imageWidth},
                                  {QStringLiteral("y2"), shape.end.y() / imageHeight}});
    }
    return {{QStringLiteral("version"), 1},
            {QStringLiteral("texts"), texts},
            {QStringLiteral("strokes"), strokes},
            {QStringLiteral("shapes"), shapes}};
}

bool SstvComposerCanvas::restoreCompositionState(const QJsonObject &state) {
    if (m_background.isNull())
        return false;
    QVector<TextBlock> texts;
    QVector<Stroke> strokes;
    QVector<Shape> shapes;
    for (const QJsonValue &value : state.value(QStringLiteral("texts")).toArray()) {
        const QJsonObject object = value.toObject();
        const QString text = object.value(QStringLiteral("text")).toString();
        if (text.trimmed().isEmpty())
            continue;
        QFont font(object.value(QStringLiteral("font")).toString(QStringLiteral("Sans Serif")));
        font.setPixelSize(qBound(8, object.value(QStringLiteral("pixelSize")).toInt(28), 160));
        if (object.contains(QStringLiteral("weight"))) {
            font.setWeight(static_cast<QFont::Weight>(
                qBound(static_cast<int>(QFont::Thin),
                       object.value(QStringLiteral("weight")).toInt(static_cast<int>(QFont::Normal)),
                       static_cast<int>(QFont::Black))));
        } else {
            font.setBold(object.value(QStringLiteral("bold")).toBool(true));
        }
        // QFont reports 0 when no explicit stretch was selected. That means
        // normal width, not a literal 0/1-percent font. Older saved image
        // templates therefore need 0 normalized to Unstretched when loaded.
        const int storedStretch = object.value(QStringLiteral("stretch"))
                                      .toInt(QFont::Unstretched);
        font.setStretch(storedStretch > 0 ? qBound(1, storedStretch, 400)
                                          : static_cast<int>(QFont::Unstretched));
        font.setItalic(object.value(QStringLiteral("italic")).toBool(false));
        const QColor color(object.value(QStringLiteral("color")).toString(QStringLiteral("#ffffffff")));
        texts.append({text, font, color.isValid() ? color : QColor(Qt::white),
                      QPointF(qBound(0.0, object.value(QStringLiteral("x")).toDouble(0.5), 1.0) * m_background.width(),
                              qBound(0.0, object.value(QStringLiteral("y")).toDouble(0.5), 1.0) * m_background.height())});
    }
    for (const QJsonValue &value : state.value(QStringLiteral("strokes")).toArray()) {
        const QJsonObject object = value.toObject();
        Stroke stroke;
        stroke.color = QColor(object.value(QStringLiteral("color")).toString(QStringLiteral("#ffffffff")));
        if (!stroke.color.isValid())
            stroke.color = Qt::white;
        stroke.width = qBound(1, qRound(object.value(QStringLiteral("width")).toDouble(0.0125)
                                       * m_background.width()), 80);
        for (const QJsonValue &pointValue : object.value(QStringLiteral("points")).toArray()) {
            const QJsonArray point = pointValue.toArray();
            if (point.size() == 2)
                stroke.points.append(QPointF(qBound(0.0, point.at(0).toDouble(), 1.0) * m_background.width(),
                                             qBound(0.0, point.at(1).toDouble(), 1.0) * m_background.height()));
        }
        if (!stroke.points.isEmpty())
            strokes.append(stroke);
    }
    for (const QJsonValue &value : state.value(QStringLiteral("shapes")).toArray()) {
        const QJsonObject object = value.toObject();
        Shape shape;
        shape.type = static_cast<ShapeType>(qBound(0, object.value(QStringLiteral("type")).toInt(), 3));
        shape.color = QColor(object.value(QStringLiteral("color")).toString(QStringLiteral("#ffffffff")));
        if (!shape.color.isValid())
            shape.color = Qt::white;
        shape.width = qBound(1, qRound(object.value(QStringLiteral("width")).toDouble(0.0125)
                                      * m_background.width()), 80);
        shape.start = QPointF(qBound(0.0, object.value(QStringLiteral("x1")).toDouble(), 1.0)
                                  * m_background.width(),
                              qBound(0.0, object.value(QStringLiteral("y1")).toDouble(), 1.0)
                                  * m_background.height());
        shape.end = QPointF(qBound(0.0, object.value(QStringLiteral("x2")).toDouble(), 1.0)
                                * m_background.width(),
                            qBound(0.0, object.value(QStringLiteral("y2")).toDouble(), 1.0)
                                * m_background.height());
        shapes.append(shape);
    }
    saveUndo();
    m_texts = texts;
    m_strokes = strokes;
    m_shapes = shapes;
    m_selectedText = -1;
    emitChanged();
    return true;
}

void SstvComposerCanvas::undo() {
    if (m_undo.isEmpty())
        return;
    m_redo.append({m_texts, m_strokes, m_shapes});
    restore(m_undo.takeLast());
}

void SstvComposerCanvas::redo() {
    if (m_redo.isEmpty())
        return;
    m_undo.append({m_texts, m_strokes, m_shapes});
    restore(m_redo.takeLast());
}

void SstvComposerCanvas::resetComposition() {
    if (m_texts.isEmpty() && m_strokes.isEmpty() && m_shapes.isEmpty())
        return;
    saveUndo();
    m_texts.clear();
    m_strokes.clear();
    m_shapes.clear();
    m_selectedText = -1;
    emitChanged();
}

void SstvComposerCanvas::clearCompositionForNewImage() {
    m_texts.clear();
    m_strokes.clear();
    m_shapes.clear();
    m_undo.clear();
    m_redo.clear();
    m_selectedText = -1;
    m_draggingText = false;
    m_panningBackground = false;
    m_dragUndoCaptured = false;
    emitChanged();
}

QRectF SstvComposerCanvas::imageRect() const {
    if (m_background.isNull())
        return QRectF();
    QSizeF size = m_background.size();
    size.scale(this->size(), Qt::KeepAspectRatio);
    return QRectF((width() - size.width()) * 0.5, (height() - size.height()) * 0.5, size.width(), size.height());
}

QPointF SstvComposerCanvas::imagePoint(const QPointF &widgetPoint) const {
    const QRectF rect = imageRect();
    if (rect.isEmpty() || m_background.isNull())
        return QPointF();
    return QPointF(qBound(0.0, (widgetPoint.x() - rect.left()) * m_background.width() / rect.width(),
                         static_cast<double>(m_background.width() - 1)),
                   qBound(0.0, (widgetPoint.y() - rect.top()) * m_background.height() / rect.height(),
                         static_cast<double>(m_background.height() - 1)));
}

QImage SstvComposerCanvas::renderComposition() const {
    if (m_background.isNull())
        return QImage();
    QImage frame = m_background.copy();
    QPainter painter(&frame);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    for (const Stroke &stroke : m_strokes) {
        if (stroke.points.size() < 2)
            continue;
        painter.setPen(QPen(stroke.color, stroke.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPolyline(stroke.points.constData(), stroke.points.size());
    }
    for (const Shape &shape : m_shapes) {
        painter.setPen(QPen(shape.color, shape.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);
        drawShape(painter, shape.type, shape.start, shape.end);
    }
    for (const TextBlock &block : m_texts) {
        painter.setPen(block.color);
        painter.setFont(block.font);
        drawTextBlock(painter, block.font, block.position, block.text);
    }
    return frame;
}

int SstvComposerCanvas::textAt(const QPointF &point) const {
    for (int i = m_texts.size() - 1; i >= 0; --i) {
        const TextBlock &block = m_texts.at(i);
        const QRectF bounds = textBounds(block.font, block.text);
        const QRectF hit(block.position.x() - bounds.width() * 0.5 - 10, block.position.y() - bounds.height() * 0.5 - 10,
                         bounds.width() + 20, bounds.height() + 20);
        if (hit.contains(point))
            return i;
    }
    return -1;
}

void SstvComposerCanvas::saveUndo() {
    m_undo.append({m_texts, m_strokes, m_shapes});
    if (m_undo.size() > MaxHistory)
        m_undo.removeFirst();
    m_redo.clear();
}

void SstvComposerCanvas::restore(const Snapshot &snapshot) {
    m_texts = snapshot.texts;
    m_strokes = snapshot.strokes;
    m_shapes = snapshot.shapes;
    m_selectedText = -1;
    emitChanged();
}

void SstvComposerCanvas::emitChanged() {
    update();
    emit compositionChanged();
    emit selectionChanged(hasSelectedText());
}

void SstvComposerCanvas::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor(QStringLiteral("#1b2022")));
    if (m_background.isNull()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("Choose an image from Gallery or Camera"));
        return;
    }
    const QRectF target = imageRect();
    painter.drawImage(target, renderComposition());
    if (hasSelectedText() && m_tool == Tool::Select) {
        const TextBlock &block = m_texts.at(m_selectedText);
        const QRectF bounds = textBounds(block.font, block.text);
        const QPointF topLeft = target.topLeft() + QPointF((block.position.x() - bounds.width() * 0.5) * target.width() / m_background.width(),
                                                            (block.position.y() - bounds.height() * 0.5) * target.height() / m_background.height());
        const QSizeF size(bounds.width() * target.width() / m_background.width(), bounds.height() * target.height() / m_background.height());
        painter.setPen(QPen(QColor(QStringLiteral("#f2ad20")), 2, Qt::DashLine));
        painter.drawRect(QRectF(topLeft, size));
    }
}

void SstvComposerCanvas::mousePressEvent(QMouseEvent *event) {
    if (m_background.isNull() || event->button() != Qt::LeftButton)
        return;
    const QPointF point = imagePoint(event->position());
    m_lastPoint = point;
    if (m_tool == Tool::Draw) {
        saveUndo();
        m_strokes.append({m_inkColor, m_inkWidth, {point}});
        return;
    }
    if (m_tool == Tool::Shape) {
        saveUndo();
        m_shapes.append({m_shapeType, m_inkColor, m_inkWidth, point, point});
        return;
    }
    m_selectedText = textAt(point);
    m_draggingText = hasSelectedText();
    m_panningBackground = !m_draggingText;
    m_dragUndoCaptured = false;
    emitChanged();
}

void SstvComposerCanvas::mouseMoveEvent(QMouseEvent *event) {
    if (m_background.isNull() || !(event->buttons() & Qt::LeftButton))
        return;
    const QPointF point = imagePoint(event->position());
    if (m_tool == Tool::Draw && !m_strokes.isEmpty()) {
        m_strokes.last().points.append(point);
        update();
    } else if (m_tool == Tool::Shape && !m_shapes.isEmpty()) {
        m_shapes.last().end = point;
        update();
    } else if (m_draggingText && hasSelectedText()) {
        if (!m_dragUndoCaptured) {
            saveUndo();
            m_dragUndoCaptured = true;
        }
        m_texts[m_selectedText].position += point - m_lastPoint;
        m_lastPoint = point;
        update();
    } else if (m_tool == Tool::Select && m_panningBackground) {
        const QPointF delta((point.x() - m_lastPoint.x()) / qMax(1, m_background.width()),
                            (point.y() - m_lastPoint.y()) / qMax(1, m_background.height()));
        m_lastPoint = point;
        if (!qFuzzyIsNull(delta.x()) || !qFuzzyIsNull(delta.y()))
            emit backgroundPanRequested(delta);
    }
}

void SstvComposerCanvas::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton)
        return;
    const bool changed = (m_tool == Tool::Draw && !m_strokes.isEmpty())
        || (m_tool == Tool::Shape && !m_shapes.isEmpty()) || m_draggingText;
    m_draggingText = false;
    m_panningBackground = false;
    m_dragUndoCaptured = false;
    if (changed)
        emitChanged();
}

void SstvComposerCanvas::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    update();
}
