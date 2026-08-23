#include "ui/sstvcomposercanvas.h"

#include <QtTest>

class SstvComposerTest : public QObject {
    Q_OBJECT
private slots:
    void rendersTextAtNativeFrameSize();
    void freehandUndoRestoresBackground();
    void compositionStateRestoresMultilineTextAndCanDeleteIt();
    void unspecifiedFontStretchRestoresAtNormalWidth();
    void simpleShapePersistsAndUndoRemovesIt();
    void newImageClearsMarkupAndUndoHistory();
    void selectedTextColorChangesImmediatelyAndIsUndoable();
    void selectedTextFontChangesImmediatelyAndIsUndoable();
};

void SstvComposerTest::rendersTextAtNativeFrameSize() {
    SstvComposerCanvas canvas;
    QImage background(320, 256, QImage::Format_RGB32);
    background.fill(Qt::black);
    canvas.setBackground(background);
    QFont font(QStringLiteral("Sans Serif"));
    font.setPixelSize(32);
    font.setBold(true);
    canvas.addTextBlock(QStringLiteral("W9WDX"), font, Qt::white);

    const QImage rendered = canvas.renderedImage();
    QCOMPARE(rendered.size(), background.size());
    QVERIFY(rendered != background);
}

void SstvComposerTest::freehandUndoRestoresBackground() {
    SstvComposerCanvas canvas;
    canvas.resize(320, 256);
    canvas.show();
    QImage background(320, 256, QImage::Format_RGB32);
    background.fill(Qt::black);
    canvas.setBackground(background);
    canvas.setTool(SstvComposerCanvas::Tool::Draw);
    canvas.setInk(Qt::red, 8);

    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 50));
    QTest::mouseMove(&canvas, QPoint(250, 180), 20);
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(250, 180));
    QVERIFY(canvas.renderedImage() != background);
    QVERIFY(canvas.canUndo());
    canvas.undo();
    QCOMPARE(canvas.renderedImage(), background);
}

void SstvComposerTest::compositionStateRestoresMultilineTextAndCanDeleteIt() {
    QImage background(320, 256, QImage::Format_RGB32);
    background.fill(Qt::black);
    QFont font(QStringLiteral("Sans Serif"));
    font.setPixelSize(34);
    font.setWeight(QFont::Black);
    font.setStretch(QFont::Condensed);
    font.setItalic(true);

    SstvComposerCanvas original;
    original.setBackground(background);
    original.addTextBlock(QStringLiteral("CQ CQ CQ\nDE W9WDX"), font, Qt::white);
    const QJsonObject state = original.compositionState();

    SstvComposerCanvas restored;
    restored.setBackground(background);
    QVERIFY(restored.restoreCompositionState(state));
    QCOMPARE(restored.renderedImage(), original.renderedImage());
    restored.setTool(SstvComposerCanvas::Tool::Select);
    restored.resize(640, 512);
    restored.show();
    QTest::mouseClick(&restored, Qt::LeftButton, Qt::NoModifier, QPoint(320, 256));
    QVERIFY(restored.hasSelectedText());
    restored.deleteSelectedText();
    QCOMPARE(restored.renderedImage(), background);
    restored.undo();
    QCOMPARE(restored.renderedImage(), original.renderedImage());
}

void SstvComposerTest::unspecifiedFontStretchRestoresAtNormalWidth() {
    QImage background(320, 256, QImage::Format_RGB32);
    background.fill(Qt::black);
    QFont font(QStringLiteral("Sans Serif"));
    font.setPixelSize(32);
    font.setBold(true);

    SstvComposerCanvas original;
    original.setBackground(background);
    original.addTextBlock(QStringLiteral("CQ CQ CQ\nDE AE6LX"), font, Qt::white,
                          QPointF(0.27, 0.26));
    QJsonObject state = original.compositionState();
    QJsonArray texts = state.value(QStringLiteral("texts")).toArray();
    QVERIFY(!texts.isEmpty());
    QJsonObject text = texts.first().toObject();
    text.insert(QStringLiteral("stretch"), 0);
    texts[0] = text;
    state.insert(QStringLiteral("texts"), texts);

    SstvComposerCanvas restored;
    restored.setBackground(background);
    QVERIFY(restored.restoreCompositionState(state));
    QVERIFY(restored.renderedImage() != background);
    const QJsonArray restoredTexts =
        restored.compositionState().value(QStringLiteral("texts")).toArray();
    QCOMPARE(restoredTexts.first().toObject().value(QStringLiteral("stretch")).toInt(),
             static_cast<int>(QFont::Unstretched));
}

void SstvComposerTest::simpleShapePersistsAndUndoRemovesIt() {
    QImage background(320, 256, QImage::Format_RGB32);
    background.fill(Qt::black);
    SstvComposerCanvas canvas;
    canvas.resize(640, 512);
    canvas.setBackground(background);
    canvas.setInk(Qt::yellow, 5);
    canvas.setShapeType(SstvComposerCanvas::ShapeType::Arrow);
    canvas.setTool(SstvComposerCanvas::Tool::Shape);
    canvas.show();
    QTest::mousePress(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
    QTest::mouseMove(&canvas, QPoint(500, 400));
    QTest::mouseRelease(&canvas, Qt::LeftButton, Qt::NoModifier, QPoint(500, 400));
    QVERIFY(canvas.renderedImage() != background);
    QCOMPARE(canvas.compositionState().value(QStringLiteral("shapes")).toArray().size(), 1);
    canvas.undo();
    QCOMPARE(canvas.renderedImage(), background);
}

void SstvComposerTest::newImageClearsMarkupAndUndoHistory() {
    QImage first(320, 256, QImage::Format_RGB32);
    first.fill(Qt::black);
    QImage second(320, 256, QImage::Format_RGB32);
    second.fill(Qt::blue);
    QFont font(QStringLiteral("Sans Serif"));
    font.setPixelSize(32);

    SstvComposerCanvas canvas;
    canvas.setBackground(first);
    canvas.addTextBlock(QStringLiteral("OLD MARKUP"), font, Qt::white);
    QVERIFY(canvas.renderedImage() != first);
    QVERIFY(canvas.canUndo());

    canvas.clearCompositionForNewImage();
    canvas.setBackground(second);
    QCOMPARE(canvas.renderedImage(), second);
    QVERIFY(!canvas.canUndo());
    QVERIFY(!canvas.canRedo());
    QCOMPARE(canvas.compositionState().value(QStringLiteral("texts")).toArray().size(), 0);
}

void SstvComposerTest::selectedTextColorChangesImmediatelyAndIsUndoable() {
    QImage background(320, 256, QImage::Format_RGB32);
    background.fill(Qt::black);
    QFont font(QStringLiteral("Sans Serif"));
    font.setPixelSize(32);
    font.setBold(true);

    SstvComposerCanvas canvas;
    canvas.setBackground(background);
    canvas.addTextBlock(QStringLiteral("W9WDX"), font, Qt::white);
    const QImage whiteText = canvas.renderedImage();
    QCOMPARE(canvas.selectedTextColor(), QColor(Qt::white));

    canvas.updateSelectedTextColor(Qt::red);
    QCOMPARE(canvas.selectedTextColor(), QColor(Qt::red));
    QVERIFY(canvas.renderedImage() != whiteText);

    canvas.undo();
    QCOMPARE(canvas.renderedImage(), whiteText);
}

void SstvComposerTest::selectedTextFontChangesImmediatelyAndIsUndoable() {
    QImage background(320, 256, QImage::Format_RGB32);
    background.fill(Qt::black);
    QFont originalFont(QStringLiteral("Sans Serif"));
    originalFont.setPixelSize(24);

    SstvComposerCanvas canvas;
    canvas.setBackground(background);
    canvas.addTextBlock(QStringLiteral("W9WDX"), originalFont, Qt::white);
    const QImage original = canvas.renderedImage();

    QFont changedFont(QStringLiteral("Serif"));
    changedFont.setPixelSize(48);
    changedFont.setBold(true);
    canvas.updateSelectedTextFont(changedFont);
    QCOMPARE(canvas.selectedTextFont(), changedFont);
    QVERIFY(canvas.renderedImage() != original);

    canvas.undo();
    QCOMPARE(canvas.renderedImage(), original);
}

QTEST_MAIN(SstvComposerTest)
#include "test_sstvcomposer.moc"
