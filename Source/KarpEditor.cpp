#include "KarpEditor.h"

#include <BinaryData.h>
#include "MnAudioUi.h"

namespace
{
using mnaudio::ui::Palette;

constexpr int margin = 8;
constexpr int gap = 8;
constexpr int barHeight = 40;
constexpr int flowHeight = 24;
constexpr int minContentWidth = 640;

juce::Font titleFont(float height)
{
    static const juce::Typeface::Ptr montserrat = juce::Typeface::createSystemTypefaceFor(
        BinaryData::Montserrat_ttf, (size_t) BinaryData::Montserrat_ttfSize);
    return juce::Font(juce::FontOptions(montserrat).withHeight(height)).boldened();
}

juce::Font textFont(float height)
{
    static const juce::Typeface::Ptr roboto = juce::Typeface::createSystemTypefaceFor(
        BinaryData::RobotoCondensed_ttf, (size_t) BinaryData::RobotoCondensed_ttfSize);
    return juce::Font(juce::FontOptions(roboto).withHeight(height)).boldened();
}

juce::String noteName(int note)
{
    static const char* const names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return juce::String(names[note % 12]) + juce::String(note / 12 - 1);
}
}

//==============================================================================
MnKarpAudioProcessorEditor::RackLookAndFeel::RackLookAndFeel()
{
    setColour(juce::TextButton::buttonColourId, Palette::cardHi());
    setColour(juce::TextButton::textColourOffId, Palette::text());
    setColour(juce::TextButton::textColourOnId, Palette::text());
    setColour(juce::PopupMenu::backgroundColourId, Palette::card());
    setColour(juce::PopupMenu::textColourId, Palette::text());
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Palette::cardHi());
    setColour(juce::PopupMenu::highlightedTextColourId, Palette::accent());
    setColour(juce::AlertWindow::backgroundColourId, Palette::card());
    setColour(juce::AlertWindow::textColourId, Palette::text());
    setColour(juce::TextEditor::backgroundColourId, Palette::bg());
    setColour(juce::TextEditor::textColourId, Palette::text());
    setColour(juce::TextEditor::outlineColourId, Palette::border());
}

juce::Font MnKarpAudioProcessorEditor::RackLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    return textFont(juce::jmin(16.0f, (float) buttonHeight * 0.6f));
}

juce::Font MnKarpAudioProcessorEditor::RackLookAndFeel::getPopupMenuFont()
{
    return textFont(14.0f);
}

void MnKarpAudioProcessorEditor::RackLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                                       const juce::Colour& backgroundColour,
                                                                       bool highlighted, bool down)
{
    auto fill = backgroundColour;
    if (down)
        fill = fill.brighter(0.16f);
    else if (highlighted)
        fill = fill.brighter(0.08f);
    if (! button.isEnabled())
        fill = fill.withMultipliedAlpha(0.5f);

    g.setColour(fill);
    g.fillRoundedRectangle(button.getLocalBounds().toFloat(), 7.0f);
}

void MnKarpAudioProcessorEditor::RackLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.setColour(findColour(juce::PopupMenu::backgroundColourId));
    g.fillRect(0, 0, width, height);
}

void MnKarpAudioProcessorEditor::RackLookAndFeel::drawPopupMenuBackgroundWithOptions(
    juce::Graphics& g, int width, int height, const juce::PopupMenu::Options&)
{
    drawPopupMenuBackground(g, width, height);
}

void MnKarpAudioProcessorEditor::RackLookAndFeel::drawPopupMenuItem(
    juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive, bool isHighlighted,
    bool isTicked, bool, const juce::String& text, const juce::String&, const juce::Drawable*,
    const juce::Colour* textColour)
{
    if (isSeparator)
    {
        g.setColour(findColour(juce::PopupMenu::textColourId).withAlpha(0.2f));
        g.fillRect(area.withSizeKeepingCentre(area.getWidth() - 16, 1));
        return;
    }

    auto row = area.reduced(0, 1);
    if (isHighlighted)
    {
        g.setColour(findColour(juce::PopupMenu::highlightedBackgroundColourId));
        g.fillRect(row);
    }

    auto colour = textColour != nullptr ? *textColour
                                        : findColour(isHighlighted ? juce::PopupMenu::highlightedTextColourId
                                                                   : juce::PopupMenu::textColourId);
    if (! isActive)
        colour = colour.withAlpha(0.45f);

    g.setColour(colour);
    g.setFont(textFont(14.0f));
    g.drawText(text, row.reduced(28, 0), juce::Justification::centredLeft, true);

    if (isTicked)
    {
        juce::Path tick;
        const auto tickArea = row.removeFromLeft(24).toFloat().reduced(7.0f, 8.0f);
        tick.startNewSubPath(tickArea.getX(), tickArea.getCentreY());
        tick.lineTo(tickArea.getCentreX() - 1.0f, tickArea.getBottom());
        tick.lineTo(tickArea.getRight(), tickArea.getY());
        g.strokePath(tick, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}

juce::Component* MnKarpAudioProcessorEditor::RackLookAndFeel::getParentComponentForMenuOptions(
    const juce::PopupMenu::Options& options)
{
    // A window of its own: inside the rack's window it would be under the plugins' windows.
    return options.getParentComponent();
}

void MnKarpAudioProcessorEditor::RackLookAndFeel::getIdealPopupMenuItemSize(
    const juce::String& text, bool isSeparator, int standardMenuItemHeight, int& idealWidth, int& idealHeight)
{
    idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight : 24;

    if (isSeparator)
    {
        idealWidth = 50;
        idealHeight = idealHeight / 2;
        return;
    }

    // The text area of drawPopupMenuItem() has 28 px at each side, plus a little room.
    idealWidth = juce::GlyphArrangement::getStringWidthInt(textFont(14.0f), text) + 56 + 16;
}

//==============================================================================
void MnKarpAudioProcessorEditor::PowerButton::paintButton(juce::Graphics& g, bool highlighted, bool down)
{
    const auto r = getLocalBounds().toFloat().reduced(1.0f);
    const bool on = getToggleState();
    const auto line = on ? Palette::accent() : Palette::dimText().withAlpha(0.55f);

    g.setColour(on ? Palette::accent().withAlpha(down ? 0.34f : highlighted ? 0.26f : 0.18f)
                   : (down ? Palette::cardHi().brighter(0.2f) : highlighted ? Palette::cardHi().brighter(0.1f)
                                                                           : Palette::cardHi()));
    g.fillEllipse(r);
    g.setColour(line);
    g.drawEllipse(r, 1.5f);

    // The power symbol: an arc open at the top, with a bar in the gap.
    const auto c = r.getCentre();
    const float radius = r.getWidth() * 0.26f;
    juce::Path symbol;
    symbol.addCentredArc(c.x, c.y, radius, radius, 0.0f, 0.75f, juce::MathConstants<float>::twoPi - 0.75f, true);
    symbol.startNewSubPath(c.x, c.y - radius * 1.2f);
    symbol.lineTo(c.x, c.y - radius * 0.1f);
    g.strokePath(symbol, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void MnKarpAudioProcessorEditor::FoldButton::paintButton(juce::Graphics& g, bool highlighted, bool down)
{
    const auto r = getLocalBounds().toFloat().reduced(1.0f);
    g.setColour(down ? Palette::cardHi().brighter(0.2f) : highlighted ? Palette::cardHi().brighter(0.1f) : Palette::cardHi());
    g.fillRoundedRectangle(r, 5.0f);
    g.setColour(Palette::border());
    g.drawRoundedRectangle(r, 5.0f, 1.0f);

    const auto c = r.getCentre();
    juce::Path arrow;
    if (folded)
        arrow.addTriangle(c.x - 3.0f, c.y - 5.0f, c.x - 3.0f, c.y + 5.0f, c.x + 4.0f, c.y);
    else
        arrow.addTriangle(c.x - 5.0f, c.y - 3.0f, c.x + 5.0f, c.y - 3.0f, c.x, c.y + 4.0f);
    g.setColour(Palette::text());
    g.fillPath(arrow);
}

//==============================================================================
juce::String MnKarpAudioProcessorEditor::FlowBar::describe(const MnKarpAudioProcessor::NoteBits& notes)
{
    juce::StringArray names;
    for (int note = 0; note < 128; ++note)
        if (notes.contains(note))
            names.add(noteName(note));

    if (names.isEmpty())
        return {};

    constexpr int maxShown = 10;
    const int extra = names.size() - maxShown;
    if (extra > 0)
    {
        names.removeRange(maxShown, extra);
        return names.joinIntoString("  ") + "  +" + juce::String(extra);
    }
    return names.joinIntoString("  ");
}

void MnKarpAudioProcessorEditor::FlowBar::paint(juce::Graphics& g)
{
    shown = describe(rack.getNotesAfter(MnKarpAudioProcessor::kordStage));

    auto area = getLocalBounds();
    const auto arrow = area.removeFromLeft(34).toFloat();
    const auto c = arrow.getCentre();
    juce::Path head;
    head.addTriangle(c.x - 5.0f, c.y - 2.0f, c.x + 5.0f, c.y - 2.0f, c.x, c.y + 5.0f);
    g.setColour(shown.isEmpty() ? Palette::border() : Palette::accent());
    g.fillPath(head);
    g.drawLine(c.x, c.y - 8.0f, c.x, c.y - 2.0f, 2.0f);

    g.setFont(textFont(14.0f));
    g.setColour(Palette::dimText().withAlpha(0.7f));
    g.drawText("MIDI from mnKord to mnArpee", area.removeFromLeft(190), juce::Justification::centredLeft);

    g.setColour(Palette::accent());
    g.setFont(textFont(15.0f));
    g.drawText(shown.isEmpty() ? juce::String("-") : shown, area, juce::Justification::centredLeft);
}

//==============================================================================
void MnKarpAudioProcessorEditor::TakeButton::paint(juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat();
    const bool over = isMouseOver(true) && available;
    g.setColour(over ? Palette::cardHi().brighter(0.08f) : Palette::cardHi());
    g.fillRoundedRectangle(r, 7.0f);

    const auto colour = available ? Palette::accent() : Palette::dimText().withAlpha(0.4f);
    auto area = getLocalBounds().reduced(10, 0);
    const auto icon = area.removeFromLeft(14).toFloat().withSizeKeepingCentre(12.0f, 14.0f);

    // A small clip with a down arrow: "take it with you".
    g.setColour(colour);
    g.drawLine(icon.getCentreX(), icon.getY(), icon.getCentreX(), icon.getBottom() - 4.0f, 2.0f);
    juce::Path head;
    head.addTriangle(icon.getX(), icon.getBottom() - 6.0f, icon.getRight(), icon.getBottom() - 6.0f,
                     icon.getCentreX(), icon.getBottom());
    g.fillPath(head);

    area.removeFromLeft(6);
    g.setFont(textFont(15.0f));
    g.drawText("Drag MIDI", area, juce::Justification::centredLeft);
}

void MnKarpAudioProcessorEditor::TakeButton::mouseDrag(const juce::MouseEvent& e)
{
    if (! dragged && available && e.getDistanceFromDragStart() > 4)
    {
        dragged = true;
        owner.dragTake();
    }
}

void MnKarpAudioProcessorEditor::TakeButton::mouseUp(const juce::MouseEvent& e)
{
    if (! dragged && available && ! e.mouseWasDraggedSinceMouseDown())
        owner.saveTake();
    dragged = false;
}

void MnKarpAudioProcessorEditor::dragTake()
{
    // A file of its own for every drag: the DAW may still be reading the previous one.
    const auto folder = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("mnKarp takes");
    folder.createDirectory();
    for (const auto& old : folder.findChildFiles(juce::File::findFiles, false, "*.mid"))
        if (old.getLastModificationTime() < juce::Time::getCurrentTime() - juce::RelativeTime::hours(1))
            old.deleteFile();

    rack.collectRecording();
    const auto file = folder.getNonexistentChildFile(
        "mnKarp take " + juce::Time::getCurrentTime().formatted("%H-%M-%S"), ".mid", false);
    if (rack.writeTake(file))
        juce::DragAndDropContainer::performExternalDragDropOfFiles({ file.getFullPathName() }, false, &takeButton);
}

void MnKarpAudioProcessorEditor::saveTake()
{
    rack.collectRecording();
    const auto start = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("mnKarp take.mid");
    fileChooser = std::make_unique<juce::FileChooser>("Save the take as a MIDI file", start, "*.mid");
    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                                 | juce::FileBrowserComponent::warnAboutOverwriting,
                             [safe = juce::Component::SafePointer<MnKarpAudioProcessorEditor>(this)](const juce::FileChooser& chooser)
                             {
                                 const auto file = chooser.getResult();
                                 if (safe == nullptr || file == juce::File())
                                     return;
                                 if (! safe->rack.writeTake(file.withFileExtension("mid")))
                                     safe->showDialog("Save failed", "The MIDI file could not be written.", false, {},
                                                      { "OK" }, 0, 0, nullptr);
                             });
}

//==============================================================================
MnKarpAudioProcessorEditor::Section::Section(MnKarpAudioProcessorEditor& ownerRef, MnKarpAudioProcessor& processor,
                                             MnKarpAudioProcessor::Stage stageIn, juce::AudioProcessor* child,
                                             const juce::String& roleIn)
    : owner(ownerRef), rack(processor), stage(stageIn), role(roleIn)
{
    const juce::String name = MnKarpAudioProcessor::stageName(stage);
    power.setTooltip("Switch " + name + " off or on. Off: the MIDI passes it untouched.");
    fold.setTooltip("Show or hide the " + name + " window");
    addAndMakeVisible(power);
    addAndMakeVisible(fold);
    addAndMakeVisible(holder);

    if (child != nullptr)
        editor.reset(child->createEditorIfNeeded());

    if (editor != nullptr)
    {
        applyZoom(rack.getZoom());
        holder.addAndMakeVisible(*editor);
        editor->setTopLeftPosition(0, 0);
        editor->addComponentListener(this);
    }
    else
    {
        missing.setText(child == nullptr ? rack.getLoadError(stage) : name + " has no window.", juce::dontSendNotification);
        missing.setFont(textFont(16.0f));
        missing.setColour(juce::Label::textColourId, Palette::danger());
        missing.setJustificationType(juce::Justification::centred);
        missing.setSize(800, 90);
        holder.addAndMakeVisible(missing);
    }

    powerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        rack.getApvts(), MnKarpAudioProcessor::stageParamId(stage), power);
    power.onStateChange = [this] { refreshPower(); };

    fold.onClick = [this]
    {
        rack.setStageFolded(stage, ! rack.isStageFolded(stage));
        applyFolded();
        owner.updateSize();
    };

    refreshPower();
    applyFolded();
}

MnKarpAudioProcessorEditor::Section::~Section()
{
    if (editor != nullptr)
        editor->removeComponentListener(this);
    powerAttachment.reset();
}

void MnKarpAudioProcessorEditor::Section::refreshPower()
{
    repaint();
}

void MnKarpAudioProcessorEditor::Section::applyFolded()
{
    fold.folded = isFolded();
    fold.repaint();
    holder.setVisible(! isFolded() && ! hidden);
    refreshPower();
}

void MnKarpAudioProcessorEditor::Section::setEditorHidden(bool shouldBeHidden)
{
    hidden = shouldBeHidden;
    applyFolded();
}

void MnKarpAudioProcessorEditor::Section::applyZoom(float factor)
{
    if (editor != nullptr)
        editor->setScaleFactor(factor);
}

void MnKarpAudioProcessorEditor::Section::setEditorWidth(int width)
{
    if (editor != nullptr && editor->getWidth() != width)
        editor->setSize(width, editor->getHeight());
}

juce::Point<int> MnKarpAudioProcessorEditor::Section::editorSize() const
{
    // The bounds in the parent include a scale the editor may have.
    const auto bounds = editor != nullptr ? editor->getBoundsInParent() : missing.getBounds();
    return { bounds.getWidth(), bounds.getHeight() };
}

int MnKarpAudioProcessorEditor::Section::preferredWidth() const
{
    return isFolded() ? 0 : editorSize().x;
}

int MnKarpAudioProcessorEditor::Section::preferredHeight() const
{
    return stripHeight + (isFolded() ? 0 : editorSize().y);
}

void MnKarpAudioProcessorEditor::Section::componentMovedOrResized(juce::Component&, bool, bool)
{
    if (! layingOut)
        owner.updateSize();
}

void MnKarpAudioProcessorEditor::Section::resized()
{
    const juce::ScopedValueSetter<bool> guard(layingOut, true);

    auto area = getLocalBounds();
    auto strip = area.removeFromTop(stripHeight);
    power.setBounds(strip.removeFromLeft(stripHeight).withSizeKeepingCentre(24, 24));
    fold.setBounds(strip.removeFromRight(stripHeight).withSizeKeepingCentre(26, 22));

    const auto size = editorSize();
    holder.setBounds(area.removeFromTop(size.y).withWidth(size.x));
    if (editor != nullptr)
        editor->setTopLeftPosition(0, 0);
}

void MnKarpAudioProcessorEditor::Section::paint(juce::Graphics& g)
{
    mnaudio::ui::paintCard(g, getLocalBounds());

    const bool on = power.getToggleState();
    auto strip = getLocalBounds().removeFromTop(stripHeight).withTrimmedLeft(stripHeight + 4).withTrimmedRight(stripHeight + 4);

    const juce::String name = MnKarpAudioProcessor::stageName(stage);
    const auto font = titleFont(17.0f);
    g.setFont(font);
    g.setColour(on ? Palette::accent() : Palette::dimText().withAlpha(0.6f));
    const int nameWidth = juce::GlyphArrangement::getStringWidthInt(font, name) + 6;
    g.drawText(name, strip.removeFromLeft(nameWidth), juce::Justification::centredLeft);

    g.setFont(textFont(14.0f));
    g.setColour(Palette::dimText().withAlpha(on ? 1.0f : 0.5f));
    g.drawText(role, strip.removeFromLeft(220), juce::Justification::centredLeft);

    if (! on)
    {
        g.setColour(Palette::danger());
        g.drawText("OFF - the MIDI passes through", strip, juce::Justification::centredRight);
    }
}

//==============================================================================
MnKarpAudioProcessorEditor::MnKarpAudioProcessorEditor(MnKarpAudioProcessor& processor)
    : AudioProcessorEditor(&processor), rack(processor), flowBar(processor)
{
    setLookAndFeel(&lookAndFeel);

    sections[MnKarpAudioProcessor::kordStage] = std::make_unique<Section>(
        *this, rack, MnKarpAudioProcessor::kordStage, rack.getStageProcessor(MnKarpAudioProcessor::kordStage),
        "chords: one key plays a chord");
    sections[MnKarpAudioProcessor::arpeeStage] = std::make_unique<Section>(
        *this, rack, MnKarpAudioProcessor::arpeeStage, rack.getStageProcessor(MnKarpAudioProcessor::arpeeStage),
        "arpeggiator: pattern of the held notes");

    prevPresetButton.onClick = [this] { stepPreset(-1); };
    nextPresetButton.onClick = [this] { stepPreset(1); };
    presetButton.onClick = [this] { showPresetMenu(); };
    presetButton.setTooltip("The rack presets: the presets of mnKord and mnArpee together with the two switches");
    saveButton.setButtonText("Save");
    saveButton.onClick = [this] { saveClicked(); };
    takeButton.setTooltip("Drag the last thing you played into the DAW as a MIDI clip, or click to save it as a file");
    zoomButton.setTooltip("The size of both windows");
    zoomButton.onClick = [this] { showZoomMenu(); };
    zoomButton.setButtonText(juce::String(juce::roundToInt(rack.getZoom() * 100.0f)) + "%");

    for (auto* component : std::initializer_list<juce::Component*> { &prevPresetButton, &nextPresetButton, &presetButton,
                                                                      &saveButton, &takeButton, &zoomButton, sections[0].get(),
                                                                      &flowBar, sections[1].get() })
        addAndMakeVisible(component);

    refreshPresetButton();
    syncWidths();
    ready = true;
    updateSize();
    startTimerHz(15);
}

MnKarpAudioProcessorEditor::~MnKarpAudioProcessorEditor()
{
    stopTimer();
    // A menu that is still open uses the look of this editor.
    juce::PopupMenu::dismissAllActiveMenus();
    // The sections own the two plugin editors; they go before the rack editor does.
    for (auto& section : sections)
        section.reset();
    setLookAndFeel(nullptr);
}

void MnKarpAudioProcessorEditor::paint(juce::Graphics& g)
{
    mnaudio::ui::paintWindow(g, getLocalBounds());

    auto brand = brandArea;
    const auto title = titleFont(24.0f);
    g.setColour(Palette::accent());
    g.setFont(title);
    const int titleWidth = juce::GlyphArrangement::getStringWidthInt(title, "mnKarp") + 4;
    g.drawText("mnKarp", brand.removeFromLeft(titleWidth), juce::Justification::centredLeft);
    g.setColour(Palette::dimText());
    g.setFont(textFont(13.0f));
    g.drawText("v" MNKARP_VERSION_STRING, brand.removeFromLeft(60), juce::Justification::centredLeft);
}

void MnKarpAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    if (shownDirty)
        mnaudio::ui::paintDirtyDot(g, presetButton.getBounds());
}

void MnKarpAudioProcessorEditor::resized()
{
    if (dialog != nullptr)
        dialog->setBounds(getLocalBounds());

    auto bar = getLocalBounds().reduced(margin, 0).removeFromTop(margin + barHeight).withTrimmedTop(margin);
    brandArea = bar.removeFromLeft(170);
    zoomButton.setBounds(bar.removeFromRight(58).withSizeKeepingCentre(58, 28));
    bar.removeFromRight(8);
    takeButton.setBounds(bar.removeFromRight(104).withSizeKeepingCentre(104, 28));

    // The preset picker, in the middle of what is left: [<] [name] [>] [Save]
    // The name gets narrower when the windows are small, so the picker never runs into Drag MIDI.
    constexpr int arrowWidth = 26, saveWidth = 56;
    const int nameWidth = juce::jlimit(110, 240, bar.getWidth() - 16 - (arrowWidth + 4 + 4 + arrowWidth + 8 + saveWidth));
    auto picker = bar.withSizeKeepingCentre(arrowWidth + 4 + nameWidth + 4 + arrowWidth + 8 + saveWidth, 30);
    prevPresetButton.setBounds(picker.removeFromLeft(arrowWidth).reduced(6, 8));
    picker.removeFromLeft(4);
    presetButton.setBounds(picker.removeFromLeft(nameWidth));
    picker.removeFromLeft(4);
    nextPresetButton.setBounds(picker.removeFromLeft(arrowWidth).reduced(6, 8));
    picker.removeFromLeft(8);
    saveButton.setBounds(picker);
    repaint();
}

//==============================================================================
int MnKarpAudioProcessorEditor::getStageEditorWidth(MnKarpAudioProcessor::Stage stage) const
{
    return sections[(size_t) stage]->editorWidth();
}

juce::Point<int> MnKarpAudioProcessorEditor::getStageVisualSize(MnKarpAudioProcessor::Stage stage) const
{
    return sections[(size_t) stage]->editorSize();
}

juce::AudioProcessorEditor* MnKarpAudioProcessorEditor::getStageEditor(MnKarpAudioProcessor::Stage stage) const
{
    return sections[(size_t) stage]->getEditor();
}

juce::String MnKarpAudioProcessorEditor::getFlowText() const
{
    return FlowBar::describe(rack.getNotesAfter(MnKarpAudioProcessor::kordStage));
}

//==============================================================================
void MnKarpAudioProcessorEditor::timerCallback()
{
    // A change of scale moves no bounds, so the sizes are looked at too.
    bool changed = false;
    for (int i = 0; i < MnKarpAudioProcessor::numStages; ++i)
    {
        const auto size = sections[(size_t) i]->editorSize();
        const bool folded = sections[(size_t) i]->isFolded();
        changed = changed || size != shownSizes[i] || folded != shownFolded[i];
    }

    if (changed)
        updateSize();

    if (getFlowText() != flowBar.shown)
        flowBar.repaint();

    if (rack.getPresetName() != shownPreset)
        refreshPresetButton();

    // Comparing the whole state is not free: twice a second is enough for the dot.
    if (++dirtyTick >= 8)
    {
        dirtyTick = 0;
        if (rack.hasTake() != takeButton.available)
        {
            takeButton.available = ! takeButton.available;
            takeButton.repaint();
        }

        if (rack.isPresetDirty() != shownDirty)
        {
            shownDirty = ! shownDirty;
            repaint(presetButton.getBounds().expanded(2));
        }
    }
}

void MnKarpAudioProcessorEditor::syncWidths()
{
    if (syncing)
        return;
    const juce::ScopedValueSetter<bool> guard(syncing, true);

    // The windows are compared on the screen. At the start (no width yet) the wider one sets it; later a window
    // whose width is not the rack's was resized (mnArpee's corner, a new zoom): its width is the new one.
    int width = rack.getWidth();
    for (const auto& section : sections)
        if (section->editorWidth() > 0)
        {
            if (width <= 0)
                width = section->editorWidth();
            else if (! ready || rack.getWidth() <= 0)
                width = juce::jmax(width, section->editorWidth());
            else if (section->editorWidth() != rack.getWidth())
            {
                width = section->editorWidth();
                break;
            }
        }

    if (width <= 0)
        return;

    // Each plugin keeps its own limits: if one does not take the width, the other is set to what it took.
    for (int pass = 0; pass < 2; ++pass)
    {
        rack.setWidth(width);
        for (const auto& section : sections)
            if (section->editorWidth() > 0)
                section->setEditorWidth(width);

        for (const auto& section : sections)
            if (section->editorWidth() > 0 && section->editorWidth() != width)
                width = section->editorWidth();

        if (width == rack.getWidth())
            break;
    }
}

void MnKarpAudioProcessorEditor::showZoomMenu()
{
    juce::PopupMenu menu;
    menu.setLookAndFeel(&lookAndFeel);
    for (const int percent : { 60, 70, 75, 85, 100, 115 })
        menu.addItem(percent, juce::String(percent) + "%", true,
                     std::abs(rack.getZoom() * 100.0f - (float) percent) < 1.0f);

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&zoomButton),
                       [safe = juce::Component::SafePointer<MnKarpAudioProcessorEditor>(this)](int result)
                       {
                           if (safe != nullptr && result > 0)
                               safe->setZoom((float) result / 100.0f);
                       });
}

void MnKarpAudioProcessorEditor::setZoom(float factor)
{
    rack.setZoom(factor);
    zoomButton.setButtonText(juce::String(juce::roundToInt(rack.getZoom() * 100.0f)) + "%");
    for (auto& section : sections)
        section->applyZoom(rack.getZoom());
    updateSize();
}

void MnKarpAudioProcessorEditor::updateSize()
{
    if (! ready)
        return;

    syncWidths();

    int contentWidth = minContentWidth;
    for (const auto& section : sections)
        contentWidth = juce::jmax(contentWidth, section->preferredWidth());

    int y = margin + barHeight + gap;
    for (int i = 0; i < MnKarpAudioProcessor::numStages; ++i)
    {
        auto& section = *sections[(size_t) i];
        shownSizes[i] = section.editorSize();
        shownFolded[i] = section.isFolded();

        const int height = section.preferredHeight();
        section.setBounds(margin, y, contentWidth, height);
        section.resized();   // the editor inside may have changed size while the section did not
        y += height;

        if (i == MnKarpAudioProcessor::kordStage)
        {
            flowBar.setBounds(margin, y + 2, contentWidth, flowHeight);
            y += flowHeight + 4;
        }
    }

    setSize(contentWidth + 2 * margin, y + margin);
}

//==============================================================================
void MnKarpAudioProcessorEditor::refreshPresetButton()
{
    shownPreset = rack.getPresetName();
    shownDirty = rack.isPresetDirty();
    repaint(presetButton.getBounds().expanded(2));
    presetButton.setButtonText(shownPreset.isEmpty() ? juce::String("(no rack preset)") : shownPreset);
}

//==============================================================================
MnKarpAudioProcessorEditor::Dialog::Dialog(const juce::String& titleIn, const juce::String& messageIn, bool withField,
                                           const juce::String& initialText, const juce::StringArray& buttonNames,
                                           int defaultButtonIn, int cancelButtonIn,
                                           std::function<void(int, juce::String)> doneIn)
    : title(titleIn), message(messageIn), defaultButton(defaultButtonIn), cancelButton(cancelButtonIn),
      done(std::move(doneIn))
{
    setWantsKeyboardFocus(true);

    if (withField)
    {
        field = std::make_unique<juce::TextEditor>();
        field->setText(initialText, false);
        field->setFont(textFont(16.0f));
        field->setIndents(10, 7);
        field->setColour(juce::TextEditor::backgroundColourId, Palette::bg());
        field->setColour(juce::TextEditor::textColourId, Palette::text());
        field->setColour(juce::TextEditor::highlightColourId, Palette::accent().withAlpha(0.35f));
        field->setColour(juce::TextEditor::outlineColourId, Palette::border());
        field->setColour(juce::TextEditor::focusedOutlineColourId, Palette::accent());
        field->setColour(juce::CaretComponent::caretColourId, Palette::accent());
        field->onReturnKey = [this] { finish(defaultButton); };
        field->onEscapeKey = [this] { finish(cancelButton); };
        addAndMakeVisible(*field);
    }

    for (int i = 0; i < buttonNames.size(); ++i)
    {
        auto* button = buttons.add(new juce::TextButton(buttonNames[i]));
        if (i == defaultButton)
        {
            button->setColour(juce::TextButton::buttonColourId, Palette::accent());
            button->setColour(juce::TextButton::textColourOffId, Palette::bg());
        }
        button->onClick = [this, i] { finish(i); };
        addAndMakeVisible(button);
    }
}

void MnKarpAudioProcessorEditor::Dialog::focusFirst()
{
    if (field != nullptr)
    {
        field->grabKeyboardFocus();
        field->selectAll();
    }
    else
    {
        grabKeyboardFocus();
    }
}

void MnKarpAudioProcessorEditor::Dialog::finish(int button)
{
    if (finished)
        return;
    finished = true;

    // The dialog (and the button that was clicked) goes away after this call has returned.
    auto callback = std::move(done);
    const auto text = field != nullptr ? field->getText().trim() : juce::String();
    if (onClose != nullptr)
        onClose();
    juce::MessageManager::callAsync([callback, button, text] { if (callback != nullptr) callback(button, text); });
}

juce::Rectangle<int> MnKarpAudioProcessorEditor::Dialog::cardBounds() const
{
    const int height = 64 + 40 + (field != nullptr ? 44 : 0) + 52;
    return getLocalBounds().withSizeKeepingCentre(juce::jmin(420, getWidth() - 40), height);
}

void MnKarpAudioProcessorEditor::Dialog::paint(juce::Graphics& g)
{
    g.fillAll(Palette::bg().withAlpha(0.72f));

    const auto card = cardBounds();
    mnaudio::ui::paintCard(g, card);

    auto area = card.reduced(20, 16);
    g.setColour(Palette::accent());
    g.setFont(titleFont(18.0f));
    g.drawText(title, area.removeFromTop(28), juce::Justification::centredLeft);

    g.setColour(Palette::text());
    g.setFont(textFont(15.0f));
    g.drawFittedText(message, area.removeFromTop(40), juce::Justification::centredLeft, 2);
}

void MnKarpAudioProcessorEditor::Dialog::resized()
{
    auto area = cardBounds().reduced(20, 16);
    area.removeFromTop(28 + 40);
    if (field != nullptr)
        field->setBounds(area.removeFromTop(34));

    auto row = area.removeFromBottom(32);
    for (int i = buttons.size(); --i >= 0;)
    {
        buttons[i]->setBounds(row.removeFromRight(juce::jmax(80, buttons[i]->getBestWidthForHeight(32) + 16)));
        row.removeFromRight(8);
    }
}

bool MnKarpAudioProcessorEditor::Dialog::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::returnKey)
    {
        finish(defaultButton);
        return true;
    }
    if (key == juce::KeyPress::escapeKey)
    {
        finish(cancelButton);
        return true;
    }
    return false;
}

void MnKarpAudioProcessorEditor::showDialog(const juce::String& title, const juce::String& message, bool withField,
                                            const juce::String& initialText, const juce::StringArray& buttonNames,
                                            int defaultButton, int cancelButton,
                                            std::function<void(int, juce::String)> done)
{
    // Only one at a time: a new one answers the old one with its cancel button.
    if (dialog != nullptr)
        dialog->finish(dialog->getCancelButton());

    dialog = std::make_unique<Dialog>(title, message, withField, initialText, buttonNames, defaultButton, cancelButton,
                                      std::move(done));
    dialog->onClose = [safe = juce::Component::SafePointer<MnKarpAudioProcessorEditor>(this)]
    {
        if (safe != nullptr && safe->dialog != nullptr)
        {
            safe->dialog->setVisible(false);
            for (auto& section : safe->sections)
                section->setEditorHidden(false);
            // Not deleted here: this runs inside the dialog's own button click.
            juce::MessageManager::callAsync([safe]
                                            {
                                                if (safe != nullptr && safe->dialog != nullptr && ! safe->dialog->isVisible())
                                                    safe->dialog.reset();
                                            });
        }
    };
    // The plugins' windows would cover the dialog: they are hidden while it is open.
    for (auto& section : sections)
        section->setEditorHidden(true);

    addAndMakeVisible(*dialog);
    dialog->setBounds(getLocalBounds());
    dialog->focusFirst();
}

//==============================================================================
void MnKarpAudioProcessorEditor::confirmLeavePreset(std::function<void()> proceed)
{
    if (! rack.isPresetDirty())
    {
        proceed();
        return;
    }

    const auto name = rack.getPresetName();
    showDialog("Unsaved changes", "The rack preset \"" + name + "\" has changes that are not saved.", false, {},
               { "Save", "Discard", "Cancel" }, 0, 2,
               [safe = juce::Component::SafePointer<MnKarpAudioProcessorEditor>(this), name, proceed](int button, juce::String)
               {
                   if (safe == nullptr || button == 2)
                       return;
                   if (button == 0 && ! safe->rack.savePreset(name))
                   {
                       safe->showSaveFailed();
                       return;
                   }
                   proceed();
                   safe->refreshPresetButton();
               });
}

void MnKarpAudioProcessorEditor::loadPresetChecked(const juce::String& name)
{
    confirmLeavePreset([safe = juce::Component::SafePointer<MnKarpAudioProcessorEditor>(this), name]
                       {
                           if (safe != nullptr)
                           {
                               safe->rack.loadPreset(name);
                               safe->refreshPresetButton();
                           }
                       });
}

void MnKarpAudioProcessorEditor::stepPreset(int delta)
{
    const auto names = MnKarpAudioProcessor::listPresets();
    if (names.isEmpty())
        return;

    const int current = names.indexOf(rack.getPresetName());
    const int next = current < 0 ? (delta > 0 ? 0 : names.size() - 1)
                                 : (current + delta + names.size()) % names.size();
    loadPresetChecked(names[next]);
}

void MnKarpAudioProcessorEditor::showPresetMenu()
{
    rack.refreshProgramList();   // picks up files copied in from outside
    const auto names = MnKarpAudioProcessor::listPresets();
    const auto current = rack.getPresetName();

    juce::PopupMenu menu;
    menu.setLookAndFeel(&lookAndFeel);
    for (int i = 0; i < names.size(); ++i)
        menu.addItem(100 + i, names[i], true, names[i] == current);
    if (names.isEmpty())
        menu.addItem(-1, "(no rack presets yet)", false);

    menu.addSeparator();
    menu.addItem(1, "Save", current.isNotEmpty());
    menu.addItem(2, "Save as...");
    menu.addItem(5, "Revert to saved", current.isNotEmpty() && rack.isPresetDirty());
    menu.addItem(3, "Delete...", current.isNotEmpty() && names.contains(current));
    menu.addItem(4, "Show the preset folder");

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&presetButton),
                       [safe = juce::Component::SafePointer<MnKarpAudioProcessorEditor>(this), names, current](int result)
                       {
                           if (safe == nullptr || result <= 0)
                               return;

                           if (result >= 100)
                               safe->loadPresetChecked(names[result - 100]);
                           else if (result == 1)
                               safe->saveAs(current);
                           else if (result == 2)
                               safe->promptName(current);
                           else if (result == 5)
                               safe->rack.loadPreset(current);
                           else if (result == 3)
                               safe->deleteDialog(current);
                           else if (result == 4)
                           {
                               MnKarpAudioProcessor::getPresetFolder().createDirectory();
                               MnKarpAudioProcessor::getPresetFolder().revealToUser();
                           }
                           safe->refreshPresetButton();
                       });
}

void MnKarpAudioProcessorEditor::saveClicked()
{
    if (rack.getPresetName().isNotEmpty())
        saveAs(rack.getPresetName());
    else
        promptName({});
}

void MnKarpAudioProcessorEditor::saveAs(const juce::String& name)
{
    if (name.isEmpty())
        return;

    // A name that is taken by another preset is only overwritten when the user says so.
    const auto names = MnKarpAudioProcessor::listPresets();
    if (name != rack.getPresetName() && names.contains(juce::File::createLegalFileName(name)))
    {
        showDialog("Replace the preset?", "A rack preset called \"" + name + "\" exists. Replace it?", false, {},
                   { "Replace", "Cancel" }, 0, 1,
                   [safe = juce::Component::SafePointer<MnKarpAudioProcessorEditor>(this), name](int button, juce::String)
                   {
                       if (safe == nullptr || button != 0)
                           return;
                       if (! safe->rack.savePreset(name))
                           safe->showSaveFailed();
                       safe->refreshPresetButton();
                   });
        return;
    }

    if (! rack.savePreset(name))
        showSaveFailed();
    refreshPresetButton();
}

void MnKarpAudioProcessorEditor::showSaveFailed()
{
    showDialog("Save failed", "The preset could not be written to " + MnKarpAudioProcessor::getPresetFolder().getFullPathName(),
               false, {}, { "OK" }, 0, 0, nullptr);
}

void MnKarpAudioProcessorEditor::deleteDialog(const juce::String& name)
{
    showDialog("Delete the preset?", "\"" + name + "\" goes to the recycle bin.", false, {}, { "Delete", "Cancel" }, 1, 1,
               [safe = juce::Component::SafePointer<MnKarpAudioProcessorEditor>(this), name](int button, juce::String)
               {
                   if (safe != nullptr && button == 0)
                   {
                       safe->rack.deletePreset(name);
                       safe->refreshPresetButton();
                   }
               });
}

void MnKarpAudioProcessorEditor::promptName(const juce::String& initial)
{
    showDialog("Save the rack preset", "The name of the preset:", true, initial, { "Save", "Cancel" }, 0, 1,
               [safe = juce::Component::SafePointer<MnKarpAudioProcessorEditor>(this)](int button, juce::String name)
               {
                   if (safe != nullptr && button == 0 && name.isNotEmpty())
                       safe->saveAs(name);
               });
}
